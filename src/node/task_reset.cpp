#include "self_node.hpp"
#include "task_reset.hpp"
#include "local_interpreter.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace node {

task_reset::task_reset(out_connection *out) : out_task("init", out), failed_(false), reset_(false),  consumed_(false)
{ }

void task_reset::process()
{
    static const con_cell ok("ok", 1);
    static const con_cell session("session",2);

    auto &e = env();

    switch (get_state()) {
    case IDLE:
	if (!consumed_) {
	    reschedule_next();
	} else {
	    set_state(KILLED);
	}
	break;
    case RECEIVED: {
	term t = get_term();
	if (t.tag() != tag_t::STR) {
	    error(reason_t::ERROR_FAIL_CONNECT,
		 "Unexpected response for init connection: "
		 + e.to_string(t));
	    failed_ = true;
	    break;
	}
	auto f = e.functor(t);
	if (f != ok) {
	    error(reason_t::ERROR_FAIL_CONNECT,
		  "Unexpected response for init connection: "
		  + e.to_string(t));
	    failed_ = true;
	    break;
	} else {
	    reset_ = true;
	}
	set_state(IDLE);
	break;
        }
    case SEND:
	set_term(e.new_term(con_cell("command",1), {con_cell("lreset",0)}));
	break;
    case KILLED:
	break;
    }
}

}}
