#include "self_node.hpp"
#include "local_interpreter.hpp"

namespace prologcoin { namespace node {

using namespace prologcoin::common;
using namespace prologcoin::interp;

const con_cell local_interpreter::me("me", 0);

bool me_builtins::peers_2(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);

    auto n_term = args[0];
    if (n_term.tag() != tag_t::INT) {
	interp.abort(interpreter_exception_wrong_arg_type("peers/2: First argument must be an integer; was " + interp.to_string(n_term)));
    }

    int64_t n = reinterpret_cast<int_cell &>(n_term).value();
    if (n < 1 || n > 100) {
	interp.abort(interpreter_exception_wrong_arg_type("peers/2: First argument must be a number within 1..100; was " + boost::lexical_cast<std::string>(n)));
    }

    auto book = interp.self().book();

    auto best = book().get_randomly_from_top_10_pt(static_cast<size_t>(n));
    auto rest = book().get_randomly_from_bottom_90_pt(static_cast<size_t>(n-best.size()));

    // Create a list out of entries
    term result = interp.empty_list();
    for (auto &e : best) {
	result = interp.new_dotted_pair(e.to_term(interp), result);
    }
    for (auto &e : rest) {
	result = interp.new_dotted_pair(e.to_term(interp), result);
    }
    
    // Unify second arg with result

    return interp.unify(args[1], result);
}

void local_interpreter::ensure_initialized()
{
    if (!initialized_) {
	initialized_ = true;
	setup_standard_lib();
	setup_modules();
    }
}

void local_interpreter::setup_modules()
{
    load_builtin(me, con_cell("peers", 2), &me_builtins::peers_2);
}

}}

