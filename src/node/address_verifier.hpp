#pragma once

#ifndef _node_address_verifier_hpp
#define _node_address_verifier_hpp

#include "connection.hpp"

namespace prologcoin { namespace node {

class task_address_verifier : public out_task {
public:
    task_address_verifier(out_connection &out);

private:
    void process_version(const common::term ver);

    static void check_fn(out_task &task);

    void check();
};

}}

#endif


