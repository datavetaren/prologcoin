#pragma once

#ifndef _node_task_publish_hpp
#define _node_task_publish_hpp

#include "connection.hpp"
#include "task.hpp"

namespace prologcoin { namespace node {

class task_publish : public out_task {
public:
    task_publish(out_connection *out);

private:
    virtual void process() override;
};

}}

#endif
