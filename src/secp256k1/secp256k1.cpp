extern "C" {

#define USE_BASIC_CONFIG 1

#include "../../../secp256k1-zkp/src/basic-config.h"
#include "secp256k1.h"
#include "../../../secp256k1-zkp/src/secp256k1.c"
#include "../../../secp256k1-zkp/src/modules/generator/main_impl.h"
#include "../../../secp256k1-zkp/src/modules/rangeproof/rangeproof_impl.h"
#include "../../../secp256k1-zkp/src/modules/rangeproof/main_impl.h"

extern "C" {

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

void dummy() {
    (void) &secp256k1_ge_set_infinity;
    (void) &secp256k1_gej_has_quad_y_var;
    (void) &secp256k1_ge_set_gej_var;
    (void) &secp256k1_gej_is_valid_var;
    (void) &secp256k1_ecmult_multi_var;
    (void) &secp256k1_ecmult_strauss_batch_single;
    (void) &secp256k1_ecmult_pippenger_batch_single;
    (void) &secp256k1_ecmult_const;
}

}

