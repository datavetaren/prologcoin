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

    inline in_connection * get_connection() { return connection_; }
    inline void set_connection(in_connection *conn) { connection_ = conn; }
    inline void reset_connection() { connection_ = nullptr; }

    inline size_t heartbeats() const { return heartbeat_count_; }

    bool execute(const common::term query);

    inline void set_query(const common::term query)
    {
	query_ = query;
        auto vars = env().find_vars(query_);
	vars_ = env().empty_list();
	for (auto v = vars.rbegin(); v != vars.rend(); ++v) {
	    vars_ = env().new_dotted_pair(
			  env().new_term(common::con_cell("=",2),
				 {env().functor(v->first,0), v->second}),
		  vars_);
	}
    }

    inline common::term query() const { return query_; }
    inline common::term query_vars()
    { return vars_; }
    
    inline common::term get_result() { return interp_.get_result_term(); }
    inline const std::string & get_text_out() { return interp_.get_text_out(); }

    inline bool in_query() const { return in_query_; }

    inline bool has_more() const { return in_query() && interp_.has_more(); }

    inline bool next() {
	interp_.reset_text_out();
	bool r = interp_.next();
	if (!r) {
	    in_query_ = false;
	}
	return r;
    }

    void heartbeat();

private:
    void setup_modules();

    self_node *self_;
    std::string id_;
    in_connection *connection_;
    local_interpreter interp_;
    bool interp_initialized_;
    common::term query_;
    bool in_query_;
    common::term vars_;
    common::utime heartbeat_;
    size_t heartbeat_count_;
};

}}

#endif

