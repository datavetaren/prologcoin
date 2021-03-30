#include "self_node.hpp"
#include "task_init_connection.hpp"
#include "local_interpreter.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace node {

task_init_connection::task_init_connection(out_connection *out) : out_task("init", out_task::TYPE_INIT_CONNECTION, out)
{ }

void task_init_connection::process()
{
    static const con_cell ok("ok", 1);
    static const con_cell session("session",2);

    auto &e = env();

    switch (get_state()) {
    case IDLE:
	break;
    case RECEIVED: {
	term t = get_term();
	if (t.tag() != tag_t::STR) {
	    error(reason_t::ERROR_FAIL_CONNECT,
		 "Unexpected response for init connection: "
		 + e.to_string(t));
	    break;
	}
	auto f = e.functor(t);
	if (f != ok) {
	    error(reason_t::ERROR_FAIL_CONNECT,
		  "Unexpected response for init connection: "
		  + e.to_string(t));
	    break;
	}
	if (connection().id().empty()) {
	    auto session_term = e.arg(t, 0);
	    if (session_term.tag() != tag_t::STR ||
		e.functor(session_term) != session) {
		error(reason_t::ERROR_FAIL_CONNECT,
		      "Unexpected session term for init connection. "
		      "Expecting session/2, was " + e.to_string(t));
		break;
	    }
	    auto id_term = e.arg(session_term, 0);
	    if (!e.is_atom(e.arg(session_term,0))) {
		error(reason_t::ERROR_FAIL_CONNECT,
		      "Unexpected session id for init connection. "
		      "Expecting atom, was " + e.to_string(id_term));
		break;
	    }
	    auto name_term = e.arg(session_term, 1);
	    if (!e.is_atom(e.arg(session_term,1))) {
		error(reason_t::ERROR_FAIL_CONNECT,
		      "Unexpected session name for init connection. "
		      "Expecting atom, was " + e.to_string(name_term));
	    }
	    connection().set_id(e.atom_name(id_term));
	    connection().set_name(e.atom_name(name_term));
	    connection().schedule(this);
	} else {
	    connection().set_connected(true);
	    self().successful_connection(ip());
	    if (connection().use_heartbeat()) {
		auto infotask = connection().create_info_task();
		connection().schedule(infotask);
		auto hbtask = connection().create_heartbeat_task();
		connection().schedule(hbtask);
		auto pubtask = connection().create_publish_task();
		connection().schedule(pubtask);
	    }
	}
	break;
        }
    case SEND:
	if (connection().id().empty()) {
	    set_term(
		  e.new_term(con_cell("command",1), {con_cell("new",0)}));
	} else if (!connection().is_connected()) {
	    set_term(
		  e.new_term(con_cell("command",1),
				{e.new_term(con_cell("connect",1),
				    {e.functor(connection().id(),0)})}));
	} else if (!connection().sent_my_name()) {
	    connection().set_sent_my_name();
	    set_term(
		  e.new_term(con_cell("command",1),
			{e.new_term(con_cell("name",1),
				       {e.functor(self().name(),0)})}));
	}
	break;
    case KILLED:
    case WAIT:
	break;
    }
}

}}
