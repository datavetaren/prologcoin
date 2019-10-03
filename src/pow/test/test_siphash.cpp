#include <iostream>
#include <sstream>
#include <assert.h>
#include <string.h>
#include "../siphash.hpp"

using namespace prologcoin::pow;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_byte_order()
{
    header("test_byte_order");

    const uint8_t bytes[] = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80 };
    uint64_t actual = u64<8>(bytes);
    uint64_t expect = 0x8070605040302010;

    std::cout << "Actual: " << std::hex << actual << std::endl;
    std::cout << "Expect: " << std::hex << expect << std::endl;

    assert(actual == expect);
}

static void test_siphash_keys()
{
    header("test_siphash_keys");

    const char *msg = "hello42";
    
    siphash_keys keys(msg, strlen(msg));

    const uint64_t expect[4] = { 0xb56773482498b8c6,
				 0xb896f61c6dc6d829,
				 0x280d80b6ad40a6cd,
				 0xa2fbd58ba697e318 };
    
    uint64_t actual[4] = { keys.k0(), keys.k1(), keys.k2(), keys.k3() };

    for (size_t i = 0; i < 4; i++) {
	std::cout << "Actual: key" << i << "=" << std::hex << actual[i] << std::endl;
	std::cout << "Expect: key" << i << "=" << std::hex << expect[i] << std::endl;
	assert(actual[i] == expect[i]);
    }
}

int main(int argc, char *argv[])
{
    test_byte_order();
    test_siphash_keys();

    return 0;
}
