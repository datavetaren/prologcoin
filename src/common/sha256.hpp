#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include "hex.hpp"

#ifndef _common_sha256_hpp
#define _common_sha256_hpp

namespace prologcoin { namespace common {


class sha256 {
public:
  static const size_t HASH_SIZE = 32;
  static const size_t BLOCK_SIZE = 64;

  static const uint8_t PADDING[64];
  
  inline sha256() { init(); }
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
     uint32_t s_[8];
     uint8_t digest_[32];
   };
   union {
     uint32_t w_[16];
     uint8_t buffer_[64];
   };
   uint32_t total_size_;
};

}}

#endif
