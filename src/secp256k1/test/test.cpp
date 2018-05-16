#include "../secp256k1.h"

int main(int argc, char *argv[])
{
    uint8_t blinding_raw[32] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
				17,18,19,20,21,22,23,24,25,26,27,28,29,30,
				31,32};
    uint64_t value_raw = 1234;

    auto *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

    struct cleanup {
	cleanup(secp256k1_context *ctx) : ctx_(ctx) { }
	~cleanup() { secp256k1_context_destroy(ctx_); }
	secp256k1_context *ctx_;
    } cleanup_(ctx);

    secp256k1_pedersen_commitment commit;
    if (!secp256k1_pedersen_commit(ctx, &commit, blinding_raw, value_raw,
				   secp256k1_generator_h)) {
	return 1;
    }

    return 0;
}
