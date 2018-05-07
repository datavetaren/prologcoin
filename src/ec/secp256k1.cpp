extern "C" {

#define USE_BASIC_CONFIG 1

#include "../../../secp256k1-zkp/src/basic-config.h"
#include "../../../secp256k1-zkp/src/secp256k1.c"

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

