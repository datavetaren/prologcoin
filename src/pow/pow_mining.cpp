#include "pow_mining.hpp"
#include "observatory.hpp"
#include "blake2.hpp"

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

static bool scan(void *obs, size_t super_difficulty, uint64_t nonce_offset,
		 projected_star &first_visible,
		 std::vector<projected_star> &found, uint32_t &nonce) {
    // static std::ofstream outfile("xxx.txt");

    bool r = false;
    switch (super_difficulty) {
    case 7: r = reinterpret_cast<observatory<7, double> *>(obs)->scan(
   	          nonce_offset, first_visible, found, nonce); break;
    case 8: r = reinterpret_cast<observatory<8, double> *>(obs)->scan(
	          nonce_offset, first_visible, found, nonce); break;
    case 9: r = reinterpret_cast<observatory<9, double> *>(obs)->scan(
	          nonce_offset, first_visible, found, nonce); break;
    default: assert("Not implemented" == nullptr);
    }
    if (r) {
	return true;
    } else {
        return false;
    }
}

bool search_proof(const siphash_keys &key, size_t super_difficulty, const pow_difficulty &difficulty, pow_proof &out_proof) {
    void *obs = nullptr;

    class cleanup {
    public:
        cleanup(size_t super_difficulty, void *obs) : super_difficulty_(super_difficulty), obs_(obs) { }
        ~cleanup() {
	  switch (super_difficulty_) {
	  case 7: delete reinterpret_cast<observatory<7, double> *>(obs_); break;
	  case 8: delete reinterpret_cast<observatory<8, double> *>(obs_); break;
	  case 9: delete reinterpret_cast<observatory<9, double> *>(obs_); break;
	  }
        }
        size_t super_difficulty_;
        void *obs_;
    };
    
    switch (super_difficulty) {
    case 7: obs = new observatory<7, double>(key); break;
    case 8: obs = new observatory<8, double>(key); break;
    case 9: obs = new observatory<9, double>(key); break;
    }

    cleanup c(super_difficulty, obs);
    
    uint32_t nonce_sum = 0;
    for (size_t proof_no = 0; proof_no < pow_proof::NUM_ROWS; proof_no++) {
	uint32_t nonce = 0;
	uint64_t nonce_offset = static_cast<uint64_t>(nonce_sum) << 32;
	projected_star first_visible;
	std::vector<projected_star> found;
	if (!scan(obs, super_difficulty, nonce_offset, first_visible, found, nonce)) {
	    return false;
	}

	uint32_t proof_round[pow_proof::ROW_SIZE];
	proof_round[0] = first_visible.id();
	proof_round[1] = nonce;
	size_t i = 2;
	for (auto &star : found) {
	    proof_round[i++] = star.id();
	}
	out_proof.set_row(proof_no, proof_round);

	nonce_sum += nonce + 1;
    }

    uint8_t proof_hash[32];
    uint8_t proof_bytes[pow_proof::TOTAL_SIZE_BYTES];
    out_proof.write(proof_bytes);

    blake2(proof_hash, sizeof(proof_hash), proof_bytes, pow_proof::TOTAL_SIZE_BYTES, nullptr, 0);

    uint8_t target[32];
    difficulty.get_target(target);
    int cmp = memcmp(proof_bytes, target, sizeof(target));

    if (cmp > 0) {
        return false;
    }

    if (!verify_pow(key, super_difficulty, difficulty, out_proof)) {
        std::cout << "Failed verification of proof!" << std::endl;
	return false;
    }

    return true;
}

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

