extern "C" {

#define USE_BASIC_CONFIG 1
#define SECP256K1_PREALLOACTED_SIZE_DECL
#define SECP256K1_ECMULT_GEN_CONTEXT_PREALLOCATED_SIZE_DECL
#if defined(_MSC_VER)
#pragma warning(disable:4319)
#endif

#define ENABLE_MODULE_ECDH
#define ENABLE_MODULE_SCHNORRSIG
#define ENABLE_MODULE_MUSIG
#define ENABLE_MODULE_RECOVERY
#define ENABLE_MODULE_GENERATOR
#define ENABLE_MODULE_RANGEPROOF
#define ENABLE_MODULE_WHITELIST
#define ENABLE_MODULE_SURJECTIONPROOF
  
#include "../../../secp256k1-zkp/src/basic-config.h"
#include "../../../secp256k1-zkp/include/secp256k1.h"
#include "../../../secp256k1-zkp/src/secp256k1.c"

//
// I couldn't find a way to get from a public key to a Pedersen commitment,
// so I added this function. This is useful in conjunction with
// secbp256k1_pedersen_verify_tally() which only takes an array of Pedersen
// commitments and check that they add up to zero. So if you have a public
// key as excess sum (for signing things) it's useful to go from that public
// key to a Pedersen commitment (pubkey*G + 0*H)
//
bool secp256k1_pedersen_commitment_load_pubkey(secp256k1_context *ctx, secp256k1_pedersen_commitment *commit, uint8_t pubkey[33])
{
    (void)ctx;

    secp256k1_ge ge;
    secp256k1_eckey_pubkey_parse(&ge, pubkey, 33);
    secp256k1_pedersen_commitment_save(commit, &ge);

    return true;
}

}
