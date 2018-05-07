#include "builtins.hpp"
#include "../common/random.hpp"
#include "../interp/interpreter_base.hpp"
#include "secp256k1.h"
#include "hash.h"
#include "hash_impl.h"

namespace prologcoin { namespace ec {

using namespace prologcoin::common;
using namespace prologcoin::interp;

static void get_checksum(uint8_t *bytes, size_t n, uint8_t checksum[4])
{
    // Silence unused warnings
    (void) &secp256k1_rfc6979_hmac_sha256_initialize;
    (void) &secp256k1_rfc6979_hmac_sha256_generate;
    (void) &secp256k1_rfc6979_hmac_sha256_finalize;

    secp256k1_sha256 ctx;
    secp256k1_sha256_initialize(&ctx);
    secp256k1_sha256_write(&ctx, bytes, n);
    uint8_t out32[32];
    secp256k1_sha256_finalize(&ctx, out32);
    secp256k1_sha256_initialize(&ctx);
    secp256k1_sha256_write(&ctx, out32, 32);
    secp256k1_sha256_finalize(&ctx, out32);

    memcpy(&checksum[0], out32, 4);
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
    } else if (args[0].tag() == tag_t::BIG) {
	auto &big = reinterpret_cast<const big_cell &>(args[0]);
	size_t nbits = interp.num_bits(big);
	if (nbits != 256 && nbits != 8+256+8 && nbits != 8+256+8+32) {
	    return false;
	}
	if (nbits == 256) {
	    interp.get_big(big, &bytes[1], nbits/8);
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
	}
	bool ok = secp256k1_ec_seckey_verify(ctx, &bytes[1]);
	return ok;
    } else {
	// Unrecognized format
	return false;
    }

    term big = interp.new_big(8+256+8+32);
    interp.set_big(big, bytes, sizeof(bytes));

    return interp.unify(args[0], big);
}

void builtins::load(interpreter_base &interp)
{
    const con_cell EC("ec", 0);

    interp.load_builtin(EC, con_cell("privkey", 1), &builtins::privkey_1);
}

}}
