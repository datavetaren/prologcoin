#pragma once

#ifndef _db_triedb_hpp
#define _db_triedb_hpp

#include <cstdint>
#include <fstream>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/functional/hash.hpp>
#include <boost/intrusive_ptr.hpp>
#include <iostream>
#include <bitset>
#include "../common/lru_cache.hpp"
#include "util.hpp"
#include "triedb_params.hpp"

namespace prologcoin { namespace db {

class triedb_exception : public std::runtime_error {
public:
    triedb_exception(const std::string &msg) : std::runtime_error(msg) { }
};

class triedb_version_exception : public triedb_exception {
public:
    triedb_version_exception(const std::string &msg) : triedb_exception(msg) { }
};

class triedb_write_exception : public triedb_exception {
public:
    triedb_write_exception(const std::string &msg) : triedb_exception(msg) { }
};

class triedb_node {
};
    
//
// We allow custom data to be stored in the leaf nodes.
// This will typically be the bitvector (one bit per frozen closure,
// likely an UTXO), the hash value of the same bitvector, and the hash value
// of the heap block itself. If heap blocks are 64k each = 8192 cells,
// then the bitvector is 8192 bits (1024 bytes), the custom data size would
// be 1024 + 20 + 20 bytes = 1064 bytes (note that the heap block itself
// is stored in the blockdb and not here.)
//
class triedb_leaf : public triedb_node {
public:
    static const size_t MAX_SIZE_IN_BYTES = sizeof(uint64_t) + 8192;
  
    inline triedb_leaf()
        : key_(0), custom_data_size_(0), custom_data_(nullptr) { }

    inline triedb_leaf(uint64_t key, const uint8_t *custom_data, size_t custom_data_size) : key_(key), custom_data_(new uint8_t[custom_data_size]), custom_data_size_(custom_data_size) {
        memcpy(custom_data_, custom_data, custom_data_size);
    }

    inline uint64_t key() const {
        return key_;
    }
  
    inline size_t custom_data_size() const {
        return custom_data_size_;
    }
    inline const uint8_t * custom_data() const {
        return custom_data_;
    }

    inline size_t serialization_size() const {
        size_t sz = sizeof(uint32_t) + sizeof(uint64_t) + custom_data_size_;
        return sz;
    }

    inline void read(const uint8_t *buffer) {
        const uint8_t *p = buffer;
	assert(((p - buffer) + sizeof(uint32_t)) < MAX_SIZE_IN_BYTES);
	size_t sz = read_uint32(p); p += sizeof(uint32_t);
	assert(((p - buffer) + sizeof(uint64_t)) < MAX_SIZE_IN_BYTES);	
	key_ = read_uint64(p); p += sizeof(uint64_t);
	assert(((p - buffer) + sizeof(uint32_t)) < MAX_SIZE_IN_BYTES);		
	custom_data_size_ = read_uint32(p); p += sizeof(uint32_t);
	assert(((p - buffer) + custom_data_size_) < MAX_SIZE_IN_BYTES);
	if (custom_data_ != nullptr) delete [] custom_data_;
	custom_data_ = new uint8_t [custom_data_size_];
	memcpy(custom_data_, p, custom_data_size_);
    }
  
    inline void write(uint8_t *buffer) const {
        uint8_t *p = buffer;
	write_uint64(p, key_); p += sizeof(uint64_t);
	write_uint32(p, custom_data_size_); p += sizeof(uint32_t);
	if (custom_data_ != nullptr) {
	    memcpy(p, custom_data_, custom_data_size_);
	}
    }

private:
    uint64_t key_;
    uint32_t custom_data_size_; // Not stored on disk
    uint8_t *custom_data_;
};

//
// The branch node of a trie. The custom data here will typically be
// two hashes (20 + 20 byets) that represent the aggregated hash for
// the bitvectors and the aggregated hash for the heap blocks.
//
class triedb_branch : public triedb_node {
public:
    static const size_t MAX_SIZE_IN_BYTES = sizeof(uint32_t)*2 + sizeof(hash_t) + 32*sizeof(uint64_t) + 1024; // We're not allowed to have custom data bigger than 1024 bytes.

    inline triedb_branch() : mask_(0), leaf_(0), ptr_(nullptr), custom_data_size_(0), custom_data_(nullptr) { }
    inline triedb_branch(const triedb_branch &other)
      : mask_(other.mask_), leaf_(other.leaf_), ptr_(nullptr), custom_data_size_(other.custom_data_size_), custom_data_(nullptr) {
        size_t n = num_children();
        ptr_ = new uint64_t[n];
	memcpy(ptr_, other.ptr_, sizeof(uint64_t)*n);
	if (custom_data_size_ > 0) {
	    custom_data_ = new uint8_t[custom_data_size_];
	    memcpy(custom_data_, other.custom_data_, custom_data_size_);
	}
    }
  
    uint32_t mask() const { return mask_; }

    inline size_t num_children() const
        { return std::bitset<triedb_params::MAX_BRANCH>(mask_).count(); }

    inline bool is_leaf(size_t sub_index) const {
        return ((leaf_ >> sub_index) & 1) != 0;
    }

    inline bool is_branch(size_t sub_index) const {
        return !is_leaf(sub_index);
    }

    inline bool is_empty(size_t sub_index) const {
        return ((mask_ >> sub_index) & 1) == 0;
    }
  
    inline size_t custom_data_size() const
        { return custom_data_size_; }
    inline const uint8_t * custom_data() const
        { return custom_data_; }

    size_t serialization_size() const;
    void read(const uint8_t *buffer);
    void write(uint8_t *buffer) const;

    inline uint64_t get_child_pointer(size_t sub_index) const {
        size_t child_index = std::bitset<triedb_params::MAX_BRANCH>(mask_ & (static_cast<uint32_t>(1) << sub_index) - 1).count();
        return ptr_[child_index];
    }

    inline void set_child_pointer(size_t sub_index, uint64_t ptr) {
        size_t child_index = std::bitset<triedb_params::MAX_BRANCH>(mask_ & (static_cast<uint32_t>(1) << sub_index) - 1).count();
        if (is_empty(sub_index)) {
  	    size_t n = num_children();
	    uint64_t *new_ptr = new uint64_t[n+1];
	    memcpy(new_ptr, ptr_, sizeof(uint64_t)*child_index);
	    new_ptr[child_index] = ptr;
	    memcpy(&new_ptr[child_index+1], &ptr_[child_index],
		   sizeof(uint64_t)*(n-child_index));
	    delete [] ptr_;
	    ptr_ = new_ptr;
        } else {
	    ptr_[child_index] = ptr;
	}
    }
  
private:
    uint32_t mask_;
    uint32_t leaf_;
    uint64_t *ptr_;
    uint32_t custom_data_size_; // Not stored on disk
    uint8_t *custom_data_;
};
    
class triedb : public triedb_params {
public:
    triedb(const std::string &dir_path);
    triedb(const triedb_params &params, const std::string &dir_path);

    void insert(uint64_t key, const uint8_t *data, size_t data_size);
  
private:
    // Return modified branch node
    triedb_branch * insert_part(triedb_branch *node, uint64_t key, const uint8_t *data, size_t data_size, size_t part);
  
    boost::filesystem::path roots_file_path() const;
    void read_roots();
    boost::filesystem::path bucket_dir_location(size_t bucket_index) const;
    boost::filesystem::path bucket_file_path(size_t bucket_index) const;
    fstream * get_bucket_stream(size_t bucket_index);
    size_t scan_last_bucket();
    uint64_t scan_last_offset();
    fstream * set_file_offset(uint64_t offset);

    void read_leaf_node(uint64_t offset, triedb_leaf &node);
    uint64_t append_leaf_node(const triedb_leaf &node);
  
    void read_branch_node(uint64_t offset, triedb_branch &node);
    uint64_t append_branch_node(const triedb_branch &node);

    inline triedb_leaf * get_leaf(uint64_t file_offset) {
        auto *r = leaf_cache_.find(file_offset);
	if (r == nullptr) {
	     auto *lf = new triedb_leaf();
	     read_leaf_node(file_offset, *lf);
	     leaf_cache_.insert(file_offset, lf);
	     return lf;
	} else {
	     return *r;
	}
    }

    triedb_leaf * get_leaf(triedb_branch *parent, size_t sub_index) {
        auto file_offset = parent->get_child_pointer(sub_index);
	return get_leaf(file_offset);
    }

    inline triedb_branch * get_branch(uint64_t file_offset) {
        auto *r = branch_cache_.find(file_offset);
	if (r == nullptr) {
	     auto *br = new triedb_branch();
	     read_branch_node(file_offset, *br);
	     branch_cache_.insert(file_offset, br);
	     return br;
	} else {
	     return *r;
	}
    }

    triedb_branch * get_branch(triedb_branch *parent, size_t sub_index) {
        auto file_offset = parent->get_child_pointer(sub_index);
	return get_branch(file_offset);
    }

    std::string dir_path_;

    struct stream_flusher {
        void evicted(size_t, fstream *f) {
	    f->close();
	    delete f;
        }
    };

    struct leaf_flusher {
        void evicted(size_t, triedb_leaf *leaf) {
	    delete leaf;
        }
    };

    struct branch_flusher {
        void evicted(size_t, triedb_branch *br) {
	    delete br;
        }
    };
  
    // Bucket index to stream
    typedef common::lru_cache<size_t, fstream *, stream_flusher> stream_cache;
    stream_flusher stream_flusher_;
    stream_cache stream_cache_;

    // Leaf cache
    typedef common::lru_cache<size_t, triedb_leaf *, leaf_flusher> leaf_cache;
    leaf_flusher leaf_flusher_;
    leaf_cache leaf_cache_;

    // Branch cache
    typedef common::lru_cache<size_t, triedb_branch *, branch_flusher> branch_cache;
    branch_flusher branch_flusher_;
    branch_cache branch_cache_;

    // Root references, one for each height.
    std::vector<uint64_t> roots_;
  
    uint64_t last_offset_;

    size_t num_heights_;
};
    
}}

#endif
