#pragma once

#ifndef _pow_verifier_hpp
#define _pow_verifier_hpp

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <fstream>
#include "siphash.hpp"
#include "fxp.hpp"
#include "flt.hpp"
#include "galaxy.hpp"
#include "camera.hpp"

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

//
// pow difficulty is a value beteen 1..inf (we use floating-point to represent it)
// It's inverse (1/difficulty) * (2^256)-1 determines the target value
// (hashes must be below target value.)
class pow_difficulty {
public:
    inline pow_difficulty(const flt1648 &val) : difficulty_(val) { 
    }

    // Increase (or decrease) difficulty with a scaling factor 'val'
    inline pow_difficulty scale(const flt1648 &val) const {
	return difficulty_ * val;
    }

    inline void get_target(uint8_t target[32]) const {
	static const flt1648 ONE(1);
	if (difficulty_ == ONE) {
	    memset(target, 0xff, 32);
	    return;
	}
	// Copy inverse difficulty to MSB
	flt1648 normalized_target = difficulty_.reciprocal();
	normalized_target.to_integer(&target[0], 32);
    }

private:
    flt1648 difficulty_;
};

// The PoW proof consists of 16 rows, where each row consists of
// 3 + 7 (i.e. 8) 32-bit integers. The last 7 integers are the id numbers of
// the stars. The first integer is the nonce sum used to compute nonce offset.
// The second is a star identifier that is visible for that nonce offset and
// the third is the nonce found for the detected dipper.
//
class pow_proof {
public:
    static const size_t NUM_ROWS = 16;
    static const size_t ROW_SIZE = 9;
    static const size_t TOTAL_SIZE = ROW_SIZE*NUM_ROWS;
    static const size_t TOTAL_SIZE_BYTES = TOTAL_SIZE*sizeof(uint32_t);

    inline pow_proof() {
	memset(&data_[0], 0, TOTAL_SIZE_BYTES);
    }

    inline pow_proof(uint32_t proof[TOTAL_SIZE]) {
	memcpy(&data_[0], &proof[0], TOTAL_SIZE_BYTES);
    }

    inline void write(const std::string &path) const {
	std::ofstream f(path, std::ios::out | std::ios::binary);
	write(f);
    }

    inline void write(std::ostream &out) const {
	uint8_t all_bytes[TOTAL_SIZE_BYTES];
	write(&all_bytes[0]);
	out.write(reinterpret_cast<char *>(&all_bytes[0]), TOTAL_SIZE_BYTES);
    }

    inline void write(uint8_t *bytes) const {
	size_t i = 0;
	for (size_t row = 0; row < NUM_ROWS; row++) {
	    for (size_t j = 0; j < ROW_SIZE; j++) {
		uint32_t word = data_[row*ROW_SIZE+j];
		bytes[i++] = static_cast<uint8_t>((word >> 24) & 0xff);
		bytes[i++] = static_cast<uint8_t>((word >> 16) & 0xff);
		bytes[i++] = static_cast<uint8_t>((word >> 8) & 0xff);
		bytes[i++] = static_cast<uint8_t>((word >> 0) & 0xff);
	    }
	}
    }

    inline void read(const uint8_t *bytes) {
	for (size_t row = 0; row < NUM_ROWS; row++) {
	    for (size_t j = 0; j < ROW_SIZE; j++) {
		uint8_t b0 = bytes[sizeof(uint32_t)*(row*ROW_SIZE+j)];
		uint8_t b1 = bytes[sizeof(uint32_t)*(row*ROW_SIZE+j)+1];
		uint8_t b2 = bytes[sizeof(uint32_t)*(row*ROW_SIZE+j)+2];
		uint8_t b3 = bytes[sizeof(uint32_t)*(row*ROW_SIZE+j)+3];
		uint32_t word = (static_cast<uint32_t>(b0) << 24) |
	    	                (static_cast<uint32_t>(b1) << 16) |
	   	                (static_cast<uint32_t>(b2) << 8) |
	  	                static_cast<uint32_t>(b3);
	        data_[row*ROW_SIZE+j] = word;
	    }
	}
    }

    inline void read(std::istream &in) {
	uint8_t bytes[TOTAL_SIZE_BYTES];
	in.read(reinterpret_cast<char *>(&bytes[0]), TOTAL_SIZE_BYTES);
	read(&bytes[0]);
    }

    inline void read(const std::string &f) {
	std::ifstream in(f);
	read(in);
    }

    inline size_t num_rows() const { return NUM_ROWS; }

    inline void print( std::ostream &out ) const {
	for (size_t i = 0; i < NUM_ROWS; i++) {
	    uint32_t row[ROW_SIZE];
	    get_row(i, row);
	    out << "Row " << std::dec << std::setfill(' ') << std::setw(2) << i << ": & 0x" << std::hex << std::setfill('0') << std::setw(8) << row[0] << " & " << std::dec << std::setw(5) << std::setfill(' ') << row[1];
	    for (size_t j = 0; j < 7; j++) {
		out << " & 0x" << std::setfill('0') << std::setw(8) << std::hex << row[2+j];
	    }
	    std::cout << " \\\\" << std::dec << std::setfill(' ') << std::endl;
	}
    }

    inline void set_row(size_t row_number, uint32_t row[ROW_SIZE]) {
	memcpy(&data_[row_number*ROW_SIZE], &row[0], ROW_SIZE*sizeof(uint32_t));
    }

    inline void get_row(size_t row_number, uint32_t row[ROW_SIZE]) const {
	memcpy(&row[0], &data_[row_number*ROW_SIZE], ROW_SIZE*sizeof(uint32_t));
    }
    
private:
    uint32_t data_[ROW_SIZE*NUM_ROWS];
};

class pow_verifier {
public:
    // 800 = ~8 pixels diff (100 = 1 pixel) in template coordinate system
    static const int32_t TOLERANCE = 800;

    inline pow_verifier(const siphash_keys &key, size_t super_difficulty) 
        : key_(key), super_difficulty_(super_difficulty) {
	galaxy_ = nullptr;
	camera_ = nullptr;
    }

    inline ~pow_verifier() { cleanup(); }

    bool project_star(size_t id, projected_star &star);
    void setup(uint64_t nonce_offset, uint32_t nonce);
    void cleanup();

private:
  template<size_t N> void setup_t(uint64_t nonce_offset, uint32_t nonce) {
	if (galaxy_ == nullptr) {
	    auto *gal = new galaxy<N,fxp1648>(key_);
	    auto *cam = new camera<N,fxp1648>(*gal,0);
	    galaxy_ = gal;
	    camera_ = cam;
	}
	auto *cam = reinterpret_cast<camera<N,fxp1648> *>(camera_);
	cam->set_target(nonce_offset, nonce);
    }

    template<size_t N> vec3<fxp1648> star_coord(size_t id) {
	auto *gal = reinterpret_cast<galaxy<N,fxp1648> *>(galaxy_);
	return gal->star_to_vector(id);
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

bool verify_pow(const siphash_keys &key, size_t super_difficulty, const pow_difficulty &difficulty, const pow_proof &proof);

bool verify_dipper(const siphash_keys &key, size_t super_difficulty, uint64_t nonce_offset, uint32_t nonce, const uint32_t star_ids[7]);

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

#endif
