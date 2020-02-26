#include <bitset>
#include "triedb.hpp"
#include "../common/checked_cast.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace prologcoin { namespace db {

//
// triedb_branch
//

size_t triedb_branch::serialization_size() const {
    size_t sz = sizeof(uint32_t) +  // total size
                sizeof(uint32_t) +  // mask
                sizeof(uint32_t) +  // leaf
                sizeof(uint64_t)*num_children() + // ptrs
                custom_data_size_;
    assert(sz < MAX_SIZE_IN_BYTES);
    return sz;
}
    
void triedb_branch::read(const uint8_t *buffer)
{
    const uint8_t *p = buffer;
    size_t sz = read_uint32(p); assert(sz >= 4 && sz < MAX_SIZE_IN_BYTES);
    p += sizeof(uint32_t);
    assert(((p - buffer) + sizeof(uint32_t)) < sz);
    mask_ = read_uint32(p); p += sizeof(uint32_t);
    assert(((p - buffer) + sizeof(uint32_t)) < sz);
    leaf_ = read_uint32(p); p += sizeof(uint32_t);
    size_t n = num_children();
    if (ptr_ != nullptr) delete [] ptr_;
    ptr_ = new uint64_t[n];
    for (size_t i = 0; i < n; i++) {
        assert(((p - buffer) + sizeof(uint64_t)) < sz);
        ptr_[i] = read_uint64(p); p += sizeof(uint64_t);
    }
    assert(((p - buffer) + sizeof(uint32_t)) < sz);
    custom_data_size_ = sz - (p - buffer);
    if (custom_data_ != nullptr) delete [] custom_data_;
    custom_data_ = nullptr;
    if (custom_data_size_ > 0) {
        custom_data_ = new uint8_t[custom_data_size_];
        memcpy(custom_data_, p, custom_data_size_);
    }
}

void triedb_branch::write(uint8_t *buffer) const
{
    uint8_t *p = buffer;
    size_t sz = serialization_size();
    write_uint32(p, common::checked_cast<uint32_t>(sz)); p += sizeof(uint32_t);
    write_uint32(p, mask_); p += sizeof(uint32_t);
    write_uint32(p, leaf_); p += sizeof(uint32_t);
    size_t n = num_children();
    for (size_t i = 0; i < n; i++) {
        write_uint64(p, ptr_[i]); p += sizeof(uint64_t);
    }
    if (custom_data_ != nullptr) {
        memcpy(p, custom_data_, custom_data_size_);
    }
}

//
// triedb
//
    
triedb::triedb(const std::string &dir_path) : triedb(triedb_params(), dir_path) {
}
    
triedb::triedb(const triedb_params &params, const std::string &dir_path)
  : triedb_params(params),
    dir_path_(dir_path),
    stream_flusher_(),
    stream_cache_(triedb_params::cache_num_streams(), stream_flusher_),
    leaf_flusher_(),
    leaf_cache_(triedb_params::cache_num_nodes(), leaf_flusher_),
    branch_flusher_(),
    branch_cache_(triedb_params::cache_num_nodes(), branch_flusher_) {
    last_offset_ = scan_last_offset();
}

triedb_branch * triedb::insert_part(triedb_branch *node, uint64_t key, const uint8_t *data, size_t data_size, size_t part)
{
    size_t sub_index = (key >> (MAX_KEY_SIZE_BITS - MAX_BRANCH_BITS - part)) & (MAX_BRANCH-1);
    auto *new_branch = new triedb_branch(*node);
    
    if (node->is_empty(sub_index)) {
        auto *new_leaf = new triedb_leaf(key, data, data_size);
	auto leaf_ptr = append_leaf_node(*new_leaf);
	leaf_cache_.insert(leaf_ptr, new_leaf);
	new_branch->set_child_pointer(sub_index, leaf_ptr);
	// TODO: update(new_branch);	
	return new_branch;
    } else if (node->is_leaf(sub_index)) {
        auto *leaf = get_leaf(node, node->get_child_pointer(sub_index));
	if (leaf->key() == key) {
	    auto *new_leaf = new triedb_leaf(key, data, data_size);
	    auto leaf_ptr = append_leaf_node(*new_leaf);
	    leaf_cache_.insert(leaf_ptr, new_leaf);
	    new_branch->set_child_pointer(sub_index, leaf_ptr);
	    // TODO: update(new_branch);
	    return new_branch;
	}
    }

    /*
    // We need to create a branch node at sub_index
	    new_branch->data_[0] = leaf;
	    size_t sub_sub_index = (leaf->key() >> (L - 2*MAX_BRANCH_BITS - _at_part)) & (MAX_BRANCH-1);
	    new_branch->mask_ = static_cast<word_t>(1) << sub_sub_index;
	    new_branch->leaf_ = static_cast<word_t>(1) << sub_sub_index;
	    auto &leaf1 = insert_part(new_branch, _at_part + MAX_BRANCH_BITS, rehash, _key, updater);
	    if (rehash) new_branch->recompute_hash();
	    parent->set_branch(sub_index, new_branch);
	    if (rehash) parent->recompute_hash();
	    return leaf1;
	}
	auto *child = parent->get_branch(sub_index);
	auto &leaf = insert_part(child, _at_part + MAX_BRANCH_BITS, rehash, _key, updater);
	parent->set_branch(sub_index, child);
	if (rehash) parent->recompute_hash();
	return leaf;
    */

    return nullptr;
}
    
boost::filesystem::path triedb::roots_file_path() const {
    auto file_path = boost::filesystem::path(dir_path_) / "roots.bin";
    return file_path;
}

void triedb::read_roots() {
    auto file_path = roots_file_path();
    bool exists = boost::filesystem::exists(file_path);
    if (!exists) {
        auto parent_dir = file_path.parent_path();
	boost::filesystem::create_directories(parent_dir);
	fstream fs;
	fs.open(file_path.string(), fstream::out | fstream::app | fstream::binary);
	fs.write(triedb_params::VERSION, triedb_params::VERSION_SZ);
	fs.close();
    }
    fstream fs;
    fs.open(file_path.string(), fstream::in | fstream::out | fstream::binary);
    fs.seekg(0, fstream::end);
    size_t file_size = fs.tellg();
    size_t num_roots = file_size / sizeof(uint64_t);
    roots_.resize(num_roots);
    fs.seekg(0, fstream::beg);
    uint8_t buffer[sizeof(uint64_t)];
    for (size_t i = 0; i < num_roots; i++) {
        fs.read(reinterpret_cast<char *>(&buffer[0]), sizeof(uint64_t));
        roots_[i] = read_uint64(buffer);
    }

    num_heights_ = num_roots;
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

fstream * triedb::set_file_offset(uint64_t offset)
{
    size_t bucket_index = offset / bucket_size();
    auto *f = get_bucket_stream(bucket_index);
    uint8_t buffer[triedb_branch::MAX_SIZE_IN_BYTES];
    size_t first_offset = bucket_index * bucket_size();
    size_t file_offset = offset - first_offset + VERSION_SZ;
    f->seekg(file_offset, fstream::beg);
    return f;
}
    
void triedb::read_leaf_node(uint64_t offset, triedb_leaf &node)
{
    uint8_t buffer[triedb_leaf::MAX_SIZE_IN_BYTES];  
    auto *f = set_file_offset(offset);
    f->read(reinterpret_cast<char *>(&buffer[0]), sizeof(uint32_t));
    uint32_t size = read_uint32(buffer); assert(size >= 4);
    f->read(reinterpret_cast<char *>(&buffer[sizeof(uint32_t)]),
	    size-sizeof(uint32_t));
    node.read(buffer);
}

uint64_t triedb::append_leaf_node(const triedb_leaf &node)
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
    }
    uint8_t buffer[triedb_leaf::MAX_SIZE_IN_BYTES];
    size_t n = node.serialization_size();
    node.write(buffer);
    auto *f = set_file_offset(offset);
    f->write(reinterpret_cast<char *>(&buffer[0]), n);
    return offset;
}
    
void triedb::read_branch_node(uint64_t offset, triedb_branch &node)
{
    uint8_t buffer[triedb_branch::MAX_SIZE_IN_BYTES];
    auto *f = set_file_offset(offset);  
    f->read(reinterpret_cast<char *>(&buffer[0]), sizeof(uint32_t));
    uint32_t size = read_uint32(buffer); assert(size >= 4);
    f->read(reinterpret_cast<char *>(&buffer[sizeof(uint32_t)]),
	    size-sizeof(uint32_t));
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
    }
    uint8_t buffer[triedb_branch::MAX_SIZE_IN_BYTES];
    size_t n = node.serialization_size();
    node.write(buffer);
    auto *f = set_file_offset(offset);
    f->write(reinterpret_cast<char *>(&buffer[0]), n);
    return offset;
}

}}
