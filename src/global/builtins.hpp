#pragma once

#ifndef _global_builtins_hpp
#define _global_builtins_hpp

#include "../common/term.hpp"
#include "../interp/interpreter.hpp"

namespace prologcoin { namespace global {

using interpreter_exception = ::prologcoin::interp::interpreter_exception;

class builtins {
public:
    using term = prologcoin::common::term;
    using interpreter_base = prologcoin::interp::interpreter_base;

    static void load(interpreter_base &interp, common::con_cell *module = nullptr);

    // commit(X): if X is not a clause, then it's equivalent to call(X).
    // If X is a clause p(Hash) :- Body, then Hash is bound to the standard
    // (term) hash of Body followed by call(Body).
    static bool commit_1(interpreter_base &interp, size_t arity, term args[] );
};

}}

#endif

