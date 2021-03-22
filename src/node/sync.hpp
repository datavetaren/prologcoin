#pragma once

#ifndef _node_sync_hpp
#define _node_sync_hpp

#include "../common/term_env.hpp"
#include "../common/utime.hpp"
#include "ip_service.hpp"
#include "local_interpreter.hpp"

namespace prologcoin { namespace node {

class self_node;
	
class sync {
public:
    sync(self_node *self);

    void start();
    void stop() { stop_ = true; }
    
    void join();
    void run();

    void check_dirty();
    void load();
    void save();
    
private:
    void setup_sync_impl();
    void update_sync_mode();
    void update_sync_meta_block();

    self_node *self_;
    std::string sync_file_;
    in_session_state *session_;
    local_interpreter &interp_;
    boost::thread thread_;
    bool stop_{false};
};

}}

#endif

