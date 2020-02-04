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
    setup_wallet_impl();
    // Make wallet inherit everything from wallet_impl
    use_module(functor("wallet_impl",0));
}

void wallet_interpreter::setup_local_builtins()
{
    static const con_cell M("wallet",0);
    load_builtin(M, con_cell("@",2), &wallet_interpreter::operator_at_2);
    load_builtin(M, con_cell("create",2), &wallet_interpreter::create_2);
}

void wallet_interpreter::setup_wallet_impl()
{
    std::string template_source = R"PROG(

%
% New key. Increment counter.
%
newkey(PublicKey, Address) :-
   wallet:numkeys(N),
   retract(wallet:numkeys(_)),
   N1 is N + 1,
   assert(wallet:numkeys(N1)),
   wallet:pubkey(N, PublicKey),
   ec:address(PublicKey, Address).

'$member2'(X, Y, [X|Xs], [Y|Ys]).
'$member2'(X, Y, [_|Xs], [_|Ys]) :- '$member2'(X, Y, Xs, Ys).

%
% Sync N heap references (to frozen closures)
%
sync(N) :-
    lastheap(H),
    H1 is H + 1,
    ((frozenk(H1, N, HeapAddrs), frozen(HeapAddrs, Closures)) @ global) @ node,
    (last(HeapAddrs, LastH) ->
        retract(wallet:lastheap(_)), assert(wallet:lastheap(LastH)) ; true),
    forall('$member2'(Closure, HeapAddress, Closures, HeapAddrs),
            ('$new_utxo_closure'(HeapAddress, Closure) ; true)).
    

%
% Iterate through next 100 frozen closures to see if we have received
% something we recognize.
%
sync :- 
    '$cache_addresses',
    sync(100).

%
% Compute public key addresses and store them in memory (using program database()
%
'$cache_addresses' :-
    (\+ current_predicate(cache:last_address/1) -> assert(cache:last_address(0)) ; true),
    cache:last_address(I),
    wallet:numkeys(N),
    '$cache_addresses_n'(I, N),
    retract(cache:last_address(_)),
    assert(cache:last_address(N)).

'$cache_addresses_n'(N, N) :- !.
'$cache_addresses_n'(I, N) :-
    wallet:pubkey(I, PubKey),
    ec:address(PubKey, Address),
    (\+ current_predicate(cache:valid_address/1) -> assert(cache:valid_address(Address)) ; true),
    (\+ cache:valid_address(Address) -> assert(cache:valid_address(Address)) ; true),
    I1 is I + 1,
    '$cache_addresses_n'(I1, N).


%
% Check this frozen closure for transaction type.
%
'$new_utxo_closure'(HeapAddress, '$freeze':F) :-
    functor(F, _, Arity),
    Arity >= 3,
    arg(2, F, TxType),
    arg(3, F, Args),
    arg(7, F, Value),
    ((current_predicate(wallet:new_utxo/4), wallet:new_utxo(TxType, HeapAddress, Value, Args)) -> true
 ; '$new_utxo'(TxType, HeapAddress, Value, Args)
    ).

%
% Check for each transaction type, currently only for 'tx1'
%
'$new_utxo'(tx1, HeapAddress, Value, args(_, _, Address)) :-
    current_predicate(cache:valid_address/1),
    cache:valid_address(Address),
    ((current_predicate(wallet:utxo/4), wallet:utxo(HeapAddress, _,_,_)) -> true
  ; assert(wallet:utxo(HeapAddress, tx1, Value, Address))).


)PROG";

    con_cell old_module = current_module();
    set_current_module(functor("wallet_impl",0));
    try {
        load_program(template_source);
    } catch (interpreter_exception &ex) {
        std::cout << "Error while loading internal wallet_impl source:" << std::endl;
        std::cout << ex.what() << std::endl;
    } catch (term_parse_exception &ex) {
        std::cout << "Error while loading internal wallet_impl source:" << std::endl;      
        std::cout << term_parser::report_string(*this, ex) << std::endl;
    } catch (token_exception &ex) {
        std::cout << "Error while loading internal wallet_impl source:" << std::endl;      
        std::cout << term_parser::report_string(*this, ex) << std::endl;
    }
    set_current_module(old_module);
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
