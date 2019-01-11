#include "pow_verifier.hpp"
#include "galaxy.hpp"
#include "camera.hpp"
#include "dipper_detector.hpp"
#include "blake2.hpp"

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

void pow_verifier::setup(uint64_t nonce_offset, uint32_t nonce) {
    switch (super_difficulty_) {
    case 7: setup_t<7>(nonce_offset, nonce); break;
    case 8: setup_t<8>(nonce_offset, nonce); break;
    case 9: setup_t<9>(nonce_offset, nonce); break;
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

bool verify_dipper(const siphash_keys &key, size_t super_difficulty, uint64_t nonce_offset, uint32_t nonce, const uint32_t star_ids[7]) {
    pow_verifier verifier(key, super_difficulty);
    std::vector<projected_star> stars;
    projected_star star;
    verifier.setup(nonce_offset, nonce);
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

bool verify_pow(const siphash_keys &key, size_t super_difficulty, const pow_difficulty &difficulty, const pow_proof &proof) {
    pow_verifier verifier(key, super_difficulty);
    std::vector<projected_star> stars;
    projected_star star;

    uint32_t row[pow_proof::ROW_SIZE];
    uint32_t nonce_sum = 0;
    for (size_t row_no = 0; row_no < proof.num_rows(); row_no++) {
        std::cout << "Verify row_no " << row_no << std::endl;
        uint64_t nonce_offset = static_cast<uint64_t>(nonce_sum) << 32;

	proof.get_row(row_no, row);

	// First check that the first entry in this row is a visible
	// star using the current nonce_offset.
	verifier.setup(nonce_offset, 0);
	uint32_t first_visible_star_id = row[0];
	projected_star first_visible_star;
	if (!verifier.project_star(first_visible_star_id, first_visible_star)) {
  	    std::cout << "Failed first visible star check (id=" << first_visible_star_id << ")" << std::endl;
	    return false;
	}
	// Then extract nonce and redirect camera to nonce_offset + nonce.
	uint32_t nonce = row[1];
	verifier.setup(nonce_offset, nonce);
	// Project the stars (remaining 7 entries)
	stars.clear();
	for (size_t star_no = 0; star_no < 7; star_no++) {
	    if (verifier.project_star(row[2+star_no], star)) {
		stars.push_back(star);
	    }
	}
	// We must have 7 visible stars
	if (stars.size() != 7) {
    	    std::cout << "Failed 7 star check" << std::endl;
	    return false;
	}
	// Check that we can detect the dipper
	dipper_detector detector(stars);
	if (!detector.search(stars)) {
      	    std::cout << "Failed to detect dipper" << std::endl;
	    return false;
	}
	nonce_sum += static_cast<uint64_t>(nonce) + 1;
    }

    // Final check is to ensure SHA256(proof) is below target.

    uint8_t proof_hash[32];
    uint8_t proof_bytes[pow_proof::TOTAL_SIZE_BYTES];
    proof.write(proof_bytes);

    blake2(proof_hash, sizeof(proof_hash), proof_bytes, pow_proof::TOTAL_SIZE_BYTES, nullptr, 0);

    uint8_t target[32];
    difficulty.get_target(target);
    int cmp = memcmp(proof_bytes, target, sizeof(target));

    if (cmp > 0) {
        return false;
    }

    return true;
}

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

