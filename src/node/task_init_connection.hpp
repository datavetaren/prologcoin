#pragma once

#ifndef _node_task_init_connection_hpp
#define _node_task_init_connection_hpp

#include "connection.hpp"
#include "task.hpp"

namespace prologcoin { namespace node {

class task_init_connection : public out_task {
public:
    task_init_connection(out_connection &out);

private:
    virtual void process() override;
};

}}

#endif
