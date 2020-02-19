#pragma once

#ifndef _db_blockdb_hpp
#define _db_blockdb_hpp

#include <cstdint>
#include <fstream>
#include "../common/lru_cache.hpp"
#include "../common/checked_cast.hpp"
#include "../common/sha1.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/functional/hash.hpp>
#include <iostream>
#include "util.hpp"
#include "blockdb_params.hpp"

namespace prologcoin { namespace db {

using fstream = std::basic_fstream<uint8_t>;

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

class blockdb_write_exception : public blockdb_exception {
public:
    blockdb_write_exception(const std::string &msg) : blockdb_exception(msg) { }
};

class blockdb;
class blockdb_bucket;

class blockdb_meta_key_entry {
public:
    blockdb_meta_key_entry() = default;
    blockdb_meta_key_entry(const blockdb_meta_key_entry &other) = default;
    inline blockdb_meta_key_entry(size_t index, size_t height) : index_(index), height_(height) { }

    inline size_t index() const { return static_cast<size_t>(index_); }
    inline size_t height() const { return static_cast<size_t>(height_); }

    inline bool operator == (const blockdb_meta_key_entry &other) const {
        return index_ == other.index_ && height_ == other.height_;
    }

    inline size_t hash_value() const {
        size_t s = 0;
	boost::hash_combine(s, index());
	boost::hash_combine(s, height());
	return s;
    }
  
private:
    uint32_t index_;
    uint32_t height_;
};

}}

namespace std {
    template<> struct hash<::prologcoin::db::blockdb_meta_key_entry> {
        inline size_t operator()(const ::prologcoin::db::blockdb_meta_key_entry &e) const {
	    return e.hash_value();
        }
    };
}

namespace prologcoin { namespace db {

struct blockdb_hash_t {
    uint8_t hash[common::sha1::HASH_SIZE];
};
    
class blockdb_meta_entry {
private:
    friend class blockdb;
    friend class blockdb_bucket;  
  
    inline blockdb_meta_entry(uint32_t index, uint32_t height)
      : index_(index), height_(height), offset_(0), size_(0) { }
public:
    static const size_t SERIALIZATION_SIZE = sizeof(uint32_t)*4 + sizeof(blockdb_hash_t);
  
    inline blockdb_meta_entry() : index_(0), height_(0), offset_(0), size_(0) { }
    inline blockdb_meta_entry( uint32_t index, uint32_t height,
			       uint32_t offset, uint32_t sz, blockdb_hash_t &h )
      : index_(index), height_(height), offset_(offset), size_(sz), hash_(h) { }

    blockdb_meta_entry(const blockdb_meta_entry &other) = default;
  
    inline uint32_t index() const { return index_; }
    inline uint32_t height() const { return height_; }
    inline uint32_t offset() const { return offset_; }
    inline uint32_t size() const { return size_; }
    inline blockdb_hash_t & hash() { return hash_; }

    inline bool is_invalid() const { return offset_ == 0; }

    bool operator == (const blockdb_meta_entry &other) const {
        return index() == other.index() && height() == other.height();
    }
    bool operator < (const blockdb_meta_entry &other) const {
        if (index() < other.index()) {
	    return true;
        } else if (index() > other.index()) {
	    return false;
	}
	return height() < other.height();
    }

    void read(uint8_t *buffer, size_t &n)
    {
	uint8_t *p = &buffer[0];
	index_ = read_uint32(p); p += sizeof(uint32_t);
	height_ = read_uint32(p); p += sizeof(uint32_t);
	offset_ = read_uint32(p); p += sizeof(uint32_t);
	size_ = read_uint32(p); p += sizeof(uint32_t);
	memcpy(&hash_, p, sizeof(hash_));

	n = SERIALIZATION_SIZE;
    }
  
    void write(uint8_t *buffer, size_t &n) const
    {
        uint8_t *p = &buffer[0];
        write_uint32(p, index_); p += sizeof(uint32_t);
	write_uint32(p, height_); p += sizeof(uint32_t);
	write_uint32(p, offset_); p += sizeof(uint32_t);
	write_uint32(p, size_); p += sizeof(uint32_t);
	memcpy(p, &hash_, sizeof(hash_));

	n = SERIALIZATION_SIZE;
    }

    void print(std::ostream &out) const;

private:
    uint32_t index_;   // Heap block number
    uint32_t height_;  // Introduced at height
    uint32_t offset_;  // Stored at offset in file
    uint32_t size_;    // Size of block
    blockdb_hash_t hash_; // Hash of block
};

class blockdb_bucket;
class blockdb;
    
class blockdb_block : public boost::noncopyable {
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
    friend class blockdb_bucket;
    friend class blockdb;

    blockdb_block() : data_(nullptr) { }
    blockdb_block(const blockdb_block &other) = default;
  
    inline blockdb_block(const blockdb_meta_entry &e)
        : entry_(e), data_(new uint8_t[e.size()]) { }
  
    inline blockdb_block(const blockdb_meta_entry &e, const void *_data)
        : entry_(e), data_(new uint8_t[e.size()]) {
         memcpy(data_, _data, e.size());
    }

    inline ~blockdb_block() {
        delete data_;
    }

    blockdb_meta_entry entry_;
    void *data_;
};

class blockdb_bucket {
public:
    static const uint8_t VERSION[16];
    static const size_t VERSION_SZ = sizeof(VERSION);

    inline blockdb_bucket(blockdb &db,
			  const blockdb_params &params,
			  const std::string &filepath_meta,
			  const std::string &filepath_data,
			  size_t first_index)
        : db_(db), params_(params), file_path_meta_(filepath_meta),
	  file_path_data_(filepath_data), initialized_(false),
	  first_index_(first_index), fstream_meta_(nullptr),
	  fstream_data_(nullptr) { }

    // Requires that provided height is higher than the existing one
    inline void add_meta_entry(size_t index, size_t height, size_t offset, size_t sz, blockdb_hash_t &h) {
        internal_add_meta_entry(
		blockdb_meta_entry(common::checked_cast<uint32_t>(index),
				   common::checked_cast<uint32_t>(height),
				   common::checked_cast<uint32_t>(offset),
				   common::checked_cast<uint32_t>(sz),
				   h));
    }

    // Return the entry for given index reflected at provided height.
    inline boost::optional<const blockdb_meta_entry &> find_entry(size_t index, size_t at_height) {
        if (!initialized_) read_meta_data();
        size_t i = index - first_index_;
	if (i >= entries_.size()) {
	    return boost::none;
	}
	blockdb_meta_entry key(index, at_height);
        auto it = std::lower_bound(entries_[i].begin(), entries_[i].end(),key);
	if (it == entries_[i].end()) {
	    return boost::none;
	}
	if (it->height() == at_height) {
	    return *it;
	} else {
   	    return *(it - 1);
	}
    }

    inline blockdb_block * read_block(const blockdb_meta_entry &e) {
        auto *b = new blockdb_block(e);
        auto *f = get_bucket_data_stream();
	f->seekg(e.offset());
	f->read(reinterpret_cast<uint8_t *>(b->data()), e.size());
	return b;
    }

    inline void append_block(blockdb_block *b) {
        auto *f = get_bucket_data_stream();
	f->write(reinterpret_cast<uint8_t *>(b->data()), b->size());
    }

    blockdb_block * new_block(const void *data, size_t sz, size_t index, size_t height);
    fstream * get_bucket_meta_stream();
    fstream * get_bucket_data_stream();
    void flush_streams();  
    void close_streams();
    inline bool empty() const { return entries_.empty(); }

    inline size_t index_offset() const { return first_index_; }
  
    void print(std::ostream &out) const;

private:
    void append_meta_data(const blockdb_meta_entry &e);
    void read_meta_data();

    inline void internal_add_meta_entry(const blockdb_meta_entry &e) {
        size_t i = e.index() - first_index_;
	if (i >= entries_.size()) {
	    entries_.resize(i+1);
	}
        entries_[i].push_back(e);
    }

    blockdb &db_;
    bool initialized_;
    blockdb_params params_;
    std::string file_path_meta_;
    std::string file_path_data_;
    size_t first_index_;
    // First sorted on index, then sorted on height.
    std::vector<std::vector<blockdb_meta_entry> > entries_;
    fstream *fstream_meta_;
    fstream *fstream_data_;  
};
    
//
// This is a framework for storing blocks with versioning.
//    
class blockdb : public blockdb_params {
public:
    blockdb(const std::string &dir_path);

    blockdb_block * find_block(size_t index, size_t from_height);
    blockdb_block * new_block(const void *data, size_t sz, size_t index, size_t from_height);

    bool is_empty() const;
    void erase_all();
    void flush();
  
private:
    friend class blockdb_bucket;
  
    boost::filesystem::path bucket_dir_location(size_t bucket_index) const;
    boost::filesystem::path bucket_file_data_path(size_t bucket_index) const;
    boost::filesystem::path bucket_file_meta_path(size_t bucket_index) const;  

    blockdb_bucket & get_bucket(size_t bucket_index);
    void save_bucket(size_t bucket_index);
  
    std::string dir_path_;

    // Each bucket can handle approximately 134 MB of address space.
    // No need to optimize this. 10000 buckets would mean 1340 GB, and
    // 10000 buckets in memory means ~64k x 10000 = 640 MB. We can optimize
    // this later if needed; it's not a consensus rule.
    std::vector<blockdb_bucket *> buckets_;

    struct block_flusher {
        block_flusher(blockdb &db) : db_(db) { }
        // We assume the block hasn't changed! Note that a new block is always
        // added instead of changing an existing one
        void evicted(const blockdb_meta_key_entry &, blockdb_block *b) {
	    delete b;
        }
    private:
        blockdb &db_;
    };

    struct stream_flusher {
        void evicted(size_t, blockdb_bucket *b) {
	    b->close_streams();
        }
    };

    typedef common::lru_cache<blockdb_meta_key_entry, blockdb_block *, block_flusher> block_cache;
    block_flusher block_flusher_;
    block_cache block_cache_;

    typedef common::lru_cache<size_t, blockdb_bucket *, stream_flusher> stream_cache;
    stream_flusher stream_flusher_;
    stream_cache stream_cache_;  
};
    
}}  

#endif
