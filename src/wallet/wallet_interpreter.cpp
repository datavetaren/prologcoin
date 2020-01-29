#include "../ec/builtins.hpp"
#include "../ec/mnemonic.hpp"
#include "../coin/builtins.hpp"
#include "wallet_interpreter.hpp"
#include "wallet.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

namespace prologcoin { namespace wallet {

wallet_interpreter::wallet_interpreter(wallet &w, const std::string &wallet_file) : file_path_(wallet_file), wallet_(w), no_coin_security_(this->disable_coin_security()) {
    load_builtins_file_io();
    ec::builtins::load(*this);
    coin::builtins::load(*this);
    setup_standard_lib();
    set_current_module(con_cell("wallet",0));
    setup_local_builtins();
}

void wallet_interpreter::setup_local_builtins()
{
    static const con_cell M("wallet",0);
    load_builtin(M, con_cell("@",2), &wallet_interpreter::operator_at_2);
    load_builtin(M, con_cell("create",2), &wallet_interpreter::create_2);
}

bool wallet_interpreter::operator_at_2(interpreter_base &interp, size_t arity, term args[]) {
    static con_cell NODE("node", 0);
    auto query = args[0];
    auto where_term = args[1];

    if (where_term != NODE) {
        throw interp::interpreter_exception_wrong_arg_type("@/2: Second argument must be the atom 'node'; was " + interp.to_string(where_term));
    }

    std::string where = interp.atom_name(where_term);

#define LL(x) reinterpret_cast<wallet_interpreter &>(interp)
	
    interp::remote_execution_proxy proxy(interp,
        [](interpreter_base &interp, term query, const std::string &where)
	   {return LL(interp).get_wallet().execute_at(query, interp, where);},
        [](interpreter_base &interp, const std::string &where)
	   {return LL(interp).get_wallet().continue_at(interp, where);},
	[](interpreter_base &interp, const std::string &where)
	   {return LL(interp).get_wallet().delete_instance_at(interp, where);});

    return proxy.start(query, where);
}

bool wallet_interpreter::create_2(interpreter_base &interp, size_t arity, term args[])
{
    if (!interp.is_string(args[0])) {
         throw interpreter_exception_wrong_arg_type(
	    "create/2: First argument must be a string (the password); was "
	    + interp.to_string(args[0]));
    }

    term sentence;

    ec::mnemonic mn(interp);
    
    if (args[1].tag() == tag_t::REF) {
        // Generate new sentence
	mn.generate_new(256);
	sentence = mn.to_sentence();
    } else {
        if (!mn.from_sentence(args[1])) {
	    throw interpreter_exception_wrong_arg_type(
	       "create/2: Invalid BIP39 sentence; was "
	    + interp.to_string(args[1]));
        }
    }

    std::string passwd = interp.list_to_string(args[0]);

    auto &w = reinterpret_cast<wallet_interpreter &>(interp).get_wallet();
    
    w.create(passwd, sentence);

    return interp.unify(args[1], sentence);
}
    
}}
