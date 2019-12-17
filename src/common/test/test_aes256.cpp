#include <common/aes256.hpp>
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

static void test_aes256()
{
    header("test_aes256");

    const uint8_t key[32] = {0x60,0x3D,0xEB,0x10,0x15,0xCA,0x71,0xBE,
			     0x2B,0x73,0xAE,0xF0,0x85,0x7D,0x77,0x81,
			     0x1F,0x35,0x2C,0x07,0x3B,0x61,0x08,0xD7,
			     0x2D,0x98,0x10,0xA3,0x09,0x14,0xDF,0xF4};
    const uint8_t iv[16] = {0x9C,0xFC,0x4E,0x96,0x7E,0xDB,0x80,0x8D,
			    0x67,0x9F,0x77,0x7B,0xC6,0x70,0x2C,0x7D};
    const uint8_t msg[16] = {0x30,0xC8,0x1C,0x46,0xA3,0x5C,0xE4,0x11,
			     0xE5,0xFB,0xC1,0x19,0x1A,0x0A,0x52,0xEF};
    
    aes256 aes(key, 32);
    aes.set_iv(iv, 16);
    uint8_t data[16];
    memcpy(data, msg, 16);
    aes.cbc_encrypt(data, 16);

    std::string expect = "39f23369a9d9bacfa530e26304231461";
    assert(hex::to_string(data, 16) == expect);

    aes.set_iv(iv, 16);
    aes.cbc_decrypt(data, 16);
    assert(memcmp(data, msg, 16) == 0);
}

int main( int argc, char *argv[] )
{
    test_aes256();
}

