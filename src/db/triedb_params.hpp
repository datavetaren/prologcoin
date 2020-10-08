#ifndef _db_triedb_params_hpp
#define _db_triedb_params_hpp

#include <stddef.h>

namespace prologcoin { namespace db {

class triedb_params {
public:
    static const char VERSION[16];
    static const size_t VERSION_SZ = sizeof(VERSION);

    static const size_t MAX_BRANCH_BITS = 5;
    static const size_t MAX_BRANCH = 1 << MAX_BRANCH_BITS;
    static const size_t MAX_KEY_SIZE_BITS = 60;
  
    static const size_t MB = 1024*1024;
    static const size_t GB = 1024*MB;

    static const size_t DEFAULT_BUCKET_SIZE = 128*MB;
    static const size_t DEFAULT_CACHE_NUM_STREAMS = 16;
    static const size_t DEFAULT_CACHE_NUM_NODES = 65536;

    inline triedb_params()
      : bucket_size_(DEFAULT_BUCKET_SIZE),
        cache_num_streams_(DEFAULT_CACHE_NUM_STREAMS),
        cache_num_nodes_(DEFAULT_CACHE_NUM_NODES),
        use_hashing_(true) { }
  
    inline size_t bucket_size() const { return bucket_size_; }
    inline void set_bucket_size(size_t sz) { bucket_size_ = sz; }

    inline size_t cache_num_streams() const { return cache_num_streams_; }
    inline void set_cache_num_streams(size_t n) { cache_num_streams_ = n; }

    inline size_t cache_num_nodes() const { return cache_num_nodes_; }
    inline void set_cache_num_nodes(size_t n) { cache_num_nodes_ = n; }

    inline bool use_hashing() const { return use_hashing_; }
    inline void set_use_hashing(bool h) { use_hashing_ = h; }
  
private:
    size_t bucket_size_;
    size_t cache_num_streams_;
    size_t cache_num_nodes_;
    bool use_hashing_;
};
    
}}

#endif
