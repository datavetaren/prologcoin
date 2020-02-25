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
#include "util.hpp"

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
    
private:
    uint32_t mask_;
    uint32_t leaf_;
    hash_t hash_;
    uint64_t *ptr_;
};
    
class triedb {
public:
    triedb(const std::string &dir_path);

private:
    std::string roots_file_path() const;
    boost::filesystem::path bucket_dir_location(size_t bucket_index) const;
    std::string bucket_file_data_path(size_t bucket_index) const;

    std::string dir_path_;    
};
    
}}

#endif
