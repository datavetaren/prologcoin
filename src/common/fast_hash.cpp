#include "fast_hash.hpp"

namespace prologcoin { namespace common {

// This is intentionally not a constant so it can be changed at
//  startup time.
uint32_t fast_hash::HASH_SEED = 38136192;

#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif

static FORCE_INLINE uint32_t rotl32 ( uint32_t x, int8_t r )
{
    return (x << r) | (x >> (32 - r));
}

static FORCE_INLINE uint32_t fmix32 ( uint32_t h )
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

void fast_hash::update(uint32_t value)
{
    uint32_t k1 = value;
    uint32_t h1 = state_;

    k1 *= C1;
    k1 = rotl32(k1, 15);
    k1 *= C2;

    h1 ^= k1;
    h1 = rotl32(h1, 13);
    h1 = h1*5+0xe6546b64;

    state_ = h1;
    count_ += 4;
}

void fast_hash::update(const uint8_t *bytes, size_t len)
{
    for (size_t i = 0; i < len; i += 8) {
	uint64_t v = 0;
	size_t limit = (i + 8 < len) ? 8 : len - i;
	for (size_t j = 0; j < limit; j++) {
	    v = (v << 8) | bytes[i+j];
	}
	update(v);
    }
}

uint32_t fast_hash::finalize()
{
    uint32_t h1 = state_;
    h1 ^= count_;
    h1 = fmix32(h1);
    return h1;
}


}}
