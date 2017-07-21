#include "../common/term_env.hpp"
#include "interpreter.hpp"

namespace prologcoin { namespace interp {

using namespace prologcoin::common;

interpreter::interpreter()
{
    term_env_ = new term_env();
    owns_term_env_ = true;
}

interpreter::interpreter(term_env &env)
{
    term_env_ = &env;
    owns_term_env_ = false;
}

interpreter::~interpreter()
{
    syntax_check_stack_.clear();
    if (owns_term_env_) {
	delete term_env_;
    }
}

void interpreter::syntax_check()
{
    while (!syntax_check_stack_.empty()) {
	auto f = syntax_check_stack_.back();
	syntax_check_stack_.pop_back();
	f();
    }
}

void interpreter::load_clause(const term &t)
{
    syntax_check_stack_.push_back(
			  std::bind(&interpreter::syntax_check_clause, this,
				    t));
    syntax_check();
}

void interpreter::load_program(const term &t)
{
    syntax_check_stack_.push_back(
			  std::bind(&interpreter::syntax_check_program, this,
				    t));
    syntax_check();
}

void interpreter::syntax_check_program(const term &t)
{
    if (!term_env_->is_list(t)) {
	throw syntax_exception_program_not_a_list(t);
    }

    size_t sz = syntax_check_stack_.size();
    for (auto clause : list_iterator(*term_env_, t)) {
	syntax_check_stack_.push_back(
			  std::bind(&interpreter::syntax_check_clause, this,
				    clause));
    }
    std::reverse(syntax_check_stack_.begin() + sz, syntax_check_stack_.end());
}

void interpreter::syntax_check_clause(const term &t)
{
    con_cell def(":-", 2);

    std::cout << "Syntax check clause: " << term_env_->to_string(t) << "\n";

    auto f = term_env_->functor(t);
    if (f == def) {
	auto head = term_env_->arg(t, 0);
	auto body = term_env_->arg(t, 1);
	syntax_check_stack_.push_back(
	      std::bind(&interpreter::syntax_check_head, this, head) );
	syntax_check_stack_.push_back(
	      std::bind(&interpreter::syntax_check_body, this, body) );
	return;
    }

    // This is a head only clause.

    syntax_check_stack_.push_back(
		  std::bind(&interpreter::syntax_check_head, this, t));
}

void interpreter::syntax_check_head(const term &t)
{
    static con_cell def(":-", 2);
    static con_cell semi(";", 2);
    static con_cell comma(",", 2);
    static con_cell cannot_prove("\\+", 1);

    if (!term_env_->is_functor(t)) {
	throw syntax_exception_clause_bad_head(t, "Head of clause is not a functor");
    }

    // Head cannot be functor ->, ; , or \+
    auto f = term_env_->functor(t);

    if (f == def || f == semi || f == comma || f == cannot_prove) {
	throw syntax_exception_clause_bad_head(t, "Clause has an invalid head; cannot be '->', ';', ',' or '\\+'");
    }
}

void interpreter::syntax_check_body(const term &t)
{
    static con_cell imply("->", 2);
    static con_cell semi(";", 2);
    static con_cell comma(",", 2);
    static con_cell cannot_prove("\\+", 1);

    if (term_env_->is_functor(t)) {
	auto f = term_env_->functor(t);
	if (f == imply || f == semi || f == comma || f == cannot_prove) {
	    auto num_args = f.arity();
	    for (size_t i = 0; i < num_args; i++) {
		auto arg = term_env_->arg(t, i);
		syntax_check_stack_.push_back(
		      std::bind(&interpreter::syntax_check_body, this, arg));
	    }
	    return;
	}
    }

    syntax_check_stack_.push_back(
		  std::bind(&interpreter::syntax_check_goal, this, t));
}

void interpreter::syntax_check_goal(const term &t)
{
    // Each goal must be a functor (e.g. a plain integer is not allowed)

    if (!term_env_->is_functor(t)) {
	auto tg = t->tag();
	// We don't know what variables will be bound to, so we need
	// to conservatively skip the syntax check.
	if (tg == tag_t::REF || tg == tag_t::GBL) {
	    return;
	}
	throw syntax_exception_bad_goal(t, "Goal is not callable.");
    }
}

}}



