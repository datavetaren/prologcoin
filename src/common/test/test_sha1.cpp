#include <common/sha1.hpp>
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

static void test_sha1()
{
    header("test_sha1");

    const char *msg = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    sha1 s;
    s.update(msg, strlen(msg));
    std::string str = s.finalize();
    assert(str == "84983e441c3bd26ebaae4aa1f95129e5e54670f1");

    const char *msgb = "abc";
    sha1 s2;
    s2.update(msgb, strlen(msgb));
    std::string strb = s2.finalize();
    assert(strb == "a9993e364706816aba3e25717850c26c9cd0d89d");
}

int main(int argc, char *argv[])
{
    test_sha1();

    return 0;
}
