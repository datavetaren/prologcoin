#include "builtins.hpp"
#include "../common/random.hpp"
#include "../interp/interpreter_base.hpp"
#include "secp256k1.h"
#include "hash.h"
#include "hash_impl.h"
#include "ripemd160.h"

namespace prologcoin { namespace ec {

using namespace prologcoin::common;
using namespace prologcoin::interp;

static void get_checksum(const uint8_t *bytes, size_t n, uint8_t checksum[4])
{
    // Silence unused warnings
    (void) &secp256k1_rfc6979_hmac_sha256_initialize;
    (void) &secp256k1_rfc6979_hmac_sha256_generate;
    (void) &secp256k1_rfc6979_hmac_sha256_finalize;

    uint8_t out32[32];
    secp256k1_sha256 ctx;
    secp256k1_sha256_initialize(&ctx);
    secp256k1_sha256_write(&ctx, bytes, n);
    secp256k1_sha256_finalize(&ctx, out32);
    secp256k1_sha256_initialize(&ctx);
    secp256k1_sha256_write(&ctx, out32, 32);
    secp256k1_sha256_finalize(&ctx, out32);

    memcpy(&checksum[0], out32, 4);
}

bool builtins::get_private_key(interpreter_base &interp, term big0, uint8_t rawkey[32])
{
    uint8_t bytes[1+32+8];

    if (big0.tag() != tag_t::BIG) {
	return false;
    }

    auto &big = reinterpret_cast<const big_cell &>(big0);
    size_t nbits = interp.num_bits(big);
    if (nbits != 256 && nbits != 8+256+8 && nbits != 8+256+8+32) {
	return false;
    }

    if (nbits == 256) {
	interp.get_big(big, &rawkey[0], nbits/8);
    } else {
	interp.get_big(big, &bytes[0], nbits/8);
	if (bytes[0] != 0x80 || bytes[33] != 0x01) {
	    return false;
	}
	if (nbits == 8+256+8+32) {
	    uint8_t checksum[4];
	    get_checksum(&bytes[0], 34, checksum);
	    if (memcmp(&bytes[34], checksum, 4) != 0) {
		return false;
	    }
	}
	memcpy(&rawkey[0], &bytes[1], 32);
    }
    return true;
}

bool builtins::get_public_key(interpreter_base &interp, term big0, uint8_t pubkey[33])
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

bool builtins::privkey_1(interpreter_base &interp, size_t arity, term args[])
{
    // Generate a new private key. We'll use the bitcoin private key
    // format where first byte is 0x80 followed by 

    uint8_t bytes[1+32+8];

    auto *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

    struct cleanup {
	cleanup(secp256k1_context *ctx) : ctx_(ctx) { }
	~cleanup() { secp256k1_context_destroy(ctx_); }
	secp256k1_context *ctx_;
    } cleanup_(ctx);

    if (args[0].tag() == tag_t::REF) {

	bytes[0] = 0x80;
	bool ok = true;
	do {
	    random::next_bytes(&bytes[1], 32);
	    ok = secp256k1_ec_seckey_verify(ctx, &bytes[1]);
	} while (!ok);
	uint8_t checksum[4];
	bytes[33] = 0x01;
	get_checksum(&bytes[0], 34, checksum);
	memcpy(&bytes[34], checksum, 4);

	term big = interp.new_big(8+256+8+32);
	interp.set_big(big, bytes, sizeof(bytes));

	return interp.unify(args[0], big);
    } else {
	if (!get_private_key(interp, args[0], &bytes[0])) {
	    return false;
	}
	bool ok = secp256k1_ec_seckey_verify(ctx, &bytes[0]);
	return ok;
    }
}

bool builtins::pubkey_2(interpreter_base &interp, size_t arity, term args[])
{
    // Generate a new private key. We'll use the bitcoin private key
    // format where first byte is 0x80 followed by 
    
    auto *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

    struct cleanup {
	cleanup(secp256k1_context *ctx) : ctx_(ctx) { }
	~cleanup() { secp256k1_context_destroy(ctx_); }
	secp256k1_context *ctx_;
    } cleanup_(ctx);
    
    uint8_t rawkey[32];
    if (!get_private_key(interp, args[0], &rawkey[0])) {
	return false;
    }

    secp256k1_pubkey pubkey;
    bool r = secp256k1_ec_pubkey_create(ctx, &pubkey, &rawkey[0]);
    if (!r) {
	return false;
    }
    uint8_t outpubkey[33];
    size_t outpubkeylen = 33;
    r = secp256k1_ec_pubkey_serialize(ctx, outpubkey, &outpubkeylen, &pubkey, SECP256K1_EC_COMPRESSED);
    if (!r) {
	return false;
    }
    if (outpubkeylen != 33) {
	return false;
    }

    term big = interp.new_big(8+256); // 33*8 bits
    interp.set_big(big, outpubkey, outpubkeylen);

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

void builtins::load(interpreter_base &interp)
{
    const con_cell EC("ec", 0);

    interp.load_builtin(EC, con_cell("privkey", 1), &builtins::privkey_1);
    interp.load_builtin(EC, con_cell("pubkey", 2), &builtins::pubkey_2);
    interp.load_builtin(EC, con_cell("address", 2), &builtins::address_2);
}

}}
