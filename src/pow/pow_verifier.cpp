#include "pow_verifier.hpp"
#include "galaxy.hpp"
#include "camera.hpp"
#include "dipper_detector.hpp"

namespace prologcoin { namespace pow {

void pow_verifier::setup(size_t proof_number, size_t nonce) {
    switch (super_difficulty_) {
    case 7: setup_t<7>(proof_number, nonce); break;
    case 8: setup_t<8>(proof_number, nonce); break;
    case 9: setup_t<9>(proof_number, nonce); break;
    default: assert("not implemented" == nullptr); break;
    }
}

void pow_verifier::cleanup() {
    switch (super_difficulty_) {
    case 7: cleanup_t<7>(); break;
    case 8: cleanup_t<8>(); break;
    case 9: cleanup_t<9>(); break;
    default: assert("not implemented" == nullptr); break;
    }
}


bool pow_verifier::project_star(size_t id, projected_star &star) {
    switch (super_difficulty_) {
    case 7: return project_star_t<7>(id, star); break;
    case 8: return project_star_t<8>(id, star); break;
    case 9: return project_star_t<9>(id, star); break;
    default: assert("not implemented" == nullptr); break;
    }
    return false;
}

bool verify_dipper(const siphash_keys &key, size_t super_difficulty, size_t proof_number, size_t nonce, const uint32_t star_ids[7]) {
    pow_verifier verifier(key, super_difficulty);
    std::vector<projected_star> stars;
    projected_star star;
    verifier.setup(proof_number, nonce);
    for (size_t star_no = 0; star_no < 7; star_no++) {
	if (verifier.project_star(star_ids[star_no], star)) {
	    stars.push_back(star);
	}
    }
    if (stars.size() != 7) {
	return false;
    }
    dipper_detector detector(stars);
    if (!detector.search(stars)) {
	return false;
    }
    return true;
}

bool verify_pow(const siphash_keys &key, size_t super_difficulty, const pow_proof &proof) {
    pow_verifier verifier(key, super_difficulty);
    std::vector<projected_star> stars;
    projected_star star;

    uint32_t row[pow_proof::ROW_SIZE];
    for (size_t row_no = 0; row_no < proof.num_rows(); row_no++) {
	proof.get_row(row_no, row);
	verifier.setup(row_no, static_cast<size_t>(row[0]));
	stars.clear();
	for (size_t star_no = 0; star_no < 7; star_no++) {
	    if (verifier.project_star(row[1+star_no], star)) {
		stars.push_back(star);
	    }
	}
	// We must have 7 stars
	if (stars.size() != 7) {
	    return false;
	}
	dipper_detector detector(stars);
	if (!detector.search(stars)) {
	    return false;
	}
    }

    return true;
}

}}
