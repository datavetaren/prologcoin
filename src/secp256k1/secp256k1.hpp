#define USE_BASIC_CONFIG 1
#define SECP256K1_PREALLOACTED_SIZE_DECL
#define SECP256K1_ECMULT_GEN_CONTEXT_PREALLOCATED_SIZE_DECL
#pragma warning(disable:4319)
#include "basic-config.h"
#include "include/secp256k1.h"
#include "include/secp256k1_ecdh.h"
#include "include/secp256k1_generator.h"
#include "include/secp256k1_schnorrsig.h"
#include "include/secp256k1_musig.h"
#include "include/secp256k1_preallocated.h"
#include "include/secp256k1_rangeproof.h"
#include "include/secp256k1_recovery.h"
#include "include/secp256k1_surjectionproof.h"
#include "include/secp256k1_whitelist.h"

// I couldn't figure out how to create a Pedersen commitment from a public key
// (with 0 value.) This is useful for secp256k1_pedersen_verify_tally()
extern "C" {
bool secp256k1_pedersen_commitment_load_pubkey(secp256k1_context *ctx, secp256k1_pedersen_commitment *commit, uint8_t pubkey[33]);
}
