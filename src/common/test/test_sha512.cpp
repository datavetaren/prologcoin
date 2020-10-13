#include <common/sha512.hpp>
#include <common/hex.hpp>
#include <iostream>
#include <assert.h>
#include <string>
#include <string.h>

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

    sha512 s2;
    s2.update("", 0);
    std::string str2 = s2.finalize();
    std::cout << "SHA512 of '' is: " << str2 << std::endl;
    assert(str2 == "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");

    sha512 s3;
    s3.update("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 448/8);
    std::string str3 = s3.finalize();
    std::cout << "SHA512 of 'abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq' is: " << str3 << std::endl;
    assert(str3 == "204a8fc6dda82f0a0ced7beb8e08a41657c16ef468b228a8279be331a703c33596fd15c13b1b07f9aa1d3bea57789ca031ad85c7a71dd70354ec631238ca3445");
}

int main(int argc, char *argv[])
{
    test_sha512();

    return 0;
}
