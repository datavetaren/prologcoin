#include <boost/filesystem/operations.hpp>
#include "blockdb.hpp"
#include "../common/hex.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace db {

struct version_buffer {
    uint8_t buffer[blockdb_params::VERSION_SZ];
};
    
static void blockdb_version_check(fstream *f, const std::string &file_path) {
    version_buffer buffer;
    memset(&buffer, 'x', sizeof(version_buffer));
    f->seekg(0, fstream::beg);
    f->read(reinterpret_cast<char *>(&buffer), blockdb_params::VERSION_SZ);
    if (f->fail()) {
        try {
	    f->exceptions(f->failbit);
        } catch (const fstream::failure &err) {
  	    throw blockdb_version_exception("Unable to read version information from " + file_path + "; " + err.what());
        }
    }
    if (memcmp(&buffer, blockdb_params::VERSION, blockdb_params::VERSION_SZ) != 0) {
        buffer.buffer[15] = '\0';
	std::string msg = "Unrecognized version while reading '";
	msg += file_path;
	msg += "'";
	msg += reinterpret_cast<char *>(buffer.buffer);
	msg += "; expected: '";
	msg += reinterpret_cast<const char *>(&blockdb_params::VERSION[0]);
	msg += "'";
	throw blockdb_version_exception(msg);
    }
}
    
void blockdb_meta_entry::print(std::ostream &out) const
{
    out << "{block_index=" << block_index_ << ",height=" << height_
	<< ",offset=" << offset_ << ",size=" << size_ << "}";
}

//
// blockdb_bucket
//
    
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

blockdb_block * blockdb_bucket::new_block(const void *data, size_t sz, size_t index, size_t height)
{
    auto *f = get_bucket_data_stream();
    f->seekg(0, fstream::end);
    size_t offset = f->tellg();
    blockdb_meta_entry e(index, height, offset, sz);
    auto *b = new blockdb_block(e, data);
    append_block(b);
    append_meta_data(e);
    
    return b;
}
    
fstream * blockdb_bucket::get_bucket_meta_stream()
{
    if (fstream_meta_ == nullptr) {
        bool exists = boost::filesystem::exists(file_path_meta_);
	if (!exists) {
	    auto parent_dir = boost::filesystem::path(file_path_meta_).parent_path();
	    boost::filesystem::create_directories(parent_dir);
	    fstream fs;
	    fs.open(file_path_meta_, fstream::out | fstream::app | fstream::binary);
	    fs.write(blockdb_params::VERSION, blockdb_params::VERSION_SZ);
	    fs.close();
	}
	fstream_meta_ = new fstream;
	fstream_meta_->open(file_path_meta_, fstream::in | fstream::out | fstream::binary);
	
	fstream_meta_->seekg(0, fstream::end);
	initialized_ = true;
	db_.stream_cache_.insert(first_index_, this);
    } else {
        // Touch it so it gets first in cache
        static_cast<void>(db_.stream_cache_.find(first_index_));
    }
    return fstream_meta_;
}

  
fstream * blockdb_bucket::get_bucket_data_stream()
{
    if (fstream_data_ == nullptr) {
        bool exists = boost::filesystem::exists(file_path_data_);
	if (!exists) {
	    auto parent_dir = boost::filesystem::path(file_path_data_).parent_path();
	    boost::filesystem::create_directories(parent_dir);

	    fstream fs;
	    fs.open(file_path_data_, fstream::out | fstream::app | fstream::binary);
	    fs.write(blockdb_params::VERSION, blockdb_params::VERSION_SZ);
	    fs.close();
	}
	  
	fstream_data_ = new fstream(file_path_data_, fstream::in | fstream::out | fstream::binary);
    }
    return fstream_data_;
}

void blockdb_bucket::flush_streams()
{
    if (fstream_meta_) fstream_meta_->flush();
    if (fstream_data_) fstream_data_->flush();    
}
    
void blockdb_bucket::close_streams()
{
    if (fstream_meta_ != nullptr) {
        fstream_meta_->close();
	delete fstream_meta_;
	fstream_meta_ = nullptr;
    }
      
    if (fstream_data_ != nullptr) {
        fstream_data_->close();
	delete fstream_data_;
	fstream_data_ = nullptr;
    }
}

void blockdb_bucket::read_meta_data()
{
    fstream *f = get_bucket_meta_stream();

    f->seekg(0, fstream::end);
    size_t max_offset = f->tellg();

    blockdb_version_check(f, file_path_meta_);

    const size_t ENTRY_SIZE = blockdb_meta_entry::SERIALIZATION_SIZE;
    uint8_t buffer[ENTRY_SIZE];
    
    for (size_t offset = blockdb_params::VERSION_SZ;
	 offset < max_offset;
	 offset += ENTRY_SIZE) {
        f->read(reinterpret_cast<char *>(buffer), ENTRY_SIZE);
	if (!(*f)) {
	    f->clear();
	    break;
	}
	blockdb_meta_entry e;
	size_t sz = 0;
	e.read(&buffer[0], sz);
	assert(sz == blockdb_meta_entry::SERIALIZATION_SIZE);
	internal_add_meta_entry(e);
    }

    initialized_ = true;
}

void blockdb_bucket::append_meta_data(const blockdb_meta_entry &e)
{
    auto *f = get_bucket_meta_stream();
    uint8_t buffer[blockdb_meta_entry::SERIALIZATION_SIZE];
    size_t n = 0;
    e.write(buffer, n);
    f->seekg(0, fstream::end);
    if (f->fail()) {
        try {
	    f->exceptions(f->failbit);
        } catch (const fstream::failure &err) {
  	    throw blockdb_write_exception("Error while seeking end of file of " + file_path_meta_ + "; " + err.what());
      }
    }
    f->write(reinterpret_cast<char *>(buffer), n);
    if (f->fail()) {
        try {
	    f->exceptions(f->failbit);
        } catch (const fstream::failure &err) {
  	    throw blockdb_write_exception("Error while writing to " + file_path_meta_ + "; " + err.what());
      }
    }
    internal_add_meta_entry(e);
}

//
// blockdb
//

blockdb::blockdb(const blockdb_params &params, const std::string &dir_path)
    : blockdb_params(params), dir_path_(dir_path),
      block_flusher_(),
      block_cache_(blockdb_params::cache_num_blocks(), block_flusher_),
      stream_flusher_(),
      stream_cache_(blockdb_params::cache_num_streams(), stream_flusher_) {
}        
    
blockdb::blockdb(const std::string &dir_path)
    : blockdb(blockdb_params(), dir_path) { }

blockdb::~blockdb() {
}
    
// At most 128 files in a directory... (64 data + 64 meta)
boost::filesystem::path blockdb::bucket_dir_location(size_t bucket_index) const {
    size_t start_bucket = (bucket_index / 64) * 64;
    size_t end_bucket = (bucket_index / 64) * 64 + 63;
    std::stringstream ss;
    ss << "buckets_" << start_bucket << "_" << end_bucket;
    std::string name = ss.str();
    auto r = boost::filesystem::path(dir_path_);
    return r / name;
}
  
boost::filesystem::path blockdb::bucket_file_data_path(size_t bucket_index) const {
    auto dir = bucket_dir_location(bucket_index);
    std::string name = "bucket_" + boost::lexical_cast<std::string>(bucket_index) + ".data.bin";
    return dir / name;
}

boost::filesystem::path blockdb::bucket_file_meta_path(size_t bucket_index) const {
    auto dir = bucket_dir_location(bucket_index);
    std::string name = "bucket_" + boost::lexical_cast<std::string>(bucket_index) + ".meta.bin";
    return dir / name;
}

blockdb_bucket & blockdb::get_bucket(size_t bucket_index) {
    if (bucket_index >= buckets_.size()) {
        buckets_.resize(bucket_index+1);
    }
    auto *b = buckets_[bucket_index];
    if (b == nullptr) {
        b = new blockdb_bucket(*this,
			       *this,
			       bucket_file_meta_path(bucket_index).string(),
			       bucket_file_data_path(bucket_index).string(),
			       bucket_index_offset(bucket_index));
	buckets_[bucket_index] = b;
    }
    return *b;
}
    
blockdb_block * blockdb::find_block(size_t block_index, size_t at_height)
{
    blockdb_bucket &b = get_bucket(bucket_index(block_index));
    auto e = b.find_entry(block_index, at_height);
    if (e == nullptr) {
        return nullptr;
    }
    blockdb_meta_key_entry key(e->block_index(), e->height());
    auto r = block_cache_.find(key);
    if (r == nullptr) {
        // Read block and add it to cache
        auto *blk = b.read_block(*e);
	block_cache_.insert(key, blk);
        return blk;
    }
    return *r;
}

void blockdb::compute_block_hash(const void *data, size_t sz, hash_t &hash)
{
    common::sha1 hasher;
    hasher.update(data, sz);
    hasher.finalize(hash.hash);
}

blockdb_block * blockdb::new_block(const void *data, size_t sz, size_t block_index, size_t at_height) {
  
    blockdb_bucket &b = get_bucket(bucket_index(block_index));
    auto *blk = b.new_block(data, sz, block_index, at_height);
    blockdb_meta_key_entry key(block_index, at_height);
    block_cache_.insert(key, blk);
    return blk;
}

bool blockdb::is_empty() const
{
    auto path = boost::filesystem::path(bucket_file_meta_path(0));
    if (boost::filesystem::exists(path)) {
        return false;
    }
    return true;
}

void blockdb::erase_all()
{
    block_cache_.clear();
    buckets_.clear();
    boost::system::error_code ec;
    boost::filesystem::remove_all(dir_path_, ec);
    if (ec) {
      throw blockdb_write_exception( "Failed while attempting to erase all; " + ec.message());
    }
}

void blockdb::flush()
{
    stream_cache_.foreach( [](size_t, blockdb_bucket *b) {
	b->flush_streams();
    } );
}
    
}}