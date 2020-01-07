#pragma once

#ifndef _common_pbkdf2_hpp
#define _common_pbkdf2_hpp

#include <stdint.h>
#include <memory>
#include "sha1.hpp"
#include "hmac.hpp"

namespace prologcoin { namespace common {

template<typename Hash = hmac<sha1> > class pbkdf2_t {
public:
  pbkdf2_t(const void *salt, size_t saltLen, size_t iter, size_t keyLen)
    : salt_(static_cast<const uint8_t *>(salt)), salt_len_(saltLen), iter_(iter), key_len_(keyLen) {
  }

  static const size_t MAX_KEY_SIZE = 64;

  void set_password(const char *password, size_t password_len) {
    size_t digest_size = Hash::HASH_SIZE;
    size_t num_blocks = (key_len_ + digest_size - 1) / digest_size;
    hash_.init(password, password_len);
    for (size_t i = 0; i < num_blocks; i++) {
      pbkdf2_F(hash_, i);
    }
  }

  const uint8_t * get_salt() const {
      return &salt_[0];
  }

  size_t get_salt_length() const {
      return salt_len_;
  }
  
  const uint8_t * get_key() const {
      return &key_[0];
  }

private:
  void pbkdf2_F(Hash &hash0, size_t i) {
    const size_t HASH_SIZE = Hash::HASH_SIZE;
    size_t key_offset = i*HASH_SIZE;
    Hash hash1 = hash0;
    hash1.update( salt_, salt_len_ );
    uint8_t block_num_bigend[4] = { 0, 0, 0, static_cast<uint8_t>(i+1) };
    hash1.update(block_num_bigend, 4);
    uint8_t U[HASH_SIZE];
    hash1.finalize(U);
    uint8_t T[HASH_SIZE];
    memcpy( T, U, HASH_SIZE);
    for (size_t r = 0; r < iter_ - 1; r++) {
      Hash hash2 = hash0;
      hash2.update( U, HASH_SIZE );
      hash2.finalize( U );
      xorstr( T, U, HASH_SIZE );
    }
    memcpy( &key_[key_offset], T, HASH_SIZE );
  }
  inline void xorstr( uint8_t *a, uint8_t *b, size_t n) {
    for (size_t i = 0; i < n; i++) {
      a[i] ^= b[i];
    }
  }
						  
  Hash hash_;
  const uint8_t *salt_;
  size_t salt_len_;
  size_t iter_;
  size_t key_len_;
  uint8_t key_[MAX_KEY_SIZE];
};

using pbkdf2 = pbkdf2_t<hmac<sha1> >;

}}
    
#endif
