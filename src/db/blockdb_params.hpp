#ifndef _db_blockdb_params_hpp
#define _db_blockdb_params_hpp


namespace prologcoin { namespace db {

class blockdb_params {
public:
    static const size_t MB = 1024*1024;
    static const size_t GB = 1024*MB;

    static const size_t DEFAULT_META_BLOCK_SIZE = 65536;
    static const size_t DEFAULT_BUCKET_SIZE = 2048;
    static const size_t DEFAULT_BLOCK_SIZE = 65536;
    static const size_t MAX_BLOCK_SIZE = 65536;
    static const size_t MAX_META_BLOCK_SIZE = 65536;
    static const size_t DEFAULT_CACHE_NUM_BLOCKS = 4*GB / DEFAULT_BLOCK_SIZE;

    inline blockdb_params() : meta_block_size_(DEFAULT_META_BLOCK_SIZE), block_size_(DEFAULT_BLOCK_SIZE), bucket_size_(DEFAULT_BUCKET_SIZE) { }

    inline size_t meta_block_size() const { return meta_block_size_; }
    inline void set_meta_block_size(size_t sz) { meta_block_size_ = sz; }
  
    inline size_t block_size() const { return block_size_; }
    inline void set_block_size(size_t sz) { block_size_ = sz; }

    inline size_t bucket_size() const { return bucket_size_; }
    inline void set_bucket_size(size_t sz) { bucket_size_ = sz; }

    inline size_t bucket_index(size_t index) const {
        return index / bucket_size_;
    }
  
private:
    size_t meta_block_size_;
    size_t block_size_;
    size_t bucket_size_;
};
    
}}

#endif
