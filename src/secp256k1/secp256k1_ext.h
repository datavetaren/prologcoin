#pragma once

#ifndef _secp256k1_ext_h
#define _secp256k1_ext_h

#include <memory.h>
#include "../../../secp256k1-zkp/src/basic-config.h"
#include "../../../secp256k1-zkp/include/secp256k1.h"
#include "../../../secp256k1-zkp/include/secp256k1_rangeproof.h"

// I couldn't figure out how to create a Pedersen commitment from a public key
// (with 0 value.) This is useful for secp256k1_pedersen_verify_tally()
extern "C" {
bool secp256k1_pedersen_commitment_load_pubkey(secp256k1_context *ctx, secp256k1_pedersen_commitment *commit, uint8_t pubkey[33]);
}

#endif
