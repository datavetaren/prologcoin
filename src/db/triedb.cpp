#include <bitset>
#include "triedb.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace prologcoin { namespace db {

//
// triedb_branch
//

size_t triedb_branch::serialization_size() const {
    size_t n = num_children();
    return 2*sizeof(uint32_t) + sizeof(hash_) + sizeof(uint64_t)*n;
}
    
void triedb_branch::read(const uint8_t *buffer)
{
    const uint8_t *p = buffer;
    mask_ = read_uint32(p); p += sizeof(uint32_t);
    leaf_ = read_uint32(p); p += sizeof(uint32_t);
    memcpy(&hash_.hash[0], p, sizeof(hash_)); p += sizeof(hash_);
    size_t n = num_children();
    if (ptr_ != nullptr) delete [] ptr_;
    ptr_ = new uint64_t[n];
    for (size_t i = 0; i < n; i++) {
        ptr_[i] = read_uint64(p); p += sizeof(uint64_t);
    }
}

void triedb_branch::write(uint8_t *buffer) const
{
    uint8_t *p = buffer;
    write_uint32(p, mask_); p += sizeof(uint32_t);
    write_uint32(p, leaf_); p += sizeof(uint32_t);
    memcpy(p, &hash_.hash[0], sizeof(hash_)); p += sizeof(hash_);
    size_t n = num_children();
    for (size_t i = 0; i < n; i++) {
        write_uint64(p, ptr_[i]); p += sizeof(uint64_t);
    }
}

//
// triedb
//
    
triedb::triedb(const std::string &dir_path) : triedb(triedb_params(), dir_path) {
}
    
triedb::triedb(const triedb_params &params, const std::string &dir_path) : triedb_params(params), dir_path_(dir_path), stream_flusher_(), stream_cache_(triedb_params::cache_num_streams(), stream_flusher_) {
    last_offset_ = scan_last_offset();
}

std::string triedb::roots_file_path() const {
    auto file_path = boost::filesystem::path(dir_path_) / "roots.bin";
    return file_path.string();
}

boost::filesystem::path triedb::bucket_dir_location(size_t bucket_index) const {
    size_t start_bucket = (bucket_index / 64) * 64;
    size_t end_bucket = (bucket_index / 64) * 64 + 63;
    std::stringstream ss;
    ss << "buckets_" << start_bucket << "_" << end_bucket;
    std::string name = ss.str();
    auto r = boost::filesystem::path(dir_path_);
    return r / name;
}

size_t triedb::scan_last_bucket() {
    size_t i;
    // First find right bucket directory
    for (i = 0;;i += 64) {
        auto dir = bucket_dir_location(i);
	if (!boost::filesystem::exists(dir)) {
	    break;
	}
    }
    if (i > 0) {
        i -= 64; // Go to previous existing bucket directory
    }

    bool something_found = false;
    for (;;i++) {
        auto file_path = bucket_file_path(i);
	if (!boost::filesystem::exists(file_path)) {
	    return i;
	}
	something_found = true;
    }
    if (something_found) {
        i--; // Go to previous existing bucket
    }
    return i;
}

uint64_t triedb::scan_last_offset() {
    size_t bucket_index = scan_last_bucket();
    auto *f = get_bucket_stream(bucket_index);
    f->seekg(0, fstream::end);
    uint64_t last_offset = f->tellg();
    last_offset -= VERSION_SZ;
    last_offset += bucket_index * bucket_size();
    return last_offset;
}
    
boost::filesystem::path triedb::bucket_file_path(size_t bucket_index) const {
    auto dir = bucket_dir_location(bucket_index);
    std::string name = "bucket_" + boost::lexical_cast<std::string>(bucket_index) + ".data.bin";
    return dir / name;
}

fstream * triedb::get_bucket_stream(size_t bucket_index) {
    auto *f = stream_cache_.find(bucket_index);
    if (f != nullptr) {
        return *f;
    }
    auto file_path = bucket_file_path(bucket_index);
    bool exists = boost::filesystem::exists(file_path);
    if (!exists) {
        auto parent_dir = file_path.parent_path();
	boost::filesystem::create_directories(parent_dir);
	fstream fs;
	fs.open(file_path.string(), fstream::out | fstream::app | fstream::binary);
	fs.write(triedb_params::VERSION, triedb_params::VERSION_SZ);
	fs.close();
    }
    auto *ff = new fstream;
    ff->open(file_path.string(), fstream::in | fstream::out | fstream::binary);
	
    ff->seekg(0, fstream::end);
    stream_cache_.insert(bucket_index, ff);
    return ff;
}

void triedb::read_branch_node(uint64_t offset, triedb_branch &node)
{
    size_t bucket_index = offset / bucket_size();
    auto *f = get_bucket_stream(bucket_index);
    uint8_t buffer[triedb_branch::MAX_SIZE_IN_BYTES];
    size_t first_offset = bucket_index * bucket_size();
    size_t file_offset = offset - first_offset + VERSION_SZ;
    f->seekg(file_offset, fstream::beg);
    f->read(reinterpret_cast<char *>(&buffer[0]), sizeof(uint32_t));
    uint32_t mask = read_uint32(buffer);
    size_t num_children = std::bitset<32>(mask).count();
    f->read(reinterpret_cast<char *>(&buffer[sizeof(uint32_t)]),
	    sizeof(uint32_t)+ // leaf
	    +sizeof(hash_t)+  // hash
	    +sizeof(uint64_t)*num_children); // ptr
    node.read(buffer);
}

uint64_t triedb::append_branch_node(const triedb_branch &node)
{
    auto offset = last_offset_;
    size_t num_bytes = node.serialization_size();
    size_t bucket_index = offset / bucket_size();
    auto first_offset = bucket_index * bucket_size();
    // Are we crossing the bucket boundary? 
    if (offset - first_offset + num_bytes >= bucket_size()) {
        // Then switch to the next bucket
        bucket_index++;
        offset = bucket_index*bucket_size();
	first_offset = bucket_index * bucket_size();
    }
    auto *f = get_bucket_stream(bucket_index);
    uint8_t buffer[triedb_branch::MAX_SIZE_IN_BYTES];
    size_t n = node.serialization_size();
    node.write(buffer);
    size_t file_offset = offset - first_offset + VERSION_SZ;
    f->seekg(file_offset, fstream::beg);
    f->write(reinterpret_cast<char *>(&buffer[0]), n);
    return offset;
}

}}
