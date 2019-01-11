#pragma once

#ifndef _pow_siphash_hpp
#define _pow_siphash_hpp

#include <stdint.h>
#include <immintrin.h>
#include <assert.h>
#include "checked_cast.hpp"
#include "blake2.hpp"

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

struct siphash_compat {};

template<uint8_t shift> inline uint64_t l(uint8_t u8) {
    return static_cast<uint64_t>(u8) << shift;
}

template<uint8_t N> inline uint64_t u64_n(const uint8_t *u8s);

template<> inline uint64_t u64_n<0>(const uint8_t *u8s) 
{ return l<0>(u8s[0]); }

template<uint8_t N> inline uint64_t u64_n(const uint8_t *u8s)
{ return l<N*8>(u8s[N]) | u64_n<N-1>(u8s); }

// Friendlier to hide the 0 based counting.
template<uint8_t N> inline uint64_t u64(const uint8_t *u8s)
{ return u64_n<N-1>(u8s); }

class siphash_keys {
public:
    inline siphash_keys(const siphash_keys &other) = default;

    inline siphash_keys(const char *msg, size_t len) 
    {
	uint8_t d32[32];
        blake2b(d32, sizeof(d32), msg, len, nullptr, 0);
	set_internal(d32);
    }

    inline siphash_keys(const uint8_t *msg, size_t len)
    {
	uint8_t d32[32];
	blake2b(d32, sizeof(d32), msg, len, nullptr, 0);
	set_internal(d32);
    }

    inline siphash_keys(const uint8_t *msg, size_t len, siphash_compat)
    {
	uint8_t d32[32];
	blake2b(d32, sizeof(d32), msg, len, nullptr, 0);
	set_internal(d32);
	uint64_t k0 = k0_;
	uint64_t k1 = k1_;
	k0_ = k0 ^ 0x736f6d6570736575ULL;
	k1_ = k1 ^ 0x646f72616e646f6dULL;
	k2_ = k0 ^ 0x6c7967656e657261ULL;
	k3_ = k1 ^ 0x7465646279746573ULL;
    }

    inline const uint64_t k0() const { return k0_; }
    inline const uint64_t k1() const { return k1_; }
    inline const uint64_t k2() const { return k2_; }
    inline const uint64_t k3() const { return k3_; }

private:
    inline void set_internal(uint8_t d32[32]) {
	k0_ = u64<8>(&d32[0]);
	k1_ = u64<8>(&d32[8]);
	k2_ = u64<8>(&d32[16]);
	k3_ = u64<8>(&d32[24]);
    }
    
    uint64_t k0_;
    uint64_t k1_;
    uint64_t k2_;
    uint64_t k3_;
};

#define SIP_ROTL(x,b) (uint64_t)( ((x) << (b)) | ( (x) >> (64 - (b))) )
#define SIPROUND \
  do { \
    v0 += v1; v2 += v3; v1 = SIP_ROTL(v1,13); \
    v3 = SIP_ROTL(v3,16); v1 ^= v0; v3 ^= v2; \
    v0 = SIP_ROTL(v0,32); v2 += v1; v0 += v3; \
    v1 = SIP_ROTL(v1,17);   v3 = SIP_ROTL(v3,21); \
    v1 ^= v2; v3 ^= v0; v2 = SIP_ROTL(v2,32); \
  } while(0)

inline static void siphash_1(const siphash_keys &keys, const uint64_t nonce, uint64_t &out) {
    uint64_t v0 = keys.k0(), v1 = keys.k1(), v2 = keys.k2(), v3 = keys.k3() ^ nonce;
    SIPROUND; SIPROUND;
    v0 ^= nonce;
    v2 ^= 0xff;
    SIPROUND; SIPROUND; SIPROUND; SIPROUND;
    out = (v0 ^ v1) ^ (v2  ^ v3);
}

#ifdef __AVX2__

#define SIP_ADD(a, b) _mm256_add_epi64(a, b)
#define SIP_XOR(a, b) _mm256_xor_si256(a, b)
#define SIP_ROTATE16 _mm256_set_epi64x(0x0D0C0B0A09080F0EULL,0x0504030201000706ULL, \
                                   0x0D0C0B0A09080F0EULL, 0x0504030201000706ULL)
#define SIP_ROT13(x) _mm256_or_si256(_mm256_slli_epi64(x,13),_mm256_srli_epi64(x,51))
#define SIP_ROT16(x) _mm256_shuffle_epi8((x), SIP_ROTATE16)
#define SIP_ROT17(x) _mm256_or_si256(_mm256_slli_epi64(x,17),_mm256_srli_epi64(x,47))
#define SIP_ROT21(x) _mm256_or_si256(_mm256_slli_epi64(x,21),_mm256_srli_epi64(x,43))
#define SIP_ROT32(x) _mm256_shuffle_epi32((x), _MM_SHUFFLE(2, 3, 0, 1))

#elif defined __SSE2__

#define SIP_ADD(a, b) _mm_add_epi64(a, b)
#define SIP_XOR(a, b) _mm_xor_si128(a, b)
#define SIP_ROT13(x) _mm_or_si128(_mm_slli_epi64(x,13),_mm_srli_epi64(x,51))
#define SIP_ROT16(x) _mm_shufflehi_epi16(_mm_shufflelo_epi16(x, _MM_SHUFFLE(2,1,0,3)), _MM_SHUFFLE(2,1,0,3))
#define SIP_ROT17(x) _mm_or_si128(_mm_slli_epi64(x,17),_mm_srli_epi64(x,47))
#define SIP_ROT21(x) _mm_or_si128(_mm_slli_epi64(x,21),_mm_srli_epi64(x,43))
#define SIP_ROT32(x) _mm_shuffle_epi32  (x, _MM_SHUFFLE(2,3,0,1))

#endif

#define SIPROUNDXN \
  do { \
    v0 = SIP_ADD(v0,v1); v2 = SIP_ADD(v2,v3); v1 = SIP_ROT13(v1); \
    v3 = SIP_ROT16(v3);  v1 = SIP_XOR(v1,v0); v3 = SIP_XOR(v3,v2); \
    v0 = SIP_ROT32(v0);  v2 = SIP_ADD(v2,v1); v0 = SIP_ADD(v0,v3); \
    v1 = SIP_ROT17(v1);                   v3 = SIP_ROT21(v3); \
    v1 = SIP_XOR(v1,v2); v3 = SIP_XOR(v3,v0); v2 = SIP_ROT32(v2); \
  } while(0)

#define SIPROUNDX2N \
  do { \
    v0 = SIP_ADD(v0,v1); v4 = SIP_ADD(v4,v5); \
    v2 = SIP_ADD(v2,v3); v6 = SIP_ADD(v6,v7); \
    v1 = SIP_ROT13(v1);  v5 = SIP_ROT13(v5); \
    v3 = SIP_ROT16(v3);  v7 = SIP_ROT16(v7); \
    v1 = SIP_XOR(v1,v0); v5 = SIP_XOR(v5,v4); \
    v3 = SIP_XOR(v3,v2); v7 = SIP_XOR(v7,v6); \
    v0 = SIP_ROT32(v0);  v4 = SIP_ROT32(v4); \
    v2 = SIP_ADD(v2,v1); v6 = SIP_ADD(v6,v5); \
    v0 = SIP_ADD(v0,v3); v4 = SIP_ADD(v4,v7); \
    v1 = SIP_ROT17(v1);  v5 = SIP_ROT17(v5); \
    v3 = SIP_ROT21(v3);  v7 = SIP_ROT21(v7); \
    v1 = SIP_XOR(v1,v2); v5 = SIP_XOR(v5,v6); \
    v3 = SIP_XOR(v3,v0); v7 = SIP_XOR(v7,v4); \
    v2 = SIP_ROT32(v2);  v6 = SIP_ROT32(v6); \
  } while(0)
 
#define SIPROUNDX4N \
  do { \
    v0 = SIP_ADD(v0,v1); v4 = SIP_ADD(v4,v5);  v8 = SIP_ADD(v8,v9); vC = SIP_ADD(vC,vD); \
    v2 = SIP_ADD(v2,v3); v6 = SIP_ADD(v6,v7);  vA = SIP_ADD(vA,vB); vE = SIP_ADD(vE,vF); \
    v1 = SIP_ROT13(v1);  v5 = SIP_ROT13(v5);   v9 = SIP_ROT13(v9);  vD = SIP_ROT13(vD); \
    v3 = SIP_ROT16(v3);  v7 = SIP_ROT16(v7);   vB = SIP_ROT16(vB);  vF = SIP_ROT16(vF); \
    v1 = SIP_XOR(v1,v0); v5 = SIP_XOR(v5,v4);  v9 = SIP_XOR(v9,v8); vD = SIP_XOR(vD,vC); \
    v3 = SIP_XOR(v3,v2); v7 = SIP_XOR(v7,v6);  vB = SIP_XOR(vB,vA); vF = SIP_XOR(vF,vE); \
    v0 = SIP_ROT32(v0);  v4 = SIP_ROT32(v4);   v8 = SIP_ROT32(v8);  vC = SIP_ROT32(vC); \
    v2 = SIP_ADD(v2,v1); v6 = SIP_ADD(v6,v5);  vA = SIP_ADD(vA,v9); vE = SIP_ADD(vE,vD); \
    v0 = SIP_ADD(v0,v3); v4 = SIP_ADD(v4,v7);  v8 = SIP_ADD(v8,vB); vC = SIP_ADD(vC,vF); \
    v1 = SIP_ROT17(v1);  v5 = SIP_ROT17(v5);   v9 = SIP_ROT17(v9);  vD = SIP_ROT17(vD); \
    v3 = SIP_ROT21(v3);  v7 = SIP_ROT21(v7);   vB = SIP_ROT21(vB);  vF = SIP_ROT21(vF); \
    v1 = SIP_XOR(v1,v2); v5 = SIP_XOR(v5,v6);  v9 = SIP_XOR(v9,vA); vD = SIP_XOR(vD,vE); \
    v3 = SIP_XOR(v3,v0); v7 = SIP_XOR(v7,v4);  vB = SIP_XOR(vB,v8); vF = SIP_XOR(vF,vC); \
    v2 = SIP_ROT32(v2);  v6 = SIP_ROT32(v6);   vA = SIP_ROT32(vA);  vE = SIP_ROT32(vE); \
  } while(0)


#ifdef __AVX2__

static inline void siphash_4(const siphash_keys &keys, uint64_t in1, uint64_t in2, uint64_t in3, uint64_t in4, uint64_t &out1, uint64_t &out2, uint64_t &out3, uint64_t &out4) {
  __m256i packet = _mm256_set_epi64x(in4,in3,in2,in1);
  __m256i v0 = _mm256_set1_epi64x(keys.k0());
  __m256i v1 = _mm256_set1_epi64x(keys.k1());
  __m256i v2 = _mm256_set1_epi64x(keys.k2());
  __m256i v3 = _mm256_set1_epi64x(keys.k3());

  v3 = SIP_XOR(v3,packet);
  SIPROUNDXN; SIPROUNDXN;
  v0 = SIP_XOR(v0,packet);
  v2 = SIP_XOR(v2,_mm256_set1_epi64x(0xffLL));
  SIPROUNDXN; SIPROUNDXN; SIPROUNDXN; SIPROUNDXN;
  packet = SIP_XOR(SIP_XOR(v0,v1),SIP_XOR(v2,v3));
  uint64_t *out64 = reinterpret_cast<uint64_t *>(&packet);
  out1 = out64[0]; // _mm256_extract_epi64(packet, 0);
  out2 = out64[1]; // _mm256_extract_epi64(packet, 1);
  out3 = out64[2]; // _mm256_extract_epi64(packet, 2);
  out4 = out64[3]; // _mm256_extract_epi64(packet, 3);
}

static inline void siphash_8(const siphash_keys &keys, uint64_t in1, uint64_t in2, uint64_t in3, uint64_t in4, uint64_t in5, uint64_t in6, uint64_t in7, uint64_t in8, uint64_t &out1, uint64_t &out2, uint64_t &out3, uint64_t &out4, uint64_t &out5, uint64_t &out6, uint64_t &out7, uint64_t &out8) {
  __m256i packet0 = _mm256_set_epi64x(in4,in3,in2,in1);
  __m256i packet4 = _mm256_set_epi64x(in8,in7,in6,in5);
  __m256i v0, v1, v2, v3, v4, v5, v6, v7;
  v4 = v0 = _mm256_set1_epi64x(keys.k0());
  v5 = v1 = _mm256_set1_epi64x(keys.k1());
  v6 = v2 = _mm256_set1_epi64x(keys.k2());
  v7 = v3 = _mm256_set1_epi64x(keys.k3());

  v3 = SIP_XOR(v3,packet0); v7 = SIP_XOR(v7,packet4);
  SIPROUNDX2N; SIPROUNDX2N;
  v0 = SIP_XOR(v0,packet0); v4 = SIP_XOR(v4,packet4);
  v2 = SIP_XOR(v2,_mm256_set1_epi64x(0xffLL));
  v6 = SIP_XOR(v6,_mm256_set1_epi64x(0xffLL));
  SIPROUNDX2N; SIPROUNDX2N; SIPROUNDX2N; SIPROUNDX2N;
  packet0 = SIP_XOR(SIP_XOR(v0,v1),SIP_XOR(v2,v3));
  packet4 = SIP_XOR(SIP_XOR(v4,v5),SIP_XOR(v6,v7));
  uint64_t *out64_0 = reinterpret_cast<uint64_t *>(&packet0);
  out1 = out64_0[0]; // _mm256_extract_epi64(packet, 0);
  out2 = out64_0[1]; // _mm256_extract_epi64(packet, 1);
  out3 = out64_0[2]; // _mm256_extract_epi64(packet, 2);
  out4 = out64_0[3]; // _mm256_extract_epi64(packet, 3);
  uint64_t *out64_4 = reinterpret_cast<uint64_t *>(&packet4);

  out5 = out64_4[0]; // _mm256_extract_epi64(packet, 0);
  out6 = out64_4[1]; // _mm256_extract_epi64(packet, 1);
  out7 = out64_4[2]; // _mm256_extract_epi64(packet, 2);
  out8 = out64_4[3]; // _mm256_extract_epi64(packet, 3);

  /*
  out1 = _mm256_extract_epi64(packet0, 0);
  out2 = _mm256_extract_epi64(packet0, 1);
  out3 = _mm256_extract_epi64(packet0, 2);
  out4 = _mm256_extract_epi64(packet0, 3);
  out5 = _mm256_extract_epi64(packet4, 0);
  out6 = _mm256_extract_epi64(packet4, 1);
  out7 = _mm256_extract_epi64(packet4, 2);
  out8 = _mm256_extract_epi64(packet4, 3);
  */
}

#elif defined __SSE2__

// 2-way sipHash-2-4 specialized to precomputed key and 8 byte nonces
static inline void siphash_2(const siphash_keys &keys, uint64_t in1, uint64_t in2, uint64_t &out1, uint64_t &out2) {
  __m128i v0, v1, v2, v3, mi;
  v0 = _mm_set1_epi64x(keys.k0());
  v1 = _mm_set1_epi64x(keys.k1());
  v2 = _mm_set1_epi64x(keys.k2());
  v3 = _mm_set1_epi64x(keys.k3());
  mi = _mm_set_epi64x(in2, in1);
	
  v3 = SIP_XOR (v3, mi);
  SIPROUNDXN; SIPROUNDXN;
  v0 = SIP_XOR (v0, mi);
  
  v2 = SIP_XOR (v2, _mm_set1_epi64x(0xffLL));
  SIPROUNDXN; SIPROUNDXN; SIPROUNDXN; SIPROUNDXN;
  mi = SIP_XOR(SIP_XOR(v0,v1),SIP_XOR(v2,v3));
  
  out1 = _mm_extract_epi64(mi, 0);
  out2 = _mm_extract_epi64(mi, 1);
}

static inline void siphash_4(const siphash_keys &keys, uint64_t in1, uint64_t in2, uint64_t in3, uint64_t in4, uint64_t &out1, uint64_t &out2, uint64_t &out3, uint64_t &out4) {
  __m128i v0, v1, v2, v3, mi, v4, v5, v6, v7, m2;
  v4 = v0 = _mm_set1_epi64x(keys.k0());
  v5 = v1 = _mm_set1_epi64x(keys.k1());
  v6 = v2 = _mm_set1_epi64x(keys.k2());
  v7 = v3 = _mm_set1_epi64x(keys.k3());

  mi = _mm_set_epi64x(in2, in1);
  m2 = _mm_set_epi64x(in4, in3);

  v3 = SIP_XOR (v3, mi);
  v7 = SIP_XOR (v7, m2);
  SIPROUNDX2N; SIPROUNDX2N;
  v0 = SIP_XOR (v0, mi);
  v4 = SIP_XOR (v4, m2);

  v2 = SIP_XOR (v2, _mm_set1_epi64x(0xffLL));
  v6 = SIP_XOR (v6, _mm_set1_epi64x(0xffLL));
  SIPROUNDX2N; SIPROUNDX2N; SIPROUNDX2N; SIPROUNDX2N;
  mi = SIP_XOR(SIP_XOR(v0,v1),SIP_XOR(v2,v3));
  m2 = SIP_XOR(SIP_XOR(v4,v5),SIP_XOR(v6,v7));
  
  out1 = _mm_extract_epi64(mi, 0);
  out2 = _mm_extract_epi64(mi, 1);
  out3 = _mm_extract_epi64(m2, 0);
  out4 = _mm_extract_epi64(m2, 1);
}
   
static inline void siphash_8(const siphash_keys &keys, uint64_t in1, uint64_t in2, uint64_t in3, uint64_t in4, uint64_t in5, uint64_t in6, uint64_t in7, uint64_t in8, uint64_t &out1, uint64_t &out2, uint64_t &out3, uint64_t &out4, uint64_t &out5, uint64_t &out6, uint64_t &out7, uint64_t &out8) {
   siphash_4(keys, in1, in2, in3, in4, out1, out2, out3, out4);
   siphash_4(keys, in5, in6, in7, in8, out5, out6, out7, out8);
}

#endif

//#ifndef NSIPHASH
//// how many siphash24 to compute in parallel
// currently 1, 2, 4, 8 are supported, but
// more than 1 requires the use of sse2 or avx2
// more than 4 requires the use of avx2
//#define NSIPHASH 1
//#endif

template<size_t N> inline void siphash(const siphash_keys &keys, const uint64_t from, uint64_t *out)
{
    switch (N) {
    case 1: {
	siphash_1(keys, from, out[0]); break;
        }
    case 4: {
	uint64_t i1 = from, i2 = from+1, i3 = from+2, i4 = from+3;
	siphash_4(keys, i1, i2, i3, i4, out[0], out[1], out[2], out[3]); break;
        }
    case 8: {
	uint64_t i1 = from, i2 = from+1, i3 = from+2, i4 = from+3;
	uint64_t i5 = from+4, i6 = from+5, i7 = from+6, i8 = from+7;
	siphash_8(keys, i1, i2, i3, i4, i5, i6, i7, i8, out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7]);
        }
    default: assert(N == 1 || N == 4 || N == 8);
    }
}

inline void siphash(const siphash_keys &keys, const uint64_t from, const uint64_t to, uint64_t *out)
{
    size_t n = checked_cast<size_t>(to - from);
    size_t i = 0;
    for (; i + 8 < n; i += 8) {
	siphash<8>(keys, from+i, &out[i]);
    }
    for (; i + 4 < n; i += 4) {
	siphash<4>(keys, from+i, &out[i]);
    }
    for (; i < n; i++) {
	siphash<1>(keys, from+i, &out[i]);
    }
}

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

#endif
