#include <assert.h>
#include <iostream>
#include <boost/filesystem.hpp>

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#endif

#include "../pow_server.hpp"
#include "../observatory.hpp"
#include "../pow_verifier.hpp"
#include "../../common/test/test_home_dir.hpp"

using namespace prologcoin::pow;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void run_server()
{
    const std::string home_dir = find_home_dir();

    // auto dir = boost::filesystem::path(home_dir);
    
    std::string pow_dir = (boost::filesystem::path(home_dir) / "src" / "pow" / "test").string();

    header("Running server at port 8080");

    pow_server server("localhost", "8080", pow_dir);
    server.run();
}

static void run_scan()
{
    header("run_scan");

    char msg[8] = "hello42";

    siphash_keys keys(msg, strlen(msg));
    observatory<8,double> obs(keys);
    obs.status();

    uint32_t nonce_sum = 0;
    for (size_t proof_number = 0; proof_number < 32; proof_number++) {
	uint64_t nonce_offset = static_cast<uint64_t>(nonce_sum) << 32;
	std::vector<projected_star> found;
	projected_star first_visible;
	uint32_t nonce = 0;
	if (obs.scan(nonce_offset, first_visible, found, nonce)) {
	    std::cout << "Found dipper for proof_number=" << proof_number << " at nonce=" << nonce;
	    uint32_t star_ids[7];
	    size_t i = 0;
	    for (auto &f : found) {
		star_ids[i++] = f.id();
		if (i == 7) break; // Extra safety
	    }
	    if (i == 7 && verify_dipper(obs.keys(), 8, proof_number, nonce, star_ids)) {
		std::cout << ": Verified! OK" << std::endl;
		nonce_sum += nonce + 1;
	    } else {
		std::cout << ": ERROR - failed to verify!" << std::endl;
		break;
	    }
	}
    }
}

int main(int argc, char *argv[])
{
    find_home_dir(argv[0]);
    
    if (argc == 1) {
	std::cout << "Running tests. If you want to run the server interactively, pass --server" << std::endl;
    } else if (argc == 2) {
	if (strcmp(argv[1], "--server") == 0) {
	    run_server();
	}
	if (strcmp(argv[1], "--scan") == 0) {
	    run_scan();
	}
    }

    return 0;
}
