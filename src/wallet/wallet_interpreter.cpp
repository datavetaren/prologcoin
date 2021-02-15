#include "../ec/builtins.hpp"
#include "../ec/mnemonic.hpp"
#include "../coin/builtins.hpp"
#include "wallet_interpreter.hpp"
#include "wallet.hpp"
#include <boost/filesystem/path.hpp>

using namespace prologcoin::common;
using namespace prologcoin::interp;

namespace prologcoin { namespace wallet {

wallet_interpreter::wallet_interpreter(wallet &w, const std::string &wallet_file) : interp::interpreter("wallet"), file_path_(wallet_file), wallet_(w), no_coin_security_(this->disable_coin_security()) {
    init();
}

void wallet_interpreter::init()
{
    set_debug_enabled();
    load_builtins_file_io();
    ec::builtins::load(*this);
    coin::builtins::load(*this);
    setup_standard_lib();
    set_current_module(con_cell("wallet",0));
    setup_local_builtins();
    setup_wallet_impl();
    // Make wallet inherit everything from wallet_impl
    use_module(functor("wallet_impl",0));
    set_auto_wam(true);
    set_retain_state_between_queries(true);
}

void wallet_interpreter::total_reset()
{
    interpreter::total_reset();
    disable_coin_security();
    init();
}

void wallet_interpreter::setup_local_builtins()
{
    static const con_cell M("wallet",0);
    load_builtin(M, con_cell("@",2), &wallet_interpreter::operator_at_2);
    load_builtin(M, con_cell("@-",2), &wallet_interpreter::operator_at_silent_2);
    load_builtin(M, con_cell("create",2), &wallet_interpreter::create_2);
    load_builtin(M, con_cell("save",0), &wallet_interpreter::save_0);
    load_builtin(M, functor("auto_save",1), &wallet_interpreter::auto_save_1);
    load_builtin(M, con_cell("load",0), &wallet_interpreter::load_0);
    load_builtin(M, con_cell("file",1), &wallet_interpreter::file_1);
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
    wallet:lastheap(H),
    H1 is H + 1,
    wallet:'@'(((frozenk(H1, N, HeapAddrs), frozen(HeapAddrs, Closures)) @ global), node),
    wallet:'@'(discard, node),
    HeapAddrs = [_|_], % At least one address!
    (last(HeapAddrs, LastH) ->
        retract(wallet:lastheap(_)), assert(wallet:lastheap(LastH)) ; true),
    forall('$member2'(Closure, HeapAddress, Closures, HeapAddrs),
            ('$new_utxo_closure'(HeapAddress, Closure) ; true)).
    

%
% Iterate through next 100 frozen closures to see if we have received
% something we recognize.
%
sync :- 
    '$cache_addresses', sync(100), !.
sync.

sync_all :-
    '$cache_addresses', sync_all0.
sync_all0 :-
    sync(100), !, sync_all0.
sync_all0.

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
       '$sum_utxo'(Values, 0, Balance)
     ; Balance = 0
    ).

%
% Spending logic
%
spend_one(Address, Amount, Fee, FinalTx, ConsumedUtxos) :-
    spend_many([Address], [Amount], Fee, FinalTx, ConsumedUtxos).

%
% spend_many is the generic predicate that can be used to 
% spend to many addresses with different amounts.
%

spend_many(Addresses, Amounts, Fee, FinalTx, ConsumedUtxos) :-
    '$cache_addresses',
    % --- Get all available UTXOs from wallet,
    findall(utxo(Value,HeapAddress), wallet:utxo(HeapAddress,_,Value,_),Utxos),
    % --- Sort all UTXOs on value 
    sort(Utxos, SortedUtxos),
    % --- Compute the total sum of funds that will be spent (excluding fee.)
    '$sum'(Amounts, AmountToSpend),
    % --- The total sum of funds that will be spent including fee.
    TotalAmount is AmountToSpend + Fee,
    % --- Find the UTXOs that we'd like to spend.
    '$choose'(SortedUtxos, TotalAmount, ChosenUtxos),
    % --- Compute its sum
    '$sum_utxo'(ChosenUtxos, 0, ChosenSum),
    % --- Rest is the remainder that will go back to self
    Rest is ChosenSum - TotalAmount,
    % --- Compute the list of instructions to open these UTXOs
    % --- Signs are the created signatures
    '$open_utxos'(ChosenUtxos, Hash, Coins, Signs, Commands),
    % --- Append a join instruction (if more than one UTXO)
    (Coins = [SumCoin] ->
       Commands1 = Commands 
     ; append(Commands, [cjoin(Coins, SumCoin)], Commands1)),
    % --- Create txs for all the provided addresses (and funds.)
    findall(tx(_, _, tx1, args(_,_,Address),_),
            member(Address, Addresses), Txs),
    % --- Collect all coin arguments for these txs into SendCoins
    '$get_coins'(Txs, SendCoins),
    % --- Do we get change?
    (Rest > 0 ->
         % --- Yes, create a new change address
         newkey(_, ChangeAddr),
         % --- All the different funds
         AllAmounts = [Fee,Rest|Amounts],
         % --- Note that SendCoins will be unified with the above
         append(Commands1,[csplit(SumCoin, AllAmounts,
                                  [_Fee,RestCoin|SendCoins]),
                           tx(RestCoin, _, tx1, args(_,_,ChangeAddr),_)|Txs],

                Commands2)
       ; AllAmounts = [Fee|Amounts],
         append(Commands1, [csplit(SumCoin, AllAmounts, [_Fee|SendCoins])|Txs],
                Commands2)
    ),
    % --- At this point Commands2 are all the combined instructions we need
    % --- Convert list to commas
    '$list_to_commas'(Commands2, Command),
    % --- The combined command as a self hash predicate (i.e. the script)
    Script = (p(Hash) :- Command),
    % --- Compute the hash for this script
    ec:hash(Script, HashValue),
    % --- Get the required signatures to "run" this script
    % --- (without them, the commitment would fail to the global sate.)
    % --- One signature per opened UTXO
    '$signatures'(Signs, HashValue, SignAssigns),
    % --- Prepend these signature statements to the script
    append(SignAssigns, [Script], Commands3),
    % --- Commands3 is the final instruction list, convert it to commas
    '$list_to_commas'(Commands3, FinalTx),
    % --- Return list of consumed utxos (so the user can remove them from
    % --- his wallet.)
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

'$open_utxo_tx'(tx1, HeapAddress, PubKeyAddress, Hash, Coin, sign(tx1,Signature,PubKeyAddress),Commands) :-
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

'$sum_utxo'([], Sum, Sum).
'$sum_utxo'([utxo(Value,_)|Rest], In, Out) :-
    '$sum_utxo'(Rest, In, Out0),
    Out is Out0 + Value.

'$get_coins'([], []).
'$get_coins'([tx(Coin,_,_,_,_)|Txs], [Coin|Coins]) :-
    '$get_coins'(Txs,Coins).

'$sum'([], 0).
'$sum'([X|Xs], Sum) :-
    '$sum'(Xs, Sum0),
    Sum is Sum0 + X.

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
    compile();
    set_current_module(old_module);
}

bool wallet_interpreter::operator_at_impl(interpreter_base &interp, size_t arity, term args[], const std::string &name, interp::remote_execute_mode mode) {
    static con_cell NODE("node", 0);
    auto query = args[0];

    term else_do;
    size_t timeout;
    auto where_term = args[1];

    std::tie(where_term, else_do, timeout) = interpreter::deconstruct_where(interp, where_term);

    if (where_term != NODE) {
        return interpreter::operator_at_impl(interp, arity, args, name, mode);
    }

    std::string where = interp.atom_name(where_term);

#define LL(x) reinterpret_cast<wallet_interpreter &>(interp)
	
    interp::remote_execution_proxy proxy(interp,
	[](interpreter_base &interp, term query, term else_do, const std::string &where, interp::remote_execute_mode mode, size_t timeout)
	  {return LL(interp).get_wallet().execute_at(query, else_do, interp, where, mode, timeout);},
	[](interpreter_base &interp, term query, term else_do, const std::string &where, interp::remote_execute_mode mode, size_t timeout)
	  {return LL(interp).get_wallet().continue_at(query, else_do, interp, where, mode, timeout);},
	[](interpreter_base &interp, const std::string &where)
	   {return LL(interp).get_wallet().delete_instance_at(interp, where);});

    proxy.set_mode(mode);
    proxy.set_timeout(timeout);
    return proxy.start(query, else_do, where);
}
	
bool wallet_interpreter::operator_at_2(interpreter_base &interp, size_t arity, term args[]) {
    return operator_at_impl(interp, arity, args, "@/2", interp::MODE_NORMAL);
}

bool wallet_interpreter::operator_at_silent_2(interpreter_base &interp, size_t arity, term args[]) {
    return operator_at_impl(interp, arity, args, "@-/2", interp::MODE_SILENT);
}

bool wallet_interpreter::operator_at_parallel_2(interpreter_base &interp, size_t arity, term args[]) {
    return operator_at_impl(interp, arity, args, "@=/2", interp::MODE_PARALLEL);
}

bool wallet_interpreter::save_0(interpreter_base &interp, size_t arity, term args[])
{
    auto &w = reinterpret_cast<wallet_interpreter &>(interp).get_wallet();
    w.save();
    return true;
}

bool wallet_interpreter::auto_save_1(interpreter_base &interp, size_t arity, term args[])
{
    auto &w = reinterpret_cast<wallet_interpreter &>(interp).get_wallet();

    static con_cell on("on",0);
    static con_cell off("off",0);    
    
    if (args[0].tag().is_ref()) {
	con_cell c = w.is_auto_save() ? on : off;
	return interp.unify(args[0], c);
    } else if (args[0] == on) {
	w.set_auto_save(true);
	return true;
    } else if (args[0] == off) {
	w.set_auto_save(false);
	return true;
    } else {
	return false;
    }
}

bool wallet_interpreter::load_0(interpreter_base &interp, size_t arity, term args[])
{
    auto &w = reinterpret_cast<wallet_interpreter &>(interp).get_wallet();
    w.load();
    return true;
}
	
bool wallet_interpreter::file_1(interpreter_base &interp, size_t arity, term args[])
{
    auto &w = reinterpret_cast<wallet_interpreter &>(interp).get_wallet();
    if (args[0].tag().is_ref()) {
	
	return interp.unify(args[0], interp.string_to_list(w.get_file()));
    } else {
	// Switch to a new file
	// If the file name is relative,
	// use the same directory as the current file.

	if (!interp.is_string(args[0])) {
	    throw interp::interpreter_exception_wrong_arg_type("file/1: Argument must be a string; was " + interp.to_string(args[0]));
	}
	auto new_file = interp.list_to_string(args[0]);
	if (new_file.empty()) {
	    w.set_file(new_file);
	    return true;
	}
	auto new_path = boost::filesystem::path(new_file);
	if (new_path.is_relative()) {
	    auto p = boost::filesystem::path(w.get_file()).parent_path();
	    p /= new_path;
	    new_path = p;
	}
	new_file = new_path.string();
	w.save(); // Save current wallet before switching to new file
	w.set_file(new_file);
	return true;
    }
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
