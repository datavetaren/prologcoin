#pragma once

#ifndef _common_merkle_trie_hpp
#define _common_merkle_trie_hpp

#include <bitset>
#include <iostream>
#include "blake2.hpp"

namespace prologcoin { namespace common {

static const uint8_t LSB_64_TABLE[64] = {
   63, 30,  3, 32, 59, 14, 11, 33,
   60, 24, 50,  9, 55, 19, 21, 34,
   61, 29,  2, 53, 51, 23, 41, 18,
   56, 28,  1, 43, 46, 27,  0, 35,
   62, 31, 58,  4,  5, 49, 54,  6,
   15, 52, 12, 40,  7, 42, 45, 16,
   25, 57, 48, 13, 10, 39,  8, 44,
   20, 47, 38, 22, 17, 37, 36, 26
};

static const int LSB_32_TABLE[32] =  {
  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

inline size_t lsb(uint64_t b) {
    unsigned int folded;
    b ^= b - 1;
    folded = (int)b ^(b >> 32);
    return LSB_64_TABLE[folded * 0x78291acf >> 26];
}

inline size_t lsb(uint32_t b) {
    return LSB_32_TABLE[((uint32_t)((b & -b) * 0x077CB531U)) >> 27];
}

inline size_t lsb(uint16_t b) {
    return lsb(static_cast<uint32_t>(b));
}

inline size_t lsb(uint8_t b) {
    return lsb(static_cast<uint32_t>(b));
}

static const unsigned char bit_rev_table_256[] = 
{
  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 
  0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 
  0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 
  0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 
  0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 
  0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
  0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 
  0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
  0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
  0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 
  0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
  0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
  0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 
  0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
  0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 
  0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

inline uint32_t bitrev(uint32_t b) {
    return (bit_rev_table_256[b & 0xff] << 24) |
           (bit_rev_table_256[(b >> 8) & 0xff] << 16) |
           (bit_rev_table_256[(b >> 16) & 0xff] << 8) |
           (bit_rev_table_256[(b >> 24) & 0xff]);
}

inline uint64_t bitrev(uint64_t b) {
    return static_cast<uint64_t>(bitrev(static_cast<uint32_t>(b >> 32))) |
           (static_cast<uint64_t>(bitrev(static_cast<uint32_t>(b))) << 32);
    
}

inline size_t msb(uint32_t b) {
    return 31 - lsb(bitrev(b));
}

inline size_t msb(uint64_t b) {
    return 63 - lsb(bitrev(b));
}
    
struct merkle_trie_hash_t {
    typedef uint8_t data_t[32];
    inline merkle_trie_hash_t() { memset(&data, 0, sizeof(data)); }

    inline bool operator == (const merkle_trie_hash_t &other) {
        return memcmp(&data, &other.data, sizeof(data)) == 0;
    }

    inline bool operator != (const merkle_trie_hash_t &other) {
        return ! operator == (other);
    }
  
    data_t data;
};

template<typename T> class merkle_trie_leaf {
public:
    inline merkle_trie_leaf() { }
    inline merkle_trie_leaf(uint64_t _key, const T &_value)
      : key_(_key), value_(_value) { }
    inline merkle_trie_leaf(uint64_t _key)
      : key_(_key) { }

    inline void compute_hash(blake2b_state *s) {
        blake2b_update(s, &key_, sizeof(key_));
        blake2b_update(s, &value_, sizeof(value_));
    }

    inline uint64_t key() const {
        return key_;
    }
  
    inline const T & value() const {
        return value_;
    }

    inline T & value() {
        return value_;
    }

    inline void set_value(const T &v) {
        value_ = v;
    }

private:
    uint64_t key_;
    T value_;
};

// If we just want to represent bitsets, then the value is ignored
template<> class merkle_trie_leaf<void> {
public:
    inline merkle_trie_leaf() { }
    inline merkle_trie_leaf(uint64_t _key) : key_(_key) { }
    inline void compute_hash(blake2b_state *s) {
	blake2b_update(s, &key_, sizeof(key_));
    }
    inline uint64_t key() const {
	return key_;
    }
private:
    uint64_t key_;
};

namespace detail {
    template<size_t N> struct derive_word_t;

    template<> struct derive_word_t<3> {
        typedef uint8_t word_t;
    };
    template<> struct derive_word_t<4> {
        typedef uint16_t word_t;
    };
    template<> struct derive_word_t<5> {
        typedef uint32_t word_t;
    };
    template<> struct derive_word_t<6> {
        typedef uint64_t word_t;
    };
}

template<typename T, size_t L> class merkle_trie_iterator;
template<typename T, size_t L> class merkle_trie_base;
    
template<typename T, size_t L> class merkle_trie_branch {
private:
    friend class merkle_trie_iterator<T,L>;
    friend class merkle_trie_base<T,L>;
    // My empirical studies show that for insertion of 1 million random
    // elements with incremental rehasing:
    // For SPARSENESS=1000000000 (1 billion, 0.1% density)
    //    MAX_BRANCH_BITS = 6 (2^6 = 64): 38374896 bytes and 109 seconds.
    //                      5 (2^5 = 32): 39021496 bytes and 75 seconds.
    //                      4 (2^4 = 16): 41020168 bytes and 56 secconds.
    //                      3 (2^3 = 8) : 46814680 bytes and 54 seconds.
    //
    // For SPARSENESS=100000000 (100 million, 1% density => more realistic for us)
    //                      6 (2^6 = 64): 36987496 bytes and 95 seconds.
    //                      5 (2^5 = 32): 34766704 bytes and 72 seconds.
    //                      4 (2^4 = 16): 41952400 bytes and 59 seconds.
    //                      3 (2^3 = 8) : 41667280 bytes and 57 seconds.
    //
    // So best memory profile is 2^5 = 32 for 1% density. And probably this
    // is going to be better as the set grows. 
    // The downside with a fanout of 32 is that more SHA256 computations
    // occur when increasing depth, but memory is more important than
    // speed once the dataset becomes very large. If we use multithreading or
    // GPUs for SHA256 computations we can probably make that performance
    // even better, so it's easier to tune CPU power than space.
    // 
    static const size_t MAX_BRANCH_BITS = 5;
    static const size_t MAX_BRANCH = 1 << MAX_BRANCH_BITS;

    typedef typename detail::derive_word_t<MAX_BRANCH_BITS>::word_t word_t;
  
public:
    typedef merkle_trie_hash_t hash_t;

    static merkle_trie_branch * new_root() {
        size_t n = sizeof(merkle_trie_branch) + sizeof(void *)*MAX_BRANCH;
        merkle_trie_branch *r = reinterpret_cast<merkle_trie_branch *>(::operator new(n));
        memset(r, 0, n);
	r->mask_ = 0;
	r->leaf_ = 0;
	r->hash_ = hash_t();
	return r;
    }

    inline merkle_trie_branch() : mask_(0), leaf_(0) { }

    inline const hash_t & hash() const {
        return hash_;
    }

    inline size_t num_bytes() const {
        auto *t = const_cast<merkle_trie_branch *>(this);
        return t->num_bytes_helper();
    }

    inline void rehash_all() {
	auto m = mask_;
	if (m == 0) {
	    return;
	}
        for (size_t i = lsb(m); i < MAX_BRANCH;) {
	    if (is_branch(i)) {
	        merkle_trie_branch *child = get_branch(i);
		child->rehash_all();
	    }
	    m &= (static_cast<word_t>(-1) << i) << 1;
	    i = (m == 0) ? MAX_BRANCH : lsb(m);
        }
	recompute_hash();
    }

    template<typename U> inline merkle_trie_leaf<T> & insert_part(merkle_trie_branch *&parent, size_t _at_part, bool rehash, uint64_t _key, U &updater) {
	size_t sub_index = (_key >> (L - MAX_BRANCH_BITS - _at_part)) & (MAX_BRANCH-1);
        if (parent->is_empty(sub_index)) {
	    reallocate_insert(parent, sub_index);
	    auto *leaf = new_leaf(_key);
	    parent->set_leaf(sub_index, leaf);
	    updater(*leaf);
	    if (rehash) parent->recompute_hash();
	    return *leaf;
	}
	if (parent->is_leaf(sub_index)) {
	    auto *leaf = parent->get_leaf(sub_index);
	    if (leaf->key() == _key) {
		if (rehash) parent->recompute_hash();
		return *leaf;
	    }
	    // We need to create a branch node at sub_index
	    auto *new_branch = reinterpret_cast<merkle_trie_branch *>(::operator new(sizeof(merkle_trie_branch) + sizeof(void *)));
	    new_branch->data_[0] = leaf;
	    size_t sub_sub_index = (leaf->key() >> (L - 2*MAX_BRANCH_BITS - _at_part)) & (MAX_BRANCH-1);
	    new_branch->mask_ = static_cast<word_t>(1) << sub_sub_index;
	    new_branch->leaf_ = static_cast<word_t>(1) << sub_sub_index;
	    auto &leaf1 = insert_part(new_branch, _at_part + MAX_BRANCH_BITS, rehash, _key, updater);
	    if (rehash) new_branch->recompute_hash();
	    parent->set_branch(sub_index, new_branch);
	    if (rehash) parent->recompute_hash();
	    return leaf1;
	}
	auto *child = parent->get_branch(sub_index);
	auto &leaf = insert_part(child, _at_part + MAX_BRANCH_BITS, rehash, _key, updater);
	parent->set_branch(sub_index, child);
	if (rehash) parent->recompute_hash();
	return leaf;
    }

    inline merkle_trie_leaf<T> * find_part(merkle_trie_branch *parent, size_t _at_part, uint64_t _key) {
	size_t sub_index = (_key >> (L - MAX_BRANCH_BITS - _at_part)) & (MAX_BRANCH-1);
	if (parent->is_empty(sub_index)) {
	    return nullptr;
	}
	if (parent->is_leaf(sub_index)) {
	    auto *leaf = parent->get_leaf(sub_index);
	    if (leaf->key() == _key) {
		return leaf;
	    } else {
		return nullptr;
	    }
	} else {
	    auto *child = parent->get_branch(sub_index);
	    return find_part(child, _at_part + MAX_BRANCH_BITS, _key);
	}
    }

    inline bool remove_part(merkle_trie_branch *&parent, size_t _at_part, bool rehash, uint64_t _key) {
	size_t sub_index = (_key >> (L - MAX_BRANCH_BITS - _at_part)) & (MAX_BRANCH-1);
        if (parent->is_empty(sub_index)) {
	    return false; // Key doesn't exist
	}
	if (parent->is_leaf(sub_index)) {
	    auto *leaf = parent->get_leaf(sub_index);
	    if (leaf->key() == _key) {
		// Element found!
		parent->delete_child(sub_index);
		reallocate_remove(parent, sub_index);
	    } else {
		// Element not found!
		return false;
	    }
	} else {
	    // Branch
	    auto *child = parent->get_branch(sub_index);
	    bool r = remove_part(child, _at_part + MAX_BRANCH_BITS, rehash, _key);
	    if (!r) {
		return false;
	    }
	    
	    if (child == nullptr) {
		reallocate_remove(parent, sub_index);
	    } else {
	        // Replace singleton X -> Y -> Z with X -> Z
	        size_t other_sub_index = 0;
		if (child->num_children() == 1 &&
		    ((other_sub_index = lsb(child->mask_)) || true) &&
		    child->is_leaf(other_sub_index)) {
		    auto *leaf = child->get_leaf(other_sub_index);
		    delete child;
		    parent->set_leaf(sub_index, leaf);
		} else {
		    parent->set_branch(sub_index, child);
		}
	    }
	}
	size_t n = parent->num_children();
	if (n == 0) {
	    // No more children. Let's delete it.
	    delete parent;
	    parent = nullptr;
	} else {
	    // Node is not deleted, so recompute hash
	    if (rehash) parent->recompute_hash();
	}
	return true;
    }

private:
    inline size_t num_bytes_helper() {
        // Compute size in bytes
        size_t bytes = sizeof(merkle_trie_branch) + num_children()*sizeof(void *);
	auto m = mask_;
	if (m == 0) {
	    return bytes;
	}
        for (size_t i = lsb(m); i < MAX_BRANCH;) {
	    if (!is_empty(i)) {
	        if (is_leaf(i)) {
	            bytes += sizeof(merkle_trie_leaf<T>);
		} else {
	  	    merkle_trie_branch *child = get_branch(i);
		    bytes += child->num_bytes_helper();
		}
	    }
	    m &= (static_cast<word_t>(-1) << i) << 1;
	    i = (m == 0) ? MAX_BRANCH : lsb(m);
        }
	return bytes;
    }
  
    inline bool is_leaf(size_t sub_index) const {
        return ((leaf_ >> sub_index) & 1) != 0;
    }

    inline bool is_branch(size_t sub_index) const {
        return !is_leaf(sub_index);
    }

    inline bool is_empty(size_t sub_index) const {
        return ((mask_ >> sub_index) & 1) == 0;
    }

    inline size_t num_children() const {
        return std::bitset<MAX_BRANCH>(mask_).count();
    }

    inline void reallocate_insert(merkle_trie_branch *&parent, size_t sub_index) {
        size_t n = parent->num_children() + 1;
        merkle_trie_branch *new_parent = reinterpret_cast<merkle_trie_branch *>(::operator new(sizeof(merkle_trie_branch) + sizeof(void *)*n));
	size_t n_left = std::bitset<MAX_BRANCH>(parent->mask_ & ((static_cast<word_t>(1) << sub_index) - 1)).count();
	size_t n_right = std::bitset<MAX_BRANCH>((parent->mask_ >> sub_index) >> 1).count();
	std::copy(parent->data_, parent->data_+n_left, &new_parent->data_[0]);
	std::copy(parent->data_+n_left, parent->data_+n_left+n_right, &new_parent->data_[n_left+1]);
	new_parent->data_[n_left] = nullptr;
	new_parent->mask_ = parent->mask_;
	new_parent->leaf_ = parent->leaf_;
	::operator delete(parent);
	parent = new_parent;
    }

    inline void reallocate_remove(merkle_trie_branch *&parent, size_t sub_index) {
        size_t n = parent->num_children() - 1;

        merkle_trie_branch *new_parent = reinterpret_cast<merkle_trie_branch *>(::operator new(sizeof(merkle_trie_branch) + sizeof(void *)*n));
	size_t n_left = std::bitset<MAX_BRANCH>(parent->mask_ & ((static_cast<word_t>(1) << sub_index) - 1)).count();
	size_t n_right = std::bitset<MAX_BRANCH>((parent->mask_ >> sub_index) >> 1).count();
	std::copy(parent->data_, parent->data_+n_left, &new_parent->data_[0]);
	std::copy(parent->data_+n_left+1, parent->data_+n_left+1+n_right, &new_parent->data_[n_left]);
	new_parent->mask_ = parent->mask_ & ~(static_cast<word_t>(1) << sub_index);
	new_parent->leaf_ = parent->leaf_ & ~(static_cast<word_t>(1) << sub_index);
	::operator delete(parent);
	parent = new_parent;
    }
      
    inline size_t get_child_index(size_t sub_index) {
        return std::bitset<MAX_BRANCH>(mask_ & (static_cast<word_t>(1) << sub_index) - 1).count();
    }

    inline merkle_trie_leaf<T> * get_leaf(size_t sub_index) {
        return reinterpret_cast<merkle_trie_leaf<T> *>(data_[get_child_index(sub_index)]);
    }

    inline merkle_trie_branch * get_branch(size_t sub_index) {
        return reinterpret_cast<merkle_trie_branch *>(data_[get_child_index(sub_index)]);
    }

    inline void delete_child(size_t sub_index) {
	if (is_leaf(sub_index)) {
	    auto *leaf = get_leaf(sub_index);
	    delete leaf;
	} else {
	    auto *branch = get_branch(sub_index);
	    ::operator delete(branch);
	}
    }

    inline void set_leaf(size_t sub_index, merkle_trie_leaf<T> *leaf) {
        mask_ |= static_cast<word_t>(1) << sub_index;
        leaf_ |= static_cast<word_t>(1) << sub_index;      
        data_[get_child_index(sub_index)] = reinterpret_cast<void *>(leaf);
    }

    inline void set_branch(size_t sub_index, merkle_trie_branch *branch) {
        mask_ |= static_cast<word_t>(1) << sub_index;
        leaf_ &= ~(static_cast<word_t>(1) << sub_index);
        data_[get_child_index(sub_index)] = reinterpret_cast<void *>(branch);
    }

    inline merkle_trie_leaf<T> * new_leaf(uint64_t _key) {
        return new merkle_trie_leaf<T>(_key);
    }

    inline void recompute_hash() {
        blake2b_state s[1];
	blake2b_init(&s[0], sizeof(hash_t::data_t));
	word_t m = mask_;
	if (m != 0) {
    	    for (size_t i = lsb(m); i < MAX_BRANCH;) {
	        if (is_leaf(i)) {
	            get_leaf(i)->compute_hash(s);
	        } else {
	            auto &h = get_branch(i)->hash();
	            blake2b_update(s, &h.data[0], sizeof(h));
	        }
   	        m &= ((static_cast<word_t>(-1) << i) << 1);
	        i = (m == 0) ? MAX_BRANCH : lsb(m);
	    }
	}
        blake2b_final(&s[0], &hash_.data[0], sizeof(hash_));
    }

    inline void internal_integrity_check() {
        assert(mask_ != 0);
	word_t m = mask_;
	for (size_t i = lsb(m); i < MAX_BRANCH;) {
	    if (is_branch(i)) {
	        auto *b = get_branch(i);
		if (b->num_children() == 1) {
		    size_t sub_index = lsb(b->mask_);
		    if (b->is_leaf(sub_index)) {
		        assert(false && "Should not be a singleton leaf with a singleton parent branch");
		    }
		}
		b->internal_integrity_check();
	    }
	    m &= (static_cast<word_t>(-1) << i) << 1;
	    i = (m == 0) ? MAX_BRANCH : lsb(m);
	}
    }

    word_t mask_;
    word_t leaf_;
    hash_t hash_;   // 32 bytes    
    void * data_[]; // Can be different things here.
};


template<typename T, size_t L> class merkle_trie_base;
    
template<typename T, size_t L> class merkle_trie_iterator {
private:
    typedef merkle_trie_branch<T,L> mtrie;  
    typedef merkle_trie_base<T,L> mbase;
  
public:
    inline merkle_trie_iterator(mbase *base) {
        base_ = base;
        spine.push_back(cursor(base_->root(),0));
	leftmost();
    }

    inline merkle_trie_iterator(mbase *base, bool) {
        base_ = base;
    }
  
    inline merkle_trie_iterator(mbase *base, uint64_t _key) {
        base_ = base;
	start_from_key(base_->root(), _key);
    }
  
    inline merkle_trie_iterator & operator ++ () {
        next();
        return *this;
    }

    inline merkle_trie_iterator & operator -- () {
        previous();
	return *this;
    }

    inline merkle_trie_iterator operator - (int i) {
        auto copy_it = *this;
        while (i > 0) {
	    --i;
	    --copy_it;
        }
	return copy_it;
    }

    inline merkle_trie_iterator operator + (int i) {
        auto copy_it = *this;
        while (i > 0) {
	    --i;
	    ++copy_it;
        }
	return copy_it;
    }
  
    inline bool operator == (const merkle_trie_iterator &other) const {
        if (other.at_end()) {
	    return at_end();
        }
	return spine == other.spine;
    }

    inline bool operator != (const merkle_trie_iterator &other) const {
        return ! operator == (other);
    }

    inline const merkle_trie_leaf<T> & operator * () const {
        return *spine.back().node->get_leaf(spine.back().index);
    }

    inline const merkle_trie_leaf<T> * operator -> () const {
        return spine.back().node->get_leaf(spine.back().index);
    }    

    static merkle_trie_iterator & erase(merkle_trie_iterator &it);

    inline bool at_end() const {
        return spine.empty();
    }

private:

    inline void leftmost() {
        if (spine.empty()) {
	    return;
        }
        auto node = spine.back().node;
	while (!spine.empty() && node->mask_ == 0) {
	    spine.pop_back();
	    if (!spine.empty()) node = spine.back().node;
	}
	if (spine.empty()) {
	    return;
	}
	auto index = spine.back().index;
	while (node->is_branch(index)) {
	    node = node->get_branch(index);
	    index = lsb(node->mask_);
  	    spine.push_back(cursor(node, index));
	}
	// At this point we've must found a leaf
    }

    inline void rightmost() {
        if (spine.empty()) {
	    return;
        }
        auto node = spine.back().node;
	while (!spine.empty() && node->mask_ == 0) {
	    spine.pop_back();
	    if (!spine.empty()) node = spine.back().node;
	}
	if (spine.empty()) {
	    return;
	}
	auto index = spine.back().index;
	while (node->is_branch(index)) {
	    node = node->get_branch(index);
	    index = msb(node->mask_);
  	    spine.push_back(cursor(node, index));
	}
	// At this point we've must found a leaf
    }
  
    inline size_t get_index(uint64_t _key, size_t at_part) {
        return (_key >> (L - mtrie::MAX_BRANCH_BITS - at_part)) & (mtrie::MAX_BRANCH-1);
    }

    inline void start_from_key(mtrie *_root, uint64_t _key) {
        mtrie *node = _root;
	size_t at_part = 0;
	size_t index = get_index(_key, at_part);
	auto m = node->mask_;
	if (node->is_empty(index)) {
  	    m &= (static_cast<typename mtrie::word_t>(-1) << index) << 1;
	    index = (m == 0) ? mtrie::MAX_BRANCH : lsb(m);
	    if (index == mtrie::MAX_BRANCH) {
	        return;
	    }
	    spine.push_back(cursor(node, index));
	    leftmost();
	    return;
	}

	while (!node->is_empty(index) && node->is_branch(index)) {
	    spine.push_back(cursor(node, index));
	    node = node->get_branch(index);
	    at_part += mtrie::MAX_BRANCH_BITS;
	    index = get_index(_key, at_part);
	}

	if (node->is_empty(index)) {
	    m = node->mask_;
  	    m &= (static_cast<typename mtrie::word_t>(-1) << index) << 1;	    
	    index = (m == 0) ? mtrie::MAX_BRANCH : lsb(m);
   	    if (index == mtrie::MAX_BRANCH) {
	        index = 31;
		spine.push_back(cursor(node, index));		
	        next();
		return;
	    }
	    spine.push_back(cursor(node, index));
	    leftmost();
	    return;
	}

	spine.push_back(cursor(node, index));
	
	if (node->is_empty(index)) {
  	    next();
	}

	// At this point we've must found a leaf, but we could get the one lower
	auto *leaf = node->get_leaf(index);
	if (leaf->key() < _key) {
	    next();
	}
    }
  
    inline void next() {
        auto node = spine.back().node;
        auto index = spine.back().index;
        typename mtrie::word_t mask_next = node->mask_ & ((static_cast<typename mtrie::word_t>(-1) << index) << 1);
        while (mask_next == 0) {
  	    spine.pop_back();
	    if (spine.empty()) {
	        return;
	    }
	    node = spine.back().node;
	    index = spine.back().index;
	    mask_next = node->mask_ & ((static_cast<typename mtrie::word_t>(-1) << index) << 1);
        }
        spine.back().index = lsb(mask_next);
        leftmost();
    }

    inline void previous() {
        if (at_end()) {
	    spine.push_back(cursor(base_->root(),0));
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

template<typename T> struct merkle_trie_updater {
    inline merkle_trie_updater(const T &_value) : value(_value) { }
    inline void operator () (merkle_trie_leaf<T> &leaf) { leaf.set_value(value); }
    T value;
};

template<> struct merkle_trie_updater<void> {
    inline merkle_trie_updater() { }
    inline void operator () (merkle_trie_leaf<void> &) { }  
};
    
template<typename T, size_t L> class merkle_trie_base {
public:
    friend class merkle_trie_iterator<T,L>;
    typedef merkle_trie_hash_t hash_t;
    typedef typename  merkle_trie_branch<T,L>::word_t word_t;

    inline merkle_trie_base() {
        root_ = merkle_trie_branch<T,L>::new_root();
	dirty = false;
	auto_rehash = true;
    }

    inline ~merkle_trie_base() {
	std::vector<merkle_trie_branch<T,L> *> visit;
	visit.push_back(root_);
	while (!visit.empty()) {
	    auto *parent = visit.back();
	    visit.pop_back();
	    auto mask = parent->mask_;
	    for (size_t i = lsb(mask); mask != 0; ) {
		if (parent->is_branch(i)) {
		    auto *branch = parent->get_branch(i);
		    visit.push_back(branch);
		} else {
		    auto *leaf = parent->get_leaf(i);
		    delete leaf;
		}
		mask &= (static_cast<word_t>(-1) << i) << 1;
		i = lsb(mask);
	    }
	    ::operator delete(parent);
	}
    }

protected:
    inline merkle_trie_leaf<T> & insert(uint64_t _key, merkle_trie_updater<T> updater) {
        auto &leaf = root_->insert_part(root_, 0, auto_rehash, _key, updater);
	if (!auto_rehash) {
	    dirty = true;
	}
	return leaf;
    }

    inline merkle_trie_leaf<T> * find(uint64_t _key) {
	auto *leaf = root_->find_part(root_, 0, _key);
	if (leaf == nullptr) {
	    return nullptr;
	}
	return leaf;
    }

public:
    inline void remove(uint64_t _index) {
	root_->remove_part(root_, 0, auto_rehash, _index);
	if (root_ == nullptr) {
	    root_ = new merkle_trie_branch<T,L>();
	}
	if (!auto_rehash) {
	    dirty = true;
	}
    }

    inline merkle_trie_iterator<T,L> & erase(merkle_trie_iterator<T,L> &it) {
        return merkle_trie_iterator<T,L>::erase(it);
    }

    inline const hash_t & hash() const {
        assert(!dirty);
        return root_->hash();
    }

    inline size_t num_bytes() const {
        return root_->num_bytes();
    }

    inline merkle_trie_iterator<T,L> begin() {
        return merkle_trie_iterator<T,L>(this);
    }

    inline merkle_trie_iterator<T,L> begin(uint64_t key) {
        return merkle_trie_iterator<T,L>(this, key);
    }

    inline merkle_trie_iterator<T,L> end() {
        return merkle_trie_iterator<T,L>(this, true);
    }

    inline void set_auto_rehash(bool b) {
        auto_rehash = b;
    }

    inline void rehash_all() {
        root_->rehash_all();
	dirty = false;
    }

    inline merkle_trie_branch<T,L> * root() {
        return root_;
    }

    inline void internal_integrity_check() {
        return root_->internal_integrity_check();
    }

private:
    merkle_trie_branch<T,L> *root_;
    bool dirty;
    bool auto_rehash;
};

template<typename T, size_t L> merkle_trie_iterator<T,L> & merkle_trie_iterator<T,L>::erase( merkle_trie_iterator<T,L> &it) 
{
    uint64_t k = (*it).key();
    it.base_->remove(k);
    it.spine.clear();
    it.start_from_key(it.base_->root_, k);
    return it;
}
    
template<typename T, size_t L> class merkle_trie : public merkle_trie_base<T,L> {
public:
    inline merkle_trie() { }

    inline void insert(uint64_t _key, const T &_value) {
        merkle_trie_updater<T> updater(_value);
        merkle_trie_base<T,L>::insert(_key, updater);
    }
  
    inline const T * find(uint64_t _key) {
	if (auto *leaf = merkle_trie_base<T,L>::find(_key)) {
	    return &(leaf->value());
	} else {
	    return nullptr;
	}
    }
};

template<size_t L> class merkle_trie<void,L> : public merkle_trie_base<void,L> {
public:
    inline merkle_trie() { }

    inline void insert(uint64_t _key) {
        merkle_trie_updater<void> updater;
	merkle_trie_base<void,L>::insert(_key, updater);
    }
    inline bool find(uint64_t _key) {
	return merkle_trie_base<void,L>::find(_key) != nullptr;
    }
};

}}

#endif

