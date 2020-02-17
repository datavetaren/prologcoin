#include <boost/filesystem/operations.hpp>
#include "blockdb.hpp"

namespace prologcoin { namespace db {

blockdb::blockdb(const std::string &dir_path) : dir_path_(dir_path), block_flusher_(*this), block_cache_(DEFAULT_CACHE_NUM_BLOCKS, block_flusher_) {
}

// At most 128 files in a directory... (64 data + 64 meta)
boost::filesystem::path blockdb::bucket_dir_location(size_t bucket_index) const {
    size_t start_bucket = (bucket_index / 64) * 64;
    size_t end_bucket = (bucket_index / 64) * 64 + 63;
    std::stringstream ss;
    ss << "buckets_" << start_bucket << "_" << end_bucket << std::endl;
    std::string name = ss.str();
    std::string r = dir_path_;
    r.append(name);
    return r;
}
  
boost::filesystem::path blockdb::bucket_file_data_path(size_t bucket_index) const {
    auto dir = bucket_dir_location(bucket_index);
    std::string name = "bucket_" + boost::lexical_cast<std::string>(bucket_index) + ".data.bin";
    dir.append(name);
    return dir;
}

boost::filesystem::path blockdb::bucket_file_meta_path(size_t bucket_index) const {
    auto dir = bucket_dir_location(bucket_index);
    std::string name = "bucket_" + boost::lexical_cast<std::string>(bucket_index) + ".meta.bin";
    dir.append(name);
    return dir;
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
