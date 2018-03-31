#include "self_node.hpp"
#include "local_interpreter.hpp"
#include "session.hpp"

namespace prologcoin { namespace node {

using namespace prologcoin::common;
using namespace prologcoin::interp;

const con_cell local_interpreter::ME("me", 0);
const con_cell local_interpreter::COLON(":", 2);
const con_cell local_interpreter::COMMA(",", 2);

bool me_builtins::id_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    const std::string &id = interp.self().id();
    auto f = interp.functor(id, 0);
    return interp.unify(args[0], f);
}

bool me_builtins::heartbeat_0(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    interp.session().heartbeat();
    return true;
}

bool me_builtins::version_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    int_cell ver_major(self_node::VERSION_MAJOR);
    int_cell ver_minor(self_node::VERSION_MINOR);
    auto ver = interp.new_term(con_cell("ver",2), {ver_major, ver_minor});
    return interp.unify(args[0], ver);
}

bool me_builtins::comment_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    term comment = interp.copy(interp.self().get_comment(),
			       interp.self().env());

    return interp.unify(args[0], comment);
}

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

self_node & local_interpreter::self()
{
    return session_.self();
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
    load_builtin(ME, con_cell("id", 1), &me_builtins::id_1);
    load_builtin(ME, con_cell("peers", 2), &me_builtins::peers_2);
    load_builtin(ME, con_cell("version",1), &me_builtins::version_1);
    load_builtin(ME, con_cell("comment",1), &me_builtins::comment_1);
    load_builtin(ME, functor("heartbeat", 0), &me_builtins::heartbeat_0);
}

}}

