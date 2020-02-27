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
	custom_data_size_ = sz - (p - buffer);
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
  
    inline uint32_t mask() const { return mask_; }
    inline void set_mask(uint32_t m) { mask_ = m; }

    inline size_t num_children() const
        { return std::bitset<triedb_params::MAX_BRANCH>(mask_).count(); }

    inline bool is_leaf(size_t sub_index) const {
        return ((leaf_ >> sub_index) & 1) != 0;
    }
    inline void set_leaf(size_t sub_index) {
	leaf_ |= (1 << sub_index); 
    }

    inline bool is_branch(size_t sub_index) const {
        return !is_leaf(sub_index);
    }

    inline void set_branch(size_t sub_index) {
	leaf_ &= ~(1 << sub_index);
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
	mask_ |= 1 << sub_index;
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

    bool is_empty() const {
        return roots_.size() == 0;
    }

    void erase_all();

    void flush();

    inline void set_update_function(
        const std::function<void(triedb &, triedb_branch &)> &branch_updater) {
       branch_update_fn_ = branch_updater;
    }

    void insert(size_t at_height, uint64_t key, const uint8_t *data, size_t data_size);
  
private:
    friend class triedb_iterator;
  
    // Return modified branch node
    std::pair<triedb_branch *, uint64_t> insert_part(triedb_branch *node, uint64_t key, const uint8_t *data, size_t data_size, size_t part);
  
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
    fstream *roots_stream_;
  
    uint64_t last_offset_;

    size_t num_heights_;

    // Update function
    // This function is called whenever a branch node needs to be updated;
    // let the client update any custom data.
    std::function<void(triedb &, triedb_branch &)> branch_update_fn_;
};

/*
class triedb_iterator {
public:
    inline triedb_iterator(triedb &db, size_t at_height) : db_(db) {
        spine_.push_back(cursor(db.get_root(at_height), 0));
	leftmost();
    }

    inline triedb_iterator(triedb &db, size_t at_height, uint64_t key) : db_(db)  {
        start_from_key(db.get_root(at_height), key);
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
        return *db_.get_leaf(parent_ptr, sub_index);
    }

    inline const triedb_leaf * operator -> () const {
        auto parent_ptr = spine_.back().parent_ptr;
        auto sub_index = spine_.back().sub_index;
        return db_.get_leaf(parent_ptr, sub_index);
    }    

    static triedb_iterator & erase(triedb_iterator &it);

    inline bool at_end() const {
        return spine_.empty();
    }

private:

    inline void leftmost() {
        if (spine_.empty()) {
	    return;
        }
        auto parent_ptr = spine_.back().parent_ptr;
	auto *parent = db_.get_branch(parent_ptr);
	while (!spine_.empty() && parent->mask() == 0) {
	    spine_.pop_back();
	    if (!spine_.empty()) {
	        parent_ptr = spine_.back().parent_ptr;
		parfent = db_.get_branch(parent_ptr);
	    }
	}
	if (spine_.empty()) {
	    return;
	}
	auto index = spine_.back().sub_index;
	while (parent->is_branch(sub_index)) {
	    parent_ptr = db_.get_child_pointer(sub_index);
  	    parent = db_.get_branch(parent, index);
	    sub_index = common::lsb(parent->mask());
  	    spine_.push_back(cursor(parent_ptr, sub_index));
	}
	// At this point we've must found a leaf
    }

    inline void rightmost() {
        if (spine_.empty()) {
	    return;
        }
        auto parent_ptr = spine_.back().parent_ptr;
	auto *parent = db_.get_branch(parent_ptr);
	while (!spine_.empty() && parent->mask() == 0) {
	    spine_.pop_back();
	    if (!spine.empty()) {
	        parent_ptr = spine.back().parent_ptr;
		parent = db_.get_branch(parent_ptr);
	    }
	}
	if (spine.empty()) {
	    return;
	}
	auto sub_index = spine_.back().sub_index;
	while (parent->is_branch(sub_index)) {
  	    parent_ptr = parent->get_child_pointer(sub_index);
	    parent = get_branch(parent_ptr);
	    sub_index = msb(parent->mask());
  	    spine_.push_back(cursor(parent_ptr, sub_index));
	}
	// At this point we've must found a leaf
    }
  
    inline size_t get_sub_index(uint64_t key, size_t part) {
        return (key >> (triedb_params::MAX_KEY_SIZE_BITS -
			triedb_params::MAX_BRANCH_BITS - part))
	       & (triedb_params::MAX_BRANCH-1);
    }

    inline void start_from_key(uint64_t parent_ptr, uint64_t key) {
        auto *parent = db_.get_branch(parent_ptr);
	size_t part = 0;
	size_t sub_index = get_sub_index(key, at_part);
	auto m = parent->mask();
	if (parent->is_empty(sub_index)) {
  	    m &= (static_cast<uint32_t>(-1) << sub_index) << 1;
	    sub_index = (m == 0) ? triedb_params::MAX_BRANCH : common::lsb(m);
	    if (sub_index == triedb_params::MAX_BRANCH) {
	        return;
	    }
	    spine_.push_back(cursor(parent_ptr, sub_index));
	    leftmost();
	    return;
	}

	while (!parent->is_empty(sub_index) && parent->is_branch(sub_index)) {
	    spine_.push_back(cursor(parent_ptr, sub_index));
	    parent_ptr = parent->get_child_pointer(sub_index);
	    parent = db_.get_parent(parent_ptr);
	    part += triedb_params::MAX_BRANCH_BITS;
	    sub_index = get_sub_index(key, part);
	}

	if (parent->is_empty(sub_index)) {
  	    m = parent->mask();
  	    m &= (static_cast<uint32_t>(-1) << index) << 1;
	    index = (m == 0) ? triedb_params::MAX_BRANCH : common::lsb(m);
   	    if (sub_index == triedb_params::MAX_BRANCH) {
	        sub_index = triedb_params::MAX_BRANCH - 1;
		spine_.push_back(cursor(parent_ptr, sub_index));
	        next();
		return;
	    }
	    spine_.push_back(cursor(parent_ptr, sub_index));
	    leftmost();
	    return;
	}

	spine_.push_back(cursor(parent_ptr, sub_index));
	
	if (parent->is_empty(sub_index)) {
  	    next();
	}

	bool found = false;
	while (!spine_.empty() && !found) {
	    auto parent_ptr = spine_.back().parent_ptr;
	    auto sub_index = spine_.back().sub_index;
	    auto parent = db_.get_parent(parent_ptr);
	    auto *leaf = db_.get_leaf(parent, sub_index);
	    found = leaf->key() >= _key;
	    if (!found) {
	        next();
	    }
	}
    }
  
    inline void next() {
        auto parent_ptr = spine_.back().parent_ptr;
        auto sub_index = spine_.back().sub_index;
	auto *parent = db_.get_parent(parent_ptr);
        auto mask_next = parent->mask() & ((static_cast<uint32_t>(-1) << sub_index) << 1);
        while (mask_next == 0) {
  	    spine_.pop_back();
	    if (spine_.empty()) {
	        return;
	    }
	    parent_ptr = spine_.back().parent_ptr;
	    sub_index = spine_.back().sub_index;
	    parent = db_.get_branch(parent_ptr);
	    mask_next = parent->mask() & ((static_cast<uint32_t>(-1) << sub_index) << 1);
        }
        spine_.back().sub_index = common::lsb(mask_next);
        leftmost();
    }

    inline void previous() {
        if (at_end()) {
	    spine_.push_back(cursor(base_->root(),0));
	    rightmost();
	    return;
        }
	auto node = spine.back().node;
	auto index = spine.back().index;
        typename mtrie::word_t mask_prev = node->mask_ & ((static_cast<typename mtrie::word_t>(-1) >> (31-index)) >> 1);

        while (mask_prev == 0) {
  	    spine.pop_back();
	    if (spine.empty()) {
	        return;
	    }
	    node = spine.back().node;
	    index = spine.back().index;
	    mask_prev = node->mask_ & ((static_cast<typename mtrie::word_t>(-1) >> (31-index)) >> 1);
        }

        spine.back().index = msb(mask_prev);
        rightmost();
    }
  
    struct cursor {
        cursor(mtrie *_node, size_t _index) : node(_node), index(_index) { }
        mtrie *node;
        size_t index;

        inline bool operator == (const cursor &other) const {
	    return node == other.node && index == other.index;
        }
    };
    merkle_trie_base<T,L> *base_;
    std::vector<cursor> spine;
};

*/    
    
}}

#endif
