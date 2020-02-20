#ifndef _db_blockdb_params_hpp
#define _db_blockdb_params_hpp


namespace prologcoin { namespace db {

class blockdb_params {
public:
    static const size_t MB = 1024*1024;
    static const size_t GB = 1024*MB;

    static const size_t DEFAULT_BUCKET_SIZE = 2048;
    static const size_t DEFAULT_BLOCK_SIZE = 65536;
    static const size_t DEFAULT_HASH_BUCKET_SIZE = 1 << 20;
    static const size_t MAX_BLOCK_SIZE = 65536;
    static const size_t DEFAULT_CACHE_NUM_BLOCKS = 4*GB / DEFAULT_BLOCK_SIZE;
    static const size_t DEFAULT_CACHE_NUM_STREAMS = 64;

    inline blockdb_params() : block_size_(DEFAULT_BLOCK_SIZE), bucket_size_(DEFAULT_BUCKET_SIZE), hash_bucket_size_(DEFAULT_HASH_BUCKET_SIZE), cache_num_blocks_(DEFAULT_CACHE_NUM_BLOCKS), cache_num_streams_(DEFAULT_CACHE_NUM_STREAMS) { }
  
    inline size_t block_size() const { return block_size_; }
    inline void set_block_size(size_t sz) { block_size_ = sz; }

    inline size_t bucket_size() const { return bucket_size_; }
    inline void set_bucket_size(size_t sz) { bucket_size_ = sz; }

    inline size_t hash_bucket_size() const { return hash_bucket_size_; }
    inline void set_hash_bucket_size(size_t sz) { hash_bucket_size_ = sz; }

    inline size_t bucket_index(size_t index) const {
        return index / bucket_size_;
    }
    inline size_t bucket_index_offset(size_t _bucket_index) const {
        return _bucket_index * bucket_size();
    }
    inline size_t cache_num_blocks() const {
        return cache_num_blocks_;
    }
    inline void set_cache_num_blocks(size_t n) {
        cache_num_blocks_ = n;
    }
    inline size_t cache_num_streams() const {
        return cache_num_streams_;
    }  
    inline void set_cache_num_streams(size_t n) {
        cache_num_streams_ = n;
    }  
private:
    size_t block_size_;
    size_t bucket_size_;
    size_t hash_bucket_size_;
    size_t cache_num_blocks_;
    size_t cache_num_streams_;  
};
    
}}

#endif
