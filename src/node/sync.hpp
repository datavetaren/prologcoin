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

    void set_debug(bool b) {
	interp_.set_debug(b);
    }
    
    const global::meta_id & get_root() const {
	return sync_root_;
    }
    
    void set_root(const global::meta_id &root_id) {
	sync_root_ = root_id;
    }

    bool is_complete() {
	return sync_complete_;
    }

    void set_complete(bool b) {
	sync_complete_ = b;
    }

    void set_init(const std::string &str) {
	sync_init_ = str;
    }

    const std::string & get_init() {
	return sync_init_;
    }

    const std::string & get_mode() {
	return sync_mode_;
    }
    
    void set_mode(const std::string &mode) {
	sync_mode_ = mode;
    }

    size_t get_at_meta_block() {
	return syncing_meta_block_;
    }

    void set_at_meta_block(size_t m) {
	syncing_meta_block_ = m;
    }

    size_t get_progress() {
	return syncing_progress_;	
    }
    
    void set_progress(size_t p) {
	syncing_progress_ = p;
    }
    
private:
    void setup_sync_impl();
    void update_sync_mode();
    void update_sync_meta_block();
    void update_sync_root_id();
    void update_sync_progress();

    self_node *self_;
    std::string sync_file_;
    in_session_state *session_;
    local_interpreter &interp_;
    boost::thread thread_;
    bool started_{false};
    bool stop_{false};
    global::meta_id sync_root_;
    bool sync_complete_{false};
    std::string sync_init_; // Source code before running sync thread
                            // Useful for testing
    std::string sync_mode_;
    size_t syncing_meta_block_{0};
    size_t syncing_progress_{0};
};

}}

#endif

