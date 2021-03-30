#include "../common/term_match.hpp"
#include "self_node.hpp"
#include "task_execute_query.hpp"
#include "local_interpreter.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace node {

task_execute_query::task_execute_query(out_connection *out,
				       term query,
				       node_delayed_t *delayed,
				       term_env &query_src,
				       interp::remote_execute_mode m)
    : out_task("execute_query", out_task::TYPE_EXECUTE_QUERY, out),
      query_src_(&query_src),
      query_(query),
      result_ready_(false),
      result_consumed_(false),
      mode_(m),
      delayed_(delayed)
{
    out->increment_pending_queries();
    if (delayed_) {
	out->increment_busy();
	delayed_->processed_fn = [&query_src,this](){
	    static_cast<local_interpreter &>(query_src).add_text(get_standard_out());
	    delayed_ = nullptr;
	};
    }
    uint64_t cost = 0;
    type_ = QUERY;
    query_copy_ = env().copy(query, query_src, cost);
}

task_execute_query::task_execute_query(out_connection *out,
				       task_execute_query::do_next,
				       term_env &query_src,
				       interp::remote_execute_mode m)
    : out_task("execute_query_next", out_task::TYPE_EXECUTE_QUERY, out),
      query_src_(&query_src),
      query_(term()),
      query_copy_(term()),
      result_ready_(false),
      result_consumed_(false),
      mode_(m),
      delayed_(nullptr)
{
    out->increment_pending_queries();
    type_ = DO_NEXT;
    if (mode_ == interp::MODE_PARALLEL) {
	self().add_parallel(this);
    }
}

task_execute_query::task_execute_query(out_connection *out,
				       task_execute_query::new_instance)
    : out_task("execute_query_new_instance", out_task::TYPE_EXECUTE_QUERY, out),
      query_src_(nullptr),
      query_(term()),
      query_copy_(term()),
      result_ready_(false),
      result_consumed_(false),
      mode_(interp::MODE_NORMAL),
      delayed_(nullptr)
{
    out->increment_pending_queries();
    type_ = NEW_INSTANCE;
}

task_execute_query::task_execute_query(out_connection *out,
				       task_execute_query::delete_instance)
    : out_task("execute_query_delete_instance", out_task::TYPE_EXECUTE_QUERY, out),
      query_src_(nullptr),
      query_(term()),
      else_do_(term()),
      query_copy_(term()),
      result_ready_(false),
      result_consumed_(false),
      mode_(interp::MODE_NORMAL),
      delayed_(nullptr)
{
    out->increment_pending_queries();
    type_ = DELETE_INSTANCE;
}

task_execute_query::~task_execute_query()
{
    if (delayed_) delayed_->interp->delayed_ready(delayed_);
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
    if (get_state() == WAIT) {
	if (!result_consumed_) {
	    return;
	}
	set_state(KILLED);
	if (delayed_) delayed_->interp->delayed_ready(delayed_);
	return;
    }
    
    if (has_connection() && !is_connected()) {
	reschedule_last();
	set_state(IDLE);
	return;
    }

    switch (get_state()) {
    case IDLE:
    case WAIT:
	break;
    case SEND:
	switch (type_) {
	case NEW_INSTANCE: set_command(con_cell("newinst",0)); break;
	case QUERY: set_query(query_copy_, mode() == interp::MODE_SILENT); break;
	case DO_NEXT: set_command(con_cell("next",0)); break;
	case DELETE_INSTANCE: set_command(con_cell("delinst",0)); break;
	}
	break;
    case RECEIVED: {
	boost::unique_lock<boost::mutex> lockit(result_cv_lock_);
	result_ = get_result_goal();
	result_ready_ = true;
	if (delayed_) {
	    if (result_ == term()) {
		delayed_->result = env().EMPTY_LIST;
	    } else {
		delayed_->result = result_;
	    }
	    delayed_->result_src = &env();
	    delayed_->standard_out = get_standard_out();
	    delayed_->interp->delayed_ready(delayed_);
	}
	connection().decrement_pending_queries();
	result_cv_.notify_one();
	set_state(WAIT);
	break;
    }
    case KILLED:
	break;
    }
}

}}
