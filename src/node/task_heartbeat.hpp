#pragma once

#ifndef _node_task_heartbeat_hpp
#define _node_task_heartbeat_hpp

#include "connection.hpp"
#include "task.hpp"

namespace prologcoin { namespace node {

class task_heartbeat : public out_task {
public:
    task_heartbeat(out_connection &out);

private:
    virtual void process() override;
};

}}

#endif
