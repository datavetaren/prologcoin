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
% Restart wallet sweep. Clean UTXO database and start from heap address 0.
% Call one sync step (which syncs 100 UTXOs)
%
resync :-
    retractall(wallet:utxo(_,_,_,_)),
    retractall(lastheap(_)),
    assert(lastheap(0)),
    sync.

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
    (\+ current_predicate(cache:valid_address/2) -> assert(cache:valid_address(Address,I)) ; true),
    (\+ cache:valid_address(Address,_) -> assert(cache:valid_address(Address,I)) ; true),
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
    arg(5, F, Coin),
    arg(1, Coin, Value),
    ((current_predicate(wallet:new_utxo/4), wallet:new_utxo(TxType, HeapAddress, Value, Args)) -> true
 ; '$new_utxo'(TxType, HeapAddress, Value, Args)
    ).

%
% Check for each transaction type, currently only for 'tx1'
%
'$new_utxo'(tx1, HeapAddress, Value, args(_, _, Address)) :-
    current_predicate(cache:valid_address/2),
    cache:valid_address(Address,_),
    ((current_predicate(wallet:utxo/4), wallet:utxo(HeapAddress, _,_,_)) -> true
  ; assert(wallet:utxo(HeapAddress, tx1, Value, Address))).


%
% Quickly get total balance
%
balance(Balance) :-
    (current_predicate(wallet:utxo/4) ->
       findall(utxo(Value,HeapAddr), wallet:utxo(HeapAddr,_,Value,_), Values),
       '$sum'(Values, 0, Balance)
     ; Balance = 0
    ).

%
% Spending logic
%
spend_tx(Address, Funds, Fee, FinalTx, ConsumedUtxos) :-
    '$cache_addresses',
    findall(utxo(Value,HeapAddress), wallet:utxo(HeapAddress,_,Value,_),
            Utxos),
    sort(Utxos, SortedUtxos), % Sorted on value
    Sum is Funds + Fee,
    '$choose'(SortedUtxos, Sum, ChosenUtxos),
    '$sum'(ChosenUtxos, 0, ChosenSum),
    Rest is ChosenSum - Sum,
    '$open_utxos'(ChosenUtxos, Hash, Coins, Signs, Commands),
    (Coins = [SumCoin] ->
       Commands1 = Commands 
     ; append(Commands, [cjoin(Coins, SumCoin)], Commands1)),
    (Rest > 0 ->
         newkey(_, ChangeAddr),
         append(Commands1, [csplit(SumCoin, [Funds,Fee,Rest],
                                   [FundsCoin,_,RestCoin]),
                            tx(FundsCoin, _, tx1, args(_,_,Address), _),
                            tx(RestCoin, _, tx1, args(_,_,ChangeAddr),_)],
                Commands2)
    ; append(Commands1, [csplit(SumCoin, [Funds,Fee],[FundsCoin,_]),
                         tx(FundsCoin, _, tx1, args(_,_,Address),_)],
             Commands2)
    ),
    '$list_to_commas'(Commands2, Command),
    Script = (p(Hash) :- Command),
    ec:hash(Script, HashValue),
    '$signatures'(Signs, HashValue, SignAssigns),
    append(SignAssigns, [Script], Commands3),
    '$list_to_commas'(Commands3, FinalTx),
    findall(H, member(utxo(_,H), ChosenUtxos), ConsumedUtxos).

retract_utxos([]).
retract_utxos([HeapAddr|HeapAddrs]) :-
    retract(wallet:utxo(HeapAddr, _, _, _)),
    retract_utxos(HeapAddrs).

'$list_to_commas'([X], C) :- !, C = X.
'$list_to_commas'([X|Xs], C) :-
    C = (X, Y), '$list_to_commas'(Xs, Y).

'$signatures'([], _, []).
'$signatures'([sign(tx1, SignVar, PubKeyAddress)|Signs], HashValue,
             [(SignVar = Signature)|Commands]) :-
    cache:valid_address(PubKeyAddress, Count),
    wallet:privkey(Count, PrivKey),
    ec:sign(PrivKey, HashValue, Signature),
    '$signatures'(Signs, HashValue, Commands).

'$open_utxos'([], _, [], [], []).
'$open_utxos'([utxo(_,HeapAddress)|Utxos], Hash,
              [Coin|Coins], [Sign|Signs], Commands) :-
   '$open_utxo'(HeapAddress, Hash, Coin, Sign, Commands0),
   '$open_utxos'(Utxos, Hash, Coins, Signs, Commands1),
   append(Commands0, Commands1, Commands).
    
'$open_utxo'(HeapAddress, Hash, Coin, Sign, Commands) :-
    wallet:utxo(HeapAddress, TxType, _, Data),
    '$open_utxo_tx'(TxType, HeapAddress, Data, Hash, Coin, Sign, Commands).

'$open_utxo_tx'(tx1, HeapAddress, PubKeyAddress, Hash, Coin, sign(tx1,Signature,PubKeyAddress), Commands) :-
    cache:valid_address(PubKeyAddress, Count),
    wallet:pubkey(Count, PubKey),
    Commands = [defrost(HeapAddress, Closure,
                       [Hash, args(Signature,PubKey,PubKeyAddress)]),
                arg(4, Closure, Coin)].

'$choose'(_, 0, []) :- !.
'$choose'([utxo(Value,HeapAddress)|Utxos], Funds, Chosen) :-
    Value =< Funds, !, Chosen = [utxo(Value,HeapAddress)|ChosenUtxos],
   Funds1 is Funds - Value,
   '$choose'(Utxos, Funds1, ChosenUtxos).
'$choose'([Utxo], _, [Utxo]) :- !.
'$choose'([Utxo|Utxos], Funds, ChosenUtxos) :-
   '$choose'(Utxos, Funds, ChosenUtxos).

'$sum'([], Sum, Sum).
'$sum'([utxo(Value,_)|Rest], In, Out) :-
    '$sum'(Rest, In, Out0),
    Out is Out0 + Value.

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
	sentence = args[1];
    }

    std::string passwd = interp.list_to_string(args[0]);

    auto &w = reinterpret_cast<wallet_interpreter &>(interp).get_wallet();
    
    w.create(passwd, sentence);

    return interp.unify(args[1], sentence);
}
    
}}
