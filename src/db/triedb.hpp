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
#include <algorithm>
#include "../common/lru_cache.hpp"
#include "../common/bits.hpp"
#include "util.hpp"
#include "triedb_params.hpp"

namespace prologcoin { namespace db {

//
// root_id: to identify a specific node in the blockchain.
//
class root_id {
public:
    static const size_t SIZE = sizeof(uint64_t);

    root_id() : value_(0) {
    }

    explicit root_id(uint64_t v) : value_(v) {
    }

    root_id(const root_id &other) = default;

    uint64_t value() const {
	return value_;
    }

    bool is_zero() const {
	return value_ == 0;
    }

    bool operator == (const root_id &other) const {
	return value_ == other.value_;
    }

    bool operator < (const root_id &other) const {
	return value_ < other.value_;
    }

    std::string str() const {
	std::stringstream ss;
	ss << value_;
	return ss.str();
    }

private:
    uint64_t value_;
};

}}

namespace std {
    template<> struct hash<prologcoin::db::root_id> {
	size_t operator () (const prologcoin::db::root_id &key) const {
	    return static_cast<size_t>(key.value());
	}
    };
}

namespace prologcoin { namespace db {

//
// root_node (stored in the root file)
//
// We have 'id' (32 bytes, typically a hash)
//         'previous_id' (so we can follow the chain backwards.)
//         'height' (so we don't have to compute it)
//         'file_offset' (where it is in the root file)
//         'ptr' (where it is in the data file)
//         'custom_data' (for more information - which will be
//                        different depending on database.)

class triedb;

class triedb_root {
private:
    friend class triedb;
    static const size_t MAX_SIZE_IN_BYTES = 4096;
public:
    triedb_root() : id_(0), previous_id_(0), ptr_(0), height_(0),
		    num_entries_(0) { }

    triedb_root(const triedb_root &parent, const root_id &child) :
	id_(child), previous_id_(parent.id()), 
	ptr_(0), height_(parent.height() + 1),
	num_entries_(parent.num_entries()) { }

    triedb_root(const root_id &id)
	: id_(id), previous_id_(),
	  ptr_(0), height_(0),
	  num_entries_(0) { }

    const root_id & id() const {
	return id_;
    }

    void set_id(const root_id &id) {
	id_ = id;
    }

    const root_id & previous_id() const {
	return previous_id_;
    }

    void set_previous_id(const root_id &id) {
	previous_id_ = id;
    }
	
    uint64_t ptr() const {
	return ptr_;
    }

    void set_ptr(uint64_t ptr) {
	ptr_ = ptr;
    }

    size_t height() const {
	return height_;
    }

    void set_height(size_t h) {
	height_ = h;
    }

    inline size_t num_entries() const {
	return static_cast<size_t>(num_entries_);
    }

    inline void set_num_entries(size_t n) {
	num_entries_ = static_cast<uint64_t>(n);
    }

    inline void increment_num_entries() {
	num_entries_++;
    }

    inline void decrement_num_entries() {
	num_entries_--;
    }

    static size_t serialization_size() {
	return sizeof(uint32_t) + // Total size
	       sizeof(uint32_t) + // height
	       sizeof(uint64_t) + // num entries
	       sizeof(uint64_t) + // previous root id
	       sizeof(uint64_t);  // ptr
    }

    std::string str() const {
	std::stringstream ss;
	ss << "triedb_root{";
	ss << "id=" << id_.str() << ",prev=" << previous_id_.str()
	   << ",ptr=" << ptr_ << ",height=" << height_ << ",num_entries="
	   << num_entries_ << "}";
	return ss.str();
    }

    void read(const uint8_t *buffer);
    void write(uint8_t *buffer) const;

private:
    // Not directly stored (it's identified by file offset)
    root_id id_;

    // Stored
    root_id previous_id_;
    uint64_t ptr_;
    size_t height_;
    size_t num_entries_;
};


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
	: key_(0), hash_size_(0), custom_data_size_(0), hash_(nullptr), custom_data_(nullptr) { }

    inline triedb_leaf(uint64_t key, const uint8_t *custom_data, size_t custom_data_size) : key_(key), hash_size_(0), custom_data_size_(custom_data_size), hash_(nullptr), custom_data_(nullptr) {
	if (custom_data_size_) {
	    custom_data_ = new uint8_t[custom_data_size_];
	    memcpy(custom_data_, custom_data, custom_data_size_);
	}
    }

    inline ~triedb_leaf() {
	if (hash_) delete [] hash_;
        if (custom_data_) delete [] custom_data_;
    }

    inline uint64_t key() const {
        return key_;
    }

    const uint8_t * hash() const {
	return hash_;
    }

    size_t hash_size() const {
	return hash_size_;
    }
    
    void set_hash(const uint8_t *h, size_t sz) {
	if (hash_ == nullptr || hash_size_ != sz) {
	    if (hash_) delete [] hash_;
	    hash_ = sz > 0 ? new uint8_t[sz] : nullptr;
	    hash_size_ = sz;
	}
	if (sz > 0) {
	    memcpy(hash_, h, sz);
	}
    }
  
    inline size_t custom_data_size() const {
        return custom_data_size_;
    }

    inline const uint8_t * custom_data() const {
        return custom_data_;
    }

    inline size_t serialization_size() const {
        size_t sz = sizeof(uint32_t) + // Total size
  	            sizeof(uint64_t) + // key
  	            sizeof(uint8_t) + // hash size
	            hash_size_ + // hash itself
	            custom_data_size_; // Value of leaf node
        return sz;
    }

    void read(const uint8_t *buffer);
    void write(uint8_t *buffer) const;

private:
    uint64_t key_;
    uint8_t hash_size_;
    uint32_t custom_data_size_; // Not stored on disk
    uint8_t *hash_;
    uint8_t *custom_data_;
};

//
// The branch node of a trie.
//
class triedb_branch : public triedb_node {
public:
    static const size_t MAX_SIZE_IN_BYTES = 1024;

    inline triedb_branch() : max_key_bits_(0), hash_size_(0), mask_(0), leaf_(0), ptr_(nullptr), hash_(nullptr) { }
    inline triedb_branch(const triedb_branch &other)
        : max_key_bits_(other.max_key_bits_), hash_size_(other.hash_size_), mask_(other.mask_), leaf_(other.leaf_), ptr_(nullptr), hash_(nullptr) {
        size_t n = num_children();
        ptr_ = new uint64_t[n];
	memcpy(ptr_, other.ptr_, sizeof(uint64_t)*n);
	if (hash_size_ > 0) {
	    hash_ = new uint8_t[hash_size_];
	    memcpy(hash_, other.hash_, hash_size_);
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
        size_t child_index = std::bitset<triedb_params::MAX_BRANCH>(mask_ & ((static_cast<uint32_t>(1) << sub_index) - 1)).count();
	size_t n = num_children();
	for (size_t i = child_index; i < n; i++) {
	    ptr_[i] = ptr_[i+1];
	}
	ptr_[n-1] = 0;
        leaf_ &= ~(1 << sub_index);
        mask_ &= ~(1 << sub_index);
    }

    inline const uint8_t * hash() const {
	return hash_;
    }

    inline size_t hash_size() const {
	return hash_size_;
    }

    inline void set_hash(const uint8_t *h, size_t sz) {
	if (hash_ == nullptr || hash_size_ != sz) {
	    if (hash_) delete [] hash_;
	    hash_ = sz > 0 ? new uint8_t[sz] : nullptr;
	    hash_size_ = sz;
	}
	if (sz > 0) {
	    memcpy(hash_, h, sz);
	}
    }

    size_t serialization_size() const {
       size_t sz = sizeof(uint32_t) +  // total size
                   sizeof(uint8_t) +  // max key bits
	           sizeof(uint8_t) + // hash size
                   sizeof(uint32_t) + // mask
                   sizeof(uint32_t) +  // leaf
                   sizeof(uint64_t)*num_children() + // ptrs
	           hash_size_; // hash itself
       assert(sz < MAX_SIZE_IN_BYTES);
       return sz;
    }

    void read(const uint8_t *buffer);
    void write(uint8_t *buffer) const;

    inline uint64_t get_child_pointer(size_t sub_index) const {
        size_t child_index = std::bitset<triedb_params::MAX_BRANCH>(mask_ & ((static_cast<uint32_t>(1) << sub_index) - 1)).count();
        return ptr_[child_index];
    }

    inline void set_child_pointer(size_t sub_index, uint64_t ptr) {
        size_t child_index = std::bitset<triedb_params::MAX_BRANCH>(mask_ & ((static_cast<uint32_t>(1) << sub_index) - 1)).count();
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
    uint8_t max_key_bits_; // A key is less than 64 bits
    uint8_t hash_size_; // Hash size in bytes (can be zero)
    uint32_t mask_;
    uint32_t leaf_;
    uint64_t *ptr_;
    uint8_t *hash_; // Optional hash (most DBs will use it though
                    // as we'll quickly want to know the new root
                    // hash after modifying the DB.)
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

    void set_leaf_hasher(std::function<void (triedb &db, triedb_leaf *leaf)> fn) {
	leaf_hasher_fn_ = fn;
    }

    const std::set<root_id> & find_roots(size_t height) const;

    // Asserts if there are more than 1. This is useful for testing only.
    root_id find_root(size_t height) const;
    
    const triedb_root & get_root(const root_id &id) const;

    root_id new_root();
    root_id new_root(const root_id &parent);

    bool is_empty(const root_id &at_root) const;
    void insert(const root_id &at_root, uint64_t key,
		const uint8_t *data, size_t data_size);
    void update(const root_id &at_root, uint64_t key,
		const uint8_t *data, size_t data_size);
    void remove(const root_id &at_root, uint64_t key);

    triedb_leaf * find(const root_id &at_root, uint64_t key,
		       std::vector<std::pair<triedb_branch *, size_t> >
		           *opt_path = nullptr);

    triedb_iterator begin(const root_id &at_root);
    triedb_iterator begin(const root_id &at_root, uint64_t key);
    triedb_iterator end(const root_id &at_root);

    triedb_leaf * get_leaf(triedb_branch *parent, size_t sub_index) {
        auto file_offset = parent->get_child_pointer(sub_index);
	return get_leaf(file_offset);
    }

    triedb_branch * get_branch(triedb_branch *parent, size_t sub_index) {
        auto file_offset = parent->get_child_pointer(sub_index);
	return get_branch(file_offset);
    }
 
    triedb_branch * get_root_branch(const root_id &at_root) {
        auto file_offset = get_root(at_root).ptr();
	return get_branch(file_offset);
    }
    
    std::pair<const uint8_t *, size_t> get_root_hash(const root_id &at_root) {
	auto br = get_root_branch(at_root);
	return std::make_pair(br->hash(), br->hash_size());
    }

    size_t num_entries(const root_id &at_root) const {
	return get_root(at_root).num_entries();
    }

private:
    friend class triedb_iterator;
  
    void branch_hasher(triedb_branch *branch);
    void leaf_hasher(triedb_leaf *leaf);

    std::function<void (triedb &db, triedb_leaf *leaf)> leaf_hasher_fn_{&triedb::leaf_hasher};

    // Return modified branch node
    void insert_or_update(const root_id &at_root, uint64_t key,
			  const uint8_t *data, size_t data_size, bool do_insert);
  
    std::pair<triedb_branch *, uint64_t> update_part(triedb_branch *node,
						     uint64_t key,
						     const uint8_t *data,
						     size_t data_size,
						     bool do_insert,
						     bool &new_entry);

    std::pair<triedb_branch *, uint64_t> remove_part(const root_id &at_root,
						     triedb_branch *node,
						     uint64_t key);
  
    boost::filesystem::path roots_file_path() const;
    fstream * get_roots_stream();
    void read_roots();
    inline void increment_num_entries(const root_id &id) {
	roots_[id].increment_num_entries();
    }
    inline void decrement_num_entries(const root_id &id) {
	roots_[id].decrement_num_entries();
    }
    void set_root(const root_id &at_root, uint64_t offset);
    boost::filesystem::path bucket_dir_location(size_t bucket_index) const;
    boost::filesystem::path bucket_file_path(size_t bucket_index) const;
    fstream * get_bucket_stream(size_t bucket_index);
    size_t scan_last_bucket();
    uint64_t scan_last_offset();
    fstream * set_file_offset(uint64_t offset);
    const triedb_root & get_root(const root_id &id);
  
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

    // Root references
    std::unordered_map<root_id, triedb_root> roots_;
    std::unordered_map<size_t, std::set<root_id> > roots_at_height_;
    fstream *roots_stream_;
  
    uint64_t last_offset_;

    bool cache_shutdown_;

    bool debug_;
};

class triedb_iterator {
public:
    inline triedb_iterator(const triedb_iterator &other) :
        db_(other.db_), root_(other.root_), spine_(other.spine_) {
    }
  
    inline triedb_iterator(triedb &db, const root_id &at_root)
      : db_(db), root_(at_root) {
        if (db.is_empty()) {
	    return;
	}
        auto parent_ptr = db.get_root(at_root).ptr();
	auto *parent = db.get_branch(parent_ptr);
	if (parent->mask() != 0) {
	    spine_.push_back(cursor(parent,
				    parent_ptr,
				    common::lsb(parent->mask())));
	    leftmost();
	}
    }

    inline triedb_iterator(triedb &db, const root_id &at_root, uint64_t key) 
	: db_(db), root_(at_root)  {
	if (!db.is_empty()) {
	    start_from_key(db.get_root(at_root).ptr(), key);
	}
    }

    inline triedb_iterator(triedb &db, const root_id &at_root, bool) 
        : db_(db), root_(at_root) {
    }

    inline triedb_iterator & operator = (const triedb_iterator &other) {
        db_ = other.db_;
	root_ = other.root_;
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

    inline triedb_leaf & operator * () {
        auto parent_ptr = spine_.back().parent_ptr;
        auto sub_index = spine_.back().sub_index;
	auto parent = db_.get_branch(parent_ptr);
        return *db_.get_leaf(parent, sub_index);
    }

    inline triedb_leaf * operator -> () {
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
    root_id root_;
    std::vector<cursor> spine_;

public:
    inline const std::vector<cursor> & path() const {
        return spine_;
    }
};

inline triedb_iterator triedb::begin(const root_id &at_root) {
    return triedb_iterator(*this, at_root);
}
    
inline triedb_iterator triedb::begin(const root_id &at_root, uint64_t key) {
    return triedb_iterator(*this, at_root, key);
}

inline triedb_iterator triedb::end(const root_id &at_root) {
    return triedb_iterator(*this, at_root, true);
}


    
inline triedb_leaf * triedb::find(const root_id &at_root, uint64_t key,
				  std::vector<
				    std::pair<triedb_branch *, size_t> >
				      *path_opt)
{
    auto it = begin(at_root, key);
    if (it == end(at_root)) {
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
