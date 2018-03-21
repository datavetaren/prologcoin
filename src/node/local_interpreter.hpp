#pragma once

#ifndef _node_local_interpreter_hpp
#define _node_local_interpreter_hpp

#include "../interp/interpreter.hpp"

namespace prologcoin { namespace node {

class self_node;
class local_interpreter;

class me_builtins {
public:
    using interpreter_base = interp::interpreter_base;
    using term = common::term;

    static local_interpreter & to_local(interpreter_base &interp)
    { return reinterpret_cast<local_interpreter &>(interp); }

    static bool peers_2(interpreter_base &interp, size_t arity, term args[]);
};

class local_interpreter : public interp::interpreter {
public:
    using interperter_base = interp::interpreter_base;
    using term = common::term;

    inline local_interpreter(self_node *self)
        : self_(self), initialized_(false) { }

    void ensure_initialized();

private:
    inline self_node & self() { return *self_; }

    friend class me_builtins;

    static const common::con_cell me;

    void setup_modules();

    self_node *self_;
    bool initialized_;
};

}}

#endif
