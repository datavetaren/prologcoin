#pragma once

#ifndef _db_blockdb_hpp
#define _db_blockdb_hpp

#include <cstdint>
#include "../common/lru_cache.hpp"
#include "blockdb_meta_data.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>

namespace prologcoin { namespace db {

class blockdb_exception : public std::runtime_error {
public:
    blockdb_exception(const std::string &msg) : std::runtime_error(msg) { }
};

class blockdb_version_exception : public blockdb_exception {
public:
    blockdb_version_exception(const std::string &msg) : blockdb_exception(msg) { }
};

class blockdb_meta_data_exception : public blockdb_exception {
public:
    blockdb_meta_data_exception(const std::string &msg) : blockdb_exception(msg) { }  
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
    friend class blockdb;
  
    inline block(size_t index, size_t height, size_t offset,
		 void *data, size_t size)
      : entry_(index, height, offset, size), data_(data), data_owner_(false) { }

    inline block(size_t index, size_t height, size_t offset, size_t size)
      : entry_(index, height, offset, size), data_(new uint8_t[size]), data_owner_(true) { }

    inline ~block() {
        if (data_owner_) delete data_;
    }

    blockdb_meta_entry entry_; // Meta-data of this block
    void *data_;
    bool data_owner_;
};

//
// This is a framework for storing blocks with versioning.
//    
class blockdb : public blockdb_params {
public:
    blockdb(const std::string &dir_path);

    block * find_block(size_t index, size_t from_height);
    block * new_block(size_t index, size_t from_height, size_t sz);

private:

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

    blockdb_bucket & get_bucket(size_t bucket_index);
    void save_bucket(size_t bucket_index);
  
    std::string dir_path_;

    // Each bucket can handle approximately 134 MB of address space.
    // No need to optimize this. 10000 buckets would mean 1340 GB, and
    // 10000 buckets in memory means ~64k x 10000 = 640 MB. We can optimize
    // this later if needed; it's not a consensus rule.
    std::vector<blockdb_bucket> buckets_;

    struct block_flusher {
        block_flusher(blockdb &db) : db_(db) { }
        // We assume the block hasn't changed! Note that a new block is always
        // added instead of changing an existing one
        void evicted(const blockdb_meta_key_entry &, block *b) {
	    delete b;
        }
    private:
        blockdb &db_;
    };

    typedef common::lru_cache<blockdb_meta_key_entry, block *, block_flusher> block_cache;
    block_flusher block_flusher_;
    block_cache block_cache_;
};
    
}}  

#endif
