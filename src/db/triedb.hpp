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
#include "../common/bits.hpp"
#include "util.hpp"
#include "triedb_params.hpp"

namespace prologcoin { namespace db {

class triedb_iterator;
    
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

class triedb_key_already_exists_exception : public triedb_exception {
public:
    triedb_key_already_exists_exception(const std::string &msg) : triedb_exception(msg) { }
};    

class triedb_key_not_found_exception : public triedb_exception {
public:
    triedb_key_not_found_exception(const std::string &msg) : triedb_exception(msg) { }
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
    static const size_t MAX_SIZE_IN_BYTES = 65536*2;
  
    inline triedb_leaf()
        : key_(0), custom_data_size_(0), custom_data_(nullptr) { }

    inline triedb_leaf(uint64_t key, const uint8_t *custom_data, size_t custom_data_size) : key_(key), custom_data_size_(custom_data_size), custom_data_(new uint8_t[custom_data_size]) {
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
    inline uint8_t * custom_data() {
        return custom_data_;
    }
    inline void allocate_custom_data(size_t data_size) {
        if (custom_data_ != nullptr) delete [] custom_data_;
	custom_data_ = new uint8_t [data_size];
	custom_data_size_ = data_size;
    }

    inline size_t serialization_size() const {
        size_t sz = sizeof(uint32_t) + sizeof(uint64_t) + custom_data_size_;
        return sz;
    }

    void read(const uint8_t *buffer);
    void write(uint8_t *buffer) const;

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

    inline triedb_branch() : max_key_bits_(0), mask_(0), leaf_(0), ptr_(nullptr), custom_data_size_(0), custom_data_(nullptr) { }
    inline triedb_branch(const triedb_branch &other)
      : max_key_bits_(other.max_key_bits_), mask_(other.mask_), leaf_(other.leaf_), ptr_(nullptr), custom_data_size_(other.custom_data_size_), custom_data_(nullptr) {
        size_t n = num_children();
        ptr_ = new uint64_t[n];
	memcpy(ptr_, other.ptr_, sizeof(uint64_t)*n);
	if (custom_data_size_ > 0) {
	    custom_data_ = new uint8_t[custom_data_size_];
	    memcpy(custom_data_, other.custom_data_, custom_data_size_);
	}
    }

    static inline uint32_t compute_max_key_bits(uint64_t key) {
        size_t required = common::msb(key) + 1;
	required = ((required + triedb_params::MAX_BRANCH_BITS - 1) / triedb_params::MAX_BRANCH_BITS) * triedb_params::MAX_BRANCH_BITS;
	return required;	
    }
  
    inline void set_max_key_bits(size_t m) { max_key_bits_ = m; }
    inline uint32_t max_key_bits() const {
        return max_key_bits_;
    }
  
    inline uint32_t mask() const { return mask_; }
    inline void set_mask(uint32_t m) { mask_ = m; }

    inline size_t num_children() const
        { return std::bitset<triedb_params::MAX_BRANCH>(mask_).count(); }

    inline bool is_leaf(size_t sub_index) const {
        return !is_empty(sub_index) && ((leaf_ >> sub_index) & 1) != 0;
    }
    inline void set_leaf(size_t sub_index) {
	leaf_ |= (1 << sub_index); 
    }

    inline bool is_branch(size_t sub_index) const {
        return !is_empty(sub_index) && !is_leaf(sub_index);
    }

    inline void set_branch(size_t sub_index) {
	leaf_ &= ~(1 << sub_index);
    }

    inline bool is_empty(size_t sub_index) const {
        return ((mask_ >> sub_index) & 1) == 0;
    }

    inline void set_empty(size_t sub_index) {
        size_t child_index = std::bitset<triedb_params::MAX_BRANCH>(mask_ & (static_cast<uint32_t>(1) << sub_index) - 1).count();
	size_t n = num_children();
	for (size_t i = child_index; i < n; i++) {
	    ptr_[i] = ptr_[i+1];
	}
	ptr_[n-1] = 0;
        leaf_ &= ~(1 << sub_index);
        mask_ &= ~(1 << sub_index);
    }
  
    inline size_t custom_data_size() const
        { return custom_data_size_; }
    inline const uint8_t * custom_data() const
        { return custom_data_; }
    inline uint8_t * custom_data() {
        return custom_data_;
    }
    inline void allocate_custom_data(size_t data_size) {
        if (custom_data_ != nullptr) delete [] custom_data_;
	custom_data_ = new uint8_t [data_size];
	custom_data_size_ = data_size;
    }
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
	mask_ |= 1 << sub_index;
    }
  
private:
    // The biggest key in this node
    // We use this to limit the height of the trie (to avoid all prefix 0s)
    uint32_t max_key_bits_;
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
    ~triedb();

    inline bool is_empty() const {
        return roots_.size() == 0;
    }

    inline void set_debug(bool b) {
        debug_ = b;
    }

    void erase_all();
    static void erase_all(const std::string &dir_path);

    void flush();

    inline void set_update_function(
        const std::function<void(triedb &, triedb_branch &)> &branch_updater) {
       branch_update_fn_ = branch_updater;
    }

    void insert(size_t at_height, uint64_t key,
		const uint8_t *data, size_t data_size);
    void update(size_t at_height, uint64_t key,
		const uint8_t *data, size_t data_size);
    void remove(size_t at_height, uint64_t key);

    const triedb_leaf * find(size_t at_height, uint64_t key,
			   std::vector<std::pair<triedb_branch *, size_t> >
			       *opt_path = nullptr);

    triedb_iterator begin(size_t at_height);
    triedb_iterator begin(size_t at_height, uint64_t key);
    triedb_iterator end(size_t at_height);

    triedb_leaf * get_leaf(triedb_branch *parent, size_t sub_index) {
        auto file_offset = parent->get_child_pointer(sub_index);
	return get_leaf(file_offset);
    }

    triedb_branch * get_branch(triedb_branch *parent, size_t sub_index) {
        auto file_offset = parent->get_child_pointer(sub_index);
	return get_branch(file_offset);
    }
 
    triedb_branch * get_root_branch(size_t at_height) {
        auto file_offset = get_root(at_height);
	return get_branch(file_offset);
    }

private:
    friend class triedb_iterator;
  
    // Return modified branch node
    void insert_or_update(size_t at_height, uint64_t key,
			  const uint8_t *data, size_t data_size, bool do_insert);
  
    std::pair<triedb_branch *, uint64_t> update_part(triedb_branch *node,
						     uint64_t key,
						     const uint8_t *data,
						     size_t data_size,
						     bool do_insert);

    std::pair<triedb_branch *, uint64_t> remove_part(size_t at_height,
						     triedb_branch *node,
						     uint64_t key);
  
    boost::filesystem::path roots_file_path() const;
    fstream * get_roots_stream();
    void read_roots();
    void set_root(size_t height, uint64_t offset);
    boost::filesystem::path bucket_dir_location(size_t bucket_index) const;
    boost::filesystem::path bucket_file_path(size_t bucket_index) const;
    fstream * get_bucket_stream(size_t bucket_index);
    size_t scan_last_bucket();
    uint64_t scan_last_offset();
    fstream * set_file_offset(uint64_t offset);

    uint64_t get_root(size_t at_height);
  
    void read_leaf_node(uint64_t offset, triedb_leaf &node);
    uint64_t append_leaf_node(const triedb_leaf &node);
  
    void read_branch_node(uint64_t offset, triedb_branch &node);
    uint64_t append_branch_node(triedb_branch *node);

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
    fstream *roots_stream_;
  
    uint64_t last_offset_;

    size_t num_heights_;

    // Update function
    // This function is called whenever a branch node needs to be updated;
    // let the client update any custom data.
    std::function<void(triedb &, triedb_branch &)> branch_update_fn_;

    bool debug_;
};

class triedb_iterator {
public:
    inline triedb_iterator(const triedb_iterator &other) :
        db_(other.db_), height_(other.height_), spine_(other.spine_) {
    }
  
    inline triedb_iterator(triedb &db, size_t at_height)
      : db_(db), height_(at_height) {
        auto parent_ptr = db.get_root(at_height);
	auto *parent = db.get_branch(parent_ptr);
	if (parent->mask() != 0) {
	    spine_.push_back(cursor(parent,
				    parent_ptr,
				    common::lsb(parent->mask())));
	    leftmost();
	}
    }

    inline triedb_iterator(triedb &db, size_t at_height, uint64_t key) 
	: db_(db), height_(at_height)  {
        start_from_key(db.get_root(at_height), key);
    }

    inline triedb_iterator(triedb &db, size_t at_height, bool) 
        : db_(db), height_(at_height) {
    }

    inline triedb_iterator & operator = (const triedb_iterator &other) {
        db_ = other.db_;
	height_ = other.height_;
	spine_ = other.spine_;
	return *this;
    }
  
    inline triedb_iterator & operator ++ () {
        next();
        return *this;
    }

    inline triedb_iterator & operator -- () {
        previous();
	return *this;
    }

    inline triedb_iterator operator - (int i) {
        auto copy_it = *this;
        while (i > 0) {
	    --i;
	    --copy_it;
        }
	return copy_it;
    }

    inline triedb_iterator operator + (int i) {
        auto copy_it = *this;
        while (i > 0) {
	    --i;
	    ++copy_it;
        }
	return copy_it;
    }
  
    inline bool operator == (const triedb_iterator &other) const {
        if (other.at_end()) {
	    return at_end();
        }
	return spine_ == other.spine_;
    }

    inline bool operator != (const triedb_iterator &other) const {
        return ! operator == (other);
    }

    inline const triedb_leaf & operator * () const {
        auto parent_ptr = spine_.back().parent_ptr;
        auto sub_index = spine_.back().sub_index;
	auto parent = db_.get_branch(parent_ptr);
        return *db_.get_leaf(parent, sub_index);
    }

    inline const triedb_leaf * operator -> () const {
        auto parent_ptr = spine_.back().parent_ptr;
        auto sub_index = spine_.back().sub_index;
	auto parent = db_.get_branch(parent_ptr);
        return db_.get_leaf(parent, sub_index);
    }    

    inline bool at_end() const {
        return spine_.empty();
    }

private:
    void leftmost();
    void rightmost();
    inline size_t get_sub_index(triedb_branch *parent, uint64_t key) {
      return (key >> (parent->max_key_bits() - triedb_params::MAX_BRANCH_BITS))
	        & (triedb_params::MAX_BRANCH-1);
    }

    void start_from_key(uint64_t parent_ptr, uint64_t key);
  
    void next();
    void previous();

    struct cursor {
        cursor(triedb_branch *_parent, uint64_t _parent_ptr, size_t _sub_index)
	    : parent(_parent),parent_ptr(_parent_ptr), sub_index(_sub_index) { }
        triedb_branch *parent;
        uint64_t parent_ptr;
        size_t sub_index;

        inline bool operator == (const cursor &other) const {
	    return parent_ptr == other.parent_ptr &&
		   sub_index == other.sub_index;
        }
    };
    triedb &db_;
    size_t height_;
    std::vector<cursor> spine_;

public:
    inline const std::vector<cursor> & path() const {
        return spine_;
    }
};

inline triedb_iterator triedb::begin(size_t at_height) {
    return triedb_iterator(*this, at_height);
}
    
inline triedb_iterator triedb::begin(size_t at_height, uint64_t key) {
    return triedb_iterator(*this, at_height, key);
}

inline triedb_iterator triedb::end(size_t at_height) {
    return triedb_iterator(*this, at_height, true);
}


    
inline const triedb_leaf * triedb::find(size_t at_height, uint64_t key,
					std::vector<
					    std::pair<triedb_branch *, size_t>
					> *path_opt)
{
    auto it = begin(at_height, key);
    if (it == end(at_height)) {
        return nullptr;
    }
    auto &leaf = *it;
    if (leaf.key() != key) {
        return nullptr;
    }
    if (path_opt != nullptr) {
        path_opt->clear();
        for (auto &e : it.path()) {
	    path_opt->push_back(std::make_pair(e.parent, e.sub_index));
        }
    }
    return &leaf;
}

}}

#endif
