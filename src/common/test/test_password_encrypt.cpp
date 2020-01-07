#include <common/pbkdf2.hpp>
#include <common/aes256.hpp>
#include <common/term_serializer.hpp>
#include <common/term_env.hpp>
#include <iostream>
#include <assert.h>
#include <string>
#include <vector>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_password_encrypt()
{
    header("test_password_encrypt");

    // Let's encrypt some term

    term_env env;
    auto t = env.parse("hello(world([1,2,42,4711]),foo,bar).");
    std::string term_str = env.to_string(t);

    std::cout << "Original term: " << term_str << std::endl;

    // Serialize it
    term_serializer::buffer_t buf;
    term_serializer ser(env);
    ser.write(buf, t);

    // Create key from password
    pbkdf2 pd("saltSALTsaltSALTsaltSALTsaltSALTsalt", 36, 4096, 25);
    pd.set_password("passwordPASSWORDpassword", 24);
    const uint8_t *key = pd.get_key();

    assert(pbkdf2::MAX_KEY_SIZE >= aes256::KEY_SIZE);

    // Encrypt serialized buffer
    aes256 aes(key, aes256::KEY_SIZE);
    assert(pd.get_salt_length() >= 16);
    aes.set_iv(pd.get_salt(), 16);
    aes.cbc_encrypt(buf);

    // Copy over to new buffer
    term_serializer::buffer_t buf2;
    buf2.resize(buf.size());
    std::copy(&buf[0], &buf[0]+buf.size(), &buf2[0]);

    // Decrypt
    aes256 aes2(key, aes256::KEY_SIZE);
    aes2.set_iv(pd.get_salt(), 16);
    aes2.cbc_decrypt(buf2);

    // Deserialize
    term_env env2;
    term_serializer ser2(env2);
    term t2 = ser2.read(buf2);

    std::string decrypted_term_str = env2.to_string(t2);

    std::cout << "Decrypt term : " << decrypted_term_str << std::endl;

    assert(term_str == decrypted_term_str);
}

int main(int argc, char *argv[])
{
    test_password_encrypt();

    return 0;
}
