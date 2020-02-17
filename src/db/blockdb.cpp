#include <boost/filesystem/operations.hpp>
#include "blockdb.hpp"

namespace prologcoin { namespace db {

blockdb::blockdb(const std::string &dir_path) : dir_path_(dir_path), block_flusher_(*this), block_cache_(DEFAULT_CACHE_NUM_BLOCKS, block_flusher_) {
}

blockdb_bucket & blockdb::get_bucket(size_t bucket_index) {
    if (bucket_index < buckets_.size() && !buckets_[bucket_index].empty()) {
        return buckets_[bucket_index];
    }
    return buckets_[bucket_index];
}
    
block * blockdb::find_block(size_t index, size_t from_height)
{
    blockdb_bucket &b = get_bucket(bucket_index(index));
    auto e = b.find_entry(index, from_height);
    if (!e.is_initialized()) {
        return nullptr;
    }
    return nullptr;
}
    
}}
