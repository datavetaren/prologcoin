#include <common/pbkdf2.hpp>
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

static void test_pbkdf2()
{
    header("test_pbkdf2");

    {
        pbkdf2 pd("salt", 4, 1, 20);
        pd.set_password("password", 8);
        const uint8_t *key = pd.get_key();
        std::string str = hex::to_string(key, 20);
        assert(str == "0c60c80f961f0e71f3a9b524af6012062fe037a6");
    }

    {
        pbkdf2 pd("salt", 4, 2, 20);
        pd.set_password("password", 8);
        const uint8_t *key = pd.get_key();
        std::string str = hex::to_string(key, 20);
        assert(str == "ea6c014dc72d6f8ccd1ed92ace1d41f0d8de8957");
    }

    {
        pbkdf2 pd("saltSALTsaltSALTsaltSALTsaltSALTsalt", 36, 4096, 25);
        pd.set_password("passwordPASSWORDpassword", 24);
        const uint8_t *key = pd.get_key();
        std::string str = hex::to_string(key, 25);
        assert(str == "3d2eec4fe41c849b80c8d83662c0e44a8b291a964cf2f07038");
    }
}

int main(int argc, char *argv[])
{
    test_pbkdf2();

    return 0;
}
