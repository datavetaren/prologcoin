#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include "hex.hpp"

/*
 #define ROL8(a, n) (((a) << (n)) | ((a) >> (8 - (n))))
 #define ROL16(a, n) (((a) << (n)) | ((a) >> (16 - (n))))
 #define ROL32(a, n) (((a) << (n)) | ((a) >> (32 - (n))))
 #define ROL64(a, n) (((a) << (n)) | ((a) >> (64 - (n))))
  
 //Rotate right operation
 #define ROR8(a, n) (((a) >> (n)) | ((a) << (8 - (n))))
 #define ROR16(a, n) (((a) >> (n)) | ((a) << (16 - (n))))
 #define ROR32(a, n) (((a) >> (n)) | ((a) << (32 - (n))))
 //Shift left operation
 #define SHL8(a, n) ((a) << (n))
 #define SHL16(a, n) ((a) << (n))
 #define SHL32(a, n) ((a) << (n))
 #define SHL64(a, n) ((a) << (n))
  
 //Shift right operation
 #define SHR8(a, n) ((a) >> (n))
 #define SHR16(a, n) ((a) >> (n))
 #define SHR32(a, n) ((a) >> (n))
 #define SHR64(a, n) ((a) >> (n))
  
 //Micellaneous macros
 #define _U8(x) ((uint8_t) (x))
 #define _U16(x) ((uint16_t) (x))
 #define _U32(x) ((uint32_t) (x))
 #define _U64(x) ((uint64_t) (x))
*/


#ifndef _common_sha512_hpp
#define _common_sha512_hpp

namespace prologcoin { namespace common {


class sha512 {
public:
  static const size_t HASH_SIZE = 64;
  static const size_t BLOCK_SIZE = 128;

  static const uint8_t PADDING[128];
  static const uint64_t K[80];
  
  inline sha512() { init(); }
  void init();
  void update(const void *p, size_t len);
  inline const std::string finalize() {
    uint8_t dig[HASH_SIZE];
    finalize(dig);
    return hex::to_string(dig, HASH_SIZE);
  }
  void finalize(uint8_t digest[HASH_SIZE]);

private:
   void block();
   union {
     uint64_t h_[8];
     uint8_t digest_[64];
   };
   union {
     uint64_t w_[16];
     uint8_t buffer_[128];
   };
   size_t size_;
   uint64_t total_size_;
};

}}

#endif
