#include <fstream>
#include <cstdint>
#include "../common/lru_cache.hpp"
#include "meta_data.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>

namespace prologcoin { namespace statedb {

class statedb_exception : public std::runtime_error {
public:
    statedb_exception(const std::string &msg) : std::runtime_error(msg) { }
};

class statedb_version_exception : public statedb_exception {
public:
    statedb_version_exception(const std::string &msg) : statedb_exception(msg) { }
};

class statedb_meta_data_exception : public statedb_exception {
public:
    statedb_meta_data_exception(const std::string &msg) : statedb_exception(msg) { }  
};

class block : public boost::noncopyable {
public:
    inline const void * data() const {
        return data_;
    }
    inline void * data() {
        return data_;
    }
    inline size_t size() const {
        return entry_.size();
    }
  
private:
    friend class statedb;
  
    inline block(size_t index, size_t height, size_t offset,
		 void *data, size_t size)
      : entry_(index, height, offset, size), data_(data), data_owner_(false) { }

    inline block(size_t index, size_t height, size_t offset, size_t size)
      : entry_(index, height, offset, size), data_(new uint8_t[size]), data_owner_(true) { }

    inline ~block() {
        if (data_owner_) delete data_;
    }

    meta_entry entry_; // Meta-data of this block
    void *data_;
    bool data_owner_;
};

typedef std::basic_fstream<uint8_t> fstream;

//
// This is a framework for storing blocks with versioning.
//    
class statedb {
public:
    static const size_t MB = 1024*1024;
    static const size_t GB = 1024*MB;
  
    static const size_t DEFAULT_BUCKET_SIZE = 2048;
    static const size_t DEFAULT_BLOCK_SIZE = 65536;
    static const size_t MAX_BLOCK_SIZE = 65536;
    static const size_t CACHE_NUM_STREAMS = 16;
    static const size_t DEFAULT_CACHE_NUM_BLOCKS = 4*GB / DEFAULT_BLOCK_SIZE;

    statedb(const std::string &dir_path);

    block * find_block(size_t index, size_t from_height);
    block * new_block(size_t index, size_t from_height, size_t sz);

private:
    inline size_t bucket_index(size_t index) const {
        return index / bucket_size_;
    }

    // At most 128 files in a directory...
    inline boost::filesystem::path bucket_dir_location(size_t bucket_index) const {
        size_t start_bucket = (bucket_index / 128) * 128;
        size_t end_bucket = (bucket_index / 128) * 128 + 127;
	std::stringstream ss;
	ss << "buckets_" << start_bucket << "_" << end_bucket << std::endl;
	std::string name = ss.str();
	std::string r = dir_path_;
	r.append(name);
	return r;
    }
  
    inline boost::filesystem::path bucket_file_path(size_t bucket_index) const {
        auto dir = bucket_dir_location(bucket_index);
	std::string name = "bucket_" + boost::lexical_cast<std::string>(bucket_index) + ".bin";
	dir.append(name);
	return dir;
    }

    bucket & get_bucket(size_t bucket_index);
    fstream * get_bucket_stream(size_t bucket_index);
    void save_bucket(size_t bucket_index);

    void read_meta_data(fstream *f, bucket &b);
    void write_meta_data(fstream *f, const bucket &b);
  
    std::vector<bucket> buckets_;
  
    std::string dir_path_;
    size_t block_size_;
    size_t bucket_size_;

    struct stream_closer {
        void evicted(size_t, fstream *f) {
	    f->close();
	    delete f;
        }
    };

    struct block_flusher {
        block_flusher(statedb &db) : db_(db) { }
        // We assume the block hasn't changed! Note that a new block is always
        // added instead of changing an existing one
        void evicted(const meta_key_entry &, block *b) {
	    delete b;
        }
    private:
        statedb &db_;
    };

    // Map bucket index to stream
    typedef common::lru_cache<size_t, fstream *, stream_closer> stream_cache_t;
    stream_closer stream_closer_;
    stream_cache_t stream_cache_;

    typedef common::lru_cache<meta_key_entry, block *, block_flusher> block_cache;
    block_flusher block_flusher_;
    block_cache block_cache_;
};
    
}}  
