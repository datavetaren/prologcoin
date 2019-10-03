#include <assert.h>
#include <iostream>

#include <boost/chrono.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

#include "../pow_verifier.hpp"
#include "../pow_mining.hpp"

using namespace prologcoin::pow;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void increment(char *msg) {
    msg[strlen(msg)-1]++;
    if (msg[strlen(msg)-1] > 'z') {
	msg[strlen(msg)-1] = '0';
	msg[strlen(msg)-2]++;
	if (msg[strlen(msg)-2] > 'z') {
	    msg[strlen(msg)-2] = '0';
	    msg[strlen(msg)-3]++;
	}
    }
}

static void spin(char *msg, size_t n) {
    while (n > 0) {
	increment(msg);
	n--;
    }
}

static void test_pow_mining()
{
    header("test_pow_mining");

    char msg[8] = "hello42";
    pow_difficulty difficulty(flt1648(1));

    // spin(msg, 33);

    for (size_t i = 0; i < 39; i++) {
	siphash_keys keys(msg, strlen(msg));
	pow_proof proof;

	auto start_time = boost::posix_time::microsec_clock::universal_time();

	if (!search_proof( keys, 8, difficulty, proof)) {
	    std::cout << "Failed to find proof for '" << msg << "' " << std::endl;
	}
	
	auto end_time = boost::posix_time::microsec_clock::universal_time();

	std::cout << "==> Proof took " << (end_time - start_time) << " seconds" << std::endl;

        // proof.write( "proof.bin" );

	spin(msg, 1);

	// std::cout << msg << ": mean=" << proof.mean_nonce() << " geometric=" << proof.geometric_mean_nonce() << std::endl;
    }

#if 0
    pow_proof proof;
    proof.read("proof.bin");
    
    auto start_time = boost::posix_time::microsec_clock::universal_time();    
    const size_t ITER = 1;
    for (size_t i = 0; i < ITER; i++) {
	siphash_keys keys(msg, strlen(msg)); 
	if (!verify_pow( keys, 8, difficulty, proof )) {
	    std::cout << "Proof is not ok" << std::endl;
	}
    }
    auto end_time = boost::posix_time::microsec_clock::universal_time();

    std::cout << "==> Verification took " << ((end_time - start_time)/ITER).total_microseconds() << " u_seconds" << std::endl;
#endif

}

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "--mining") == 0) {
	test_pow_mining();
    } else {
        header("main");
    }

    return 0;
}
