#pragma once

#ifndef _pow_verifier_hpp
#define _pow_verifier_hpp

#include <stdio.h>
#include <stdlib.h>
#include "siphash.hpp"
#include "fxp.hpp"
#include "galaxy.hpp"
#include "camera.hpp"

namespace prologcoin { namespace pow {

// The PoW proof consists of 32 rows, where each row consists of
// 1 + 7 (i.e. 8) 32-bit integers. The last 7 integers are the id numbers of
// the stars. The first integer is the nonce used for that proof number
// (which is determined by the row number.)
//
class pow_proof {
public:
    static const size_t NUM_ROWS = 32;
    static const size_t ROW_SIZE = 8;
    static const size_t TOTAL_SIZE = ROW_SIZE*NUM_ROWS;

    inline pow_proof(uint32_t proof[TOTAL_SIZE]) {
	memcpy(&data_[0], &proof[0], TOTAL_SIZE*sizeof(uint32_t));
    }

    inline size_t num_rows() const { return NUM_ROWS; }

    inline void get_row(size_t row_number, uint32_t row[ROW_SIZE]) const {
	memcpy(&row[0], &data_[row_number*ROW_SIZE], ROW_SIZE*sizeof(uint32_t));
    }
    
private:
    uint32_t data_[8*NUM_ROWS];
};

class pow_verifier {
public:
    static const int32_t TOLERANCE = 100;

    inline pow_verifier(const siphash_keys &key, size_t super_difficulty) 
        : key_(key), super_difficulty_(super_difficulty) {
	galaxy_ = nullptr;
	camera_ = nullptr;
    }

    inline ~pow_verifier() { cleanup(); }

    bool project_star(size_t id, projected_star &star);
    void setup(size_t proof_number, size_t nonce);
    void cleanup();

private:
    template<size_t N> void setup_t(size_t proof_number, size_t nonce) {
	if (galaxy_ == nullptr) {
	    auto *gal = new galaxy<N,fxp1648>(key_);
	    auto *cam = new camera<N,fxp1648>(*gal,0);
	    galaxy_ = gal;
	    camera_ = cam;
	}
	auto *cam = reinterpret_cast<camera<N,fxp1648> *>(camera_);
	cam->set_target(proof_number, nonce);
    }

    template<size_t N> bool project_star_t(size_t id, projected_star &star) {
	auto *cam = reinterpret_cast<camera<N,fxp1648> *>(camera_);
	return cam->project_star(id, star);
    }

    template<size_t N> void cleanup_t() {
	auto *gal = reinterpret_cast<galaxy<N,fxp1648> *>(galaxy_);
	auto *cam = reinterpret_cast<camera<N,fxp1648> *>(camera_);
        delete gal;
        delete cam;
    }

    const siphash_keys &key_;
    size_t super_difficulty_;
    void *galaxy_;
    void *camera_;
};

bool verify_pow(const siphash_keys &key, size_t super_difficulty, const pow_proof &proof);

bool verify_dipper(const siphash_keys &key, size_t super_difficulty, size_t proof_number, size_t nonce, const uint32_t star_ids[7]);

}}

#endif
