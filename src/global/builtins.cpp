#include "builtins.hpp"
#include "../coin/builtins.hpp"
#include "global.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

namespace prologcoin { namespace global {

global & builtins::get_global(interpreter_base &interp) {
    return reinterpret_cast<global_interpreter &>(interp).get_global();
}

void builtins::load(interpreter_base &interp, con_cell *module0)
{
    const con_cell M0("user", 0);
    const con_cell M = (module0 == nullptr) ? M0 : *module0;

    interp.load_builtin(M, interp.functor("current_height", 1), &builtins::current_height_1);
    
    // This will override the coin::reward_2 builtin (and this will use
    // coin::reward_2 builtin under the hood)
    interp.load_builtin(M, con_cell("reward", 2), &builtins::reward_2);

    interp.load_builtin(M, interp.functor("increment_height", 0), &builtins::increment_height_0);

    interp.load_builtin(con_cell("ref",2), builtin(&builtins::ref_2));
}

// Important to disable this by default for most interpreters or
// otherwise you can mess things up badly. For example, a bignum
// is using the tag bits so a bignum can be mistakingly be seen
// as an unbound variable, and binding it will destroy the bignum
// number. This builtin is for internal testing.
bool builtins::ref_2(interpreter_base &interp, size_t arity, common::term args[] ) {
    if (args[1].tag().is_ref()) {
	if (args[0].tag().is_ref()) {
	    ref_cell &rc = reinterpret_cast<ref_cell &>(args[0]);
	    return interp.unify(args[1], int_cell(rc.index()));
	} else {
	    std::string msg = "ref/2: First argument is not allowed to be instantiated; it must be an unbound variable; was " + interp.to_string(args[0]);
	    throw interpreter_exception_wrong_arg_type(msg);
	    return false;
	}
    } else {
	if (args[1].tag() != tag_t::INT) {
	    std::string msg = "ref/2: Second argument must be of integer type; was " + interp.to_string(args[1]);
	    throw interpreter_exception_wrong_arg_type(msg);
	}
	auto &heap_addr = reinterpret_cast<int_cell &>(args[1]);
	term t = interp.heap_get(heap_addr.value());
	return interp.unify(args[0], t);
    }
}

bool builtins::current_height_1(interpreter_base &interp, size_t arity, term args[] )
{
    auto &g = get_global(interp);
    int64_t height = static_cast<int64_t>(g.current_height());

    return interp.unify(args[0], int_cell(height));
}
    
bool builtins::reward_2(interpreter_base &interp, size_t arity, term args[] )
{
  // We need to override coin::reward_2 with this one as coin does not have
  // access to current height.
  
  if (args[0].tag() != tag_t::INT && args[0].tag() != tag_t::REF) {
        throw interpreter_exception_wrong_arg_type(
	   "reward/2: First argument, Height, must be an integer or an unbound variable; was " + interp.to_string(args[0]));
    }
    auto &g = get_global(interp);
    int64_t height = g.current_height();
    if (args[0].tag() == tag_t::INT) {
        height = reinterpret_cast<int_cell &>(args[0]).value();
    } else {
        assert(args[0].tag() == tag_t::REF);
        interp.unify(args[0], int_cell(height));
	args[0] = interp.deref(args[0]);
    }
    if (static_cast<size_t>(height) != g.current_height()) {
        return false;
    }
    auto r = prologcoin::coin::builtins::reward_2(interp, arity, args);
    return r;
}

bool builtins::increment_height_0(interpreter_base &interp, size_t arity, term args[] ) {
    auto &g = get_global(interp);
    g.increment_height();
    return true;
}

}}
