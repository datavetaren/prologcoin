#pragma once

#ifndef _ec_keys_hpp
#define _ec_keys_hpp

#include "../interp/interpreter_base.hpp"

extern "C" {
typedef struct secp256k1_context_struct secp256k1_context;
typedef struct secp256k1_scratch_space_struct secp256k1_scratch_space;  
}

namespace prologcoin { namespace ec {

class secp256k1_ctx : public prologcoin::interp::managed_data {
public:
    secp256k1_ctx();
    secp256k1_ctx(unsigned int flags);
    virtual ~secp256k1_ctx() override;

    inline operator secp256k1_context * () {
        return ctx_;
    }
    inline secp256k1_scratch_space * scratch() {
        return scratch_;
    }
private:
    secp256k1_context *ctx_;
    secp256k1_scratch_space *scratch_;
};

void checksum(const uint8_t *in, size_t len, uint8_t out[4]);
    
class public_key {
public:
    static const size_t SIZE = 33;

    public_key();

    inline public_key(const uint8_t bytes[SIZE]) { set_bytes(bytes); }
    inline public_key(const public_key &other) { set_bytes(other.bytes_); }
    inline public_key & operator = (const public_key &other) {
      set_bytes(other.bytes_);
      return *this;
    }

    inline uint8_t & operator [] (size_t index) { return bytes_[index]; }
    inline const uint8_t & operator [] (size_t index) const { return bytes_[index]; }

    inline void set_bytes(const uint8_t bytes[SIZE]) {
        std::copy(bytes, &bytes[SIZE], bytes_);
    }
  
    inline const uint8_t * bytes() const { return bytes_; }

    void compute_fingerprint(uint8_t fingerprint[4]) const;

private:
    uint8_t bytes_[SIZE];
};
    
class private_key {
public:
    static const size_t SIZE = 32;
    enum key_create_t { NO_KEY = 0, NEW_KEY = 1 };
  
    private_key() = delete;

protected:
    private_key(const private_key &other) = default;

public:
    private_key(key_create_t k);
    private_key(key_create_t k, secp256k1_ctx &ctx);

    void create_new(secp256k1_ctx &ctx);

    uint8_t & operator [] (size_t index) { return bytes_[index]; }
    const uint8_t & operator [] (size_t index) const { return bytes_[index]; }
  
    inline const uint8_t * bytes() const { return bytes_; }

    bool compute_public_key(secp256k1_ctx &ctx, public_key &pubkey) const;

    void compute_fingerprint(uint8_t fingerprint[4]) const;
  
private:
    uint8_t bytes_[SIZE];
};

class chain_code {
public:
    static const size_t SIZE = 32;

    inline chain_code() {
        memset(bytes_, 0, SIZE);
    }
    inline chain_code(uint8_t code[SIZE]) {
        std::copy(code, code+SIZE, bytes_);
    }

    inline chain_code & operator =  (uint8_t code[SIZE]) {
        std::copy(code, code+SIZE, bytes_);
	return *this;
    }

    inline uint8_t & operator [] (size_t index) { return bytes_[index]; }
    inline const uint8_t & operator [] (size_t index) const { return bytes_[index]; }  
  
private:
    uint8_t bytes_[SIZE];
};

class extended_key {
public:
    static const uint32_t HARDENED_KEY = 1 << 31;
  
    enum extended_type { EXTENDED_PUBLIC = 0, EXTENDED_PRIVATE = 1 };

    inline extended_key(extended_type t) : type_(t), level_(0), child_number_(0) { memset(fingerprint_, 0, 4); }
    inline extended_type type() const { return type_; }
    inline size_t level() const { return level_; }
    inline const uint8_t * fingerprint() const { return fingerprint_; }
    inline uint32_t child_number() const { return child_number_; }
    inline void set_level(size_t level) { level_ = level; }
    inline void set_fingerprint(const uint8_t fingerp[4]) { std::copy(fingerp, &fingerp[4], fingerprint_); }
    inline void set_child_number(uint32_t child_num) { child_number_ = child_num; }
    inline chain_code & get_chain_code() { return chain_code_; }
    inline const chain_code & get_chain_code() const { return chain_code_; }
    inline void set_chain_code(const chain_code &c) { chain_code_ = c; }
    inline void set_chain_code(uint8_t code[32]) { chain_code_ = code; }

private:
    extended_type type_;
    size_t level_;
    uint8_t fingerprint_[4];
    uint32_t child_number_;
    chain_code chain_code_;
};
    
class extended_public_key : public public_key, public extended_key {
public:
    inline extended_public_key() : extended_key(extended_key::EXTENDED_PUBLIC) { }
    inline extended_public_key(public_key &pubkey) : public_key(pubkey), extended_key(extended_key::EXTENDED_PUBLIC) { }

    inline void set_public_key(public_key &pubkey) { static_cast<public_key *>(this)->operator = (pubkey); }

    void write(uint8_t data[78]) const;
    bool read(uint8_t data[78]);

    std::string to_string() const;
    common::term to_term(common::term_env &env) const;
  
private:
    chain_code chain_code_;
};

class hd_keys;
class extended_private_key : public private_key, public extended_key {
public:
    extended_private_key() : private_key(private_key::NO_KEY), extended_key(extended_key::EXTENDED_PRIVATE) { }
    extended_private_key(private_key &privkey) : private_key(privkey), extended_key(extended_key::EXTENDED_PRIVATE) { }

    void write(uint8_t bytes[78]) const;
    bool read(uint8_t data[78]);
  
    std::string to_string() const;
    common::term to_term(common::term_env &env) const;  
  
    void set_from_hash(const uint8_t hash[64]);

    void compute_extended_public_key(secp256k1_ctx &ctx, extended_public_key &pubkey);
  
private:
    inline extended_private_key(secp256k1_ctx &ctx) : private_key(private_key::NO_KEY, ctx), extended_key(extended_key::EXTENDED_PRIVATE) { }

    friend class hd_keys;
};
    
class hd_keys {
public:
    hd_keys(secp256k1_ctx &ctx);
    hd_keys(secp256k1_ctx &ctx, const uint8_t *seed, size_t seed_len);
    void set_seed(const uint8_t *seed, size_t seed_len);

    const extended_private_key & master_private() { return master_; }
    const extended_public_key & master_public() { return master_public_; }
  
    static inline uint32_t H(uint32_t i) { return i + extended_key::HARDENED_KEY; }

    bool generate_child(const extended_public_key &parent, uint32_t index, extended_public_key &out);
    bool generate_child(const extended_private_key &parent, uint32_t index, extended_private_key &out);
    bool generate_child(const extended_private_key &parent, uint32_t index, extended_public_key &out);

private:
    secp256k1_ctx &ctx_;
    extended_private_key master_;
    extended_public_key master_public_; // Computed from master
};

}}

#endif
