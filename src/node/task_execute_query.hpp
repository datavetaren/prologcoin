#pragma once

#ifndef _node_task_execute_query_hpp
#define _node_task_execute_query_hpp

#include "connection.hpp"

namespace prologcoin { namespace node {

class task_execute_query : public out_task {
public:
    struct do_next { };
    struct new_instance { };
    struct delete_instance { };

    task_execute_query(out_connection *out,
		       const term query,
		       term_env &query_src,
		       interp::remote_execute_mode mode);
    task_execute_query(out_connection *out, do_next n,
		       term_env &query_src,
		       interp::remote_execute_mode mode);
    task_execute_query(out_connection *out, new_instance i);
    task_execute_query(out_connection *out, delete_instance i);

    inline interp::remote_execute_mode mode() const { return mode_; }

    term_env & query_src() { return *query_src_; }
    term query() { return query_; }
    void wait_for_result();
    bool is_result_ready() const;
    inline term get_result() const { return result_; }
    inline bool failed() const { return result_ == term(); }
    inline void consume_result() { result_consumed_ = true; }

private:
    virtual void process() override;

    enum { NEW_INSTANCE = 0, 
	   QUERY = 1,
	   DO_NEXT = 2,
	   DELETE_INSTANCE = 3 } type_;

    term_env *query_src_;
    term query_;
    term query_copy_;
    term result_;
    bool result_ready_;
    bool result_consumed_;
    boost::mutex result_cv_lock_;
    boost::condition_variable result_cv_;
    interp::remote_execute_mode mode_;
};

}}

#endif


