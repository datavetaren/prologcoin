#pragma once

#ifndef _common_local_service_hpp
#define _common_local_service_hpp

#include <vector>
#include <boost/thread.hpp>
#include "utime.hpp"

namespace prologcoin { namespace common {
	
class local_service {
public:
    local_service(utime::us tick = utime::us(100000))
  	: tick_(tick), killed_(false), num_workers_(0) {
    }

    ~local_service() {
	kill();
	join();
    }

    void run();

    void join() {
	for (auto w : workers_) {
	    w->join();
	    delete w;
	}
	workers_.clear();
    }
    
    void kill() {
	boost::unique_lock<boost::mutex> lockit(lock_);	
	killed_ = true;
	queue_changed_.notify_all();
    }

    void add(const std::function<void ()> &fn) {
	boost::unique_lock<boost::mutex> lockit(lock_);
	size_t n = queue_.size();
	for (size_t i = 0; i < n; i++) {
	    if (queue_[i] == nullptr) {
		queue_[i] = fn;
		queue_changed_.notify_one();
		return;
	    }
	}
	queue_.push_back(fn);
	queue_changed_.notify_one();
    }

    size_t num_workers() const {
	return num_workers_;
    }

    void add_workers(size_t cnt) {
	boost::unique_lock<boost::mutex> lockit(lock_);
	internal_add_workers(cnt);
    }

    void ensure_workers(size_t cnt) {
	boost::unique_lock<boost::mutex> lockit(lock_);
	if (num_workers_ >= cnt) {
	    return;
	}
	size_t dcnt = cnt - num_workers_;
	internal_add_workers(dcnt);
    }

private:
    void internal_add_workers(size_t cnt) {
	num_workers_ += cnt;
	for (size_t i = 0; i < cnt; i++) {
	    workers_.push_back(new boost::thread( [&]{ run(); } ));
	}
    }
    
    std::function<void ()> get_next_function() {
	size_t n = queue_.size();
	for (size_t i = 0; i < n; i++) {
	    if (auto f = queue_[i]) {
		queue_[i] = nullptr;
		return f;
	    }
	}
	return nullptr;
    }

    utime::us tick_;
    bool killed_;
    boost::mutex lock_;
    std::vector<std::function<void ()> > queue_;
    boost::condition_variable queue_changed_;
    std::vector<boost::thread *> workers_;
    size_t num_workers_;
};
	
}}

#endif
