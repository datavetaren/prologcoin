#include "../common/term_match.hpp"
#include "../common/checked_cast.hpp"
#include "self_node.hpp"
#include "task_heartbeat.hpp"
#include "local_interpreter.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace node {

task_heartbeat::task_heartbeat(out_connection *out)
    : out_task("heartbeat", out)
{ }

void task_heartbeat::process()
{
    static const con_cell colon(":", 2);
    static const con_cell me("me",0);

    auto &e = env();

    switch (get_state()) {
    case IDLE:
	break;
    case RECEIVED:
	// Update address entry with most recent time
	self().book()().update_time(ip());
	reschedule(utime::us(self().get_timer_interval_microseconds()));
	break;
    case SEND:
	set_term(
	      e.new_term(con_cell("query",2),
		    {e.new_term(colon,
				{me, e.functor("heartbeat",0)}),
		     con_cell("true",0) // Silent
		    })
	      );
	break;
    case KILLED:
	break;
    }
}

}}
