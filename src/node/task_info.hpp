#pragma once

#ifndef _node_task_info_hpp
#define _node_task_info_hpp

#include "connection.hpp"
#include "task.hpp"

namespace prologcoin { namespace node {

class task_info : public out_task {
public:
    task_info(out_connection &out);

private:
    virtual void process() override;
};

}}

#endif
