#pragma once

#ifndef _interp_interpreter_exception_hpp
#define _interp_interpreter_exception_hpp

#include "../common/term_env.hpp"

namespace prologcoin { namespace interp {

// Syntax exceptions...

// Interpreter exceptions...

class interpreter_exception : public std::runtime_error
{
public:
    interpreter_exception(const std::string &msg)
	: std::runtime_error(msg) { }
};

class syntax_exception : public interpreter_exception
{
public:
    syntax_exception(const common::term &t, const std::string &msg)
	: interpreter_exception(msg), term_(t) { }
    ~syntax_exception() noexcept(true) { }

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

class syntax_exception_bad_goal : public syntax_exception
{
public:
    syntax_exception_bad_goal(const common::term &t, const std::string &msg)
	: syntax_exception(t, msg) { }
};

class interpreter_exception_out_of_funds : public interpreter_exception
{
public:
    interpreter_exception_out_of_funds(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_stack_overflow : public interpreter_exception
{
public:
    interpreter_exception_stack_overflow(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_undefined_predicate : public interpreter_exception
{
public:
    interpreter_exception_undefined_predicate(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_wrong_arg_type : public interpreter_exception
{
public:
      interpreter_exception_wrong_arg_type(const std::string &msg)
	  : interpreter_exception(msg) { }
};

class interpreter_exception_missing_arg : public interpreter_exception 
{
public:
      interpreter_exception_missing_arg(const std::string &msg)
	  : interpreter_exception(msg) { }    
};

class interpreter_exception_file_not_found : public interpreter_exception
{
public:
      interpreter_exception_file_not_found(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_nothing_told : public interpreter_exception
{
public:
    interpreter_exception_nothing_told(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_argument_not_number : public interpreter_exception
{
public:
    interpreter_exception_argument_not_number(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_not_sufficiently_instantiated : public interpreter_exception
{
public:
    interpreter_exception_not_sufficiently_instantiated(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_not_list : public interpreter_exception
{
public:
    interpreter_exception_not_list(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_unsupported : public interpreter_exception
{
public:
    interpreter_exception_unsupported(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_undefined_function : public interpreter_exception
{
public:
    interpreter_exception_undefined_function(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_already_in_critical_section : public interpreter_exception
{
public:
    interpreter_exception_already_in_critical_section(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_remote_exception : public interpreter_exception
{
public:
    interpreter_remote_exception(const std::string &msg)
	: interpreter_exception(msg) { }
};
	
}}

#endif
