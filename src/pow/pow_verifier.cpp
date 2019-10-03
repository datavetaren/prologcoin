#include "pow_verifier.hpp"
#include "galaxy.hpp"
#include "camera.hpp"
#include "dipper_detector.hpp"
#include "blake2.hpp"
#include "isqrt.hpp"

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

    uint32_t row[pow_proof::ROW_SIZE];
    uint32_t nonce_sum = 0;
    for (size_t row_no = 0; row_no < proof.num_rows(); row_no++) {
        // std::cout << "Verify row_no " << row_no << std::endl;
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

	static const size_t NUM_STARS = 7;

	projected_star stars[NUM_STARS];

	for (size_t star_no = 0; star_no < NUM_STARS; star_no++) {
	    if (!verifier.project_star(row[2+star_no], stars[star_no])) {
		std::cout << "Failed to project star (id=" << row[2+star_no] << ")" << std::endl;
		return false;
	    }
	}

	// Instead of instantiating the dipper detector, we'll
	// use the light weight version of the same thing.

	static const int32_t dipper_dx[NUM_STARS] = { 0, 185, 325, 502, 588, 830, 805 };
	static const int32_t dipper_dy[NUM_STARS] = { 0, 116,  99,  93, -30,  63, 236 };

	static const int32_t dx12 = 185;
	static const int32_t dy12 = 116;
	static const int32_t d12 = isqrt(dx12*dx12+dy12*dy12);

        int32_t x0 = stars[0].x(), y0 = stars[0].y();
	int32_t x1 = stars[1].x(), y1 = stars[1].y();
	int32_t dx01 = x1-x0, dy01 = y1-y0;

	const int32_t d01_2 = dx01*dx01+dy01*dy01;

	// If the distance between the stars is too small (5 units) then
	// fail
	if (d01_2 < 5*5) {
	    return false;
	}
	const int32_t d01 = isqrt(d01_2);

	// Scale tolerance
	int32_t tol = pow_verifier::TOLERANCE*d01/d12;
	int32_t r2 = tol*tol/100;

	// Compute rotation/scaling matrix, that takes a vector from
	// the dipper default coordinate system to the placement
	// of the coordinate system in the sky.
	int32_t R_00 = (dx12*dx01+dy12*dy01) / d12;
	int32_t R_01 = (dy12*dx01-dx12*dy01) / d12;
	int32_t R_10 = -R_01;
	int32_t R_11 = R_00;

	for (size_t i = 2; i < NUM_STARS; i++) {
	    int32_t dx1i = dipper_dx[i];
	    int32_t dy1i = dipper_dy[i];

	    int32_t xip = (R_00*dx1i + R_01*dy1i) / d12 + x0;
	    int32_t yip = (R_10*dx1i + R_11*dy1i) / d12 + y0;

	    int32_t xi = stars[i].x(), yi = stars[i].y();
	    int32_t dx = xi - xip, dy = yi - yip;

	    if (100*(dx*dx+dy*dy) > r2) {
		return false;
	    }
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

