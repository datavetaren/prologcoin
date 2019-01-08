#pragma once

#ifndef _ec_secp256k1_h
#define _ec_secp256k1_h

#ifdef _WIN32
#pragma warning( disable : 4319)
#endif
  
#ifndef USE_BASIC_CONFIG
#define USE_BASIC_CONFIG 1
#endif
#ifndef SECP256K1_BUILD
#define SECP256K1_BUILD
#endif

#include <memory.h>
#include "../../../secp256k1-zkp/src/basic-config.h"

#include "../../../secp256k1-zkp/include/secp256k1.h"
#include "../../../secp256k1-zkp/src/ecmult.h"
#include "../../../secp256k1-zkp/src/ecmult_gen.h"
#include "../../../secp256k1-zkp/include/secp256k1_generator.h"
#include "../../../secp256k1-zkp/include/secp256k1_rangeproof.h"
#include "../../../secp256k1-zkp/src/util.h"
#include "../../../secp256k1-zkp/src/scratch.h"
#include "../../../secp256k1-zkp/src/scratch_impl.h"
#include "../../../secp256k1-zkp/src/scalar_impl.h"
#include "../../../secp256k1-zkp/src/group_impl.h"
#include "../../../secp256k1-zkp/src/field_impl.h"
#include "../../../secp256k1-zkp/src/hash_impl.h"
#include "../../../secp256k1-zkp/src/eckey_impl.h"
#include "../../../secp256k1-zkp/src/ecmult_const_impl.h"
#include "../../../secp256k1-zkp/src/ecmult_gen_impl.h"

static void * secp256k1_refs[] = {
    (void *)&secp256k1_scratch_create,
    (void *)&secp256k1_scratch_destroy,
    (void *)&secp256k1_fe_cmp_var,
    (void *)&secp256k1_ge_set_xy,
    (void *)&secp256k1_ge_is_valid_var,
    (void *)&secp256k1_ge_set_infinity,
    (void *)&secp256k1_ge_set_gej_var,
    (void *)&secp256k1_gej_eq_x_var,
    (void *)&secp256k1_gej_has_quad_y_var,
    (void *)&secp256k1_scalar_inverse_var,
    (void *)&secp256k1_gej_is_valid_var,
    (void *)&secp256k1_ecmult_const,
    (void *)&secp256k1_ecmult_context_build,
    (void *)&secp256k1_ecmult_context_clone,
    (void *)&secp256k1_ecmult_context_clear,
    (void *)&secp256k1_ecmult_context_is_built,
    (void *)&secp256k1_ecmult_multi_var,
    (void *)&secp256k1_ecmult_strauss_batch_single,
    (void *)&secp256k1_ecmult_pippenger_batch_single,
    (void *)&secp256k1_ecmult_gen_context_init,
    (void *)&secp256k1_ecmult_gen_context_build,
    (void *)&secp256k1_ecmult_gen_context_clone,
    (void *)&secp256k1_ecmult_gen_context_clear,
    (void *)&secp256k1_ecmult_gen_context_is_built,
    (void *)&secp256k1_eckey_pubkey_parse,
    (void *)&secp256k1_eckey_pubkey_serialize,
    (void *)&secp256k1_eckey_privkey_tweak_add,
    (void *)&secp256k1_eckey_privkey_tweak_mul,
    (void *)&secp256k1_eckey_pubkey_parse,
    (void *)&secp256k1_eckey_pubkey_serialize,
    (void *)&secp256k1_eckey_pubkey_tweak_add,
    (void *)&secp256k1_eckey_pubkey_tweak_mul,
};

inline void * dummy_secp2561k1_refs() {
   return secp256k1_refs;
}


#endif
