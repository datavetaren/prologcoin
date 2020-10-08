#include "meta_entry.hpp"
#include "../pow/pow_verifier.hpp"

using namespace prologcoin::pow;

namespace prologcoin { namespace global {

bool meta_entry::validate_pow() const {
    siphash_keys key(reinterpret_cast<const char *>(get_id().hash()), get_id().hash_size());
    return verify_pow(key, DEFAULT_SUPER_DIFFICULTY,
		      get_pow_difficulty(), get_pow_proof());
}

}}



