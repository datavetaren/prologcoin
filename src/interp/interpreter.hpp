#pragma once

#ifndef _interp_interpreter_hpp
#define _interp_interpreter_hpp

#include <istream>
#include <vector>
#include "../common/term_env.hpp"

namespace prologcoin { namespace interp {

class syntax_exception : public std::runtime_error
{
public:
    syntax_exception(const common::term &t, const std::string &msg)
	: std::runtime_error(msg), term_(t) { }

    const common::term & get_term() const { return term_; }

private:
    common::term term_;
};

class syntax_exception_program_not_a_list : public syntax_exception
{
public:
    syntax_exception_program_not_a_list(const common::term &t)
	: syntax_exception(t, "Program is not a list") { }
};

class syntax_exception_not_a_clause : public syntax_exception
{
public:
    syntax_exception_not_a_clause(const common::term &t)
	: syntax_exception(t, "Not a clause") { }
};

class syntax_exception_clause_bad_head : public syntax_exception
{
public:
    syntax_exception_clause_bad_head(const common::term &t, const std::string &msg)
	: syntax_exception(t, msg) { }

};

class interpreter {
public:
    typedef common::term term;

    interpreter();
    interpreter(common::term_env &env);
    ~interpreter();

    inline common::term_env & env() { return *term_env_; }

    void load_clause(const std::string &str);
    void load_clause(std::istream &is);
    void load_clause(const term &t);

    void load_program(const std::string &str);
    void load_program(std::istream &is);
    void load_program(const term &clauses);

    class list_iterator : public common::term_iterator {
    public:
	list_iterator(common::term_env &env, const common::term &t)
            : term_iterator(env, t) { }

	list_iterator begin() {
	    return *this;
	}

	list_iterator end() {
	    return list_iterator(env(), env().empty_list());
	}
    };

private:

    void syntax_check();

    void syntax_check_program(const term &term);
    void syntax_check_clause(const term &term);
    void syntax_check_head(const term &head);
    void syntax_check_body(const term &body);
    void syntax_check_goals(term &goals);
    void syntax_check_goal(term &goal);

    bool owns_term_env_;
    common::term_env *term_env_;

    std::vector<std::function<void ()> > syntax_check_stack_;
};

}}

#endif
