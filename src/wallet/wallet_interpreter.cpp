#include "../ec/builtins.hpp"
#include "../coin/builtins.hpp"
#include "wallet_interpreter.hpp"

namespace prologcoin { namespace wallet {

wallet_interpreter::wallet_interpreter(const std::string &wallet_file) : file_path_(wallet_file) {
    load_builtins_file_io();
    ec::builtins::load(*this);
    coin::builtins::load(*this);
    setup_standard_lib();
}

// Generate a new wallet file with some initial source code.
// In particular create a new seed for keys.
void wallet_interpreter::create(const char *password)
{
}
    
}}
