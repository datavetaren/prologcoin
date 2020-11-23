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

    void total_reset();

    void execute_at(term query, term_env &query_src, const std::string &where, bool silent);
    void continue_at(term_env &query_src, const std::string &where);
    void delete_instance_at(term_env &query_src, const std::string &where);

    wallet & get_wallet() { return wallet_; }
  
private:
    void init();
    void setup_local_builtins();
    void setup_wallet_impl();

    static bool create_2(interpreter_base &interp, size_t arity, term args[]);
    static bool save_0(interpreter_base &interp, size_t arity, term args[]);
    static bool load_0(interpreter_base &interp, size_t arity, term args[]);    
    static bool file_1(interpreter_base &interp, size_t arity, term args[]);
    static bool auto_save_1(interpreter_base &interp, size_t arity, term args[]);
    static bool operator_at_impl(interpreter_base &interp, size_t arity, term args[], bool silent);
    static bool operator_at_2(interpreter_base &interp, size_t arity, term args[]);
    static bool operator_at_silent_2(interpreter_base &interp, size_t arity, term args[]);    
  
    std::string file_path_;
    wallet &wallet_;

    // This is ok, because the wallet interpreter is a local only thing
    // and not part of consensus.
    heap::disabled_coin_security no_coin_security_;
};
    
}}

#endif
