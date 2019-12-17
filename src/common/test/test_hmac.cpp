#include <common/sha1.hpp>
#include <common/hmac.hpp>
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

static void test_hmacsha1()
{
  header("test_hmacsha1");

  {
    const uint8_t key[20] = {0xb,0xb,0xb,0xb,0xb,0xb,0xb,0xb,
			     0xb,0xb,0xb,0xb,0xb,0xb,0xb,0xb,
			     0xb,0xb,0xb,0xb};
    hmac<sha1> h;
    h.init(key, 20);
    h.update("Hi There", 8);
    uint8_t digest[hmac<sha1>::HASH_SIZE];
    h.finalize(digest);
    std::string str = hex::to_string(digest, sizeof(digest));
    assert(str == "b617318655057264e28bc0b6fb378c8ef146be00");
  }

  {
    hmac<sha1> h;
    h.init("Jefe", 4);
    h.update("what do ya want for nothing?", 28);
    uint8_t digest[h.HASH_SIZE];
    h.finalize(digest);
    std::string str = hex::to_string(digest, sizeof(digest));
    assert(str == "effcdf6ae5eb2fa2d27416d5f184df9c259a7c79");
  }
}

int main(int argc, char *argv[])
{
    test_hmacsha1();

    return 0;
}
