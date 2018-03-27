#pragma once

#include <string>

#ifndef _common_fast_hash_hpp
#define _common_fast_hash_hpp

namespace prologcoin { namespace common {

//
// 32-bit Murmur3 hashing. Not cryptographically secure, but convenient
// to use for internal data structures.
//
class fast_hash {
public:
    inline fast_hash() { reset(); }
    inline void reset() { state_ = HASH_SEED; count_ = 0; }

    friend inline fast_hash & operator << (fast_hash &h,const std::string &str)
    { h.update(str.c_str(), str.size()); return h; }

    friend inline fast_hash & operator << (fast_hash &h, int val)
    {
	h << static_cast<uint64_t>(val);
	return h;
    }

    friend inline fast_hash & operator << (fast_hash &h, unsigned short val)
    {
        h << static_cast<uint64_t>(val);
	return h;
    }

    friend inline fast_hash & operator << (fast_hash &h, unsigned long val)
    {
	h << static_cast<uint64_t>(val);
	return h;
    }

    friend inline fast_hash & operator << (fast_hash &h, int64_t val)
    {
	h << static_cast<uint64_t>(val);
	return h;
    }

    friend inline fast_hash & operator << (fast_hash &h, uint64_t val)
    { 
	auto high = static_cast<uint32_t>(val >> 32);
	if (high != 0) {
	    h.update(high);
	}
	h.update(static_cast<uint32_t>(val & 0xffffffff));
        return h;
    }

    inline operator uint32_t () {
	return finalize();
    }

    void update(uint32_t value);
    void update(const uint8_t *bytes, size_t len);

    inline void update(const char *bytes, size_t len)
    { update(reinterpret_cast<const uint8_t *>(bytes), len); }

    uint32_t finalize();

private:
    uint32_t state_;
    size_t count_;

    // Not const! (so we can change it at boot time!)
    static uint32_t HASH_SEED;
    static const uint32_t C1 = 0xcc9e2d51;
    static const uint32_t C2 = 0x1b873593;
    
};

}}

#endif
