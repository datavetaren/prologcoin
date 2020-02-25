#ifndef _db_triedb_params_hpp
#define _db_triedb_params_hpp

#include <stddef.h>

namespace prologcoin { namespace db {

class triedb_params {
public:
    static const char VERSION[16];
    static const size_t VERSION_SZ = sizeof(VERSION);
  
    static const size_t MB = 1024*1024;
    static const size_t GB = 1024*MB;

    static const size_t DEFAULT_BUCKET_SIZE = 128*MB;
    static const size_t DEFAULT_CACHE_NUM_STREAMS = 16;

    inline triedb_params()
        : bucket_size_(DEFAULT_BUCKET_SIZE) { }
  
    inline size_t bucket_size() const { return bucket_size_; }
    inline void set_bucket_size(size_t sz) { bucket_size_ = sz; }

    inline size_t cache_num_streams() const { return cache_num_streams_; }
    inline void set_cache_num_streams(size_t n) { cache_num_streams_ = n; }
  
private:
    size_t bucket_size_;
    size_t cache_num_streams_;
};
    
}}

#endif
