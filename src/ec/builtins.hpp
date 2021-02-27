#pragma once

#ifndef _ec_builtins_hpp
#define _ec_builtins_hpp

#include "../common/term.hpp"
#include "../interp/interpreter.hpp"
#include "keys.hpp"

namespace prologcoin { namespace ec {

class secp256k1_ctx;
    
using interpreter_exception = ::prologcoin::interp::interpreter_exception;

class interpreter_exception_not_public_key : public interpreter_exception {
public:
  interpreter_exception_not_public_key(const std::string &msg) :
      interpreter_exception(msg) { }
};

class interpreter_exception_tweak : public interpreter_exception {
public:
  interpreter_exception_tweak(const std::string &msg) :
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
    static const size_t RAW_SIG_SIZE = 64;

    static musig_env & get_musig_env(interpreter_base &interp);

    static secp256k1_ctx & get_secp256k1_ctx(interpreter_base &interp);
  
    static void load(interpreter_base &interp, common::con_cell *module = nullptr);
    static void load_consensus(interpreter_base &interp);  

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

    // sign(PrivKey, Data, Signature)
    // Compute Signature by signing PrivKey on Data.
    static bool sign_3(interpreter_base &interp, size_t arity, term args[] );

    // validate(PubKey, Data, Signature) true iff Signature is valid w.r.t
    // public key and data.
    static bool validate_3(interpreter_base &interp, size_t arity, term args[] );

  
    // hash(Data, Hash) true iff Hash is the hash of Data.
    static bool hash_2(interpreter_base &interp, size_t arity, term args[] );

    // pubkey_tweak_add(X, Y, Z)
    // Z = X + G*Y
    static bool pubkey_tweak_add_3(interpreter_base &interp, size_t arity, term args[] );
    // privkey_tweak_add(X, Y, Z)
    static bool privkey_tweak_add_3(interpreter_base &interp, size_t arity, term args[] );
  
    // musig verification (does not require a musig session)
  
    // musig_combine(+PubKeys, -CombinedPubKey, -CombinedPubKeyHash) combine
    // public keys and output the combined public key and its hash.
    static bool musig_combine_3(interpreter_base &interp, size_t arity, term args[] );

    // musig_verify(+Data, +CombinedPubKey, +FinalSig)
    static bool musig_verify_3(interpreter_base &interp, size_t arity, term args[] );
    // musig_secret(+Data, +FinalSig, +NonceCommitments, +PubKeys, +PartialSigs, +Adaptor, -Secret)
    static bool musig_secret_7(interpreter_base &interp, size_t arity, term args[] );

    // musig sessions (to actually sign something)

    // Extract session from arg
    static musig_session * get_musig_session(interpreter_base &interp, term arg);
  
    // musig_start(-Session, +CombinedPubKey, +CombinedPubKeyHash, +MyIndex,
    //       +NumSigners, +PrivateKey, +Data)
    // Create new a MuSig session
    static bool musig_start_7(interpreter_base &interp, size_t arity, term args[] );

    // musig_set_public_key(+Session, +Index, +PubKey)
    // For verification only; given the index, set the public key.
    static bool musig_set_public_key_3(interpreter_base &interp, size_t arity, term args[] );

    // musig_nonce_commit(+Session, -NonceCommitment)
    // Start session by getting my nonce commitment
    static bool musig_nonce_commit_2(interpreter_base &interp, size_t arity, term args[] );

    // musig_prepare(+Session, +NonceCommitments, -MyNonce)
    // Set all nonce commitments (from all participants) and return my
    // own nonce.
    static bool musig_prepare_3(interpreter_base &interp, size_t arity, term args[] );

    // musig_nonces(+Session, +Nonces, [+Adaptor])
    // Set all nonces. (Adaptor is optional.)
    static bool musig_nonces_3(interpreter_base &interp, size_t arity, term args[] );
    // musig_partial_sign(+Session, -PartialSignature)
    // (PartialSignature to send to the others. Will include adaptor
    //  signature if musig_set_adaptor has been called)
    static bool musig_partial_sign_2(interpreter_base &interp, size_t arity, term args[] );

    // musig_partial_sign_adapt(+Session, +PartialSignature, +PrivateAdaptor, -AdaptedPartialSignature)
    static bool musig_partial_sign_adapt_4(interpreter_base &interp, size_t arity, term args[] );

    // musig_final_sign(+Session, +PartialSignatures, -FinalSig)
    // musig_final_sign(+Session, +PartialSignatures, +Tweak, -FinalSig)  
    static bool musig_final_sign_4(interpreter_base &interp, size_t arity, term args[] );

    // musig_nonce_negated(+Session, NonceNegated);
    static bool musig_nonce_negated_2(interpreter_base &interp, size_t arity, term args[] );

    // musig_end(+Session)
    static bool musig_end_1(interpreter_base &interp, size_t arity, term args[]);

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

    // BIP32 & BIP39

    // master_key(+Seed, -MasterPrivate, -MasterPublic)
    // Seed can be a list of integers (raw seed)
    //          or a list of words (bip39)
    static bool master_key_3(interpreter_base &interp, size_t arity, term args[]);

    // Words (seed)
    //
    // words(-WordList)
    // words(+Length, -WordList)
    static bool words_2(interpreter_base &interp, size_t arity, term args[]);
  
    // child_pubkey(+ParentPubKey, +Path, -ChildKey)
    // Example path: m/h(2)/3    (master / hardened child 2 / child 3)
    //               'm/' can be skipped; h(2)/3 means the same thing
    static bool child_pubkey_3(interpreter_base &interp, size_t arity, term args[]);
    static bool child_privkey_3(interpreter_base &interp, size_t arity, term args[]);
    // normal_key/2(+ExtendedKey, Key)
    // True iff Key is the normal key from the extended key.
    static bool normal_key_2(interpreter_base &interp, size_t arity, term args[]);

    // bc1(PubKey, Bc1Address)
    // True iff Bc1Address is the string representation of the bc1 address
    // for P2WPKH.
    static bool bc1_2(interpreter_base &interp, size_t arity, term args[]);

    static uint32_t bech32_polymod(uint8_t values[], size_t n);
    static void bech32_hrp_expand(const std::string &s, uint8_t data[], size_t &n);
    static void bech32_create_checksum(const std::string &hrp, uint8_t data[], size_t n, uint8_t out[6]);

    // Encryption
    // encrypt(+Input, +Password, +Iterations, -Output)
    // If input is the term encrypt(Term), then Term is attempted for decryption.
    // If input is does not use 'encrypt' as functor, then we apply encryption is not of decryption.
    static bool encrypt_4(interpreter_base &interp, size_t arity, term args[]);

    static term encrypt(interpreter_base &interp, term input, const std::string &passwd, int64_t iter);

private:
    static bool decrypt(interpreter_base &interp, term input, const uint8_t *key, term result);
    static bool derive_child(const std::string &pname, interpreter_base &interp, term parent, term path, term result);

public:
  
    static void pedersen_test();

    static bool get_hashed_1_data(uint8_t *data, size_t data_len,
				  uint8_t hash[RAW_HASH_SIZE]);
    static bool get_hashed_2_data(uint8_t *data, size_t data_len,
				  uint8_t hash[RAW_HASH_SIZE]);
    static bool get_hashed_1_term(interpreter_base &interp, const term data,
				  uint8_t hash[RAW_HASH_SIZE]);
    static bool get_hashed_2_term(interpreter_base &interp, const term data,
				  uint8_t hash[RAW_HASH_SIZE]);  
  
    static void get_checksum(const uint8_t *bytes, size_t n, uint8_t checksum[4]);

private:
    static bool is_int_list(interpreter_base &interp, term lst);

    static bool get_bignum(interpreter_base &interp, term big, uint8_t *bytes, size_t &n);
    static term new_bignum(interpreter_base &interp, const uint8_t *bytes, size_t n);
    static bool get_private_key(interpreter_base &interp, term big0, private_key &rawkey);
    static bool get_private_key(interpreter_base &interp, term big0, private_key &rawkey, bool &checksum_existed);
    static bool get_public_key(interpreter_base &interp, term big0, public_key &rawkey);
    static bool compute_public_key(interpreter_base &interp,
	  			   private_key &priv_raw,
				   public_key &pub_raw);
    static term create_public_key(interpreter_base &interp,
				  public_key &pub_raw);
    static bool get_address(public_key &pubkey, uint8_t address[20]);
    static bool compute_signature(interpreter_base &interp,
				  uint8_t hashed_data[32],
				  private_key &priv_raw,
				  uint8_t signature[64]);
    static bool compute_signature(interpreter_base &interp, const term data,
				  const term privkey, term &out_signature);
    static bool verify_signature(interpreter_base &interp,
			         uint8_t hash[32], public_key &pubkey,
				 uint8_t sign_data[64]);

    static bool verify_signature(interpreter_base &interp, const term data,
				 const term pubkey, const term signature);
    static bool get_hashed_term(interpreter_base &interp, const term data,
				uint8_t hash[32], size_t count);
    static bool get_signature_data(interpreter_base &interp, const term sign,
				   uint8_t sign_data[64]);
    static bool compute_pedersen_commit(interpreter_base &interp,
					const term blinding,
					const term value,
					uint8_t commit[33]);
    static bool new_private_key(interpreter_base &interp, private_key &rawkey);
    static term create_private_key(interpreter_base &interp, private_key &rawkey, bool with_checksum = true);
};

}}

#endif

