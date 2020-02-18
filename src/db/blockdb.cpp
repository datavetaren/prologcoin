#include <boost/filesystem/operations.hpp>
#include "blockdb.hpp"

namespace prologcoin { namespace db {

void blockdb_meta_entry::print(std::ostream &out) const
{
    out << "{index=" << index_ << ",height=" << height_ << ",offset=" << offset_ << ",size=" << size_ << "}";
}

const uint8_t blockdb_bucket::VERSION[16] = "blockdb_1.0    ";
    
void blockdb_bucket::print(std::ostream &out) const
{
    size_t num_entries = 0;
    for (auto &v : entries_) {
        num_entries += v.size();
    }
    out << "bucket{num_entries=" << num_entries << ",[";
    bool first = true;
    for (auto &v : entries_) {
        for (auto &e : v) {
	    if (!first) out << ", ";
	    e.print(out);
	    first = false;
        }
    }
    out << "]}";
}

void blockdb_bucket::read_meta_data()
{
    fstream *f = get_bucket_meta_stream();

    f->seekg(0, fstream::end);
    size_t max_offset = f->tellg();
    
    f->seekg(0);
    uint8_t buffer[1024];
    
    f->read(buffer, 16);
    if (memcmp(buffer, VERSION, VERSION_SZ) != 0) {
        buffer[15] = '\0';
	std::string msg = "Unrecognized version: ";
	msg += reinterpret_cast<char *>(&buffer[0]);
	msg += "; expected: '";
	msg += reinterpret_cast<const char *>(&VERSION[0]);
	msg += "'";
	throw blockdb_version_exception(msg);
    }

    size_t ent_size = blockdb_meta_entry::SERIALIZATION_SIZE;
    
    for (size_t offset = 0; offset < max_offset; offset += ent_size) {
        f->read(buffer, ent_size);

	if (!(*f)) {
	    break;
	}
	blockdb_meta_entry e;
	size_t sz = 0;
	e.read(&buffer[0], sz);
	assert(sz == blockdb_meta_entry::SERIALIZATION_SIZE);
	internal_add_meta_entry(e);
    }
}

void blockdb_bucket::append_meta_data(const blockdb_meta_entry &e)
{
    if (fstream_meta_ == nullptr) {
        bool exists = boost::filesystem::exists(file_path_meta_);
	fstream_meta_ = new fstream(file_path_meta_,
			    fstream::out | fstream::binary | fstream::ate);
	if (!exists) {
	    fstream_meta_->write(VERSION, VERSION_SZ);
	}
    }
    uint8_t buffer[blockdb_meta_entry::SERIALIZATION_SIZE];
    size_t n = 0;
    e.write(buffer, n);
    fstream_meta_->write(buffer, n);

    if (fstream_meta_->fail()) {
      throw blockdb_write_exception("Error while writing to " + file_path_meta_);
    }
}

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
    if (bucket_index >= buckets_.size()) {
        buckets_.resize(bucket_index+1);
    }
    auto *b = buckets_[bucket_index];
    if (b == nullptr) {
      b = new blockdb_bucket(*this,
			     bucket_file_meta_path(bucket_index).string(),
			     bucket_file_data_path(bucket_index).string(),
			     bucket_offset(bucket_index));
	buckets_[bucket_index] = b;
    }
    return *b;
}
    
blockdb_block * blockdb::find_block(size_t index, size_t from_height)
{
    blockdb_bucket &b = get_bucket(bucket_index(index));
    auto e = b.find_entry(index, from_height);
    if (!e.is_initialized()) {
        return nullptr;
    }
    blockdb_meta_key_entry key(e->index(), e->height());
    auto r = block_cache_.find(key);
    if (!r.is_initialized()) {
        // Read block and add it to cache
        auto *blk = b.read_block(*e);
	block_cache_.insert(key, blk);
        return blk;
    }
    return *r;
}

blockdb_block * blockdb::new_block(void *data, size_t sz, size_t index, size_t height) {  
    blockdb_bucket &b = get_bucket(bucket_index(index));
    auto *f = b.get_bucket_data_stream();
    f->seekg(0, fstream::end);
    size_t offset = f->tellg();
    blockdb_meta_entry e(index, height, offset, sz);
    auto *blk = new blockdb_block(e, data);
    b.append_block(blk);
    blockdb_meta_key_entry key(index, height);
    block_cache_.insert(key, blk);
    return blk;
}
    
}}
