#include "builtins.hpp"
#include "../interp/interpreter_base.hpp"

namespace prologcoin { namespace coin {

using namespace prologcoin::common;
using namespace prologcoin::interp;

bool builtins::reward_2(interpreter_base &interp, size_t arity, term args[]) {
    if (args[0].tag() != tag_t::INT) {
        throw interpreter_exception_argument_not_number(
          "reward/2: First argument, Height, must be an integer value.");
    }

    int64_t height = static_cast<int_cell &>(args[0]).value();
    
    int64_t reward = 0;
    if (height == 0) {
        reward = 42445243724242;
    } else {
        reward = 21000000000 >> (height / 100000);
    }
    auto disabled = interp.disable_coin_security();
    term coin = interp.new_term( con_cell("$coin",2), {int_cell(reward)} );
    return interp.unify(coin, args[1]);
}

/*
bool builtins::cjoin_2(interpreter_base &interp, size_t arity, term args[]) {
    if (!interp.is_list(args[0])) {
        interp.abort(interpreter_exception_not_list("cjoin/2: First argument is not a list: " + interp.to_string(args[0])));
    }
    term in_coins = args[0];
    while (!interp.is_empty_list(in_coins)) {
        term coin = arg(
    }
}
*/

static bool is_coin(interpreter_base &interp, term t) {
    static const con_cell coin("$coin", 2);
    if (t.tag() != tag_t::STR) {
        return false;
    }
    if (interp.functor(t) != coin) {
        return false;
    }
    return true;
}

static bool is_coin_spent(interpreter_base &interp, term t) {
    assert(is_coin(interp, t));
    return interp.arg(t, 1).tag() != tag_t::REF;
}

static void spend_coin(interpreter_base &interp, term t) {
    assert(is_coin(interp, t));
    assert(!is_coin_spent(interp, t));
    interp.unify(interp.arg(t, 1), term_env::EMPTY_LIST);
}

static int64_t coin_value(interpreter_base &interp, term t) {
    assert(is_coin(interp, t));
    term v = interp.arg(t, 0);
    assert(v.tag() == tag_t::INT);
    return static_cast<int_cell &>(v).value();
}

static bool are_all_integers(interpreter_base &interp, term t) {
    while (!interp.is_empty_list(t)) {
        auto elem = interp.arg(t, 0);
	if (elem.tag() != tag_t::INT) {
	    return false;
	}
        t = interp.arg(t, 1);
    }
    return true;
}
    
bool builtins::csplit_3(interpreter_base &interp, size_t arity, term args[]) {
    if (!is_coin(interp, args[0])) {
      throw interpreter_exception_not_a_coin(
         "csplit/3: First argument must a '$coin'/2 term; was "
	 + interp.to_string(args[0]));
    }
    if (is_coin_spent(interp, args[0])) {
      throw interpreter_exception_not_a_coin(
         "csplit/3: Coin is already spent; " + interp.to_string(args[0]));
    }
    if (!interp.is_list(args[1]) || !are_all_integers(interp, args[1])) {
        throw interpreter_exception_not_list("csplit/3: Second argument must be a list of integers; was " + interp.to_string(args[1]));
    }

    // Check that it sums up appropriately
    int64_t rem = coin_value(interp, args[0]);
    term values = args[1];
    while (!interp.is_empty_list(values)) {
        auto val = interp.arg(values, 0);
	rem -= static_cast<int_cell &>(val).value();
	if (rem < 0) {
	    throw interpreter_exception_not_enough_coins("csplit/3: Second argument list of values sum exceeds coin value in first argument");
	}
        values = interp.arg(values, 1);
    }

    // Spend input coin
    spend_coin(interp, args[0]);

    // Create list of output coins

    auto disabled = interp.disable_coin_security();

    static const con_cell coin("$coin", 2);
	
    term out = args[2];
    values = args[1];
    rem = coin_value(interp, args[0]);
    while (rem > 0 && !interp.is_empty_list(values)) {
        auto val0 = interp.arg(values, 0);
	auto val = static_cast<int_cell &>(val0).value();
	rem -= val;

	auto new_coin = interp.new_term(coin, {int_cell(val)});
	
	if (!interp.unify(interp.new_term(term_env::DOTTED_PAIR, {new_coin}), out)) {
	    return false;
	}
	out = interp.arg(out, 1);
			     
	values = interp.arg(values, 1);
    }

    if (rem > 0) {
        auto rem_coin = interp.new_term(coin, {int_cell(rem)});    
	if (!interp.unify(interp.new_term(term_env::DOTTED_PAIR, {rem_coin}), out)) {
	    return false;
	}
	out = interp.arg(out, 1);	
    }

    interp.unify(out, term_env::EMPTY_LIST);

    return true;
}

void builtins::load(interpreter_base &interp)
{
    interp.load_builtin(con_cell("reward", 2), &builtins::reward_2);
    interp.load_builtin(con_cell("csplit", 3), &builtins::csplit_3);    
}

}}
