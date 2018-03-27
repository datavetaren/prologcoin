#pragma once

#ifndef _node_address_verifier_hpp
#define _node_address_verifier_hpp

#include "connection.hpp"

namespace prologcoin { namespace node {

class address_verifier : public out_task {
public:
    address_verifier(out_connection &out);

private:
    int version_major_;
    int version_minor_;

    void process_version(const common::term ver);

    enum fail_t { ERROR_UNRECOGNIZED, ERROR_VERSION };

    static void check_fn(out_task &task);

    void check();

    void fail(fail_t t);
};

}}

#endif


