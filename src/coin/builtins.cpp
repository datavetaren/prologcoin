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

void builtins::load(interpreter_base &interp)
{
    interp.load_builtin(con_cell("reward", 2), &builtins::reward_2);
}

}}
