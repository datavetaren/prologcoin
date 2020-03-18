#pragma once

#ifndef _global_builtins_hpp
#define _global_builtins_hpp

#include "../common/term.hpp"
#include "../interp/interpreter.hpp"

namespace prologcoin { namespace global {

using interpreter_exception = ::prologcoin::interp::interpreter_exception;

class global;
    
class builtins {
public:
    using term = prologcoin::common::term;
    using interpreter_base = prologcoin::interp::interpreter_base;

    static global & get_global(interpreter_base &interp);
    static void load(interpreter_base &interp, common::con_cell *module = nullptr);
    // reward(Height, Coin)
    static bool reward_2(interpreter_base &interp, size_t arity, term args[] );

    static bool current_height_1(interpreter_base &interp, size_t arity, term args[] );

    static bool increment_height_0(interpreter_base &interp, size_t arity, term args[] );
};

}}

#endif

