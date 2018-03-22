#pragma once

#ifndef _node_local_interpreter_hpp
#define _node_local_interpreter_hpp

#include "../interp/interpreter.hpp"

namespace prologcoin { namespace node {

class self_node;
class local_interpreter;
class in_session_state;

class me_builtins {
public:
    using interpreter_base = interp::interpreter_base;
    using term = common::term;

    static local_interpreter & to_local(interpreter_base &interp)
    { return reinterpret_cast<local_interpreter &>(interp); }

    static bool heartbeat_0(interpreter_base &interp, size_t arity, term args[]);
    static bool peers_2(interpreter_base &interp, size_t arity, term args[]);
};

class local_interpreter : public interp::interpreter {
public:
    using interperter_base = interp::interpreter_base;
    using term = common::term;

    inline local_interpreter(in_session_state *session)
        :session_(session), initialized_(false) { }

    void ensure_initialized();

    inline in_session_state & session() { return *session_; }

    static const common::con_cell me;

private:
    self_node & self();

    friend class me_builtins;

    void setup_modules();

    in_session_state *session_;
    bool initialized_;
};

}}

#endif
