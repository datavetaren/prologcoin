#include <assert.h>
#include <iostream>
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

    // spin(msg, 33);

    for (size_t i = 0; i < 39; i++) {
	siphash_keys keys(msg, strlen(msg));
	pow_difficulty difficulty(flt1648(1));
	pow_proof proof;
	search_proof( keys, 8, difficulty, proof);

	spin(msg, 1);

	// std::cout << msg << ": mean=" << proof.mean_nonce() << " geometric=" << proof.geometric_mean_nonce() << std::endl;

    }
}

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "-mining") == 0) {
	test_pow_mining();
    } else {
        header("main");
    }

    return 0;
}


