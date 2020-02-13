#include "meta_data.hpp"

namespace prologcoin { namespace statedb {

class block {
public:
    inline const void * data() const {
        return data_;
    }
    inline void * data() {
        return data_;
    }
    inline size_t size() const {
        return entry_.size();
    }
  
private:
    friend class statedb;
  
    inline block(size_t index, size_t height, size_t offset,
		 void *data, size_t size)
      : entry_(index, height, offset, size), data_(data) { }

    meta_entry entry_; // Meta-data of this block
    void *data_;
};

//
// This is a framework for storing blocks with versioning.
//    
class statedb {
public:
    statedb(const std::string &dir_path);

    block * find_block(size_t index, size_t from_height);
    block * new_block(size_t index, size_t from_height, void *data, size_t sz);

private:
    std::string dir_path_;
};
    
}}  
