#pragma once

#ifndef _common_aes256_hpp
#define _common_aes256_hpp

#include <stdint.h>
#include <assert.h>
#include <memory.h>
#include <vector>

namespace prologcoin { namespace common {

class aes256 {
public:
  static const size_t KEY_SIZE = 32;
  static const size_t KEY_EXP_SIZE = 240;
  static const size_t BLOCK_SIZE = 16;

  aes256(const uint8_t *key, size_t key_len) {
    assert(key_len == KEY_SIZE);
    key_expansion(key, round_key_);
    memset(iv_, 0, sizeof(iv_));
  }
  void set_iv(const uint8_t *iv, size_t iv_size) {
    assert(iv_size == BLOCK_SIZE);
    memcpy(iv_, iv, iv_size);
  }

  void cbc_encrypt(std::vector<uint8_t> &buf) {
      cbc_encrypt(&buf[0], buf.size());
  }
  
  void cbc_encrypt(uint8_t *buf, size_t len) {
    uint8_t *iv = iv_;
    for (size_t i = 0; i < len; i += BLOCK_SIZE) {
      xor_with_iv(&buf[i], iv);
      encrypt_block(&buf[i]);
      iv = &buf[i];
    }
    memcpy(iv_, iv, BLOCK_SIZE);
  }

  void cbc_decrypt(std::vector<uint8_t> &buf) {
    cbc_decrypt(&buf[0], buf.size());
  }
  
  void cbc_decrypt(uint8_t *buf, size_t len) {
    uint8_t next_iv[BLOCK_SIZE];
    for (size_t i = 0; i < len; i += BLOCK_SIZE) {
      memcpy(next_iv, &buf[i], BLOCK_SIZE);
      decrypt_block(&buf[i]);
      xor_with_iv(&buf[i], iv_);
      memcpy(iv_, next_iv, BLOCK_SIZE);
    }
  }
  
private:
  void xor_with_iv(uint8_t *buf, const uint8_t *iv) {
    for (size_t i = 0; i < BLOCK_SIZE; i++) {
      buf[i] ^= iv[i];
    }
  }

  static const size_t Nb = 4;
  static const size_t Nk = 8;
  static const size_t Nr = 14;
  static const uint8_t sbox[256];
  static const uint8_t rsbox[256];

  static const uint8_t Rcon[11];

  static void key_expansion(const uint8_t *key, uint8_t *round_key) {
    uint8_t tempa[4];
  
    for (size_t i = 0; i < Nk; ++i) {
      round_key[(i * 4) + 0] = key[(i * 4) + 0];
      round_key[(i * 4) + 1] = key[(i * 4) + 1];
      round_key[(i * 4) + 2] = key[(i * 4) + 2];
      round_key[(i * 4) + 3] = key[(i * 4) + 3];
    }

    // All other round keys are found from the previous round keys.
    for (size_t i = Nk; i < Nb * (Nr + 1); i++) {
      {
	size_t k = (i - 1) * 4;
	tempa[0] = round_key[k + 0];
	tempa[1] = round_key[k + 1];
	tempa[2] = round_key[k + 2];
	tempa[3] = round_key[k + 3];
      }

      if (i % Nk == 0) {
	// This function shifts the 4 bytes in a word to the left once.
	// [a0,a1,a2,a3] becomes [a1,a2,a3,a0]
	
	// Function RotWord()
	{
	  const uint8_t u8tmp = tempa[0];
	  tempa[0] = tempa[1];
	  tempa[1] = tempa[2];
	  tempa[2] = tempa[3];
	  tempa[3] = u8tmp;
	}
	
        // SubWord() is a function that takes a four-byte input word and 
        // applies the S-box to each of the four bytes to produce an output word.
	
        // Function Subword()
	{
	  tempa[0] = sbox[tempa[0]];
	  tempa[1] = sbox[tempa[1]];
	  tempa[2] = sbox[tempa[2]];
	  tempa[3] = sbox[tempa[3]];
	}
	
	tempa[0] = tempa[0] ^ Rcon[i/Nk];
      }

      if (i % Nk == 4) {
	// Function Subword()
	{
	  tempa[0] = sbox[tempa[0]];
	  tempa[1] = sbox[tempa[1]];
	  tempa[2] = sbox[tempa[2]];
	  tempa[3] = sbox[tempa[3]];
	}
      }

      size_t j = i * 4, k =(i - Nk) * 4;
      round_key[j + 0] = round_key[k + 0] ^ tempa[0];
      round_key[j + 1] = round_key[k + 1] ^ tempa[1];
      round_key[j + 2] = round_key[k + 2] ^ tempa[2];
      round_key[j + 3] = round_key[k + 3] ^ tempa[3];
    }
  }

  inline void add_round_key(uint8_t round) {
    uint8_t i,j;
    for (i = 0; i < 4; ++i) {
      for (j = 0; j < 4; ++j) {
	state_[i][j] ^= round_key_[(round * Nb * 4) + (i * Nb) + j];
      }
    }
  }

  inline void sub_bytes() {
    uint8_t i, j;
    for (i = 0; i < 4; ++i) {
      for (j = 0; j < 4; ++j) {
	state_[j][i] = sbox[state_[j][i]];
      }
    }
  }

  inline void shift_rows() {
    uint8_t temp;

    // Rotate first row 1 columns to left  
    temp         = state_[0][1];
    state_[0][1] = state_[1][1];
    state_[1][1] = state_[2][1];
    state_[2][1] = state_[3][1];
    state_[3][1] = temp;

    // Rotate second row 2 columns to left  
    temp         = state_[0][2];
    state_[0][2] = state_[2][2];
    state_[2][2] = temp;

    temp         = state_[1][2];
    state_[1][2] = state_[3][2];
    state_[3][2] = temp;

    // Rotate third row 3 columns to left
    temp         = state_[0][3];
    state_[0][3] = state_[3][3];
    state_[3][3] = state_[2][3];
    state_[2][3] = state_[1][3];
    state_[1][3] = temp;
  }

  inline static uint8_t xtime(uint8_t x)
  {
    return ((x<<1) ^ (((x>>7) & 1) * 0x1b));
  }

  // MixColumns function mixes the columns of the state matrix
  void mix_columns() {
    uint8_t tmp, tm, t;
    for (uint8_t i = 0; i < 4; ++i) {  
      t   = state_[i][0];
      tmp = state_[i][0] ^ state_[i][1] ^ state_[i][2] ^ state_[i][3];

      tm  = state_[i][0] ^ state_[i][1];
      tm  = xtime(tm);
      state_[i][0] ^= tm ^ tmp;

      tm  = state_[i][1] ^ state_[i][2];
      tm = xtime(tm);
      state_[i][1] ^= tm ^ tmp;

      tm  = state_[i][2] ^ state_[i][3];
      tm = xtime(tm);
      state_[i][2] ^= tm ^ tmp;

      tm  = state_[i][3] ^ t;
      tm = xtime(tm);
      state_[i][3] ^= tm ^ tmp;
    }
  }

  inline static uint8_t multiply(uint8_t x, uint8_t y) {
    return (((y & 1) * x) ^
	    ((y>>1 & 1) * xtime(x)) ^
	    ((y>>2 & 1) * xtime(xtime(x))) ^
	    ((y>>3 & 1) * xtime(xtime(xtime(x)))) ^
	    ((y>>4 & 1) * xtime(xtime(xtime(xtime(x))))));
  }

  void inv_mix_columns() {
    uint8_t a, b, c, d;
    for (size_t i = 0; i < 4; ++i) { 
      a = state_[i][0];
      b = state_[i][1];
      c = state_[i][2];
      d = state_[i][3];
      
      state_[i][0] = multiply(a, 0x0e) ^ multiply(b, 0x0b) ^ multiply(c, 0x0d) ^ multiply(d, 0x09);
      state_[i][1] = multiply(a, 0x09) ^ multiply(b, 0x0e) ^ multiply(c, 0x0b) ^ multiply(d, 0x0d);
      state_[i][2] = multiply(a, 0x0d) ^ multiply(b, 0x09) ^ multiply(c, 0x0e) ^ multiply(d, 0x0b);
      state_[i][3] = multiply(a, 0x0b) ^ multiply(b, 0x0d) ^ multiply(c, 0x09) ^ multiply(d, 0x0e);
    }
  }

  void inv_sub_bytes() {
    for (uint8_t i = 0; i < 4; ++i) {
      for (uint8_t j = 0; j < 4; ++j) {
	state_[j][i] = rsbox[state_[j][i]];
      }
    }
  }

  void inv_shift_rows() {
    uint8_t temp;

    // Rotate first row 1 columns to right  
    temp = state_[3][1];
    state_[3][1] = state_[2][1];
    state_[2][1] = state_[1][1];
    state_[1][1] = state_[0][1];
    state_[0][1] = temp;
    
    // Rotate second row 2 columns to right 
    temp = state_[0][2];
    state_[0][2] = state_[2][2];
    state_[2][2] = temp;
    
    temp = state_[1][2];
    state_[1][2] = state_[3][2];
    state_[3][2] = temp;
    
    // Rotate third row 3 columns to right
    temp = state_[0][3];
    state_[0][3] = state_[1][3];
    state_[1][3] = state_[2][3];
    state_[2][3] = state_[3][3];
    state_[3][3] = temp;
  }

  void encrypt_block(uint8_t block[BLOCK_SIZE]) {
    memcpy(state_, block, BLOCK_SIZE);

    add_round_key(0);

    for (size_t round = 1; round < Nr; round++) {
      sub_bytes();
      shift_rows();
      mix_columns();
      add_round_key(round);
    }

    sub_bytes();
    shift_rows();
    add_round_key(Nr);

    memcpy(block, state_, BLOCK_SIZE);
  }

  void decrypt_block(uint8_t block[BLOCK_SIZE]) {
    memcpy(state_, block, BLOCK_SIZE);

    add_round_key(Nr);

    for (size_t round = Nr - 1; round > 0; round--) {
      inv_shift_rows();
      inv_sub_bytes();
      add_round_key(round);
      inv_mix_columns();
    }

    inv_shift_rows();
    inv_sub_bytes();
    add_round_key(0);

    memcpy(block, state_, BLOCK_SIZE);
  }

  uint8_t round_key_[KEY_EXP_SIZE];
  uint8_t iv_[BLOCK_SIZE];
  uint8_t state_[4][4];

};

}}
    
#endif
