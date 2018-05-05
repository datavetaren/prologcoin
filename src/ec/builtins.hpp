#pragma once

#ifndef _ec_builtins_hpp
#define _ec_builtins_hpp

#include "../common/term.hpp"
#include "../interp/interpreter.hpp"

namespace prologcoin { namespace ec {

class builtins {
public:
    using term = prologcoin::common::term;
    using interpreter_base = prologcoin::interp::interpreter_base;

    static void load(interpreter_base &interp);

    static bool privkey_1(interpreter_base &interp, size_t arity, term args[] );
};

}}

#endif

