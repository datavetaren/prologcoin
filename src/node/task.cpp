#include "task.hpp"
#include "connection.hpp"

namespace prologcoin { namespace node {

out_task::out_task(out_connection &out, void (*fn)(out_task &task) )
    : out_(&out), env_(&out.env()), fn_(fn), state_(IDLE), when_(utime::now())
{
}

void out_task::reschedule(utime t)
{ connection().reschedule(*this, t); }

void out_task::reschedule_last()
{ connection().reschedule_last(*this); }

bool out_task::is_connected() const
{ return connection().is_connected(); }

const ip_service & out_task::ip() const
{ return connection().ip(); }

self_node & out_task::self()
{ return connection().self(); }

}}


