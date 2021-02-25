#include "../secp256k1/secp256k1.hpp"
#include "builtins.hpp"
#include "../common/random.hpp"
#include "../common/utime.hpp"
#include "../interp/interpreter_base.hpp"
#include "../common/term_serializer.hpp"
#include "../common/hex.hpp"
#include "../common/ripemd160.hpp"
#include "keys.hpp"
#include "mnemonic.hpp"
#include "../common/pbkdf2.hpp"
#include "../common/sha256.hpp"
#include "../common/sha512.hpp"
#include "../common/aes256.hpp"

#include "src/util.h"
#include "src/hash_impl.h"

namespace prologcoin { namespace ec {

using namespace prologcoin::common;
using namespace prologcoin::interp;

static const size_t MAX_SIGNERS = 1024;
    
class musig_session {
public:
    static const size_t RAW_KEY_SIZE = builtins::RAW_KEY_SIZE;
    static const size_t RAW_HASH_SIZE = builtins::RAW_HASH_SIZE;

    musig_session(size_t id, secp256k1_context *ctx, size_t num_signers);
    ~musig_session();

    inline secp256k1_musig_session * internal() const { return &session_; }

    inline size_t id() const { return id_; }
    inline size_t num_signers() const { return num_signers_; }
    inline int nonce_is_negated() const { return nonce_is_negated_; }
    inline size_t my_index() const { return my_index_; }
    inline secp256k1_musig_session_signer_data * my_signer_data() const {
        return &signers_[my_index_];
    }
    inline secp256k1_pubkey * my_public_key() const { return &pubkeys_[my_index_]; }
  
    bool init(size_t my_index, const secp256k1_pubkey &combined_pubkey,
	      const uint8_t pubkey_hash[RAW_HASH_SIZE],
	      const private_key &privkey,
	      const uint8_t datahash[RAW_HASH_SIZE]);

    bool prepare(const uint8_t * const *nonce_commitment,
		 secp256k1_pubkey &nonce);
  
    inline void get_nonce_commitment(uint8_t nonce_commitment[RAW_HASH_SIZE]) {
        memcpy(nonce_commitment, nonce_commitment_, RAW_HASH_SIZE);
    }

    inline void set_nonce(size_t index, secp256k1_pubkey &nonce) {
        assert(secp256k1_musig_set_nonce(ctx_, &signers_[index], &nonce) == 1);
    }

    inline void set_public_key(size_t index, secp256k1_pubkey &pubkey) {
        if (pubkeys_ == nullptr) {
	    pubkeys_ = new secp256k1_pubkey[num_signers_];
        }
	pubkeys_[index] = pubkey;
    }

    inline bool combine_nonces(secp256k1_pubkey *adaptor) {
        return secp256k1_musig_session_combine_nonces(
	      ctx_, &session_, signers_, num_signers_,
	      &nonce_is_negated_, adaptor) == 1;
    }

    inline bool partial_sign(secp256k1_musig_partial_signature &signature) {
        return secp256k1_musig_partial_sign(ctx_, &session_, &signature) == 1;
    }

    inline bool partial_sign_adapt(secp256k1_musig_partial_signature &signature, uint8_t adaptor_secret[RAW_KEY_SIZE] ) {
	return secp256k1_musig_partial_sig_adapt(
		  ctx_, &signature, &signature,
		  adaptor_secret, nonce_is_negated_) == 1;
    }

    inline bool final_sign(secp256k1_musig_partial_signature *signatures,
			   secp256k1_schnorrsig &final_sig,
			   uint8_t *tweak) {
	return secp256k1_musig_partial_sig_combine(ctx_, &session_, &final_sig,
						   signatures, num_signers_,
						   tweak) == 1;
    }

private:
    size_t id_;
    secp256k1_context *ctx_;
    mutable secp256k1_musig_session session_;
    size_t num_signers_;
    size_t my_index_;
    secp256k1_musig_session_signer_data *signers_;
    uint8_t session_id_[RAW_KEY_SIZE];
    uint8_t nonce_commitment_[RAW_HASH_SIZE];
    uint8_t data_hash_[RAW_HASH_SIZE];
    int nonce_is_negated_;
    uint8_t adaptor_secret_[RAW_KEY_SIZE];
    secp256k1_pubkey adaptor_;
    mutable secp256k1_pubkey *pubkeys_;
};

class musig_env : public prologcoin::interp::managed_data {
public:
    inline musig_env(secp256k1_context *ctx) : ctx_(ctx), session_count_(0) { }
    inline size_t new_session_id() {
        return ++session_count_;
    }
    inline musig_session * new_session(size_t num_signers) {
        auto id = new_session_id();
	auto *session = new musig_session(id, ctx_, num_signers);
	session_[id] = session;
	return session;
    }
    inline musig_session * get_session(size_t id) {
        return session_[id];
    }
    inline void delete_session(size_t id) {
        auto *session = session_[id];
	delete session;
	session_.erase(id);
    }

private:
    secp256k1_context *ctx_;
    size_t session_count_;
    std::unordered_map<size_t, musig_session *> session_;
};    
    
static secp256k1_ctx & get_ctx(interpreter_base &interp) {
    static const common::con_cell CONTEXT("$ecctx", 0);

    secp256k1_ctx *c;
    
    if ((c = reinterpret_cast<secp256k1_ctx *>(
		   interp.get_managed_data(CONTEXT))) == nullptr) {
        c = new secp256k1_ctx(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
        interp.set_managed_data(CONTEXT, c);
    }

    return *c;
}

secp256k1_ctx & builtins::get_secp256k1_ctx(interpreter_base &interp)
{
    return get_ctx(interp);
}
    
musig_session::musig_session(size_t id, secp256k1_context *ctx, size_t num_signers) :
    id_(id),
    ctx_(ctx),
    num_signers_(num_signers),
    my_index_(static_cast<size_t>(-1)),
    signers_(nullptr),
    pubkeys_ (nullptr) {
    signers_ = new secp256k1_musig_session_signer_data[num_signers];
    common::random::next_bytes(session_id_, sizeof(session_id_));
    memset(nonce_commitment_, 0, sizeof(nonce_commitment_));
    memset(&adaptor_, 0, sizeof(adaptor_));
    memset(&adaptor_secret_[0], 0, sizeof(adaptor_secret_));
}

musig_session::~musig_session() {
    delete [] signers_;
    delete [] pubkeys_;
}

bool musig_session::init(size_t my_index, const secp256k1_pubkey &combined_pubkey, const uint8_t pubkey_hash[RAW_HASH_SIZE], const private_key &privkey, const uint8_t data_hash[RAW_HASH_SIZE]) {
    if (secp256k1_musig_session_initialize(ctx_, &session_,
					   signers_, nonce_commitment_,
					   session_id_, data_hash,
					   &combined_pubkey,
					   &pubkey_hash[0],
					   num_signers_,
					   my_index,
					   &privkey[0]) != 1) {
        return false;
    }
    my_index_ = my_index;
    memcpy(data_hash_, data_hash, RAW_HASH_SIZE);
    return true;
}

bool musig_session::prepare(const uint8_t * const *nonce_commitments,
			    secp256k1_pubkey &nonce) {
    if (secp256k1_musig_session_get_public_nonce(ctx_, &session_,
						 signers_, &nonce,
						 nonce_commitments,
						 num_signers_,
						 nullptr) != 1) {
        return false;
    }
    return true;
}

musig_env & builtins::get_musig_env(interpreter_base &interp) {
    musig_env *env = reinterpret_cast<musig_env *>(interp.get_managed_data(con_cell("$musige",0)));
    if (env == nullptr) {
        env = new musig_env(get_ctx(interp));
	interp.set_managed_data(con_cell("$musige",0), env);
    }
    return *env;
}
    
void builtins::get_checksum(const uint8_t *bytes, size_t n, uint8_t checksum[4])
{
    // Silence unused warnings
    (void) &secp256k1_rfc6979_hmac_sha256_initialize;
    (void) &secp256k1_rfc6979_hmac_sha256_generate;
    (void) &secp256k1_rfc6979_hmac_sha256_finalize;

    uint8_t out32[RAW_KEY_SIZE];
    secp256k1_sha256 ctx;
    secp256k1_sha256_initialize(&ctx);
    secp256k1_sha256_write(&ctx, bytes, n);
    secp256k1_sha256_finalize(&ctx, out32);
    secp256k1_sha256_initialize(&ctx);
    secp256k1_sha256_write(&ctx, out32, 32);
    secp256k1_sha256_finalize(&ctx, out32);

    memcpy(&checksum[0], out32, 4);
}

term builtins::create_private_key(interpreter_base &interp, private_key &rawkey, bool with_checksum)
{
    if (!with_checksum) {
        term big = interp.new_big(private_key::SIZE*8);
	interp.set_big(big, &rawkey[0], private_key::SIZE);
	return big;
    }
  
    uint8_t bytes[1+private_key::SIZE+1+4];

    memcpy(&bytes[1], &rawkey[0], private_key::SIZE);

    uint8_t checksum[4];
    bytes[0] = 0x80;
    bytes[RAW_KEY_SIZE+1] = 0x01;
    get_checksum(&bytes[0], private_key::SIZE+2, checksum);
    memcpy(&bytes[34], checksum, 4);

    term big = interp.new_big(8+(private_key::SIZE)*8+8+32);
    interp.set_big(big, bytes, sizeof(bytes));

    return big;
}

bool builtins::get_bignum(interpreter_base &interp, term big0, uint8_t *bytes, size_t &n)
{
    if (big0.tag() != tag_t::INT && big0.tag() != tag_t::BIG) {
	return false;
    }

    if (big0.tag() == tag_t::INT) {
	if (n < 8) {
	    return false;
	}
	uint64_t val = reinterpret_cast<const int_cell &>(big0).value();
	n = 8;
	for (size_t i = 0; i < 8; i++) {
	    bytes[i] = static_cast<uint8_t>((val >> ((8-i-1)*8)) & 0xff);
	}
	return true;
    }

    auto &big = reinterpret_cast<const big_cell &>(big0);
    size_t nbits = interp.num_bits(big);

    size_t nbytes = (nbits + 7) / 8;

    if (nbytes > n) {
	return false;
    }
    n = nbytes;
    interp.get_big(big, &bytes[0], n);

    return true;
}

term builtins::new_bignum(interpreter_base &interp, const uint8_t *bytes, size_t n)
{
    term big = interp.new_big(n*8);
    interp.set_big(big, bytes, n);
    return big;
}

bool builtins::get_private_key(interpreter_base &interp, term big0, private_key &rawkey) {
    bool checksum_existed = false;
    return get_private_key(interp, big0, rawkey, checksum_existed);
}
    
bool builtins::get_private_key(interpreter_base &interp, term big0, private_key &rawkey, bool &checksum_existed)
{
    uint8_t bytes[1+private_key::SIZE+8];

    if (big0.tag() != tag_t::BIG) {
	return false;
    }

    static const size_t BITS = private_key::SIZE*8;

    auto &big = reinterpret_cast<const big_cell &>(big0);
    size_t nbits = interp.num_bits(big);
    if (nbits != BITS && nbits != 8+BITS+8 && nbits != 8+BITS+8+32) {
	return false;
    }

    if (nbits == BITS) {
        checksum_existed = false;
	interp.get_big(big, &rawkey[0], nbits/8);
    } else {
        checksum_existed = true;
	interp.get_big(big, &bytes[0], nbits/8);
	if (bytes[0] != 0x80 || bytes[33] != 0x01) {
	    return false;
	}
	if (nbits == 8+BITS+8+32) {
	    uint8_t checksum[4];
	    get_checksum(&bytes[0], private_key::SIZE+2, checksum);
	    if (memcmp(&bytes[private_key::SIZE+2], checksum, 4) != 0) {
		return false;
	    }
	}
	memcpy(&rawkey[0], &bytes[1], private_key::SIZE);
    }
    return true;
}

bool builtins::get_public_key(interpreter_base &interp, term big0, public_key &pubkey)
{
    if (big0.tag() != tag_t::BIG) {
	return false;
    }

    auto &big = reinterpret_cast<const big_cell &>(big0);
    size_t nbits = interp.num_bits(big);
    if (nbits != 8+256) {
	return false;
    }
    interp.get_big(big, &pubkey[0], nbits/8);
    if (pubkey[0] != 0x02 && pubkey[0] != 0x03) {
	return false;
    }
    return true;
}

bool builtins::new_private_key(interpreter_base &interp, private_key &rawkey)
{
    auto &ctx = get_ctx(interp);
    rawkey.create_new(ctx);
    return true;
}

bool builtins::privkey_1(interpreter_base &interp, size_t arity, term args[])
{
    // Generate a new private key. We'll use the bitcoin private key
    // format where first byte is 0x80 followed by 

    auto &ctx = get_ctx(interp);

    if (args[0].tag().is_ref()) {

        private_key rawkey(private_key::NEW_KEY, ctx);
	term big = create_private_key(interp, rawkey);
	return interp.unify(args[0], big);
    } else {
        private_key key(private_key::NO_KEY, ctx);
	if (!get_private_key(interp, args[0], key)) {
	    return false;
	}
	bool ok = secp256k1_ec_seckey_verify(ctx, &key[0]);
	return ok;
    }
}

bool builtins::privkey_2(interpreter_base &interp, size_t arity, term args[])
{
    if (args[0].tag().is_ref()) {
	throw interpreter_exception_not_sufficiently_instantiated(
		  "ec:privkey/2: First argument, a raw private key"
                  " must be given.");
    }

    private_key rawkey(private_key::NO_KEY, get_ctx(interp));
    size_t n = private_key::SIZE;
    if (!get_bignum(interp, args[0], &rawkey[0], n)) {
	return false;
    }

    size_t dn = private_key::SIZE - n;

    // Pad with leading 0s
    memmove(&rawkey[dn], &rawkey[0], n);
    memset(&rawkey[0], 0, dn);
    
    term r = create_private_key(interp, rawkey);

    return interp.unify(args[1], r);
}

bool builtins::compute_public_key(interpreter_base &interp,
				  private_key &priv_raw,
				  public_key &pub_raw)
{
    auto &ctx = get_ctx(interp);

    secp256k1_pubkey pubkey;
    bool r = secp256k1_ec_pubkey_create(ctx, &pubkey, &priv_raw[0]);
    if (!r) {
	return false;
    }
    size_t pub_raw_len = public_key::SIZE;
    r = secp256k1_ec_pubkey_serialize(ctx, &pub_raw[0], &pub_raw_len,
				      &pubkey, SECP256K1_EC_COMPRESSED);
    if (!r) {
	return false;
    }
    if (pub_raw_len != public_key::SIZE) {
	return false;
    }
    return true;
}

term builtins::create_public_key(interpreter_base &interp,
				 public_key &pub_raw)
{
    term big = interp.new_big(public_key::SIZE*8);
    interp.set_big(big, &pub_raw[0], public_key::SIZE);
    return big;
}

bool builtins::pubkey_2(interpreter_base &interp, size_t arity, term args[])
{
    
    private_key pkey(private_key::NO_KEY, get_ctx(interp));
    if (!get_private_key(interp, args[0], pkey)) {
	return false;
    }
    public_key pub_raw;
    if (!compute_public_key(interp, pkey, pub_raw)) {
	return false;
    }
    term big = create_public_key(interp, pub_raw);
    return interp.unify(args[1], big);
}

bool builtins::get_address(public_key &pubkey, uint8_t addr[25])
{
    uint8_t out32[32];

    secp256k1_sha256 ctx;
    secp256k1_sha256_initialize(&ctx);
    secp256k1_sha256_write(&ctx, &pubkey[0], 33);
    secp256k1_sha256_finalize(&ctx, out32);

    ripemd160(out32, 32, &addr[1]);
    addr[0] = 0x00;
    get_checksum(&addr[0], 21, &addr[21]);

    return true;
}

bool builtins::address_2(interpreter_base &interp, size_t arity, term args[])
{
    // Generate a bitcoin address from the public key.
    public_key pubkey;
    if (!get_public_key(interp, args[0], pubkey)) {
	return false;
    }

    uint8_t addr[25];
    get_address(pubkey, addr);

    term big = interp.new_big(sizeof(addr)*8);
    interp.set_big(big, addr, sizeof(addr));
    
    return interp.unify(args[1], big);
}

// MuSig does a single hash computation for nonce commitments
bool builtins::get_hashed_1_data(uint8_t *data, size_t data_len,
			         uint8_t hash[32])
{
    secp256k1_sha256 ctx;
    secp256k1_sha256_initialize(&ctx);
    secp256k1_sha256_write(&ctx, data, data_len);
    secp256k1_sha256_finalize(&ctx, hash);
    return true;
}

// Normally we take SHA256(SHA256(data))
bool builtins::get_hashed_2_data(uint8_t *data, size_t data_len,
			         uint8_t hash[32])
{
    secp256k1_sha256 ctx;
    secp256k1_sha256_initialize(&ctx);
    secp256k1_sha256_write(&ctx, data, data_len);
    secp256k1_sha256_finalize(&ctx, hash);
    secp256k1_sha256_initialize(&ctx);
    secp256k1_sha256_write(&ctx, hash, 32);
    secp256k1_sha256_finalize(&ctx, hash);

    return true;
}

bool builtins::get_hashed_term(interpreter_base &interp, const term data,
			       uint8_t hash[32], size_t count)
{
    assert(count == 1 || count == 2);
  
    if (data.tag() == tag_t::BIG) {
	auto &big_data = reinterpret_cast<const big_cell &>(data);
	big_iterator bi = interp.get_heap().begin(big_data);
	big_iterator bi_end = interp.get_heap().end(big_data);

	// If the data size is exactly 32 bytes, then don't hash anything.
	// Just return the data as is. This enables compatibility with
	// computing signatures over data that has already been hashed
	// manually.
	size_t data_size = bi_end - bi;
	if (data_size == 32) {
	    for (size_t i = 0; i < data_size; i++, ++bi) {
		hash[i] = *bi;
	    }
	    return true;
	}

	// If the data is a bignum with more (or less) than 32 bytes, then
	// hash the raw data (excluding any metadata.)
	static const size_t BUFFER_CAPACITY = 32;
	uint8_t buffer[BUFFER_CAPACITY];
	size_t buffer_len = 0;

	secp256k1_sha256 ctx;
	secp256k1_sha256_initialize(&ctx);
	while (bi != bi_end) {
	    while (bi != bi_end && buffer_len < sizeof(buffer)) {
		buffer[buffer_len++] = *bi;
		++bi;
	    }
	    secp256k1_sha256_write(&ctx, buffer, buffer_len);
	    buffer_len = 0;
	}
	secp256k1_sha256_finalize(&ctx, hash);

	if (count == 2) {
	    // Apply SHA256 once again.
	    secp256k1_sha256_initialize(&ctx);
	    secp256k1_sha256_write(&ctx, hash, 32);
	    secp256k1_sha256_finalize(&ctx, hash);
	}

	return true;
    }

    // The data is anything but a bignum. We'll serialize the data and
    // then compute the SHA256 hash of it.
    term_serializer ser(interp);
    term_serializer::buffer_t buf;
    ser.write(buf, data);

    if (count == 1) {
        return get_hashed_1_data(&buf[0], buf.size(), hash);
    } else {
        return get_hashed_2_data(&buf[0], buf.size(), hash);      
    }
}

bool builtins::get_hashed_1_term(interpreter_base &interp, const term data,
				 uint8_t hash[32]) {
    return get_hashed_term(interp, data, hash, 1);
}

bool builtins::get_hashed_2_term(interpreter_base &interp, const term data,
				 uint8_t hash[32]) {
    return get_hashed_term(interp, data, hash, 2);
}

bool builtins::compute_signature(interpreter_base &interp,
				 uint8_t hashed_data[32],
				 private_key &priv_raw,
				 uint8_t signature[64])
{
    auto &ctx = get_ctx(interp);

    secp256k1_ecdsa_signature sig;
    if (secp256k1_ecdsa_sign(ctx, &sig, hashed_data, &priv_raw[0],
			      nullptr, nullptr) != 1) {
	return false;
    }

    if (secp256k1_ecdsa_signature_serialize_compact(ctx, &signature[0], &sig) != 1) {
	return false;
    }

    return true;
}

bool builtins::compute_signature(interpreter_base &interp, const term data,
				 const term privkey, term &out_signature)
{
    private_key rawkey(private_key::NO_KEY, get_ctx(interp));
    if (!get_private_key(interp, privkey, rawkey)) {
	return false;
    }

    uint8_t hash[32];
    if (!get_hashed_2_term(interp, data, hash)) {
	return false;
    }

    uint8_t signature[64];
    compute_signature(interp, hash, rawkey, signature);

    // Generate a new private key. We'll use the bitcoin private key
    // format where first byte is 0x80 followed by 
    
    term bigsig = interp.new_big(64*8);
    interp.set_big(bigsig, signature, 64);

    out_signature = bigsig;
    
    return true;
}

bool builtins::get_signature_data(interpreter_base &interp, term big0, uint8_t sign[64])
{
    if (big0.tag() != tag_t::BIG) {
	return false;
    }

    auto &big = reinterpret_cast<const big_cell &>(big0);
    size_t nbits = interp.num_bits(big);
    if (nbits != 512) {
	return false;
    }
    interp.get_big(big, &sign[0], nbits/8);
    return true;
}

bool builtins::verify_signature(interpreter_base &interp,
				uint8_t hash[32],
				public_key &pubkey,
				uint8_t sign_data[64])
{
    auto &ctx = get_ctx(interp);

    secp256k1_pubkey pubkey1;
    if (secp256k1_ec_pubkey_parse(ctx, &pubkey1, &pubkey[0], 33) != 1) {
	return false;
    }

    secp256k1_ecdsa_signature sig;
    if (secp256k1_ecdsa_signature_parse_compact(ctx, &sig, sign_data) != 1) {
	return false;
    }

    if (secp256k1_ecdsa_verify(ctx, &sig, hash, &pubkey1) != 1) {
	return false;
    }
    return true;
}

bool builtins::verify_signature(interpreter_base &interp,
				const term data,
				const term pubkey,
				const term signature)
{
    public_key pubkey_raw;
    if (!get_public_key(interp, pubkey, pubkey_raw)) {
	return false;
    }

    uint8_t sign_data[64];
    if (!get_signature_data(interp, signature, sign_data)) {
	return false;
    }

    uint8_t hash[32];
    if (!get_hashed_2_term(interp, data, hash)) {
	return false;
    }

    return verify_signature(interp, hash, pubkey_raw, sign_data);
}

bool builtins::sign_3(interpreter_base &interp, size_t arity, term args[] )
{
    if (args[0].tag().is_ref()) {
	throw interpreter_exception_not_sufficiently_instantiated(
	  "ec:sign/3: Missing private key as first argument.");
    }

    term out;
    if (!compute_signature(interp, args[1], args[0], out)) {
        return false;
    }
    bool r = interp.unify(args[2], out);

    return r;
}

bool builtins::validate_3(interpreter_base &interp, size_t arity, term args[] )
{
    if (args[0].tag().is_ref()) {
	throw interpreter_exception_not_sufficiently_instantiated(
	  "ec:validate/3: Missing public key as first argument.");
    }
    if (args[2].tag().is_ref()) {
	throw interpreter_exception_not_sufficiently_instantiated(
	  "ec:validate/3: Missing signature as third argument.");
    }
    if (args[2].tag() != tag_t::BIG) {
         throw interpreter_exception_wrong_arg_type(
	    "ec:validate/3: Third argument must be a signature; was "
	    + interp.to_string(args[2]));
    }
    
    if (!verify_signature(interp, args[1], args[0], args[2])) {
        return false;
    }
    return true;
}

bool builtins::hash_2(interpreter_base &interp, size_t arity, term args[] )
{
    static const con_cell HASH("$hash", 1);

    if (args[0].tag() == tag_t::STR) {
        if (interp.functor(args[0]) == HASH) {
	    term hash_arg = interp.arg(args[0], 0);
	    if (hash_arg.tag() != tag_t::BIG) {
	        throw interpreter_exception_wrong_arg_type("hash/2: Hash argument must be a bignum; was " + interp.to_string(hash_arg));
	    }
	    return interp.unify(args[1], hash_arg);
        }
    }

    uint8_t hashed[32];
    if (!get_hashed_2_term(interp, args[0], hashed)) {
        return false;
    }

    auto big = interp.new_big(RAW_HASH_SIZE*8);
    interp.set_big(big, hashed, RAW_HASH_SIZE);
    
    bool r =  interp.unify(args[1], big);
    
    return r;
}

bool builtins::compute_pedersen_commit(interpreter_base &interp,
				       const term blinding,
				       const term value,
				       uint8_t commit_raw[33])
{
    private_key blinding_raw(private_key::NO_KEY, get_ctx(interp));

    if (!get_private_key(interp, blinding, blinding_raw)) {
	return false;
    }
    if (value.tag() != tag_t::INT) {
	return false;
    }
    uint64_t value_raw = reinterpret_cast<const int_cell &>(value).value();

    auto &ctx = get_ctx(interp);

    secp256k1_pedersen_commitment commit;
    if (secp256k1_pedersen_commit(ctx, &commit, &blinding_raw[0], value_raw,
				  secp256k1_generator_h) != 1) {
	return false;
    }

    if (secp256k1_pedersen_commitment_serialize(ctx, commit_raw, &commit) != 1) {
	return false;
    }

    return true;
}

bool builtins::pproof_3(interpreter_base &interp,
			size_t arity,
			term args[])
{
    using namespace prologcoin::common;

    const term blinding = args[0];
    const term value = args[1];

    private_key blinding_raw(private_key::NO_KEY, get_ctx(interp));

    if (!get_private_key(interp, blinding, blinding_raw)) {
	return false;
    }
    if (value.tag() != tag_t::INT) {
	return false;
    }

    uint64_t value_raw = reinterpret_cast<const int_cell &>(value).value();
    static_cast<void>(value_raw);

    auto &ctx = get_ctx(interp);

    // Generate random value
    private_key b1(private_key::NO_KEY, ctx), b2(private_key::NO_KEY, ctx), b3(private_key::NO_KEY, ctx);

    bool ok = false;

    // The original commit
    secp256k1_pedersen_commitment commit;
    if (secp256k1_pedersen_commit(ctx, &commit, &blinding_raw[0], value_raw, secp256k1_generator_h) != 1) {
	return false;
    }

    uint8_t commit_raw[33];
    secp256k1_pedersen_commitment_serialize(ctx, commit_raw, &commit);

    uint8_t commit1_raw[33], commit2_raw[33], commit3_raw[33];
    secp256k1_pedersen_commitment commit1, commit2, commit3;

    while (!ok) {
        b1.create_new(ctx);
        b2.create_new(ctx);

	// Compute b3 = b - b1 - b2
	uint8_t *bs[] = { &blinding_raw[0], &b1[0], &b2[0] };
	if (secp256k1_pedersen_blind_sum(ctx, &b3[0], bs, 3, 1) != 1) {
	    continue;
	}

	uint64_t v1 = random::next_int(value_raw);
	uint64_t v2 = value_raw - v1;

	// Create pedersen commit b1*G+v1*H + b2*G+v2*H

	if (secp256k1_pedersen_commit(ctx, &commit1, &b1[0], v1, secp256k1_generator_h) != 1) {
	    continue;
	}
	if (secp256k1_pedersen_commit(ctx, &commit2, &b2[0], v2, secp256k1_generator_h) != 1) {
	    continue;
	}
	if (secp256k1_pedersen_commit(ctx, &commit3, &b3[0], 0, secp256k1_generator_h) != 1) {
	    continue;
	}

	secp256k1_pedersen_commitment_serialize(ctx, commit1_raw, &commit1);
	secp256k1_pedersen_commitment_serialize(ctx, commit2_raw, &commit2);
	secp256k1_pedersen_commitment_serialize(ctx, commit3_raw, &commit3);

	ok = true;
    }

    // Safety Check that Pedersen commit works as intended
    secp256k1_pedersen_commitment * pos[] = { &commit };
    secp256k1_pedersen_commitment * neg[] = { &commit1, &commit2, &commit3 };
    if (secp256k1_pedersen_verify_tally(ctx, pos, 1, neg, 3) != 1) {
	return false;
    }

    public_key pub_b3;
    
    // We need the public key for b3
    if (!compute_public_key(interp, b3, pub_b3)) {
	return false;
    }

    // Sign something with b3
    uint8_t signature[64];
    uint8_t msg[8];
    utime ut = utime::now();
    ut.to_bytes(msg);
    uint8_t hash[32];
    get_hashed_2_data(msg, sizeof(msg), hash);
    if (!compute_signature(interp, hash, b3, signature)) {
	return false;
    }

    // At this point we have commit     = original commit
    //                       commit1    = b1*G+v1*H,
    //                       commit2    = b2*G+v2*H
    //                       public key = b3*G
    //                       utime
    //                       signature(hash(utime)) with b3

    // This is all the information we need for pproof/6

    term term_commit = new_bignum(interp, commit_raw, 33);
    term term_commit1 = new_bignum(interp, commit1_raw, 33);
    term term_commit2 = new_bignum(interp, commit2_raw, 33);
    term term_pubkey = new_bignum(interp, &pub_b3[0], public_key::SIZE);
    term term_utime = int_cell(static_cast<int64_t>(ut.in_us()));
    term term_signature = new_bignum(interp, signature, 64);

    term pproof = interp.new_term(con_cell("pproof", 6),
			  { term_commit, term_commit1, term_commit2,
			    term_pubkey,term_utime, term_signature });

    return interp.unify(args[2], pproof);
}

bool builtins::pverify_1(interpreter_base &interp, size_t arity, term args[])
{
    using namespace prologcoin::common;

    term pproof = args[0];

    // Check that the functor is pproof/6
    if (pproof.tag() != tag_t::STR ||
	interp.functor(pproof) != con_cell("pproof",6)) {
	return false;
    }

    term term_commit = interp.arg(pproof, 0);
    term term_commit1 = interp.arg(pproof, 1);
    term term_commit2 = interp.arg(pproof, 2);
    term term_pubkey = interp.arg(pproof, 3);
    term term_utime = interp.arg(pproof, 4);
    term term_signature = interp.arg(pproof, 5);

    uint8_t commit_raw[33];
    uint8_t commit1_raw[33];
    uint8_t commit2_raw[33];
    public_key pubkey_raw;
    uint8_t signature_raw[64];

    size_t n1 = 33, n2 = 33, n3 = 33, n4 = 33, n5 = 64;

    if (!get_bignum(interp, term_commit, commit_raw, n1) ||
	!get_bignum(interp, term_commit1, commit1_raw, n2) ||
	!get_bignum(interp, term_commit2, commit2_raw, n3) ||
	!get_bignum(interp, term_pubkey, &pubkey_raw[0], n4) ||
	!get_bignum(interp, term_signature, signature_raw, n5)) {
	return false;
    }
    if (n1 != 33 || n2 != 33 || n3 != 33 || n4 != 33 || n5 != 64) {
	return false;
    }

    if (term_utime.tag() != tag_t::INT) {
	return false;
    }
    uint64_t ut_val = static_cast<uint64_t>(reinterpret_cast<const int_cell &>(term_utime).value());

    auto &ctx = get_ctx(interp);

    secp256k1_pedersen_commitment commit, commit1, commit2, commit3;

    if (secp256k1_pedersen_commitment_parse(ctx, &commit, commit_raw) != 1) {
	return false;
    }
    if (secp256k1_pedersen_commitment_parse(ctx, &commit1, commit1_raw) != 1) {
	return false;
    }
    if (secp256k1_pedersen_commitment_parse(ctx, &commit2, commit2_raw) != 1) {
	return false;
    }

    if (secp256k1_pedersen_commitment_load_pubkey(ctx, &commit3, &pubkey_raw[0]) != 1) {
    	return false;
    }
    uint8_t commit3_raw[33];
    secp256k1_pedersen_commitment_serialize(ctx, commit3_raw, &commit3);

    //
    // Positive commit
    //
    secp256k1_pedersen_commitment * positive_commits[] = { &commit };

    //
    // Negative commits
    //
    secp256k1_pedersen_commitment * negative_commits[] = { &commit1, &commit2, &commit3 };

    //
    // Verify that the Pedersen commitments add up to zero
    //
    if (secp256k1_pedersen_verify_tally( ctx, positive_commits, 1,
					 negative_commits, 3) != 1) {
	return false;
    }

    //
    // Verify signature
    //

    // First hash message which is utime
    utime ut(ut_val);
    uint8_t msg[8];
    ut.to_bytes(msg);
    uint8_t hash[32];
    get_hashed_2_data(msg, sizeof(msg), hash);
    if (!verify_signature(interp, hash, pubkey_raw, signature_raw)) {
	return false;
    }

    return true;
}

void builtins::pedersen_test()
{
    auto *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

    struct cleanup {
	cleanup(secp256k1_context *ctx) : ctx_(ctx) { }
	~cleanup() { secp256k1_context_destroy(ctx_); }
	secp256k1_context *ctx_;
    } cleanup_(ctx);

    uint8_t b1[32] = { 0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
    uint8_t b2[32] = { 0x20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 };

    uint8_t * bs[] = { &b1[0], &b2[0] };
    uint8_t bout[32];

    if (secp256k1_pedersen_blind_sum(ctx, bout, bs, 2, 2) != 1) {
	std::cout << "Failed blind sum\n";
	return;
    }

    std::cout << "b1: " << hex::to_string(b1, 32) << std::endl;
    std::cout << "b2: " << hex::to_string(b2, 32) << std::endl;
    std::cout << "bo: " << hex::to_string(bout, 32) << std::endl;

    std::cout << "Everything is ok\n";
    
}

bool builtins::pubkey_tweak_add_3(interpreter_base &interp, size_t arity, term args[] )
{
    secp256k1_pubkey pubkey;
    term pubkey_term = args[0];
    public_key pubkey_data;

    auto &ctx = get_ctx(interp);
  
    if (!get_public_key(interp, pubkey_term, pubkey_data) ||
        secp256k1_ec_pubkey_parse(ctx,&pubkey,&pubkey_data[0],RAW_KEY_SIZE+1) != 1) {
       throw interpreter_exception_not_public_key("pubkey_tweak_add/3: First argument is not a public key; was " + interp.to_string(pubkey_term));
   }

    term tweak_term = args[1];
    private_key tweak_data(private_key::NO_KEY, ctx);
    if (!get_private_key(interp, tweak_term, tweak_data)) {
       throw interpreter_exception_wrong_arg_type("pubkey_tweak_add/3: Second argument is not a private key (or secret); was " + interp.to_string(tweak_term));      
    }

    if (secp256k1_ec_pubkey_tweak_add(ctx, &pubkey, &tweak_data[0]) != 1) {
        throw interpreter_exception_tweak("pubkey_tweak_add/3: Failed while tweaking public key.");
    }

    size_t result_raw_len = public_key::SIZE;
    public_key result_raw;
    int r = secp256k1_ec_pubkey_serialize(ctx, &result_raw[0], &result_raw_len, &pubkey, SECP256K1_EC_COMPRESSED);
    if (r != 1 || result_raw_len != public_key::SIZE) {
        throw interpreter_exception_tweak("pubkey_tweak_add/3: Failed when serializing tweaked public key.");      
    }
    
    term result_pubkey = create_public_key(interp, result_raw);
    return interp.unify(args[2], result_pubkey);
}

bool builtins::privkey_tweak_add_3(interpreter_base &interp, size_t arity, term args[] )
{
    term privkey_term = args[0];

    auto &ctx = get_ctx(interp);

    private_key privkey_data(private_key::NO_KEY, ctx);

    bool with_checksum = false;
    if (!get_private_key(interp, privkey_term, privkey_data, with_checksum)) {
       throw interpreter_exception_wrong_arg_type("privkey_tweak_add/3: First argument is not a private key (or secret); was " + interp.to_string(privkey_term));
   }

    term tweak_term = args[1];
    private_key tweak_data(private_key::NO_KEY, ctx);
    if (!get_private_key(interp, tweak_term, tweak_data)) {
       throw interpreter_exception_wrong_arg_type("privkey_tweak_add/3: Second argument is not a private key (or secret); was " + interp.to_string(tweak_term));      
    }

    if (secp256k1_ec_privkey_tweak_add(ctx, &privkey_data[0], &tweak_data[0]) != 1) {
        throw interpreter_exception_tweak("pubkey_tweak_add/3: Failed while tweaking private key.");
    }

    term result_privkey = create_private_key(interp, privkey_data, with_checksum);
    return interp.unify(args[2], result_privkey);
}

bool builtins::musig_combine_3(interpreter_base &interp, size_t arity, term args[])
{
    if (!interp.is_list(args[0])) {
        throw interpreter_exception_not_list("musig_combine/3: First argument is not a list (of public keys); was " + interp.to_string(args[0]));
    }

    auto &ctx = get_ctx(interp);
    
    term l = args[0];
    size_t n = interp.list_length(l);
    std::vector<secp256k1_pubkey> pubkeys;
    while (!interp.is_empty_list(l)) {
        secp256k1_pubkey pubkey;
        term pubkey_term = interp.arg(l, 0);
	public_key pubkey_data;
	if (!get_public_key(interp, pubkey_term, pubkey_data) ||
	     secp256k1_ec_pubkey_parse(ctx, &pubkey, &pubkey_data[0], public_key::SIZE) != 1) {
	     throw interpreter_exception_not_public_key("musig_combine/3: Not a public key: " + interp.to_string(pubkey_term));
        }
        pubkeys.push_back(pubkey);
	l = interp.arg(l, 1);
    }

    // We have all the public keys. Call combiner.
    secp256k1_pubkey combined_pubkey;
    uint8_t combined_hash32[32];
    int r = secp256k1_musig_pubkey_combine(ctx, ctx.scratch(),
					   &combined_pubkey, combined_hash32,
					   &pubkeys[0], n);
    if (!r) {
        return false;
    }

    // Create bignums for combined pubkey and hash32

    size_t pub_raw_len = RAW_KEY_SIZE+1;
    uint8_t pub_raw[RAW_KEY_SIZE+1];
    r = secp256k1_ec_pubkey_serialize(ctx, &pub_raw[0], &pub_raw_len, &combined_pubkey, SECP256K1_EC_COMPRESSED);
    if (!r || pub_raw_len != RAW_KEY_SIZE+1) {
       return false;
    }

    term term_pubkey = new_bignum(interp, pub_raw, RAW_KEY_SIZE+1);
    term term_hash32 = new_bignum(interp, combined_hash32, 32);

   return interp.unify(args[1], term_pubkey) &&
          interp.unify(args[2], term_hash32);
}

bool builtins::musig_verify_3(interpreter_base &interp, size_t arity, term args[])
{
    auto &ctx = get_ctx(interp);

    uint8_t hash[RAW_HASH_SIZE];
    if (!get_hashed_2_term(interp, args[0], hash)) {
        throw interpreter_exception_wrong_arg_type("musig_verify/3: Couldn't compute hash of first argument: " + interp.to_string(args[0]));
    }
    
    secp256k1_pubkey pubkey;
    public_key pubkey_data;
    if (!get_public_key(interp, args[1], pubkey_data) ||
	secp256k1_ec_pubkey_parse(ctx, &pubkey, &pubkey_data[0], public_key::SIZE) != 1) {
        throw interpreter_exception_wrong_arg_type("musig_verify/3: Second argument is not a public key; was " + interp.to_string(args[1]));
    }

    uint8_t sig_data[RAW_SIG_SIZE];
    size_t n = RAW_SIG_SIZE;
    if (!get_bignum(interp, args[2], sig_data, n)) {
        throw interpreter_exception_wrong_arg_type("musig_verify/3: Third argument is not a schnorr signature; was " + interp.to_string(args[2]));
    }

    secp256k1_schnorrsig sig;
    if (secp256k1_schnorrsig_parse(ctx, &sig, sig_data) != 1) {
        throw interpreter_exception_wrong_arg_type("musig_verify/3: Could not parse schnorr signature in third argument: " + interp.to_string(args[2]));
    }

    if (secp256k1_schnorrsig_verify(ctx, &sig, hash, &pubkey) != 1) {
	return false;
    }

    return true;
}

bool builtins::musig_secret_7(interpreter_base &interp, size_t arity, term args[])
{
    auto &ctx = get_ctx(interp);

    uint8_t hash[RAW_HASH_SIZE];
    if (!get_hashed_2_term(interp, args[0], hash)) {
        throw interpreter_exception_wrong_arg_type("musig_secret/7: Couldn't compute hash of first argument: " + interp.to_string(args[0]));
    }
    
    uint8_t final_sig_data[RAW_SIG_SIZE];
    size_t final_sig_data_size = RAW_SIG_SIZE;
    if (!get_bignum(interp, args[1], final_sig_data, final_sig_data_size)) {
        throw interpreter_exception_wrong_arg_type("musig_secret/7: Second argument is not a schnorr signature; was " + interp.to_string(args[1]));
    }

    secp256k1_schnorrsig final_sig;
    if (secp256k1_schnorrsig_parse(ctx, &final_sig, final_sig_data) != 1) {
        throw interpreter_exception_wrong_arg_type("musig_secret/7: Could not parse schnorr signature in second argument: " + interp.to_string(args[1]));
    }

    term nonces = args[2];
    if (!interp.is_list(nonces)) {
        throw interpreter_exception_not_list("musig_secret/7: Third argument is not a list of nonces; was " + interp.to_string(nonces));
    }
    size_t n = interp.list_length(nonces);
    if (n > MAX_SIGNERS) {
        std::stringstream msg;
	msg << "musig_secret/7: List of nonces exceeds maximum of ";
	msg << MAX_SIGNERS << "; was of length " << n;
	throw interpreter_exception_not_list(msg.str());
    }

    std::unique_ptr<secp256k1_pubkey> nonces_data(new secp256k1_pubkey[n]);
    std::unique_ptr<uint8_t> nonce_commitments_data(new uint8_t[n*RAW_HASH_SIZE]);    
    std::unique_ptr<uint8_t *> nonce_commitments_ptr(new uint8_t * [n]);

    size_t i = 0;
    while (nonces != interpreter_base::EMPTY_LIST) {
        term nonce_term = interp.arg(nonces, 0);
	public_key nonce_raw;
	bool r = get_public_key(interp, nonce_term, nonce_raw);
	if (!r || secp256k1_ec_pubkey_parse(ctx, &(nonces_data.get())[i], &nonce_raw[0],public_key::SIZE) != 1) {
  	    std::stringstream msg;
	    msg << "musig_secret/7: Third argument is not a list of nonces; element " << (i+1) << " was " << interp.to_string(nonce_term);
            throw interpreter_exception_not_list(msg.str());
	}
	nonces = interp.arg(nonces,1);
	
	(nonce_commitments_ptr.get())[i] = &(nonce_commitments_data.get())[i*RAW_HASH_SIZE];
	get_hashed_1_data(&nonce_raw[0], public_key::SIZE,
	                  nonce_commitments_ptr.get()[i]);
	i++;
    }

    term pubkeys_list = args[3];
    if (!interp.is_list(pubkeys_list)) {
        throw interpreter_exception_not_list("musig_secret/7: Fourth argument is not a list (of public keys); was " + interp.to_string(args[0]));
    }
    
    if (n != interp.list_length(pubkeys_list)) {
        std::stringstream msg;
	msg << "musig_secret/7: Number of pubkeys does not match ";
	msg << "nonces; expected " << n << " but was ";
	msg << interp.list_length(pubkeys_list);
	throw interpreter_exception_musig(msg.str());
    }
    
    std::vector<secp256k1_pubkey> pubkeys;
    while (!interp.is_empty_list(pubkeys_list)) {
        secp256k1_pubkey pubkey;
        term pubkey_term = interp.arg(pubkeys_list, 0);
	public_key pubkey_data;
	if (!get_public_key(interp, pubkey_term, pubkey_data) ||
	     secp256k1_ec_pubkey_parse(ctx, &pubkey, &pubkey_data[0], public_key::SIZE) != 1) {
	     throw interpreter_exception_not_public_key("musig_secret/7: Not a public key: " + interp.to_string(pubkey_term));
        }
        pubkeys.push_back(pubkey);
	pubkeys_list = interp.arg(pubkeys_list, 1);
    }

    // Partial sigs

    term partial_sigs_list = args[4];
    if (!interp.is_list(partial_sigs_list)) {
        throw interpreter_exception_not_list("musig_secret/7: Fifth argument is not a list of partial signatures; was " + interp.to_string(partial_sigs_list));
    }
    
    if (n != interp.list_length(partial_sigs_list)) {
        std::stringstream msg;
	msg << "musig_secret/7: Number of partial signatures does not match ";
	msg << "number of public keys; expected " << n << " signatures but ";
	msg << "was " << interp.list_length(partial_sigs_list);
	throw interpreter_exception_musig(msg.str());
    }

    std::unique_ptr<secp256k1_musig_partial_signature> psigs(new secp256k1_musig_partial_signature[n]);
    i = 0;
    while (!interp.is_empty_list(partial_sigs_list)) {
        term signature_term = interp.arg(partial_sigs_list, 0);
	uint8_t signature[RAW_KEY_SIZE];
	size_t signature_size = RAW_KEY_SIZE;
	bool r = get_bignum(interp, signature_term, signature, signature_size);
	if (!r || signature_size != RAW_KEY_SIZE) {
  	    std::stringstream msg;
	    msg << "musig_secret/4";
	    msg << ": Second argument is not a list of partial signatures; element " << (i+1) << " was " << interp.to_string(signature_term);
            throw interpreter_exception_not_list(msg.str());
	}
	partial_sigs_list = interp.arg(partial_sigs_list,1);

	if (secp256k1_musig_partial_signature_parse(ctx, &(psigs.get())[i], signature) != 1) {
	    std::stringstream msg;
	    msg << "musig_secret/7";
 	    msg << ": Partial signature could not be parsed: ";
	    msg << interp.to_string(signature_term);
  	    throw interpreter_exception_wrong_arg_type(msg.str());
	}
	i++;
    }

    // Get adaptor
    secp256k1_pubkey adaptor;
    term adaptor_term = args[5];
    public_key adaptor_data;
    bool r = get_public_key(interp, adaptor_term, adaptor_data);
    if (!r || secp256k1_ec_pubkey_parse(ctx, &adaptor, &adaptor_data[0], public_key::SIZE) != 1) {
        std::stringstream msg;
	msg << "musig_secret/" << arity;
	msg << ": Sixth argument is not an adaptor (public key); was";
	msg << interp.to_string(adaptor_term);
        throw interpreter_exception_not_list(msg.str());
    }
    
    secp256k1_pubkey combined_pubkey;
    uint8_t combined_hash32[32];
    if (secp256k1_musig_pubkey_combine(ctx, ctx.scratch(),
				       &combined_pubkey, combined_hash32,
				       &pubkeys[0], n) != 1) {
      throw interpreter_exception_musig("musig_secret/7: Failed when combining public keys.");
    }

    std::unique_ptr<secp256k1_musig_session_signer_data> sigdata(new secp256k1_musig_session_signer_data[n]);
    
    secp256k1_musig_session session;
    if (secp256k1_musig_session_initialize_verifier(
	    ctx, &session, sigdata.get(), hash,
	    &combined_pubkey, combined_hash32,
	    nonce_commitments_ptr.get(), n) != 1) {
      throw interpreter_exception_musig("musig_secret/7: Failed when initializing internal verifier session.");
    }

    // Set nonces
    for (i = 0; i < n; i++) {
        if (secp256k1_musig_set_nonce(ctx, &sigdata.get()[i], &(nonces_data.get())[i]) != 1) {
        std::stringstream msg;
	msg << "musig_secret/" << arity;
	msg << ": Unable to set nonce number " << i << " failed";
	throw interpreter_exception_musig(msg.str());
      }
    }

    int nonce_is_negated = 0;
    // Combine nonces
    if (secp256k1_musig_session_combine_nonces(ctx, &session,
					       sigdata.get(), n,
					       &nonce_is_negated,
					       &adaptor) != 1) {
        std::stringstream msg;
	msg << "musig_secret/" << arity;
	msg << ": Unable to combine nonces";
	throw interpreter_exception_musig(msg.str());      
    }
    
    // Iterate and verify each partial signature
    for (i = 0; i < n; i++) {
       int r = secp256k1_musig_partial_sig_verify(
	  ctx, &session, &(sigdata.get())[i], &(psigs.get())[i], &pubkeys[i]);
       if (r != 1) {
	   return false;
       }
    }

    uint8_t secret[RAW_KEY_SIZE];
    memset(secret, 0, sizeof(secret));
    
    if (secp256k1_musig_extract_secret_adaptor(
         ctx, secret, &final_sig, psigs.get(), n, nonce_is_negated) != 1) {
         return false;
    }
    
    term t = new_bignum(interp, secret, RAW_KEY_SIZE);
    return interp.unify( args[6], t);
}

musig_session * builtins::get_musig_session(interpreter_base &interp, term arg) {
    if (arg.tag() != tag_t::STR || interp.functor(arg) != con_cell("$musig",1)) {
        return nullptr;
    }
    auto musig_id = interp.arg(arg,0);
    if (musig_id.tag() != tag_t::INT) {
        return nullptr;
    }
    auto id = static_cast<size_t>(musig_id.value());

    auto &env = get_musig_env(interp);

    auto *session = env.get_session(id);
    return session;
}

bool builtins::musig_start_7(interpreter_base &interp, size_t arity, term args[])
{
    auto &ctx = get_ctx(interp);

    if (!args[0].tag().is_ref()) {
        throw interpreter_exception_wrong_arg_type("musig_start/7: First argument must be an unbound variable; was " + interp.to_string(args[0]));
    }

    public_key pubkey_raw;
    if (!get_public_key(interp, args[1], pubkey_raw)) {
        throw interpreter_exception_wrong_arg_type("musig_start/7: Second argument must be a public key; was " + interp.to_string(args[1]));
    }
    secp256k1_pubkey combined_pubkey;
    if (secp256k1_ec_pubkey_parse(ctx, &combined_pubkey, &pubkey_raw[0], public_key::SIZE) != 1) {
        throw interpreter_exception_wrong_arg_type("musig_start/7: Second argument must be a valid public key; was " + interp.to_string(args[1]));
    }

    uint8_t pubhash[RAW_HASH_SIZE];
    size_t n = RAW_HASH_SIZE;
    if (!get_bignum(interp, args[2], &pubhash[0], n) || n != RAW_HASH_SIZE) {
        throw interpreter_exception_wrong_arg_type("musig_start/7: Third argument must be a hash (of all public keys); was " + interp.to_string(args[2]));
    }

    if (args[3].tag() != tag_t::INT) {
        throw interpreter_exception_wrong_arg_type("musig_start/7: Fourth argument must be an integer index (my index position of public keys); was " + interp.to_string(args[3]));
    }
    int64_t my_index = reinterpret_cast<int_cell &>(args[3]).value();

    if (args[4].tag() != tag_t::INT) {
        throw interpreter_exception_wrong_arg_type("musig_start/7: Fifth argument must be an integer denoting number of signers; was " + interp.to_string(args[4]));
    }
    int64_t num_signers = reinterpret_cast<int_cell &>(args[4]).value();

    if (num_signers <= 0 || static_cast<size_t>(num_signers) > MAX_SIGNERS) {
        throw interpreter_exception_wrong_arg_type("musig_start/7: Number of signers must be 1..1024; was " + interp.to_string(args[4]));       
    }

    private_key privkey(private_key::NO_KEY, ctx);
    if (!get_private_key(interp, args[5], privkey)) {
        throw interpreter_exception_wrong_arg_type("musig_start/7: Sixth argument must be a private key for signing; was " + interp.to_string(args[5]));
    }

    uint8_t datahash[RAW_HASH_SIZE];
    if (!get_hashed_2_term(interp, args[6], datahash)) {
        return false;
    }

    auto &env = get_musig_env(interp);
    
    musig_session *session = env.new_session(num_signers);
    if (!session->init(my_index, combined_pubkey, pubhash, privkey, datahash)) {
        env.delete_session(session->id());
        return false;
    }

    auto musig_key = interp.new_term(con_cell("$musig",1), {int_cell(session->id())});

    return interp.unify(args[0], musig_key);
}

bool builtins::musig_set_public_key_3(interpreter_base &interp, size_t arity, term args[])
{
    auto *session = get_musig_session(interp, args[0]);
    if (session == nullptr) {
        throw interpreter_exception_wrong_arg_type("musig_set_public_key/3: First argument must be MuSig session; was " + interp.to_string(args[0]));
    }

    if (args[1].tag() != tag_t::INT) {
        throw interpreter_exception_wrong_arg_type("musig_set_public_key/3: Second argument must be an index; was " + interp.to_string(args[1]));
    }

    int64_t val = reinterpret_cast<int_cell &>(args[1]).value();
    if (val < 0 || static_cast<size_t>(val) >= session->num_signers()) {
        throw interpreter_exception_wrong_arg_type("musig_set_public_key/3: Index is out of range; was " + interp.to_string(args[1]));
    }

    size_t index = static_cast<size_t>(val);

    auto &ctx = get_ctx(interp);
    
    term pubkey_term = args[2];
    secp256k1_pubkey pubkey;
    public_key pubkey_data;
    bool r = get_public_key(interp, pubkey_term, pubkey_data);
    if (!r || secp256k1_ec_pubkey_parse(ctx, &pubkey, &pubkey_data[0], public_key::SIZE) != 1) {
      std::stringstream msg;
	    msg << "musig_set_public_key/" << arity;
	    msg << ": Third argument is not a public key; was";
	    msg << interp.to_string(pubkey_term);
            throw interpreter_exception_not_list(msg.str());
    }

    session->set_public_key(index, pubkey);

    return true;
}

bool builtins::musig_nonce_commit_2(interpreter_base &interp, size_t arity, term args[])
{
    auto *session = get_musig_session(interp, args[0]);

    if (session == nullptr) {
        throw interpreter_exception_wrong_arg_type("musig_nonce_commit/2: First argument must be MuSig session; was " + interp.to_string(args[0]));
    }

    uint8_t nonce_commit[RAW_HASH_SIZE];
    session->get_nonce_commitment(nonce_commit);

    term big = new_bignum(interp, nonce_commit, RAW_HASH_SIZE);
    return interp.unify(args[1], big);
}

bool builtins::musig_prepare_3(interpreter_base &interp, size_t arity, term args[]) {
    auto *session = get_musig_session(interp, args[0]);
    if (session == nullptr) {
        throw interpreter_exception_wrong_arg_type("musig_prepare/3: First argument must be MuSig session; was " + interp.to_string(args[0]));
    }

    term nonce_commits = args[1];
    if (!interp.is_list(nonce_commits)) {
        throw interpreter_exception_not_list("musig_prepare/3: Second argument is not a list of nonce commitments; was " + interp.to_string(nonce_commits));
    }
    size_t n = interp.list_length(nonce_commits);
    if (n != session->num_signers()) {
        std::stringstream msg;
        msg << "musig_prepare/3: Number of nonce commitments does not match number of signers. ";
        msg << "Number of signers is " << session->num_signers() << " and number of commitments is " << n << ".";
        throw interpreter_exception_not_list(msg.str());
    }

    std::unique_ptr<uint8_t> nonce_commitments_data(new uint8_t[n*RAW_HASH_SIZE]);
    std::unique_ptr<uint8_t *> nonce_commitments_ptr(new uint8_t * [n]);

    size_t i = 0;
    while (nonce_commits != interpreter_base::EMPTY_LIST) {
        term nonce_commit = interp.arg(nonce_commits, 0);
	size_t n = RAW_HASH_SIZE;
	bool r = get_bignum(interp, nonce_commit, &(nonce_commitments_data.get())[i*RAW_HASH_SIZE],n);
	if (!r || n != RAW_HASH_SIZE) {
  	    std::stringstream msg;
	    msg << "musig_prepare/3: Second argument is not a list of nonce commitments; element " << (i+1) << " was " << interp.to_string(nonce_commit);
            throw interpreter_exception_not_list(msg.str());
	}
	nonce_commits = interp.arg(nonce_commits,1);
	(nonce_commitments_ptr.get())[i] = &(nonce_commitments_data.get())[i*RAW_HASH_SIZE];
	i++;
    }

    secp256k1_pubkey nonce;
    if (!session->prepare(nonce_commitments_ptr.get(), nonce)) {
        throw interpreter_exception_wrong_arg_type("musig_prepare/3: Couldn't get public nonce. MuSig session could be in an invalid state.");
    }

    auto &ctx = get_ctx(interp);
    size_t nonce_raw_len = public_key::SIZE;
    public_key nonce_raw;
    if (secp256k1_ec_pubkey_serialize(ctx, &nonce_raw[0], &nonce_raw_len,
				      &nonce, SECP256K1_EC_COMPRESSED) != 1) {
        throw interpreter_exception_musig("musig_prepare/3: Couldn't get public nonce. MuSig session could be in an invalid state.");
    }

    term nonce_term = create_public_key(interp, nonce_raw);
    
    return interp.unify(args[2], nonce_term);
}

bool builtins::musig_nonces_3(interpreter_base &interp, size_t arity, term args[]) {
    auto *session = get_musig_session(interp, args[0]);
    if (session == nullptr) {
        std::stringstream msg;
	msg << "musig_nonces/" << arity;
	msg << ": First argument must be MuSig session; was ";
	msg << interp.to_string(args[0]);
        throw interpreter_exception_wrong_arg_type(msg.str());
    }

    term nonces_list = args[1];
    if (!interp.is_list(nonces_list)) {
        std::stringstream msg;
	msg << "musig_nonces/" << arity;
        msg << ": Second argument is not a list of nonces; was ";
	msg << interp.to_string(nonces_list);
	throw interpreter_exception_wrong_arg_type(msg.str());
    }
    size_t n = interp.list_length(nonces_list);
    if (n != session->num_signers()) {
        std::stringstream msg;
        msg << "musig_nonces/" << arity;
	msg << ": Number of nonces does not match number of signers. ";
        msg << "Number of signers is " << session->num_signers() << " and number of nonces is " << n << ".";
        throw interpreter_exception_musig(msg.str());
    }

    auto &ctx = get_ctx(interp);
    size_t i = 0;
    while (nonces_list != interpreter_base::EMPTY_LIST) {
        term nonce_term = interp.arg(nonces_list, 0);
	public_key nonce;
	bool r = get_public_key(interp, nonce_term, nonce);
	if (!r) {
  	    std::stringstream msg;
	    msg << "musig_nonces/" << arity;
	    msg << ": Second argument is not a list of nonces; element " << (i+1) << " was " << interp.to_string(nonce_term);
            throw interpreter_exception_not_list(msg.str());
	}
	nonces_list = interp.arg(nonces_list,1);

        secp256k1_pubkey nonce_pubkey;
	if (secp256k1_ec_pubkey_parse(ctx, &nonce_pubkey, &nonce[0], public_key::SIZE) != 1) {
	    std::stringstream msg;
	    msg << "musig_nonces/" << arity;
 	    msg << ": Nonce is not a public key; was ";
	    msg << interp.to_string(nonce_term);
  	    throw interpreter_exception_wrong_arg_type(msg.str());
	}
	
	session->set_nonce(i, nonce_pubkey);
	i++;
    }

    secp256k1_pubkey adaptor;
    secp256k1_pubkey *adaptor_used = nullptr;
    if (arity >= 3) {
        term adaptor_term = args[2];
	public_key adaptor_data;
	bool r = get_public_key(interp, adaptor_term, adaptor_data);
	if (!r) {
  	    std::stringstream msg;
	    msg << "musig_nonces/" << arity;
	    msg << ": Third argument is not a public key (adaptor); was";
	    msg << interp.to_string(adaptor_term);
            throw interpreter_exception_not_list(msg.str());
	}
	if (secp256k1_ec_pubkey_parse(ctx, &adaptor, &adaptor_data[0], public_key::SIZE) != 1) {
	    std::stringstream msg;
	    msg << "musig_nonces/" << arity;
 	    msg << ": Third argument is not a public key (adaptor); was ";
	    msg << interp.to_string(adaptor_term);	    
  	    throw interpreter_exception_wrong_arg_type(msg.str());
	}
	adaptor_used = &adaptor;
    }
    
    if (!session->combine_nonces(adaptor_used)) {
        std::stringstream msg;
	msg << "musig_nonces/" << arity;	
        msg << ": Couldn't combine nonces. MuSig session was probably invalid.";
	throw interpreter_exception_musig(msg.str());
    }

    return true;
}

bool builtins::musig_partial_sign_2(interpreter_base &interp, size_t arity, term args[]) {
    auto *session = get_musig_session(interp, args[0]);
    if (session == nullptr) {
	throw interpreter_exception_wrong_arg_type(
	   "musig_partial_sign/2: First argument must be MuSig session; was "
	   + interp.to_string(args[0]));
    }

    secp256k1_musig_partial_signature sig;
    if (!session->partial_sign(sig)) {
	throw interpreter_exception_musig(
	  "musig_partial_sign/2: Couldn't compute partial signature.");
    }

    auto &ctx = get_ctx(interp);

    uint8_t signature[RAW_KEY_SIZE];
    if (secp256k1_musig_partial_signature_serialize(ctx, signature, &sig) != 1) {
	throw interpreter_exception_musig(
	  "musig_partial_sign/2: Couldn't serialize partial signature.");
    }

    term t = new_bignum(interp, signature, RAW_KEY_SIZE);

    return interp.unify(args[1], t);
}

bool builtins::musig_partial_sign_adapt_4(interpreter_base &interp, size_t arity, term args[]) {
    auto *session = get_musig_session(interp, args[0]);
    if (session == nullptr) {
	throw interpreter_exception_wrong_arg_type(
	   "musig_partial_sign_adapt/4: First argument must be MuSig session; was "
	   + interp.to_string(args[0]));
    }

    auto &ctx = get_ctx(interp);    
    term partial_signature_term = args[1];
    uint8_t partial_signature_data[RAW_KEY_SIZE];
    size_t partial_signature_size = RAW_KEY_SIZE;
    bool r = get_bignum(interp, partial_signature_term, partial_signature_data,
			partial_signature_size);
    if (!r || partial_signature_size != RAW_KEY_SIZE) {
      std::stringstream msg;
      msg << "musig_partial_sign_adapt/4";
      msg << ": Second argument is not a partial signature; was ";
      msg << interp.to_string(partial_signature_term);
      throw interpreter_exception_not_list(msg.str());
    }

    secp256k1_musig_partial_signature partial_signature;
    if (secp256k1_musig_partial_signature_parse(
	  ctx, &partial_signature, partial_signature_data) != 1) {
      std::stringstream msg;
      msg << "musig_partial_sign_adapt/4";
      msg << ": Partial signature could not be parsed: ";
      msg << interp.to_string(partial_signature_term);
      throw interpreter_exception_wrong_arg_type(msg.str());
    }

    term adaptor_private_term = args[2];
    private_key adaptor_secret(private_key::NO_KEY, ctx);
    if (!get_private_key(interp, adaptor_private_term, adaptor_secret)) {
      std::stringstream msg;
      msg << "musig_partial_sign_adapt/4";
      msg << ": Third argument must be a private key (adaptor secret); was ";
      msg << interp.to_string(adaptor_private_term);
      throw interpreter_exception_wrong_arg_type(msg.str());
    }
    
    if (!session->partial_sign_adapt(partial_signature, &adaptor_secret[0])) {
	throw interpreter_exception_musig(
   	      "musig_partial_sign_adapt/4: Couldn't adapt signature.");
    }

    uint8_t adapted_data[RAW_KEY_SIZE];
    if (secp256k1_musig_partial_signature_serialize(ctx, adapted_data, &partial_signature) != 1) {
	throw interpreter_exception_musig(
	  "musig_partial_sign_adapt/4: Couldn't serialize adapted signature.");
    }

    term t = new_bignum(interp, adapted_data, RAW_KEY_SIZE);

    return interp.unify(args[3], t);
}

bool builtins::musig_final_sign_4(interpreter_base &interp, size_t arity, term args[]) {
    auto *session = get_musig_session(interp, args[0]);
    if (session == nullptr) {
        std::stringstream msg;
	msg << "musig_final_sign/" << arity;
	msg << ": First argument must be MuSig session; was ";
	msg << interp.to_string(args[0]);
        throw interpreter_exception_wrong_arg_type(msg.str());
    }

    term partial_signatures_list = args[1];
    if (!interp.is_list(partial_signatures_list)) {
        std::stringstream msg;
	msg << "musig_final_sign/" << arity;
        msg << ": Second argument is not a list of partial signatures; was ";
	msg << interp.to_string(partial_signatures_list);
	throw interpreter_exception_wrong_arg_type(msg.str());
    }
    size_t n = interp.list_length(partial_signatures_list);
    if (n != session->num_signers()) {
        std::stringstream msg;
        msg << "musig_final_sign/" << arity;
	msg << ": Number of signatures does not match number of signers. ";
        msg << "Number of signers is " << session->num_signers() << " and number of signatures is " << n << ".";
        throw interpreter_exception_musig(msg.str());
    }

    std::unique_ptr<secp256k1_musig_partial_signature> sigs(new secp256k1_musig_partial_signature[n]);

    auto &ctx = get_ctx(interp);
    size_t i = 0;
    while (partial_signatures_list != interpreter_base::EMPTY_LIST) {
        term signature_term = interp.arg(partial_signatures_list, 0);
	uint8_t signature[RAW_KEY_SIZE];
	size_t n = RAW_KEY_SIZE;
	bool r = get_bignum(interp, signature_term, signature, n);
	if (!r || n != RAW_KEY_SIZE) {
  	    std::stringstream msg;
	    msg << "musig_final_sign/" << arity;
	    msg << ": Second argument is not a list of partial signatures; element " << (i+1) << " was " << interp.to_string(signature_term);
            throw interpreter_exception_not_list(msg.str());
	}
	partial_signatures_list = interp.arg(partial_signatures_list,1);

	if (secp256k1_musig_partial_signature_parse(ctx, &(sigs.get())[i], signature) != 1) {
	    std::stringstream msg;
	    msg << "musig_final_sign/" << arity;
 	    msg << ": Partial signature could not be parsed: ";
	    msg << interp.to_string(signature_term);
  	    throw interpreter_exception_wrong_arg_type(msg.str());
	}
	i++;
    }

    size_t last_arg_index = arity - 1;

    private_key tweak_data(private_key::NO_KEY, ctx);
    uint8_t *tweak_used = nullptr;
    if (arity == 4) {
        term tweak_term = args[2];
	if (tweak_term != interpreter_base::EMPTY_LIST) {
	  if (!get_private_key(interp, tweak_term, tweak_data)) {
	    std::stringstream msg;
	    msg << "musig_final_sign/" << arity;	
	    msg << ": Couldn't parse private key (or secret) in third argument.";
	    throw interpreter_exception_wrong_arg_type(msg.str());	     
	  }
	  tweak_used = &tweak_data[0];
	}
    }
    
    secp256k1_schnorrsig final_sig;
    if (!session->final_sign(sigs.get(), final_sig, tweak_used)) {
        std::stringstream msg;
	msg << "musig_final_sign/" << arity;	
        msg << ": Couldn't combine signatures. MuSig session was probably invalid.";
	throw interpreter_exception_musig(msg.str());
    }

    uint8_t raw_sig[64];
    if (secp256k1_schnorrsig_serialize(ctx, raw_sig, &final_sig) != 1) {
        std::stringstream msg;
	msg << "musig_final_sign/" << arity;	
        msg << ": Couldn't serialize final signature. MuSig session was probably invalid.";
	throw interpreter_exception_musig(msg.str());
    }

    term raw_sig_term = new_bignum(interp, raw_sig, 64);

    return interp.unify(args[last_arg_index], raw_sig_term);
}

bool builtins::musig_nonce_negated_2(interpreter_base &interp, size_t arity, term args[] ) {
    auto *session = get_musig_session(interp, args[0]);
    if (session == nullptr) {
	throw interpreter_exception_wrong_arg_type(
	   "musig_nonce_negated/2: First argument must be MuSig session; was "
	   + interp.to_string(args[0]));
    }

    bool neg = session->nonce_is_negated();
    term neg_term = neg ? con_cell("true",0) : con_cell("false",0);
    return interp.unify(args[1], neg_term);
}

bool builtins::musig_end_1(interpreter_base &interp, size_t arity, term args[] ) {
    auto *session = get_musig_session(interp, args[0]);
    if (session == nullptr) {
	throw interpreter_exception_wrong_arg_type(
	   "musig_nonce_negated/2: First argument must be MuSig session; was "
	   + interp.to_string(args[0]));
    }

    auto &env = get_musig_env(interp);
    env.delete_session(session->id());

    return true;
}

void builtins::load(interpreter_base &interp, con_cell *module0)
{
    const con_cell EC("ec", 0);
    const con_cell M = (module0 == nullptr) ? EC : *module0;

    interp.load_builtin(M, con_cell("privkey", 1), &builtins::privkey_1);
    interp.load_builtin(M, con_cell("privkey", 2), &builtins::privkey_2);
    interp.load_builtin(M, con_cell("pubkey", 2), &builtins::pubkey_2);
    interp.load_builtin(M, con_cell("address", 2), &builtins::address_2);
    interp.load_builtin(M, con_cell("sign", 3), &builtins::sign_3);
    interp.load_builtin(M, interp.functor("validate", 3), &builtins::validate_3);
    interp.load_builtin(M, con_cell("hash", 2), &builtins::hash_2);

    interp.load_builtin(M, interp.functor("pubkey_tweak_add", 3), &builtins::pubkey_tweak_add_3);
    interp.load_builtin(M, interp.functor("privkey_tweak_add", 3), &builtins::privkey_tweak_add_3);    
    
    // interp.load_builtin(EC, con_cell("pcommit",3), &builtins::pcommit_3);
    interp.load_builtin(M, con_cell("pproof", 3), &builtins::pproof_3);
    interp.load_builtin(M, con_cell("pverify", 1), &builtins::pverify_1);

    // MuSig
    interp.load_builtin(M, interp.functor("musig_combine", 3), &builtins::musig_combine_3);
    interp.load_builtin(M, interp.functor("musig_verify", 3), &builtins::musig_verify_3);
    interp.load_builtin(M, interp.functor("musig_secret", 7), &builtins::musig_secret_7);

    interp.load_builtin(M, interp.functor("musig_start", 7), &builtins::musig_start_7);
    interp.load_builtin(M, interp.functor("musig_set_public_key", 3), &builtins::musig_set_public_key_3);
    interp.load_builtin(M, interp.functor("musig_nonce_commit", 2), &builtins::musig_nonce_commit_2);
    interp.load_builtin(M, interp.functor("musig_prepare", 3), &builtins::musig_prepare_3);
    interp.load_builtin(M, interp.functor("musig_nonces", 2), &builtins::musig_nonces_3);
    interp.load_builtin(M, interp.functor("musig_nonces", 3), &builtins::musig_nonces_3);    
    interp.load_builtin(M, interp.functor("musig_partial_sign", 2), &builtins::musig_partial_sign_2);
    interp.load_builtin(M, interp.functor("musig_partial_sign_adapt",4), &builtins::musig_partial_sign_adapt_4);
    interp.load_builtin(M, interp.functor("musig_final_sign", 3), &builtins::musig_final_sign_4);
    interp.load_builtin(M, interp.functor("musig_final_sign", 4), &builtins::musig_final_sign_4);
    interp.load_builtin(M, interp.functor("musig_nonce_negated", 2), &builtins::musig_nonce_negated_2);
    interp.load_builtin(M, interp.functor("musig_end", 1), &builtins::musig_end_1);

    // BIP32 & BIP39

    interp.load_builtin(M, interp.functor("master_key", 3), &builtins::master_key_3);
    interp.load_builtin(M, interp.functor("master_key", 4), &builtins::master_key_3);
    interp.load_builtin(M, interp.functor("words", 1), &builtins::words_2);
    interp.load_builtin(M, interp.functor("words", 2), &builtins::words_2);
    interp.load_builtin(M, interp.functor("child_pubkey", 3), &builtins::child_pubkey_3);
    interp.load_builtin(M, interp.functor("child_privkey", 3), &builtins::child_privkey_3);
    interp.load_builtin(M, interp.functor("normal_key", 2), &builtins::normal_key_2);
    interp.load_builtin(M, interp.functor("bc1", 2), &builtins::bc1_2);    

    // Encryption
    
    interp.load_builtin(M, interp.functor("encrypt", 4), &builtins::encrypt_4);
    
}

void builtins::load_consensus(interpreter_base &interp) {
    const con_cell EC("ec", 0);
    const con_cell M = EC;

    interp.load_builtin(M, con_cell("address", 2), &builtins::address_2);
    interp.load_builtin(M, interp.functor("validate", 3), &builtins::validate_3);
    interp.load_builtin(M, con_cell("hash", 2), &builtins::hash_2);

    interp.load_builtin(M, interp.functor("pubkey_tweak_add", 3), &builtins::pubkey_tweak_add_3);
    interp.load_builtin(M, interp.functor("privkey_tweak_add", 3), &builtins::privkey_tweak_add_3);    
    
    interp.load_builtin(M, con_cell("pverify", 1), &builtins::pverify_1);

    // MuSig
    interp.load_builtin(M, interp.functor("musig_combine", 3), &builtins::musig_combine_3);
    interp.load_builtin(M, interp.functor("musig_verify", 3), &builtins::musig_verify_3);
    interp.load_builtin(M, interp.functor("musig_secret", 7), &builtins::musig_secret_7);
}

template<typename XKey> static void execute_path(const std::string &pname, interpreter_base &interp, XKey &key, term path)
{
    static const con_cell H("h",1);
    static const con_cell M("m",0);
    static const con_cell SLASH("/",2);

    hd_keys hd(get_ctx(interp));

    std::vector<term> ops;
    while (path.tag() == tag_t::STR && interp.functor(path) == SLASH) {
        ops.push_back(interp.arg(path, 1));
	path = interp.arg(path, 0);
    }
    ops.push_back(path);
    std::reverse(ops.begin(), ops.end());

    bool first = true;
    
    for (auto op : ops) {
        // std::cout << "EXECUTE: " << interp.to_string(op) << std::endl;
	if (op.tag() == tag_t::STR) {
	    auto f = interp.functor(op);
	    if (f != H) {
  	        std::stringstream msg;
	        msg << pname << ": Unexpected path component: " << interp.to_string(op);
	        throw interpreter_exception_wrong_arg_type(msg.str());
	    }
	    if (key.type() != extended_key::EXTENDED_PRIVATE) {
  	        std::stringstream msg;
	        msg << pname << ": Can only apply hardening if parent key is private: " << interp.to_string(op);
	        throw interpreter_exception_wrong_arg_type(msg.str());
	    }
	    op = interp.arg(op,0);
	    if (op.tag() != tag_t::INT) {
  	        std::stringstream msg;
	        msg << pname << ": Unexpected path component: " << interp.to_string(op);
	        throw interpreter_exception_wrong_arg_type(msg.str());
	    }
	    int64_t val = reinterpret_cast<int_cell &>(op).value();
	    if (val < 0 || val >= extended_key::HARDENED_KEY) {
  	        std::stringstream msg;
	        msg << pname << ": Hardened child number is outside its valid range: " << interp.to_string(op);
	        throw interpreter_exception_wrong_arg_type(msg.str());	        
	    }
	    uint32_t child_num = static_cast<uint32_t>(val);
	    hd.generate_child(key, hd_keys::H(child_num), key);
	} else if (op.tag() == tag_t::CON) {
	    if (!first || op != M) {
  	        std::stringstream msg;
	        msg << pname << ": Unexpected path component: " << interp.to_string(op);
	        throw interpreter_exception_wrong_arg_type(msg.str());
	    }
	} else if (op.tag() == tag_t::INT) {
	    int64_t val = reinterpret_cast<int_cell &>(op).value();
	    if (val < 0 || val >= extended_key::HARDENED_KEY) {
  	        std::stringstream msg;
	        msg << pname << ": Child number is outside its valid range: " << interp.to_string(op);
	        throw interpreter_exception_wrong_arg_type(msg.str());	        
	    }
	    uint32_t child_num = static_cast<uint32_t>(val);
	    hd.generate_child(key, child_num, key);
	} else {
	    std::stringstream msg;
	    msg << pname << ": Unexpected path component: " << interp.to_string(op);
	    throw interpreter_exception_wrong_arg_type(msg.str());
	}
	first = false;
    }
}

bool builtins::is_int_list(interpreter_base &interp, term lst)
{
    while (!interp.is_empty_list(lst)) {
        if (interp.arg(lst, 0).tag() != tag_t::INT) {
	    return false;
        }
	lst = interp.arg(lst, 1);
    }
    return true;
}

bool builtins::master_key_3(interpreter_base &interp, size_t arity, term args[]) {
    std::string pname = "master_key/";
    pname += boost::lexical_cast<std::string>(arity);
  
    term seed = args[0];
    if (seed.tag() == tag_t::REF) {
        // Generate a new seed, use bignum
        uint8_t bytes[32];
	random::next_bytes(bytes, 32);
	seed = new_bignum(interp, bytes, 32);
	interp.unify(args[0], seed);
    }

    static const size_t MAX_SEED_LEN = 128;
    uint8_t seed_bytes[MAX_SEED_LEN];
    size_t seed_bytes_num = 128;
    if (seed.tag() == tag_t::BIG) {
        if (!get_bignum(interp, seed, seed_bytes, seed_bytes_num)) {
	    return false;
        }
    } else if (seed.tag() == tag_t::STR) {
        if (!interp.is_list(args[0])) {
	    throw interpreter_exception_not_list(pname + ": First argument is not a proper list; was " + interp.to_string(args[0]));
	}

	// Check if list is a list of integers (= raw seed)
	if (is_int_list(interp, seed)) {
	    size_t n = interp.list_length(seed);
	    if (interp.list_length(seed) > MAX_SEED_LEN) {
	        std::stringstream msg;
		msg << pname << ": Seed cannot exceed 128 bytes; length of list is " << n;
		throw interpreter_exception_wrong_arg_type(msg.str());
	    }
	    seed_bytes_num = 0;
	    while (!interp.is_empty_list(seed)) {
	        term elem = interp.arg(seed, 0);
	        if (elem.tag() != tag_t::INT) {
	            std::stringstream msg;
		    msg << pname << ": Seed must be a list of integers; encountered " << interp.to_string(elem);
		    throw interpreter_exception_wrong_arg_type(msg.str());	      
		}
		auto ival = reinterpret_cast<int_cell &>(elem);
		if (ival.value() < 0 || ival.value() > 255) {
		    std::stringstream msg;
		    msg << pname << ": Seed must be a list of integers within 0 and 255; encountered " << interp.to_string(elem);
		    throw interpreter_exception_wrong_arg_type(msg.str());	      
		}
		seed_bytes[seed_bytes_num++] = static_cast<uint8_t>(ival.value());
		seed = interp.arg(seed, 1);
	    }
	} else {
	    // List of words.
	    mnemonic mem(interp);
	    term chkseed = seed;
	    while (!interp.is_empty_list(chkseed)) {
	        term word = interp.arg(chkseed, 0);
		if (!mem.is_valid_word(word)) {
	   	      std::string word_str = interp.to_string(word);
		      throw interpreter_exception_not_list(pname + ": '" + word_str + "' is not a legal word from the expected word list");		  
		}
		chkseed = interp.arg(chkseed, 1);
	    }
	    if (!mem.from_sentence(seed)) {
	        throw interpreter_exception_wrong_arg_type(pname + ": Checksum computation failed on word sentence.");
	    }
	    
	    std::string passphrase = "TREZOR";
	    if (arity == 4) {
	      if (!interp.is_string(args[1])) {
		throw interpreter_exception_wrong_arg_type(pname + ": Second argument must be a proper string; was " + interp.to_string(args[1]));
		}
	        passphrase = interp.list_to_string(args[1]);
	    }
	    mem.compute_seed(seed_bytes, passphrase);
	    seed_bytes_num = 64;
	}
    } else {
        throw interpreter_exception_not_list(pname + ": Seed must be either a list of integers, a list of words or a bignum; was " + interp.to_string(seed));
    }

    term private_output = (arity == 3) ? args[1] : args[2];
    term public_output = (arity == 3) ? args[2] : args[3];    
	    
    hd_keys hd(get_ctx(interp), seed_bytes, seed_bytes_num);
    return interp.unify(private_output, hd.master_private().to_term(interp)) &&
           interp.unify(public_output, hd.master_public().to_term(interp));
}

bool builtins::words_2(interpreter_base &interp, size_t arity, term args[]) {
    std::string pname = "words/";
    pname += boost::lexical_cast<std::string>(arity);
    
    mnemonic mem(interp);
    size_t num_words = 24;
    term out = args[0];
    if (arity == 2) {
        out = args[1];
        if (args[0].tag() != tag_t::INT) {
	    throw interpreter_exception_not_list(pname + ": Number of words must be an integer; was " + interp.to_string(args[0]));
        }
        int64_t val = reinterpret_cast<int_cell &>(args[0]).value();
        if (val != 12 && val != 15 && val != 18 && val != 21 && val != 24) {
	    throw interpreter_exception_not_list(pname + ": Number of words must be 12, 15, 18, 21 or 24; was " + interp.to_string(args[0]));
        }
        num_words = static_cast<size_t>(val);
    }
    size_t ent = (num_words/3)*32;
    mem.generate_new(ent);
    return interp.unify(out, mem.to_sentence());
}

bool builtins::derive_child(const std::string &pname, interpreter_base &interp, term parent, term path, term result) {
    if (parent.tag() != tag_t::BIG) {
        std::stringstream msg;
        msg << pname << ": First argument must be a valid extended parent key; was " << interp.to_string(parent);
        throw interpreter_exception_wrong_arg_type(msg.str());
    }
  
    static const size_t XKEY_LEN = 82;
    uint8_t xkey_bytes[XKEY_LEN];
    size_t xkey_bytes_num = XKEY_LEN;
    if (!get_bignum(interp, parent, xkey_bytes, xkey_bytes_num) || xkey_bytes_num != 82) {
        std::stringstream msg;
        msg << pname << ": First argument must be a valid extended parent key; was " << interp.to_string(parent);
        throw interpreter_exception_wrong_arg_type(msg.str());      
    }
    
    // Checksum
    uint8_t checksum[4];
    get_checksum(xkey_bytes, XKEY_LEN-4, checksum);
    if (memcmp(checksum, &xkey_bytes[XKEY_LEN-4], 4) != 0) {
        std::stringstream msg;
        msg << pname << ": Provided extended key checksum failed; " << interp.to_string(parent);
        throw interpreter_exception_wrong_arg_type(msg.str());      
    }

    // Check if key is public or private

    if (xkey_bytes[0] == 0x04 &&
	xkey_bytes[1] == 0x88 &&
	xkey_bytes[2] == 0xB2 &&
	xkey_bytes[3] == 0x1E) {

        // If attempting generating a private child but parent is public!
        if (pname == "child_privkey/3") {
	    std::stringstream msg;
	    msg << pname << ": Cannot derive private keys from a public parent; " << interp.to_string(parent);
	    throw interpreter_exception_wrong_arg_type(msg.str());      
        }
      
        // Extended public key
        extended_public_key xpub;
	if (!xpub.read(xkey_bytes)) {
	    std::stringstream msg;
            msg << pname << ": Couldn't parse parent public key; " << interp.to_string(parent);
            throw interpreter_exception_wrong_arg_type(msg.str());      
	}

	execute_path(pname, interp, xpub, path);
	
	return interp.unify(result, xpub.to_term(interp));
    } else if (xkey_bytes[0] == 0x04 &&
	       xkey_bytes[1] == 0x88 &&
	       xkey_bytes[2] == 0xAD &&
	       xkey_bytes[3] == 0xE4) {
        // Extended private key
        extended_private_key xpriv;
	if (!xpriv.read(xkey_bytes)) {
	    std::stringstream msg;
            msg << pname << ": Couldn't parse parent private key; " << interp.to_string(parent);
            throw interpreter_exception_wrong_arg_type(msg.str());      
	  
	}
	execute_path(pname, interp, xpriv, path);

	if (pname == "child_pubkey/3") {
	     extended_public_key xpub;
	     xpriv.compute_extended_public_key(get_ctx(interp), xpub);
	     return interp.unify(result, xpub.to_term(interp));
	} else {
	     return interp.unify(result, xpriv.to_term(interp));
	}
    } else {
	std::stringstream msg;
	msg << pname << ": Unrecognized parent key; " << interp.to_string(parent);
	throw interpreter_exception_wrong_arg_type(msg.str());      
    }
}

bool builtins::child_pubkey_3(interpreter_base &interp, size_t arity, term args[]) {
    return derive_child("child_pubkey/3", interp, args[0], args[1], args[2]);
}

bool builtins::child_privkey_3(interpreter_base &interp, size_t arity, term args[]) {
    return derive_child("child_privkey/3", interp, args[0], args[1], args[2]);
}

bool builtins::normal_key_2(interpreter_base &interp, size_t arity, term args[]) {
    const std::string pname = "normal_key/2";

    term xkey = args[0];
    
    if (xkey.tag() != tag_t::BIG) {
        std::stringstream msg;
        msg << pname << ": First argument must be a valid extended key; was " << interp.to_string(xkey);
        throw interpreter_exception_wrong_arg_type(msg.str());
    }
  
    static const size_t XKEY_LEN = 82;
    uint8_t xkey_bytes[XKEY_LEN];
    size_t xkey_bytes_num = XKEY_LEN;
    if (!get_bignum(interp, xkey, xkey_bytes, xkey_bytes_num) || xkey_bytes_num != 82) {
        std::stringstream msg;
        msg << pname << ": First argument must be a valid extended key; was " << interp.to_string(xkey);
        throw interpreter_exception_wrong_arg_type(msg.str());      
    }
    
    // Checksum
    uint8_t checksum[4];
    get_checksum(xkey_bytes, XKEY_LEN-4, checksum);
    if (memcmp(checksum, &xkey_bytes[XKEY_LEN-4], 4) != 0) {
        std::stringstream msg;
        msg << pname << ": Provided extended key checksum failed; " << interp.to_string(xkey);
        throw interpreter_exception_wrong_arg_type(msg.str());      
    }

    // Check if key is public or private

    if (xkey_bytes[0] == 0x04 &&
	xkey_bytes[1] == 0x88 &&
	xkey_bytes[2] == 0xB2 &&
	xkey_bytes[3] == 0x1E) {
      
        // Extended public key
        extended_public_key xpub;
	if (!xpub.read(xkey_bytes)) {
	    std::stringstream msg;
            msg << pname << ": Couldn't parse extended public key; " << interp.to_string(xkey);
            throw interpreter_exception_wrong_arg_type(msg.str());      
	}

	term normal_key = create_public_key(interp, xpub);
	return interp.unify(args[1], normal_key);
    } else if (xkey_bytes[0] == 0x04 &&
	       xkey_bytes[1] == 0x88 &&
	       xkey_bytes[2] == 0xAD &&
	       xkey_bytes[3] == 0xE4) {
        // Extended private key
        extended_private_key xpriv;
	if (!xpriv.read(xkey_bytes)) {
	    std::stringstream msg;
            msg << pname << ": Couldn't parse extended private key; " << interp.to_string(xkey);
            throw interpreter_exception_wrong_arg_type(msg.str());      
	  
	}

	term normal_key = create_private_key(interp, xpriv, true);
	return interp.unify(args[1], normal_key);
    } else {
	std::stringstream msg;
	msg << pname << ": Unrecognized extended key; " << interp.to_string(xkey);
	throw interpreter_exception_wrong_arg_type(msg.str());      
    }
       
}

uint32_t builtins::bech32_polymod(uint8_t values[], size_t n)
{
    static const uint32_t GEN[5] = {
	0x3b6a57b2, 0x26508e6d, 0x1ea119fa, 0x3d4233dd, 0x2a1462b3	    
    };

    uint32_t chk = 1;
    for (size_t i = 0; i < n; i++) {
	auto v = values[i];
	assert(v >= 0 && v < 32);
	auto b = chk >> 25;
	chk = ((chk & 0x1ffffff) << 5) ^ v;
	for (size_t j = 0; j < 5; j++) {
	    chk ^= ((b >> j) & 1) ? GEN[j] : 0;
	}
    }
    return chk;
}

void builtins::bech32_hrp_expand(const std::string &s, uint8_t data[], size_t &n)
{
    assert(n >= 1+2*s.size());
    size_t i = 0;
    for (auto ch : s) {
	data[i++] = static_cast<uint8_t>(ch) >> 5;
    }
    data[i++] = 0;
    for (auto ch : s) {
	data[i++] = static_cast<uint8_t>(ch) & 31;
    }
    n = i;
}

void builtins::bech32_create_checksum(const std::string &hrp, uint8_t data[], size_t n, uint8_t out[6])
{
    size_t exp_len = 2*hrp.size()+1;
    assert(exp_len < 10);
    uint8_t hrp_data[10];
    bech32_hrp_expand(hrp, hrp_data, exp_len);
    uint8_t values[exp_len+n+6];
    static uint8_t ZERO[6] = {0,0,0,0,0,0};
    std::copy(hrp_data, hrp_data+exp_len, values);
    std::copy(data, data+n, &values[exp_len]);
    std::copy(ZERO, ZERO+6, &values[exp_len+n]);
    auto polymod = bech32_polymod(values, exp_len+n+6) ^ 1;
    for (size_t i = 0; i < 6; i++) {
	out[i] = (polymod >> (5 * (5 - i))) & 31;
    }
}

bool builtins::bc1_2(interpreter_base &interp, size_t arity, term args[])
{
    static char table[32] = {
	'q', 'p', 'z', 'r', 'y', '9', 'x', '8',
	'g', 'f', '2', 't', 'v', 'd', 'w', '0',
	's', '3', 'j', 'n', '5', '4', 'k', 'h',
	'c', 'e', '6', 'm', 'u', 'a', '7', 'l' };

    std::string hrp = "bc";
    
    std::string str = hrp + "1";
    
    // Generate a bc1 P2WPKH address from the public key.

    public_key pubkey;
    if (!get_public_key(interp, args[0], pubkey)) {
	return false;
    }

    uint8_t addr[25];
    get_address(pubkey, addr);

    uint8_t values[8*20/5];

    const uint8_t *p = &addr[1];
    size_t i = 1;
    uint16_t w = p[0] << 3; // 00000 xxxxx yyy
    size_t wi = 13;
    size_t bo = 0;
    size_t vi = 0;
    while (bo < 5+8*20) {
	if (wi < 5) {
	    w |= p[i] << (8 - wi);
	    wi += 8;
	    i++;
	}
	size_t value = (w >> 11);
	values[vi++] = value;
	str += table[value];
	w <<= 5;
	wi -= 5;
	bo += 5;
    }

    uint8_t chk[6];
    bech32_create_checksum(hrp, values, vi, chk);
    for (size_t i = 0; i < 6; i++) {
	str += table[chk[i]];
    }

    auto result = interp.string_to_list(str);
    return interp.unify(result, args[1]);
}

bool builtins::decrypt(interpreter_base &interp, term input, const uint8_t *key, term result) {

    term inner = interp.arg(input, 0);

    if (inner.tag() != tag_t::BIG) {
         throw interpreter_exception_wrong_arg_type(
	    "encrypt/4: Inner argument of input must be a big number; was "
	    + interp.to_string(inner));
    }

    big_cell big = reinterpret_cast<big_cell &>(inner);
    size_t num_bits = interp.num_bits(big);
    if (num_bits % 128 != 0) {
         throw interpreter_exception_wrong_arg_type(
	    "encrypt/4: Inner argument of input must be a big number in multiples of 128 bits; was " + boost::lexical_cast<std::string>(num_bits));
    }
    size_t num_bytes = num_bits / 8;
    term_serializer::buffer_t buf;
    buf.resize(num_bytes);
    interp.get_big(big, &buf[0], num_bytes);

    if (num_bytes < 32) {
         throw interpreter_exception_wrong_arg_type(
	    "encrypt/4: Inner argument has too few bytes, must be at least 32 bytes or more; wss " + boost::lexical_cast<std::string>(num_bits/8));
        
    }

    // Decrypt
    aes256 aes(key, 32);
    aes.cbc_decrypt(buf);

    // Check checksum
    sha256 hash;
    uint8_t checksum[sha256::HASH_SIZE];
    hash.update(&buf[0], buf.size()-16);
    hash.finalize(checksum);

    // Wrong password
    if (memcmp(checksum, &buf[buf.size()-15], 15) != 0) {
        return false;
    }

    size_t num_zeros = buf[buf.size()-16];
    if (num_zeros > 16) {
        return false;
    }

    term_serializer ser(interp);
    term t = ser.read(buf, buf.size()-16-num_zeros);

    return interp.unify(result, t);
}

term builtins::encrypt(interpreter_base &interp, term input, const std::string &passwd, int64_t iter)
{
    static const con_cell ENCRYPT = con_cell("encrypt",1);
  
    // Derive encryption key
    pbkdf2_t<hmac<sha512> > keygen("encrypted", 9, iter, 64);
    keygen.set_password(passwd.c_str(), passwd.size());
    const uint8_t *key = keygen.get_key();
    
    const size_t block_size = aes256::BLOCK_SIZE;
    
    uint8_t checksum[sha256::HASH_SIZE];

    term_serializer ser(interp);
    term_serializer::buffer_t buf;
    ser.write(buf, input);
    size_t pad_n = block_size - buf.size() % block_size;
    if (pad_n < 16) pad_n += 16;
    size_t num_zeros = 0;
    while (pad_n > 16) {
        buf.push_back(0);
        pad_n--;
	num_zeros++;
    }

    sha256 hash;
    hash.update(&buf[0], buf.size());
    hash.finalize(checksum);

    buf.push_back(static_cast<uint8_t>(num_zeros));
    for (size_t i = 0; i < 15; i++) {
        buf.push_back(checksum[i]);
    }

    // At this point buf should be even number of AES256 blocks (16 bytes)

    aes256 aes(key, 32);
    aes.cbc_encrypt(buf);

    // Construct term
    term big = interp.new_big(buf.size()*8);
    interp.set_big(big, &buf[0], buf.size());

    term result = interp.new_term( ENCRYPT, { big });
    
    return result;
}

bool builtins::encrypt_4(interpreter_base &interp, size_t arity, term args[]) {
    static const con_cell ENCRYPT = con_cell("encrypt",1);

    if (!interp.is_string(args[1])) {
         throw interpreter_exception_wrong_arg_type(
	    "encrypt/4: Second argument must be a string; was "
	    + interp.to_string(args[1]));
    }
    if (args[2].tag() != tag_t::INT) {
         throw interpreter_exception_wrong_arg_type(
	    "encrypt/4: Third argument must be an integer; was "
	    + interp.to_string(args[2]));
    }
    int64_t iter = reinterpret_cast<int_cell &>(args[2]).value();
    if (iter < 1 || iter > 1000000) {
         throw interpreter_exception_wrong_arg_type(
	    "encrypt/4: Number of iterations must be with 1 and 1000000; was "
	    + interp.to_string(args[2]));
    }

    std::string passwd = interp.list_to_string(args[1]);
    
    if (args[3].tag() == tag_t::STR && interp.functor(args[3]) == ENCRYPT) {
        // Derive encryption key
        pbkdf2_t<hmac<sha512> > keygen("encrypted", 9, iter, 64);
	keygen.set_password(passwd.c_str(), passwd.size());
	const uint8_t *key = keygen.get_key();

        return decrypt(interp, args[3], key, args[0]);
    }

    auto result = encrypt(interp, args[0], passwd, iter);
    return interp.unify(args[3], result);
}

}}
