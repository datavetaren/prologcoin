#include "../secp256k1/secp256k1.hpp"
#include "builtins.hpp"
#include "../common/random.hpp"
#include "../common/utime.hpp"
#include "../interp/interpreter_base.hpp"
#include "../common/term_serializer.hpp"
#include "../common/hex.hpp"
#include "ripemd160.h"

#include "src/util.h"
#include "src/hash_impl.h"

namespace prologcoin { namespace ec {

using namespace prologcoin::common;
using namespace prologcoin::interp;

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

term builtins::create_private_key(interpreter_base &interp, uint8_t rawkey[RAW_KEY_SIZE])
{
    uint8_t bytes[1+RAW_KEY_SIZE+1+4];

    memcpy(&bytes[1], rawkey, RAW_KEY_SIZE);

    uint8_t checksum[4];
    bytes[0] = 0x80;
    bytes[RAW_KEY_SIZE+1] = 0x01;
    get_checksum(&bytes[0], RAW_KEY_SIZE+2, checksum);
    memcpy(&bytes[34], checksum, 4);

    term big = interp.new_big(8+(RAW_KEY_SIZE)*8+8+32);
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

bool builtins::get_private_key(interpreter_base &interp, term big0, uint8_t rawkey[builtins::RAW_KEY_SIZE])
{
    uint8_t bytes[1+RAW_KEY_SIZE+8];

    if (big0.tag() != tag_t::BIG) {
	return false;
    }

    static const size_t BITS = RAW_KEY_SIZE*8;

    auto &big = reinterpret_cast<const big_cell &>(big0);
    size_t nbits = interp.num_bits(big);
    if (nbits != BITS && nbits != 8+BITS+8 && nbits != 8+BITS+8+32) {
	return false;
    }

    if (nbits == BITS) {
	interp.get_big(big, &rawkey[0], nbits/8);
    } else {
	interp.get_big(big, &bytes[0], nbits/8);
	if (bytes[0] != 0x80 || bytes[33] != 0x01) {
	    return false;
	}
	if (nbits == 8+BITS+8+32) {
	    uint8_t checksum[4];
	    get_checksum(&bytes[0], RAW_KEY_SIZE+2, checksum);
	    if (memcmp(&bytes[RAW_KEY_SIZE+2], checksum, 4) != 0) {
		return false;
	    }
	}
	memcpy(&rawkey[0], &bytes[1], RAW_KEY_SIZE);
    }
    return true;
}

bool builtins::get_public_key(interpreter_base &interp, term big0, uint8_t pubkey[builtins::RAW_KEY_SIZE+1])
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

bool builtins::new_private_key(uint8_t rawkey[builtins::RAW_KEY_SIZE])
{
    auto *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

    struct cleanup {
	cleanup(secp256k1_context *ctx) : ctx_(ctx) { }
	~cleanup() { secp256k1_context_destroy(ctx_); }
	secp256k1_context *ctx_;
    } cleanup_(ctx);

    bool ok = true;
    do {
	random::next_bytes(&rawkey[0], RAW_KEY_SIZE);
	ok = secp256k1_ec_seckey_verify(ctx, &rawkey[0]);
    } while (!ok);

    return true;
}

bool builtins::privkey_1(interpreter_base &interp, size_t arity, term args[])
{
    // Generate a new private key. We'll use the bitcoin private key
    // format where first byte is 0x80 followed by 

    uint8_t bytes[1+RAW_KEY_SIZE+8];

    auto *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

    struct cleanup {
	cleanup(secp256k1_context *ctx) : ctx_(ctx) { }
	~cleanup() { secp256k1_context_destroy(ctx_); }
	secp256k1_context *ctx_;
    } cleanup_(ctx);

    if (args[0].tag() == tag_t::REF) {

	new_private_key(&bytes[1]);
	term big = create_private_key(interp, &bytes[1]);

	return interp.unify(args[0], big);
    } else {
	if (!get_private_key(interp, args[0], &bytes[0])) {
	    return false;
	}
	bool ok = secp256k1_ec_seckey_verify(ctx, &bytes[0]);
	return ok;
    }
}

bool builtins::privkey_2(interpreter_base &interp, size_t arity, term args[])
{
    if (args[0].tag() == tag_t::REF) {
	throw interpreter_exception_not_sufficiently_instantiated(
		  "ec:privkey/2: First argument, a raw private key"
                  " must be given.");
    }

    uint8_t rawkey[RAW_KEY_SIZE];
    size_t n = RAW_KEY_SIZE;
    if (!get_bignum(interp, args[0], &rawkey[0], n)) {
	return false;
    }

    size_t dn = RAW_KEY_SIZE - n;

    // Pad with leading 0s
    memmove(&rawkey[dn], &rawkey[0], n);
    memset(&rawkey[0], 0, dn);
    
    term r = create_private_key(interp, rawkey);

    return interp.unify(args[1], r);
}

bool builtins::compute_public_key(uint8_t priv_raw[builtins::RAW_KEY_SIZE],
				  uint8_t pub_raw[builtins::RAW_KEY_SIZE+1])
{
    auto *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

    struct cleanup {
	cleanup(secp256k1_context *ctx) : ctx_(ctx) { }
	~cleanup() { secp256k1_context_destroy(ctx_); }
	secp256k1_context *ctx_;
    } cleanup_(ctx);

    secp256k1_pubkey pubkey;
    bool r = secp256k1_ec_pubkey_create(ctx, &pubkey, &priv_raw[0]);
    if (!r) {
	return false;
    }
    size_t pub_raw_len = RAW_KEY_SIZE+1;
    r = secp256k1_ec_pubkey_serialize(ctx, &pub_raw[0], &pub_raw_len,
				      &pubkey, SECP256K1_EC_COMPRESSED);
    if (!r) {
	return false;
    }
    if (pub_raw_len != RAW_KEY_SIZE+1) {
	return false;
    }
    return true;
}

term builtins::create_public_key(interpreter_base &interp,
				 uint8_t pub_raw[builtins::RAW_KEY_SIZE+1])
{
    term big = interp.new_big((RAW_KEY_SIZE+1)*8);
    interp.set_big(big, pub_raw, RAW_KEY_SIZE+1);
    return big;
}

bool builtins::pubkey_2(interpreter_base &interp, size_t arity, term args[])
{
    
    uint8_t priv_raw[RAW_KEY_SIZE];
    if (!get_private_key(interp, args[0], &priv_raw[0])) {
	return false;
    }
    uint8_t pub_raw[RAW_KEY_SIZE+1];
    if (!compute_public_key(&priv_raw[0], &pub_raw[0])) {
	return false;
    }
    term big = create_public_key(interp, pub_raw);
    return interp.unify(args[1], big);
}

bool builtins::get_address(uint8_t pubkey[33], uint8_t addr[25])
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
    uint8_t pubkey[33];
    if (!get_public_key(interp, args[0], pubkey)) {
	return false;
    }

    uint8_t addr[25];
    get_address(pubkey, addr);

    term big = interp.new_big(sizeof(addr)*8);
    interp.set_big(big, addr, sizeof(addr));
    
    return interp.unify(args[1], big);
}

bool builtins::get_hashed_data(uint8_t *data, size_t data_len,
			       uint8_t hash[32])
{
    secp256k1_sha256 ctx;
    secp256k1_sha256_initialize(&ctx);
    secp256k1_sha256_write(&ctx, data, data_len);
    secp256k1_sha256_finalize(&ctx, hash);

    return true;
}

bool builtins::get_hashed2_data(interpreter_base &interp, const term data,
				uint8_t hash[32])
{
    if (data.tag() == tag_t::BIG) {
	auto &big_data = reinterpret_cast<const big_cell &>(data);
	big_iterator bi = interp.get_heap().begin(big_data);
	big_iterator bi_end = interp.get_heap().end(big_data);

	// If the data size is exactly 32 bytes, then don't hash anything.
	// Just return the data as is. This enables compatibility with
	// computing signatures over data that has already been hashed.
	size_t data_size = bi_end - bi;
	if (data_size == 32) {
	    for (size_t i = 0; i < data_size; i++, ++bi) {
		hash[i] = *bi;
	    }
	    return true;
	}

	// If the data is a bignum with more (or less) than 32 bytes, then
	// use SHA256 on it and return the hashed value.
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

	return true;
    }

    // The data is anything but a bignum. We'll serialize the data and
    // then compute the SHA256 hash of it.
    term_serializer ser(interp);
    term_serializer::buffer_t buf;
    ser.write(buf, data);

    secp256k1_sha256 ctx;
    secp256k1_sha256_initialize(&ctx);
    secp256k1_sha256_write(&ctx, &buf[0], buf.size());
    secp256k1_sha256_finalize(&ctx, hash);

    return true;
}

bool builtins::compute_signature(uint8_t hashed_data[32],
				 uint8_t priv_raw[builtins::RAW_KEY_SIZE],
				 uint8_t signature[64])
{
    auto *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

    struct cleanup {
	cleanup(secp256k1_context *ctx) : ctx_(ctx) { }
	~cleanup() { secp256k1_context_destroy(ctx_); }
	secp256k1_context *ctx_;
    } cleanup_(ctx);

    secp256k1_ecdsa_signature sig;
    if (!secp256k1_ecdsa_sign(ctx, &sig, hashed_data, priv_raw,
			      nullptr, nullptr)) {
	return false;
    }

    if (!secp256k1_ecdsa_signature_serialize_compact(ctx, &signature[0], &sig)) {
	return false;
    }

    return true;
}

bool builtins::compute_signature(interpreter_base &interp, const term data,
				 const term privkey, term &out_signature)
{
    uint8_t rawkey[RAW_KEY_SIZE];
    if (!get_private_key(interp, privkey, rawkey)) {
	return false;
    }

    uint8_t hash[32];
    if (!get_hashed2_data(interp, data, hash)) {
	return false;
    }

    uint8_t signature[64];
    compute_signature(hash, rawkey, signature);

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

bool builtins::verify_signature(uint8_t hash[32],
				uint8_t pubkey[33],
				uint8_t sign_data[64])
{
    auto *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY);

    struct cleanup {
	cleanup(secp256k1_context *ctx) : ctx_(ctx) { }
	~cleanup() { secp256k1_context_destroy(ctx_); }
	secp256k1_context *ctx_;
    } cleanup_(ctx);

    secp256k1_pubkey pubkey1;
    if (!secp256k1_ec_pubkey_parse(ctx, &pubkey1, pubkey, 33)) {
	return false;
    }

    secp256k1_ecdsa_signature sig;
    if (!secp256k1_ecdsa_signature_parse_compact(ctx, &sig, sign_data)) {
	return false;
    }

    if (!secp256k1_ecdsa_verify(ctx, &sig, hash, &pubkey1)) {
	return false;
    }
    return true;
}

bool builtins::verify_signature(interpreter_base &interp,
				const term data,
				const term pubkey,
				const term signature)
{
    uint8_t pubkey_raw[33];
    if (!get_public_key(interp, pubkey, pubkey_raw)) {
	return false;
    }

    uint8_t sign_data[64];
    if (!get_signature_data(interp, signature, sign_data)) {
	return false;
    }

    uint8_t hash[32];
    if (!get_hashed2_data(interp, data, hash)) {
	return false;
    }

    return verify_signature(hash, pubkey_raw, sign_data);
}

bool builtins::sign_3(interpreter_base &interp, size_t arity, term args[] )
{
    if (args[0].tag() == tag_t::REF) {
	throw interpreter_exception_not_sufficiently_instantiated(
		  "ec:sign/3: First argument, a public or private key"
                  " (private for signing), must be given.");
    }

    if (args[2].tag() == tag_t::REF) {
	term out;
	if (!compute_signature(interp, args[1], args[0], out)) {
	    return false;
	}
	return interp.unify(args[2], out);

    } else {
	if (!verify_signature(interp, args[1], args[0], args[2])) {
	    return false;
	}
	return true;
    }
}

bool builtins::compute_pedersen_commit(interpreter_base &interp,
				       const term blinding,
				       const term value,
				       uint8_t commit_raw[33])
{
    uint8_t blinding_raw[RAW_KEY_SIZE];

    if (!get_private_key(interp, blinding, blinding_raw)) {
	return false;
    }
    if (value.tag() != tag_t::INT) {
	return false;
    }
    uint64_t value_raw = reinterpret_cast<const int_cell &>(value).value();

    auto *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

    struct cleanup {
	cleanup(secp256k1_context *ctx) : ctx_(ctx) { }
	~cleanup() { secp256k1_context_destroy(ctx_); }
	secp256k1_context *ctx_;
    } cleanup_(ctx);

    secp256k1_pedersen_commitment commit;
    if (!secp256k1_pedersen_commit(ctx, &commit, blinding_raw, value_raw,
				   secp256k1_generator_h)) {
	return false;
    }

    if (!secp256k1_pedersen_commitment_serialize(ctx, commit_raw, &commit)) {
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

    uint8_t blinding_raw[RAW_KEY_SIZE];

    if (!get_private_key(interp, blinding, blinding_raw)) {
	return false;
    }
    if (value.tag() != tag_t::INT) {
	return false;
    }

    uint64_t value_raw = reinterpret_cast<const int_cell &>(value).value();
    static_cast<void>(value_raw);

    // Generate random value
    uint8_t b1[RAW_KEY_SIZE], b2[RAW_KEY_SIZE], b3[RAW_KEY_SIZE];

    bool ok = false;

    auto *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

    // The original commit
    secp256k1_pedersen_commitment commit;
    if (!secp256k1_pedersen_commit(ctx, &commit, blinding_raw, value_raw, secp256k1_generator_h)) {
	return false;
    }

    uint8_t commit_raw[33];
    secp256k1_pedersen_commitment_serialize(ctx, commit_raw, &commit);

    struct cleanup {
	cleanup(secp256k1_context *ctx) : ctx_(ctx) { }
	~cleanup() { secp256k1_context_destroy(ctx_); }
	secp256k1_context *ctx_;
    } cleanup_(ctx);

    uint8_t commit1_raw[33], commit2_raw[33], commit3_raw[33];
    secp256k1_pedersen_commitment commit1, commit2, commit3;

    while (!ok) {
	new_private_key(b1);
	new_private_key(b2);

	// Compute b3 = b - b1 - b2
	uint8_t *bs[] = { &blinding_raw[0], &b1[0], &b2[0] };
	if (!secp256k1_pedersen_blind_sum(ctx, b3, bs, 3, 1)) {
	    continue;
	}

	uint64_t v1 = random::next_int(value_raw);
	uint64_t v2 = value_raw - v1;

	// Create pedersen commit b1*G+v1*H + b2*G+v2*H

	if (!secp256k1_pedersen_commit(ctx, &commit1, b1, v1, secp256k1_generator_h)) {
	    continue;
	}
	if (!secp256k1_pedersen_commit(ctx, &commit2, b2, v2, secp256k1_generator_h)) {
	    continue;
	}
	if (!secp256k1_pedersen_commit(ctx, &commit3, b3, 0, secp256k1_generator_h)) {
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
    if (!secp256k1_pedersen_verify_tally(ctx, pos, 1, neg, 3)) {
	return false;
    }

    uint8_t pub_b3[RAW_KEY_SIZE+1];
    
    // We need the public key for b3
    if (!compute_public_key(b3, pub_b3)) {
	return false;
    }

    // Sign something with b3
    uint8_t signature[64];
    uint8_t msg[8];
    utime ut = utime::now();
    ut.to_bytes(msg);
    uint8_t hash[32];
    get_hashed_data(msg, sizeof(msg), hash);
    if (!compute_signature(hash, b3, signature)) {
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
    term term_pubkey = new_bignum(interp, pub_b3, 33);
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
    uint8_t pubkey_raw[33];
    uint8_t signature_raw[64];

    size_t n1 = 33, n2 = 33, n3 = 33, n4 = 33, n5 = 64;

    if (!get_bignum(interp, term_commit, commit_raw, n1) ||
	!get_bignum(interp, term_commit1, commit1_raw, n2) ||
	!get_bignum(interp, term_commit2, commit2_raw, n3) ||
	!get_bignum(interp, term_pubkey, pubkey_raw, n4) ||
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

    auto *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY);

    struct cleanup {
	cleanup(secp256k1_context *ctx) : ctx_(ctx) { }
	~cleanup() { secp256k1_context_destroy(ctx_); }
	secp256k1_context *ctx_;
    } cleanup_(ctx);


    secp256k1_pedersen_commitment commit, commit1, commit2, commit3;

    if (!secp256k1_pedersen_commitment_parse(ctx, &commit, commit_raw)) {
	return false;
    }
    if (!secp256k1_pedersen_commitment_parse(ctx, &commit1, commit1_raw)) {
	return false;
    }
    if (!secp256k1_pedersen_commitment_parse(ctx, &commit2, commit2_raw)) {
	return false;
    }

    if (!secp256k1_pedersen_commitment_load_pubkey(ctx, &commit3, pubkey_raw)) {
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
    if (!secp256k1_pedersen_verify_tally( ctx, positive_commits, 1,
					  negative_commits, 3)) {
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
    get_hashed_data(msg, sizeof(msg), hash);
    if (!verify_signature(hash, pubkey_raw, signature_raw)) {
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

    if (!secp256k1_pedersen_blind_sum(ctx, bout, bs, 2, 2)) {
	std::cout << "Failed blind sum\n";
	return;
    }

    std::cout << "b1: " << hex::to_string(b1, 32) << std::endl;
    std::cout << "b2: " << hex::to_string(b2, 32) << std::endl;
    std::cout << "bo: " << hex::to_string(bout, 32) << std::endl;

    std::cout << "Everything is ok\n";
    
}

void builtins::load(interpreter_base &interp)
{
    const con_cell EC("ec", 0);

    interp.load_builtin(EC, con_cell("privkey", 1), &builtins::privkey_1);
    interp.load_builtin(EC, con_cell("privkey", 2), &builtins::privkey_2);
    interp.load_builtin(EC, con_cell("pubkey", 2), &builtins::pubkey_2);
    interp.load_builtin(EC, con_cell("address", 2), &builtins::address_2);
    interp.load_builtin(EC, con_cell("sign", 3), &builtins::sign_3);

    // interp.load_builtin(EC, con_cell("pcommit",3), &builtins::pcommit_3);
    interp.load_builtin(EC, con_cell("pproof", 3), &builtins::pproof_3);
    interp.load_builtin(EC, con_cell("pverify", 1), &builtins::pverify_1);
}

}}
