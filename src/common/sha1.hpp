#pragma once

#include <stdint.h>
#include <string>
#include <string.h>
#include <memory>
#include "hex.hpp"

#ifndef _common_sha1_hpp
#define _common_sha1_hpp

namespace prologcoin { namespace common {

#define prologcoin_rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))
#define prologcoin_blk0(i) (block->l[i] = (prologcoin_rol(block->l[i],24)&0xff00ff00) |(prologcoin_rol(block->l[i],8)&0x00ff00ff))
#define prologcoin_blk(i) (block->l[i&15] = prologcoin_rol(block->l[(i+13)&15]^block->l[(i+8)&15]^block->l[(i+2)&15]^block->l[i&15],1))

#define prologcoin_R0(v, w, x, y, z, i) \
    z+=((w&(x^y))^y)+prologcoin_blk0(i)+0x5a827999+prologcoin_rol(v,5);w=prologcoin_rol(w,30);
#define prologcoin_R1(v, w, x, y, z, i) \
    z+=((w&(x^y))^y)+prologcoin_blk(i)+0x5a827999+prologcoin_rol(v,5);w=prologcoin_rol(w,30);
#define prologcoin_R2(v, w, x, y, z, i) \
    z+=(w^x^y)+prologcoin_blk(i)+0x6ed9eba1+prologcoin_rol(v,5);w=prologcoin_rol(w,30);
#define prologcoin_R3(v, w, x, y, z, i) \
    z+=(((w|x)&y)|(w&x))+prologcoin_blk(i)+0x8f1bbcdc+prologcoin_rol(v,5);w=prologcoin_rol(w,30);
#define prologcoin_R4(v, w, x, y, z, i) \
    z+=(w^x^y)+prologcoin_blk(i)+0xca62c1d6+prologcoin_rol(v,5);w=prologcoin_rol(w,30);

class sha1 {
public:
  static const size_t HASH_SIZE = 20;
  static const size_t BLOCK_SIZE = 64;
    
  sha1() {
    init();
  }

  void init() {
    state[0] = 0x67452301;
    state[1] = 0xefcdab89;
    state[2] = 0x98badcfe;
    state[3] = 0x10325476;
    state[4] = 0xc3d2e1f0;
    count[0] = count[1] = 0;
  }

  void update(const void *p, size_t len) {
    const uint8_t *data = reinterpret_cast<const uint8_t *>(p);
    size_t i, j;

    j = (count[0] >> 3) & 63;
    if ((count[0] += (uint32_t) (len << 3)) < (len << 3)) {
        count[1]++;
    }
    count[1] += (uint32_t) (len >> 29);
    if ((j + len) > 63) {
        memcpy(&buffer[j], data, (i = 64 - j));
        transform(buffer);
        for (; i + 63 < len; i += 64) {
	    memcpy(buffer, data + i, 64);
            transform(buffer);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&buffer[j], &data[i], len - i);
  }

  const std::string finalize() {
    uint8_t dig[HASH_SIZE];
    finalize(dig);
    return hex::to_string(dig, HASH_SIZE);
  }
  
  void finalize(uint8_t digest[HASH_SIZE]) {
    uint32_t i;
    uint8_t finalcount[8];

    for (i = 0; i < 8; i++) {
        finalcount[i] = (uint8_t) ((count[(i >= 4 ? 0 : 1)]
                >> ((3 - (i & 3)) * 8)) & 255);
    }
    update((uint8_t *) "\200", 1);
    while ((count[0] & 504) != 448) {
        update((uint8_t *) "\0", 1);
    }
    update(finalcount, 8);
    for (i = 0; i < HASH_SIZE; i++) {
        digest[i] = (uint8_t)
                ((state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }

    i = 0;
    memset(buffer, 0, 64);
    memset(state, 0, 20);
    memset(count, 0, 8);
    memset(finalcount, 0, 8);
  }

private:

   inline void transform(const uint8_t buffer[64]) {
     uint32_t a, b, c, d, e;
     typedef union {
       uint8_t c[64];
       uint32_t l[16];
     } CHAR64LONG16;
     CHAR64LONG16 *block;

     block = (CHAR64LONG16*)buffer;

     /* Copy state to working vars */
     a = state[0];
     b = state[1];
     c = state[2];
     d = state[3];
     e = state[4];
     
     /* 4 rounds of 20 operations each. Loop unrolled. */
     prologcoin_R0(a, b, c, d, e, 0);
     prologcoin_R0(e, a, b, c, d, 1);
     prologcoin_R0(d, e, a, b, c, 2);
     prologcoin_R0(c, d, e, a, b, 3);
     prologcoin_R0(b, c, d, e, a, 4);
     prologcoin_R0(a, b, c, d, e, 5);
     prologcoin_R0(e, a, b, c, d, 6);
     prologcoin_R0(d, e, a, b, c, 7);
     prologcoin_R0(c, d, e, a, b, 8);
     prologcoin_R0(b, c, d, e, a, 9);
     prologcoin_R0(a, b, c, d, e, 10);
     prologcoin_R0(e, a, b, c, d, 11);
     prologcoin_R0(d, e, a, b, c, 12);
     prologcoin_R0(c, d, e, a, b, 13);
     prologcoin_R0(b, c, d, e, a, 14);
     prologcoin_R0(a, b, c, d, e, 15);
     prologcoin_R1(e, a, b, c, d, 16);
     prologcoin_R1(d, e, a, b, c, 17);
     prologcoin_R1(c, d, e, a, b, 18);
     prologcoin_R1(b, c, d, e, a, 19);
     prologcoin_R2(a, b, c, d, e, 20);
     prologcoin_R2(e, a, b, c, d, 21);
     prologcoin_R2(d, e, a, b, c, 22);
     prologcoin_R2(c, d, e, a, b, 23);
     prologcoin_R2(b, c, d, e, a, 24);
     prologcoin_R2(a, b, c, d, e, 25);
     prologcoin_R2(e, a, b, c, d, 26);
     prologcoin_R2(d, e, a, b, c, 27);
     prologcoin_R2(c, d, e, a, b, 28);
     prologcoin_R2(b, c, d, e, a, 29);
     prologcoin_R2(a, b, c, d, e, 30);
     prologcoin_R2(e, a, b, c, d, 31);
     prologcoin_R2(d, e, a, b, c, 32);
     prologcoin_R2(c, d, e, a, b, 33);
     prologcoin_R2(b, c, d, e, a, 34);
     prologcoin_R2(a, b, c, d, e, 35);
     prologcoin_R2(e, a, b, c, d, 36);
     prologcoin_R2(d, e, a, b, c, 37);
     prologcoin_R2(c, d, e, a, b, 38);
     prologcoin_R2(b, c, d, e, a, 39);
     prologcoin_R3(a, b, c, d, e, 40);
     prologcoin_R3(e, a, b, c, d, 41);
     prologcoin_R3(d, e, a, b, c, 42);
     prologcoin_R3(c, d, e, a, b, 43);
     prologcoin_R3(b, c, d, e, a, 44);
     prologcoin_R3(a, b, c, d, e, 45);
     prologcoin_R3(e, a, b, c, d, 46);
     prologcoin_R3(d, e, a, b, c, 47);
     prologcoin_R3(c, d, e, a, b, 48);
     prologcoin_R3(b, c, d, e, a, 49);
     prologcoin_R3(a, b, c, d, e, 50);
     prologcoin_R3(e, a, b, c, d, 51);
     prologcoin_R3(d, e, a, b, c, 52);
     prologcoin_R3(c, d, e, a, b, 53);
     prologcoin_R3(b, c, d, e, a, 54);
     prologcoin_R3(a, b, c, d, e, 55);
     prologcoin_R3(e, a, b, c, d, 56);
     prologcoin_R3(d, e, a, b, c, 57);
     prologcoin_R3(c, d, e, a, b, 58);
     prologcoin_R3(b, c, d, e, a, 59);
     prologcoin_R4(a, b, c, d, e, 60);
     prologcoin_R4(e, a, b, c, d, 61);
     prologcoin_R4(d, e, a, b, c, 62);
     prologcoin_R4(c, d, e, a, b, 63);
     prologcoin_R4(b, c, d, e, a, 64);
     prologcoin_R4(a, b, c, d, e, 65);
     prologcoin_R4(e, a, b, c, d, 66);
     prologcoin_R4(d, e, a, b, c, 67);
     prologcoin_R4(c, d, e, a, b, 68);
     prologcoin_R4(b, c, d, e, a, 69);
     prologcoin_R4(a, b, c, d, e, 70);
     prologcoin_R4(e, a, b, c, d, 71);
     prologcoin_R4(d, e, a, b, c, 72);
     prologcoin_R4(c, d, e, a, b, 73);
     prologcoin_R4(b, c, d, e, a, 74);
     prologcoin_R4(a, b, c, d, e, 75);
     prologcoin_R4(e, a, b, c, d, 76);
     prologcoin_R4(d, e, a, b, c, 77);
     prologcoin_R4(c, d, e, a, b, 78);
     prologcoin_R4(b, c, d, e, a, 79);
     
     /* Add the working vars back into context.state[] */
     state[0] += a;
     state[1] += b;
     state[2] += c;
     state[3] += d;
     state[4] += e;
     
     /* Wipe variables */
     a = b = c = d = e = 0;
   }

  uint32_t state[5];
  uint32_t count[2];
  uint8_t buffer[64];
};

}}

#endif
