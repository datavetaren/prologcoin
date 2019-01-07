#pragma once

#ifndef _pow_mining_hpp
#define _pow_mining_hpp

#include "siphash.hpp"
#include "fxp.hpp"
#include "pow_verifier.hpp"

namespace prologcoin { namespace pow {

bool search_proof(const siphash_keys &key, size_t super_difficulty, const pow_difficulty &difficulty, pow_proof &out_proof);
}}

#endif


