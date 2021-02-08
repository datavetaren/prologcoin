#include "../common/term_match.hpp"
#include "self_node.hpp"
#include "task_execute_query.hpp"
#include "local_interpreter.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace node {

task_execute_query::task_execute_query(out_connection *out,
				       const term query,
				       term_env &query_src,
				       interp::remote_execute_mode m)
    : out_task("execute_query", out),
      query_src_(&query_src),
      query_(query),
      result_ready_(false),
      result_consumed_(false),
      mode_(m)
{
    uint64_t cost = 0;
    type_ = QUERY;
    query_copy_ = env().copy(query, query_src, cost);
    if (mode_ == interp::MODE_PARALLEL) {
	self().add_parallel(this);
    }
}

task_execute_query::task_execute_query(out_connection *out,
				       task_execute_query::do_next,
				       term_env &query_src,
				       interp::remote_execute_mode m)
    : out_task("execute_query_next", out),
      query_src_(&query_src),
      query_(term()),
      query_copy_(term()),
      result_ready_(false),
      result_consumed_(false),
      mode_(m)
{
    type_ = DO_NEXT;
    if (mode_ == interp::MODE_PARALLEL) {
	self().add_parallel(this);
    }
}

task_execute_query::task_execute_query(out_connection *out,
				       task_execute_query::new_instance)
    : out_task("execute_query_new_instance", out),
      query_src_(nullptr),
      query_(term()),
      query_copy_(term()),
      result_ready_(false),
      result_consumed_(false),
      mode_(interp::MODE_NORMAL)
{
    type_ = NEW_INSTANCE;
}

task_execute_query::task_execute_query(out_connection *out,
				       task_execute_query::delete_instance)
    : out_task("execute_query_delete_instance", out),
      query_src_(nullptr),
      query_(term()),
      query_copy_(term()),
      result_ready_(false),
      result_consumed_(false),
      mode_(interp::MODE_NORMAL)
{
    type_ = DELETE_INSTANCE;
}

void task_execute_query::wait_for_result()
{
    while (!result_ready_) {
	boost::unique_lock<boost::mutex> lockit(result_cv_lock_);
	result_cv_.wait(lockit);
    }
}

bool task_execute_query::is_result_ready() const {
    return result_ready_;
}

void task_execute_query::process()
{
    if (has_connection() && !is_connected()) {
	reschedule_last();
	set_state(IDLE);
	return;
    }

    if (get_state() == IDLE) {
	if (!result_consumed_) {
	    reschedule_next();
	    return;
	}
	set_state(KILLED);
    }

    if (get_state() == SEND) {
	switch (type_) {
	case NEW_INSTANCE: set_command(con_cell("newinst",0)); break;
	case QUERY: set_query(query_copy_, mode() == interp::MODE_SILENT); break;
	case DO_NEXT: set_command(con_cell("next",0)); break;
	case DELETE_INSTANCE: set_command(con_cell("delinst",0)); break;
	}
    } else if (get_state() == RECEIVED) {
	boost::unique_lock<boost::mutex> lockit(result_cv_lock_);
	result_ = get_result_goal();
	result_ready_ = true;
	result_cv_.notify_one();
	if (mode() == interp::MODE_PARALLEL) {
	    self().notify_parallel();
	}
	set_state(IDLE);
	return;
    }
}

}}
