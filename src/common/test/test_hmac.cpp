#include <common/sha1.hpp>
#include <common/hmac.hpp>
#include <common/hex.hpp>
#include <iostream>
#include <assert.h>
#include <string>
#include <common/sha512.hpp>

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

static void test_hmacsha2()
{
    header("test_hmacsha2");
  
    hmac<sha512> h;
    uint8_t key[20] = {0xb,0xb,0xb,0xb,0xb,0xb,0xb,0xb,0xb,0xb,
		       0xb,0xb,0xb,0xb,0xb,0xb,0xb,0xb,0xb,0xb};
    uint8_t data[8] = {'H', 'i', ' ', 'T', 'h', 'e', 'r', 'e'};

    h.init(key, 20);
    h.update(data, 8);
    uint8_t result[64];
    h.finalize(result);
    
    std::cout << "HMAC-SHA512: " << hex::to_string(result, 64) << std::endl;

    assert(hex::to_string(result,64) ==
	   "87aa7cdea5ef619d4ff0b4241a1d6cb02379f4e2ce4ec2787ad0b30545e17cde"
	   "daa833b7d6b8a702038b274eaea3f4e4be9d914eeb61f1702e696c203a126854");
}

int main(int argc, char *argv[])
{
    test_hmacsha1();
    test_hmacsha2();

    return 0;
}
