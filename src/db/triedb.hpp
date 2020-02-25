#pragma once

#ifndef _db_triedb_hpp
#define _db_triedb_hpp

#include <cstdint>
#include <fstream>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/functional/hash.hpp>
#include <boost/intrusive_ptr.hpp>
#include <iostream>
#include <bitset>
#include "../common/lru_cache.hpp"
#include "util.hpp"
#include "triedb_params.hpp"

namespace prologcoin { namespace db {

class triedb_exception : public std::runtime_error {
public:
    triedb_exception(const std::string &msg) : std::runtime_error(msg) { }
};

class triedb_version_exception : public triedb_exception {
public:
    triedb_version_exception(const std::string &msg) : triedb_exception(msg) { }
};

class triedb_write_exception : public triedb_exception {
public:
    triedb_write_exception(const std::string &msg) : triedb_exception(msg) { }
};

class triedb_leaf {
    virtual ~triedb_leaf() { }
    virtual size_t size() = 0;
    virtual void read(const uint8_t *buffer) = 0;
    virtual void write(uint8_t *buffer) = 0;
};

class triedb_branch {
public:
    static const size_t MAX_SIZE_IN_BYTES = sizeof(uint32_t)*2 + sizeof(hash_t) + 32*sizeof(uint64_t);

    triedb_branch() : mask_(0), leaf_(0), hash_(), ptr_(nullptr) { }
  
    uint32_t mask() const { return mask_; }

    inline size_t num_children() const
        { return std::bitset<32>(mask_).count(); }
  
    size_t serialization_size() const;
    void read(const uint8_t *buffer);
    void write(uint8_t *buffer) const;
  
private:
    uint32_t mask_;
    uint32_t leaf_;
    hash_t hash_;
    uint64_t *ptr_;
};
    
class triedb : public triedb_params {
public:
    triedb(const std::string &dir_path);
    triedb(const triedb_params &params, const std::string &dir_path);  

private:
    std::string roots_file_path() const;
    boost::filesystem::path bucket_dir_location(size_t bucket_index) const;
    boost::filesystem::path bucket_file_path(size_t bucket_index) const;
    fstream * get_bucket_stream(size_t bucket_index);
    size_t scan_last_bucket();
    uint64_t scan_last_offset();
  
    void read_branch_node(uint64_t offset, triedb_branch &node);
    uint64_t append_branch_node(const triedb_branch &node);

    std::string dir_path_;

    struct stream_flusher {
        void evicted(size_t, fstream *f) {
	    f->close();
	    delete f;
        }
    };

    // Bucket index to stream
    typedef common::lru_cache<size_t, fstream *, stream_flusher> stream_cache;
    stream_flusher stream_flusher_;
    stream_cache stream_cache_;

    uint64_t last_offset_;
};
    
}}

#endif
