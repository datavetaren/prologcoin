#include "pow_mining.hpp"
#include "observatory.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace pow {

static void scan(void *obs, size_t super_difficulty, size_t proof_number,
		 std::vector<projected_star> &found, size_t &nonce) {

    static std::ofstream outfile("xxx.txt");

    bool r = false;
    switch (super_difficulty) {
    case 7: r = reinterpret_cast<observatory<7, double> *>(obs)->scan(
		  proof_number, found, nonce); break;
    case 8: r = reinterpret_cast<observatory<8, double> *>(obs)->scan(
		  proof_number, found, nonce); break;
    case 9: r = reinterpret_cast<observatory<9, double> *>(obs)->scan(
		  proof_number, found, nonce); break;
    default: assert("Not implemented" == nullptr);
    }

    std::cout << nonce << std::endl;
    outfile << nonce << std::endl;
}

bool search_proof(const siphash_keys &key, size_t super_difficulty, const pow_difficulty &difficulty, pow_proof &out_proof) {
    void *obs = nullptr;
    switch (super_difficulty) {
    case 7: obs = new observatory<7, double>(key); break;
    case 8: obs = new observatory<8, double>(key); break;
    case 9: obs = new observatory<9, double>(key); break;
    }
    for (size_t proof_no = 0; proof_no < pow_proof::NUM_ROWS; proof_no++) {
	size_t nonce = 0;
	std::vector<projected_star> found;
	scan(obs, super_difficulty, proof_no, found, nonce);
	uint32_t proof_round[8];
	size_t i = 0;
	for (auto &star : found) {
	    proof_round[i++] = star.id();
	}
	proof_round[i] = nonce;
	out_proof.set_row(proof_no, proof_round);
    }

    uint8_t proof_hash[32];
    uint8_t proof_bytes[pow_proof::TOTAL_SIZE_BYTES];
    out_proof.write(proof_bytes);
    blake2(proof_hash, sizeof(proof_hash), proof_bytes, pow_proof::TOTAL_SIZE_BYTES, nullptr, 0);

    uint8_t target[32];
    difficulty.get_target(target);
    int cmp = memcmp(proof_bytes, target, sizeof(target));
	
    return cmp <= 0;
}

}}
