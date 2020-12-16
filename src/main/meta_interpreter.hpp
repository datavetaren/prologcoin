#pragma once

#ifndef _main_meta_interpreter_hpp
#define _main_meta_interpreter_hpp

#include "../common/term_env.hpp"
#include "../common/term_serializer.hpp"
#include "../interp/interpreter.hpp"
#include "../node/self_node.hpp"
#include "../terminal/terminal.hpp"
#include "../wallet/wallet.hpp"

namespace prologcoin { namespace main {

class meta_interpreter : public interp::interpreter {
public:
    meta_interpreter(const std::string &home_dir);
    ~meta_interpreter();

    const std::string & home_dir() const { return home_dir_; }
    
    void total_reset();

    void pulse();
    std::string flush_text();

    term_env & env() { return *this; }
    
private:
    void init();
    void setup_local_builtins();

    terminal::terminal * get_node_terminal(const std::string &node_name);

    interp::remote_return_t execute_at_node(term query, term_env &query_src, const std::string &where, bool silent);
    interp::remote_return_t continue_at_node(term_env &query_src, const std::string &where);
    bool delete_instance_at_node(term_env &query_src, const std::string &where);

    interp::remote_return_t execute_at_wallet(term query, term_env &query_src, const std::string &where, bool silent);
    interp::remote_return_t continue_at_wallet(term_env &query_src, const std::string &where);
    bool delete_instance_at_wallet(term_env &query_src, const std::string &where);
    
    static bool operator_at_impl(interpreter_base &interp, size_t arity, term args[], bool silent);
    static bool operator_at_2(interpreter_base &interp, size_t arity, term args[]);
    static bool operator_at_silent_2(interpreter_base &interp, size_t arity, term args[]);

    static bool erase_all_0(interpreter_base &interp, size_t arity, term args[]);
    static bool start_3(interpreter_base &interp, size_t arity, term args[]);
    static bool connect_2(interpreter_base &interp, size_t arity, term args[]);
  
    std::string home_dir_;

    // This is ok, because the main interpreter is a local only thing
    // and not part of consensus.
    heap::disabled_coin_security no_coin_security_;

    std::stringstream text_;

    // All nodes
    std::map<std::string, std::pair<node::self_node *, terminal::terminal *> > nodes_;
    // All wallets
    std::map<std::string, wallet::wallet *> wallets_;
};
    
}}

#endif
