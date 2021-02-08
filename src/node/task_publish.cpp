#include "self_node.hpp"
#include "task_publish.hpp"
#include "local_interpreter.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace node {

task_publish::task_publish(out_connection *out) : out_task("publish", out)
{ }


//
// Publish my own address
// (this will be optional in future, because a mobile wallet may not
//  have an official address for incoming connections.)
//
void task_publish::process()
{
    static const con_cell colon(":", 2);
    static const con_cell me("me",0);

    auto &e = env();

    switch (get_state()) {
    case IDLE:
	break;
    case RECEIVED:
	// This is a one time event only.
	break;
    case SEND:
	set_term(
	      e.new_term(con_cell("query",2),
		    {e.new_term(colon, {me,
					e.new_term(e.functor("add_address",2),
					   {e.EMPTY_LIST,
					    int_cell(self().port())})}),
		     con_cell("true",0)
		    }
		    ));
	break;
    case KILLED:
	break;
    }
}

}}
