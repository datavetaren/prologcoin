#pragma once

#ifndef _common_hmac_hpp
#define _common_hmac_hpp

#include <stdint.h>
#include <memory>
#include <string.h>

namespace prologcoin { namespace common {

template<typename Hash> class hmac {
public:
  static const size_t HASH_SIZE = Hash::HASH_SIZE;
  static const size_t BLOCK_SIZE = Hash::BLOCK_SIZE;

  inline hmac() { }

  inline void init(const void *key, size_t key_len) {
    if (key_len > BLOCK_SIZE) {
      Hash h;
      h.update(key, key_len);
      h.finalize(key_block_);
      if (BLOCK_SIZE > HASH_SIZE) {
	memset(&key_block_[h.HASH_SIZE], 0, BLOCK_SIZE - HASH_SIZE);
      }
    } else {
      memcpy(key_block_, key, key_len);
      memset(&key_block_[key_len], 0, BLOCK_SIZE - key_len);
    }
    uint8_t inner_block[BLOCK_SIZE];
    for (size_t i = 0; i < BLOCK_SIZE; i++) {
      inner_block[i] = key_block_[i] ^ 0x36;
    }
    inner_hash_.init();
    inner_hash_.update(inner_block, BLOCK_SIZE);
  }

  inline void update(const void *p, size_t len) {
    inner_hash_.update(p, len);
  }

  inline void finalize(uint8_t digest[HASH_SIZE]) {
    Hash outer;
    uint8_t outer_block[BLOCK_SIZE];
    for (size_t i = 0; i < BLOCK_SIZE; i++) {
      outer_block[i] = key_block_[i] ^ 0x5c;
    }
    outer.update(outer_block, BLOCK_SIZE);
    uint8_t inner_digest[HASH_SIZE];
    inner_hash_.finalize(inner_digest);
    outer.update(inner_digest, HASH_SIZE);
    outer.finalize(digest);
  }

  inline const size_t hash_size() const {
    return HASH_SIZE;
  }
  
private:      
  Hash inner_hash_;
  uint8_t key_block_[Hash::BLOCK_SIZE];
};

}}

#endif
