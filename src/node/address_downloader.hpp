#pragma once

#ifndef _node_address_downloader_hpp
#define _node_address_downaloder_hpp

#include "connection.hpp"

namespace prologcoin { namespace node {

class task_address_downloader : public out_task {
public:
    task_address_downloader(out_connection &out);

private:
    virtual void process();

    size_t count_;
    utime last_checked_;
};

}}

#endif


