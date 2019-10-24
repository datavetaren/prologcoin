#pragma once

#ifndef _common_merkle_trie_hpp
#define _common_merkle_trie_hpp

namespace prologcoin { namespace common {

template<typename T, size_t L, size_t H = 4096> class merkle_trie {
public:
    inline merkle_trie() : mask(0), count(0), data(nullptr) { }

    inline void insert(size_t index, T value) {
        size_t child_index = (index >> (L-6)) & 0x3f;
	if ((mask & (1 << child_index)) == 0) {
	    allocate_sub_trie(child_index);
	}
    }

private:
    typedef merkle_trie<T, L-6, H> sub_merkle_trie;
    void allocate_sub_node(size_t child_index) {
        size_t new_size = std::bitset<64>(mask).count()+1;
        new sub_merkle_trie *new_array = static_cast<sub_merkle_trie *>(::operator new(sizeof(sub_merkle_trie)*new_size));
        size_t child_i = 0;
        for (size_t i = 0; i < 64; i++) {
	    if (child_index == child_i) {
	        get_sub_node[child_i] = new sub_merkle_trie();
		mask |= (1 << i);
	    } else if (mask & (1 << i)) {
	        get_sub_node[child_i]
	        child_i++;
	    }
        }
    }
  
    sub_merkle_trie *& get_sub_node(size_t child_index) {
        size_t offset = 0;
        if (count >= H) {
	    offset = 32;
        }
	return reinterpret_cast<sub_merkle_trie *&>(&data[offset]);
	
    }

    uint64_t mask;
    uint64_t count;
    char     *data; // Can be different things here.
                    // if count >= H then SHA256 is stored first (32 bytes) followed by
                    // trie children nodes.
                    // otherwise it's just the trie children nodes.
};
    
}}

#endif

