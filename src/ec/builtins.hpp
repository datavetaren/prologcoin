#pragma once

#ifndef _ec_builtins_hpp
#define _ec_builtins_hpp

#include "../common/term.hpp"
#include "../interp/interpreter.hpp"

namespace prologcoin { namespace ec {

using interpreter_exception = ::prologcoin::interp::interpreter_exception;
    
class interpreter_exception_not_public_key : public interpreter_exception {
public:
  interpreter_exception_not_public_key(const std::string &msg) :
      interpreter_exception(msg) { }
};

class interpreter_exception_musig : public interpreter_exception {
public:
  interpreter_exception_musig(const std::string &msg) :
      interpreter_exception(msg) { }
};

class musig_env;
class musig_session;
    
class builtins {
public:
    using term = prologcoin::common::term;
    using interpreter_base = prologcoin::interp::interpreter_base;

    static const size_t RAW_KEY_SIZE = 32;
    static const size_t RAW_HASH_SIZE = 32;

    static musig_env & get_musig_env(interpreter_base &interp);
  
    static void load(interpreter_base &interp, common::con_cell *module = nullptr);

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

    // sign(X, Data, Signature) true iff Signature is the obtained signature for
    // signing Data using X. If Signature is provided (not a variable), then
    // the same predicate can be used to verify the signature, but then
    // X is the public key.
    static bool sign_3(interpreter_base &interp, size_t arity, term args[] );

    // hash(Data, Hash) true iff Hash is the hash of Data.
    static bool hash_2(interpreter_base &interp, size_t arity, term args[] );
  
    // musig verification (does not require a musig session)
  
    // musig_combine(+PubKeys, -CombinedPubKey, -CombinedPubKeyHash) combine
    // public keys and output the combined public key and its hash.
    static bool musig_combine_3(interpreter_base &interp, size_t arity, term args[] );

    // musig_verify(+Data, +CombinedPubKey, +FinalSig)
    static bool musig_verify_3(interpreter_base &interp, size_t arity, term args[] );

    // musig sessions (to actually sign something)
  
    // musig_start(-Session, +CombinedPubKey, +CombinedPubKeyHash, +MyIndex,
    //       +NumSigners, +PrivateKey, +Data)
    // Create new a MuSig session
    static bool musig_start_7(interpreter_base &interp, size_t arity, term args[] );

    // Extract session from arg
    static musig_session * get_musig_session(interpreter_base &interp, term arg);
  
    // musig_nonce_commit(+Session, -NonceCommitment)
    // Start session by getting my nonce commitment
    static bool musig_nonce_commit_2(interpreter_base &interp, size_t arity, term args[] );

    // musig_prepare(+Session, +NonceCommitments, -MyNonce)
    // Set all nonce commitments (from all participants) and return my
    // own nonce.
    static bool musig_prepare_3(interpreter_base &interp, size_t arity, term args[] );

    // musig_nonces(+Session, +Nonces, [+Adaptor])
    // Set all nonces.
    static bool musig_nonces_2(interpreter_base &interp, size_t arity, term args[] );
    static bool musig_nonces_3(interpreter_base &interp, size_t arity, term args[] );

    // musig_sign(+Session, -PartialSign)
    static bool musig_sign_2(interpreter_base &interp, size_t arity, term args[] );

    // musig_sign(+Session, +Signatures, -FinalSig)
    static bool musig_sign_3(interpreter_base &interp, size_t arity, term args[] );

    // musig_end(+Session)
    static void musig_end(interpreter_base &interp, size_t arity, term args[]);

    
  
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

    static void get_checksum(const uint8_t *bytes, size_t n, uint8_t checksum[4]);

    static bool get_bignum(interpreter_base &interp, term big, uint8_t *bytes, size_t &n);
    static term new_bignum(interpreter_base &interp, const uint8_t *bytes, size_t n);
    static bool get_private_key(interpreter_base &interp, term big0, uint8_t rawkey[RAW_KEY_SIZE]);
    static bool get_public_key(interpreter_base &interp, term big0, uint8_t rawkey[RAW_KEY_SIZE+1]);
    static bool compute_public_key(interpreter_base &interp,
	  			   uint8_t priv_raw[RAW_KEY_SIZE],
				   uint8_t pub_raw[RAW_KEY_SIZE+1]);
    static term create_public_key(interpreter_base &interp,
				  uint8_t pub_raw[RAW_KEY_SIZE+1]);
    static bool get_address(uint8_t pubkey[32], uint8_t address[20]);
    static bool compute_signature(interpreter_base &interp,
				  uint8_t hashed_data[32],
				  uint8_t priv_raw[RAW_KEY_SIZE],
				  uint8_t signature[64]);
    static bool compute_signature(interpreter_base &interp, const term data,
				  const term privkey, term &out_signature);
    static bool verify_signature(interpreter_base &interp,
			         uint8_t hash[32], uint8_t pubkey[33],
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
    static bool new_private_key(interpreter_base &interp, uint8_t rawkey[RAW_KEY_SIZE]);
    static term create_private_key(interpreter_base &interp, uint8_t rawkey[RAW_KEY_SIZE]);
};

}}

#endif

