#include "../../common/hex.hpp"
#include "../../common/term.hpp"
#include "../mnemonic.hpp"

using namespace prologcoin::common;
using namespace prologcoin::ec;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

// Test vectors taken from BIP39

static void test_english()
{
    term_env env;

    mnemonic mem(env);
    uint8_t bytes[16];
    hex::from_string("7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f", bytes, 16);
    mem.set(bytes, 16);
    term sent = mem.to_sentence();

    std::cout << "WE GOT: " << env.to_string(sent) << std::endl;
}

int main(int argc, char *argv[])
{
    test_english();

    return 0;
}

