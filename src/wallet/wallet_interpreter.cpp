#include "../ec/builtins.hpp"
#include "../coin/builtins.hpp"
#include "wallet_interpreter.hpp"
#include "wallet.hpp"

namespace prologcoin { namespace wallet {

wallet_interpreter::wallet_interpreter(wallet &w, const std::string &wallet_file) : file_path_(wallet_file), wallet_(w) {
    load_builtins_file_io();
    ec::builtins::load(*this);
    coin::builtins::load(*this);
    setup_standard_lib();
    setup_local_builtins();
}

void wallet_interpreter::setup_local_builtins()
{
    load_builtin(con_cell("@",2), &wallet_interpreter::operator_at_2);
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
    
// Generate a new wallet file with some initial source code.
// In particular create a new seed for keys.
void wallet_interpreter::create(const char *password)
{
}
    
}}
