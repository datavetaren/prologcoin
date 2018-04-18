#pragma once

#ifndef _node_session_hpp
#define _node_session_hpp

#include "../common/utime.hpp"
#include "local_interpreter.hpp"

namespace prologcoin { namespace node {

class in_connection;

class in_session_state {
public:
    in_session_state(self_node *self, in_connection *conn);

    inline self_node & self() { return *self_; }

    inline const std::string & id() const { return id_; }
    inline common::term_env & env() { return interp_; }
    inline interp::interpreter & interp() { return interp_; }

    inline in_connection * get_connection() { return connection_; }
    inline void set_connection(in_connection *conn) { connection_ = conn; }
    inline void reset_connection() { connection_ = nullptr; }

    inline size_t heartbeats() const { return heartbeat_count_; }

    inline common::term query() const { return interp_.query(); }

    // Return a dotted pair with the query and its vars.
    common::term query_closure();

    bool execute(const common::term query);
    bool next();
    bool at_end();
    void delete_instance();
    bool reset();
    void local_reset();

    inline common::term get_result() { return interp_.get_result_term(); }
    inline const std::string & get_text_out() {return interp_.get_text_out();}

    inline bool has_more() const { return interp_.has_more(); }

    void heartbeat();

private:
    void setup_modules();

    self_node *self_;
    std::string id_;
    in_connection *connection_;
    local_interpreter interp_;
    bool interp_initialized_;
    common::utime heartbeat_;
    size_t heartbeat_count_;
};

}}

#endif

