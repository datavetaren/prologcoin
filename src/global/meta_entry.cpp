#include "meta_entry.hpp"
#include "../pow/pow_verifier.hpp"

using namespace prologcoin::common;
using namespace prologcoin::pow;

namespace prologcoin { namespace global {

bool meta_id::from_term(term_env &src, term t) {
    if (t.tag() != tag_t::BIG) {
	return false;
    }
    big_cell &big = reinterpret_cast<big_cell &>(t);
    if (src.num_bits(big) != hash_size()*8) {
	return false;
    }
    src.get_big(big, hash_, meta_id::HASH_SIZE);

    return true;
}

term meta_id::to_term(term_env &dst) const {
    return dst.new_big(hash_, hash_size());
}

bool meta_entry::validate_pow() const {
    siphash_keys key(reinterpret_cast<const char *>(get_id().hash()), get_id().hash_size());
    return verify_pow(key, DEFAULT_SUPER_DIFFICULTY,
		      get_pow_difficulty(), get_pow_proof());
}

}}



