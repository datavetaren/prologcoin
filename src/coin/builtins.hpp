#pragma once

#ifndef _coin_builtins_hpp
#define _coin_builtins_hpp

#include "../common/term.hpp"
#include "../interp/interpreter_base.hpp"

namespace prologcoin { namespace coin {

using interpreter_exception = prologcoin::interp::interpreter_exception;
    
class interpreter_exception_not_a_coin : public interpreter_exception
{
public:
    interpreter_exception_not_a_coin(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_coin_already_spent : public interpreter_exception
{
public:
    interpreter_exception_coin_already_spent(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_not_all_integers : public interpreter_exception {
public:
    interpreter_exception_not_all_integers(const std::string &msg)
	: interpreter_exception(msg) { }  
};

class interpreter_exception_not_enough_coins : public interpreter_exception {
public:
    interpreter_exception_not_enough_coins(const std::string &msg)
	: interpreter_exception(msg) { }  
};
    
class builtins {
public:
    using term = prologcoin::common::term;
    using interpreter_base = prologcoin::interp::interpreter_base;

    static void load(interpreter_base &interp);

    // reward(Height, Coin)
    // Compute inflation reward at Height and make the sum available in Coin
    static bool reward_2(interpreter_base &interp, size_t arity, term args[] );

    // cjoin(InCoinList, CoinOut)
    // Sum smaller coins into a single new coin CoinOut.
    static bool cjoin_2(interpreter_base &interp, size_t arity, term args[] );

    // csplit(InCoin, Values, OutCoinList)
    // Split single InCoin into multiple smaller coins whose aggregate sum
    // is the same as InCoin.
    static bool csplit_3(interpreter_base &interp, size_t arity, term args[] );
     
};

}}
  
#endif
