#pragma once

#ifndef _global_global_interpreter_hpp
#define _global_global_interpreter_hpp

#include "../common/term_env.hpp"
#include "../interp/interpreter.hpp"

namespace prologcoin { namespace global {

class global_interpreter;
    
class global_builtins {
public:
    using interpreter_base = interp::interpreter_base;
    using meta_context = interp::meta_context;
    using meta_reason_t = interp::meta_reason_t;

    using term = common::term;

    static global_interpreter & to_global(interpreter_base &interp)
    { return reinterpret_cast<global_interpreter &>(interp); }

    static bool commit_1(interpreter_base &interp, size_t arity, term args[] );
};

class global_interpreter_exception : public interp::interpreter_exception {
public:
    global_interpreter_exception(const std::string &msg) :
	interpreter_exception(msg) { }
};

class global_interpreter : public interp::interpreter {
public:
    using interperter_base = interp::interpreter_base;
    using term = common::term;

    global_interpreter();

    bool reset();
};

}}

#endif
