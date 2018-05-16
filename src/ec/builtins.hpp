#pragma once

#ifndef _ec_builtins_hpp
#define _ec_builtins_hpp

#include "../common/term.hpp"
#include "../interp/interpreter.hpp"

namespace prologcoin { namespace ec {

class builtins {
public:
    using term = prologcoin::common::term;
    using interpreter_base = prologcoin::interp::interpreter_base;

    static void load(interpreter_base &interp);

    // privkey(X) true iff X is a private key.
    // Can be used to generate new private keys.
    static bool privkey_1(interpreter_base &interp, size_t arity, term args[] );

    // privkey(X,Y) true iff Y is a private key with checksum using raw key X
    // as base.
    static bool privkey_2(interpreter_base &interp, size_t arity, term args[]);

    // pubkey(X, Y) true iff Y is public key of private key X.
    // Can also be used to generate the public key Y from X.
    static bool pubkey_2(interpreter_base &interp, size_t arity, term args[] );

    // address(X, Y) true iff Y is the bitcoin address of public key X.
    // Can also be used to generate the address Y from the public key X.
    static bool address_2(interpreter_base &interp, size_t arity, term args[] );
    // sign(X, Data, Signture) true iff Signature is the obtained signature for
    // signing Data using X. If Signature is provided (not a variable), then
    // the same predicate can be used to verify the signature, but then
    // X is the public key.
    static bool sign_3(interpreter_base &interp, size_t arity, term args[] );

    // pcommit(P, R, V) true iff P is a Pedersen commitment, i.e.
    // P = r*G + v*H. P is a variable.
    // static bool pcommit_3(interpreter_base &interp, size_t arity, term args[]);

    // pproof(R, V, Proof).
    // Generate a zero knowledge proof for this Pedersen commitment
    // P = r*G + v*H.
    static bool pproof_3(interpreter_base &interp, size_t arity, term args[]);

    // pverify(Proof)
    // Verifies the given proof.
    static bool pverify_1(interpreter_base &interp, size_t arity, term args[]);

    static void pedersen_test();

private:
    static const size_t RAW_KEY_SIZE = 32;

    static void get_checksum(const uint8_t *bytes, size_t n, uint8_t checksum[4]);

    static bool get_bignum(interpreter_base &interp, term big, uint8_t *bytes, size_t &n);
    static term new_bignum(interpreter_base &interp, const uint8_t *bytes, size_t n);
    static bool get_private_key(interpreter_base &interp, term big0, uint8_t rawkey[RAW_KEY_SIZE]);
    static bool get_public_key(interpreter_base &interp, term big0, uint8_t rawkey[RAW_KEY_SIZE+1]);
    static bool compute_public_key(uint8_t priv_raw[RAW_KEY_SIZE],
				   uint8_t pub_raw[RAW_KEY_SIZE+1]);
    static term create_public_key(interpreter_base &interp,
				  uint8_t pub_raw[RAW_KEY_SIZE+1]);
    static bool get_address(uint8_t pubkey[32], uint8_t address[20]);
    static bool compute_signature(uint8_t hashed_data[32],
				  uint8_t priv_raw[RAW_KEY_SIZE],
				  uint8_t signature[64]);
    static bool compute_signature(interpreter_base &interp, const term data,
				  const term privkey, term &out_signature);
    static bool verify_signature(uint8_t hash[32], uint8_t pubkey[33],
				 uint8_t sign_data[64]);

    static bool verify_signature(interpreter_base &interp, const term data,
				 const term pubkey, const term signature);
    static bool get_hashed_data(uint8_t *data, size_t data_len,
				uint8_t hash[32]);
    static bool get_hashed2_data(interpreter_base &interp, const term data,
				 uint8_t hash[32]);
    static bool get_signature_data(interpreter_base &interp, const term sign,
				   uint8_t sign_data[64]);
    static bool compute_pedersen_commit(interpreter_base &interp,
					const term blinding,
					const term value,
					uint8_t commit[33]);
    static bool new_private_key(uint8_t rawkey[RAW_KEY_SIZE]);
    static term create_private_key(interpreter_base &interp, uint8_t rawkey[RAW_KEY_SIZE]);
};

}}

#endif

