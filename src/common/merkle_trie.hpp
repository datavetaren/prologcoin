#pragma once

#ifndef _common_merkle_trie_hpp
#define _common_merkle_trie_hpp

#include <bitset>
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
    inline merkle_trie_leaf(uint64_t _index, size_t _value)
      : index_(_index), value_(_value) { }

    inline void compute_hash(blake2b_state *s) {
        blake2b_update(s, &value_, sizeof(value_));
    }

    inline uint64_t index() const {
        return index_;
    }
  
    inline const T & value() const {
        return value_;
    }

    inline void set_value(const T &v) {
        value_ = v;
    }

private:
    uint64_t index_;
    T value_;
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
    
template<typename T, size_t L> class merkle_trie_branch {
private:
    friend class merkle_trie_iterator<T,L>;
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

    inline void insert_part(merkle_trie_branch *&parent, size_t _at_part, bool rehash, uint64_t _index, const T &_value) {
	size_t sub_index = (_index >> (L - MAX_BRANCH_BITS - _at_part)) & (MAX_BRANCH-1);
        if (parent->is_empty(sub_index)) {
	    reallocate_insert(parent, sub_index);
	    auto *leaf = new_leaf(_index, _value);
	    parent->set_leaf(sub_index, leaf);
	    if (rehash) parent->recompute_hash();
	    return;
	}
	if (parent->is_leaf(sub_index)) {
	    auto *leaf = parent->get_leaf(sub_index);
	    if (leaf->index() == _index) {
	        leaf->set_value(_value);
		if (rehash) parent->recompute_hash();
		return;
	    }
	    // We need to create a branch node at sub_index
	    auto *new_branch = reinterpret_cast<merkle_trie_branch *>(::operator new(sizeof(merkle_trie_branch) + sizeof(void *)));
	    new_branch->data_[0] = leaf;
	    size_t sub_sub_index = (leaf->index() >> (L - 2*MAX_BRANCH_BITS - _at_part)) & (MAX_BRANCH-1);
	    new_branch->mask_ = static_cast<word_t>(1) << sub_sub_index;
	    new_branch->leaf_ = static_cast<word_t>(1) << sub_sub_index;
	    insert_part(new_branch, _at_part + MAX_BRANCH_BITS, rehash, _index, _value );
	    if (rehash) new_branch->recompute_hash();
	    
	    parent->set_branch(sub_index, new_branch);
	    if (rehash) parent->recompute_hash();
	    return;
	}
	auto *child = parent->get_branch(sub_index);
	insert_part(child, _at_part + MAX_BRANCH_BITS, rehash, _index, _value);
	parent->set_branch(sub_index, child);
	if (rehash) parent->recompute_hash();
    }

    inline merkle_trie_leaf<T> * find_part(merkle_trie_branch *parent, size_t _at_part, uint64_t _index) {
	size_t sub_index = (_index >> (L - MAX_BRANCH_BITS - _at_part)) & (MAX_BRANCH-1);
	if (parent->is_empty(sub_index)) {
	    return nullptr;
	}
	if (parent->is_leaf(sub_index)) {
	    auto *leaf = parent->get_leaf(sub_index);
	    if (leaf->index() == _index) {
		return leaf;
	    } else {
		return nullptr;
	    }
	} else {
	    auto *child = parent->get_branch(sub_index);
	    return find_part(child, _at_part + MAX_BRANCH_BITS, _index);
	}
    }

    inline bool remove_part(merkle_trie_branch *&parent, size_t _at_part, bool rehash, uint64_t _index) {
	size_t sub_index = (_index >> (L - MAX_BRANCH_BITS - _at_part)) & (MAX_BRANCH-1);
        if (parent->is_empty(sub_index)) {
	    return false; // Key doesn't exist
	}
	if (parent->is_leaf(sub_index)) {
	    auto *leaf = parent->get_leaf(sub_index);
	    if (leaf->index() == _index) {
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
	    bool r = remove_part(child, _at_part + MAX_BRANCH_BITS, rehash, _index);
	    if (!r) {
		return false;
	    }
	    if (child == nullptr) {
		// Branch node was deleted
		parent->delete_child(sub_index);
		reallocate_remove(parent, sub_index);
	    } else {
		parent->set_branch(sub_index, child);
	    }
	}
	size_t n = parent->num_children();
	if (n == 0) {
	    // This should be deleted
	    parent = nullptr;
	} else {
	    if (n == 1) {
		// Replace singleton X -> Y -> Z with X -> Z
		size_t other_sub_index = lsb(parent->mask_);
		if (parent->is_branch(other_sub_index)) {
		    auto *branch = parent->get_branch(other_sub_index);
		    if (branch->num_children() == 1) {
			size_t sub_sub_index = lsb(branch->mask_);
			if (branch->is_leaf(sub_sub_index)) {
			    auto *leaf = branch->get_leaf(sub_sub_index);
			    ::operator delete(branch);
			    parent->set_leaf(other_sub_index, leaf);
			}
		    }
		}
	    }
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
	size_t n_right = std::bitset<MAX_BRANCH>((parent->mask_ >> sub_index) >> 2).count();
	std::copy(parent->data_, parent->data_+n_left, &new_parent->data_[0]);
	if (parent->data_+n_left+1 < parent->data_+n_left+n_right) {
	    std::copy(parent->data_+n_left+1, parent->data_+n_left+n_right, &new_parent->data_[n_left]);
	}
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
	    delete get_leaf(sub_index);
	} else {
	    // Branch!
	    ::operator delete( get_branch(sub_index) );
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

    inline merkle_trie_leaf<T> * new_leaf(size_t _index, const T &_value) {
        return new merkle_trie_leaf<T>(_index, _value);
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

    word_t mask_;
    word_t leaf_;
    hash_t hash_;   // 32 bytes    
    void * data_[]; // Can be different things here.
};


template<typename T, size_t L> class merkle_trie_iterator {
private:
    typedef merkle_trie_branch<T,L> mtrie;
  
public:
    inline merkle_trie_iterator(mtrie *root) {
        if (root == nullptr) return;
        spine.push_back(cursor(root,0));
	leftmost();
    }

    inline merkle_trie_iterator & operator ++ () {
        next();
        return *this;
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

private:
    inline bool at_end() const {
        return spine.empty();
    }
    inline void leftmost() {
        if (spine.empty()) {
	    return;
        }
        auto node = spine.back().node;
	while (node->mask_ == 0) {
	    spine.pop_back();
	    node = spine.back().node;
	}
	auto index = spine.back().index;
	while (node->is_branch(index)) {
	    node = node->get_branch(index);
	    index = lsb(node->mask_);
  	    spine.push_back(cursor(node, index));
	}
	// At this point we've must found a leaf
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
  
    struct cursor {
        cursor(mtrie *_node, size_t _index) : node(_node), index(_index) { }
        mtrie *node;
        size_t index;

        inline bool operator == (const cursor &other) const {
	    return node == other.node && index == other.index;
        }
    };
    std::vector<cursor> spine;
};

template<typename T, size_t L> class merkle_trie {
public:
    typedef merkle_trie_hash_t hash_t;
  
    merkle_trie() {
        root = new merkle_trie_branch<T,L>();
	dirty = false;
	auto_rehash = true;
    }

    inline void insert(uint64_t _index, const T &_value) {
        root->insert_part(root, 0, auto_rehash, _index, _value);
	if (!auto_rehash) {
	    dirty = true;
	}
    }

    inline const T * find(uint64_t _index) const {
	auto *leaf = root->find_part(root, 0, _index);
	if (leaf == nullptr) {
	    return nullptr;
	}
	return &leaf->value();
    }

    inline void remove(uint64_t _index) {
	root->remove_part(root, 0, auto_rehash, _index);
	if (!auto_rehash) {
	    dirty = true;
	}
    }

    inline const hash_t & hash() const {
        assert(!dirty);
        return root->hash();
    }

    inline size_t num_bytes() const {
        return root->num_bytes();
    }

    inline merkle_trie_iterator<T,L> begin() {
        return merkle_trie_iterator<T,L>(root);
    }

    inline merkle_trie_iterator<T,L> end() {
        return merkle_trie_iterator<T,L>(nullptr);
    }

    inline void set_auto_rehash(bool b) {
        auto_rehash = b;
    }

    inline void rehash_all() {
        root->rehash_all();
	dirty = false;
    }
  
private:
    merkle_trie_branch<T,L> *root;
    bool dirty;
    bool auto_rehash;
};

}}

#endif

