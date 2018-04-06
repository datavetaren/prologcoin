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

    static bool id_1(interpreter_base &interp, size_t arity, term args[]);
    static bool name_1(interpreter_base &interp, size_t arity, term args[]);
    static bool heartbeat_0(interpreter_base &interp, size_t arity, term args[]);
    static bool version_1(interpreter_base &interp, size_t arity, term args[]);
    static bool comment_1(interpreter_base &interp, size_t arity, term args[]);
    static bool peers_2(interpreter_base &interp, size_t arity, term args[]);

    static bool add_address_2(interpreter_base &interp, size_t arity, term args[]);
    static bool connections_0(interpreter_base &interp, size_t arity, term args[]);
    static bool mailbox_1(interpreter_base &interp, size_t arity, term args[]);
    static bool send_2(interpreter_base &interp, size_t arity, term args[]);
    static bool check_mail_0(interpreter_base &interp, size_t arity, term args[]);
};

class local_interpreter : public interp::interpreter {
public:
    using interperter_base = interp::interpreter_base;
    using term = common::term;

    inline local_interpreter(in_session_state &session)
        :session_(session), initialized_(false) { }

    void ensure_initialized();

    inline in_session_state & session() { return session_; }

    inline const std::string & get_text_out() { return text_out_; }
    inline void reset_text_out() { text_out_.clear(); }

    inline void add_text(const std::string &str) { text_out_ += str; }

    static const common::con_cell ME;
    static const common::con_cell COLON;
    static const common::con_cell COMMA;

private:
    self_node & self();

    friend class me_builtins;

    void setup_modules();

    in_session_state &session_;
    bool initialized_;
    std::string text_out_;
};

}}

#endif
