#include "task.hpp"
#include "connection.hpp"

namespace prologcoin { namespace node {

using namespace prologcoin::common;

out_task::out_task(const char *description, out_connection &out)
    : description_(description), out_(&out), env_(&out.env()),
      state_(IDLE), when_(utime::now())
{
}

out_task::~out_task()
{
    // printf("Deleted: %p\n", this);
}

bool out_task::comparator(const out_task *t1, const out_task *t2)
{
    return t1->get_when() > t2->get_when();
}

void out_task::reschedule(utime t)
{ connection().reschedule(this, t); }

void out_task::reschedule_last()
{ connection().reschedule_last(this); }

void out_task::reschedule_next()
{ connection().reschedule_next(this); }

void out_task::trigger_now()
{ connection().trigger_now(); }

bool out_task::is_connected() const
{ return connection().is_connected(); }

const ip_service & out_task::ip() const
{ return connection().ip(); }

self_node & out_task::self()
{ return connection().self(); }

void out_task::stop()
{ connection().stop(); }

term out_task::get_result() const
{
    static const con_cell ok_1("ok",1);
    static const con_cell result_5("result",5);

    term r = get_term();
    if (r.tag() != tag_t::STR) {
	return term();
    }
    if (env().functor(r) != ok_1) {
	return term();
    }
    term result = env().arg(r, 0);
    if (env().functor(result) != result_5) {
	return term();
    }
    return result;
}

bool out_task::has_more() const
{
    static const con_cell more_0("more", 0);

    term result = get_result();
    if (result == term()) {
	return false;
    }
    term state = env().arg(result, 2);
    if (!env().is_atom(state)) {
	return false;
    }
    return state == more_0;
}

bool out_task::at_end() const
{
    static const con_cell at_end_0("at_end", 0);

    term result = get_result();
    if (result == term()) {
	return false;
    }
    term state = env().arg(result, 2);
    if (!env().is_atom(state)) {
	return false;
    }
    return state == at_end_0;
}

term out_task::get_result_goal() const
{
    term r = get_result();
    if (r == term()) {
	return r;
    }
    return env().arg(r, 0);
}

uint64_t out_task::get_cost() const
{
    term r = get_result();
    if (r == term()) {
	return 0;
    }
    term c = env().arg(r, 4);
    if (c.tag() != tag_t::INT) {
	return 0;
    } else {
	return static_cast<uint64_t>(reinterpret_cast<int_cell &>(c).value());
    }
}

void out_task::error(const reason_t &reason)
{
    set_state(KILLED);
    connection().error(reason, "");
}

void out_task::error(const reason_t &reason, const std::string &msg)
{
    set_state(KILLED);
    connection().error(reason, msg);
}


}}


