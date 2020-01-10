#pragma once

#ifndef _wallet_wallet_interpreter_hpp
#define _wallet_wallet_interpreter_hpp

#include "../common/term_env.hpp"
#include "../common/term_serializer.hpp"
#include "../interp/interpreter.hpp"

namespace prologcoin { namespace wallet {

class wallet_interpreter : public interp::interpreter {
public:
    wallet_interpreter(const std::string &wallet_file);

    void create(const char *password);
    void save();

private:
    std::string file_path_;
};
    
}}

#endif
