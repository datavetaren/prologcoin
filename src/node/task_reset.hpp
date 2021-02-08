#pragma once

#ifndef _node_task_reset_hpp
#define _node_task_reset_hpp

#include "connection.hpp"
#include "task.hpp"

namespace prologcoin { namespace node {

class task_reset : public out_task {
public:
    task_reset(out_connection *out);

    inline void consume() {
	consumed_ = true;
	trigger_now();
    }

    inline bool failed() const {
	return failed_;
    }

    inline bool is_reset() const {
	return reset_;
    }

private:
    virtual void process() override;

    bool failed_;
    bool reset_;
    bool consumed_;
};

}}

#endif
