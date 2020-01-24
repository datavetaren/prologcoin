#pragma once

#ifndef _wallet_wallet_interpreter_hpp
#define _wallet_wallet_interpreter_hpp

#include "../common/term_env.hpp"
#include "../common/term_serializer.hpp"
#include "../interp/interpreter.hpp"

namespace prologcoin { namespace wallet {

class wallet;
    
class wallet_interpreter : public interp::interpreter {
public:
    wallet_interpreter(wallet &w, const std::string &wallet_file);

    void create(const char *password);
    void save();

    void execute_at(term query, term_env &query_src, const std::string &where);
    void continue_at(term_env &query_src, const std::string &where);
    void delete_instance_at(term_env &query_src, const std::string &where);
  
private:
    void setup_local_builtins();

    wallet & get_wallet() { return wallet_; }
  
    static bool operator_at_2(interpreter_base &interp, size_t arity, term args[]);
  
    std::string file_path_;
    wallet &wallet_;
};
    
}}

#endif
