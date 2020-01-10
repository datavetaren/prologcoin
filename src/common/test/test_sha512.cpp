#include <common/sha512.hpp>
#include <common/hex.hpp>
#include <iostream>
#include <assert.h>
#include <string>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_sha512()
{
    header("test_sha512");

    const char *msg = "abc";
    sha512 s;
    s.update(msg, strlen(msg));
    std::string str = s.finalize();
    std::cout << "SHA512 of 'abc' is: " << str << std::endl;
    assert(str == "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");
}

int main(int argc, char *argv[])
{
    test_sha512();

    return 0;
}
