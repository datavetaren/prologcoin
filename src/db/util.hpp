#pragma once

#ifndef _db_util_hpp
#define _db_util_hpp

#include "../common/sha1.hpp"

namespace prologcoin { namespace db {

using fstream = std::fstream;

static inline uint32_t read_uint32(const uint8_t *buf)
{
    return (static_cast<uint32_t>(buf[0]) & 0xff) |
            ((static_cast<uint32_t>(buf[1]) & 0xff) << 8) |
            ((static_cast<uint32_t>(buf[2]) & 0xff) << 16) |
            ((static_cast<uint32_t>(buf[3]) & 0xff) << 24);
}

static inline void write_uint32(uint8_t *buf, uint32_t v)
{
    buf[0] = static_cast<uint8_t>(v & 0xff);
    buf[1] = static_cast<uint8_t>((v >> 8) & 0xff);
    buf[2] = static_cast<uint8_t>((v >> 16) & 0xff);
    buf[3] = static_cast<uint8_t>((v >> 24) & 0xff);
}

static inline uint64_t read_uint64(const uint8_t *buf)
{
    return (static_cast<uint64_t>(buf[0]) & 0xff) |
            ((static_cast<uint64_t>(buf[1]) & 0xff) << 8) |
            ((static_cast<uint64_t>(buf[2]) & 0xff) << 16) |
            ((static_cast<uint64_t>(buf[3]) & 0xff) << 24) |
            ((static_cast<uint64_t>(buf[4]) & 0xff) << 32) |
            ((static_cast<uint64_t>(buf[5]) & 0xff) << 40) |
            ((static_cast<uint64_t>(buf[6]) & 0xff) << 48) |
            ((static_cast<uint64_t>(buf[7]) & 0xff) << 56);
}

static inline void write_uint64(uint8_t *buf, uint64_t v)
{
    buf[0] = static_cast<uint8_t>(v & 0xff);
    buf[1] = static_cast<uint8_t>((v >> 8) & 0xff);
    buf[2] = static_cast<uint8_t>((v >> 16) & 0xff);
    buf[3] = static_cast<uint8_t>((v >> 24) & 0xff);
    buf[4] = static_cast<uint8_t>((v >> 32) & 0xff);
    buf[5] = static_cast<uint8_t>((v >> 40) & 0xff);
    buf[6] = static_cast<uint8_t>((v >> 48) & 0xff);
    buf[7] = static_cast<uint8_t>((v >> 56) & 0xff);
}
    
struct hash_t {
    bool operator == (const hash_t &other) const {
        return memcmp(hash, other.hash, sizeof(hash)) == 0;
    }
  
    uint8_t hash[common::sha1::HASH_SIZE];
};
    
}}

#endif
