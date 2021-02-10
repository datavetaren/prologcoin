#pragma once

#ifndef _common_local_service_hpp
#define _common_local_service_hpp

#include <vector>
#include <boost/thread.hpp>
#include <unordered_map>
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
	for (auto &m : label_locks_) {
	    delete m.second;
	}
    }

    void kill() {
	boost::unique_lock<boost::mutex> lockit(lock_);	
	killed_ = true;
	queue_changed_.notify_all();
    }

    void add(const std::function<void ()> &fn, const std::string &label = "") {
	boost::unique_lock<boost::mutex> lockit(lock_);
	size_t n = queue_.size();
	for (size_t i = 0; i < n; i++) {
	    if (queue_[i].second == nullptr) {
		queue_[i].first = label;
		queue_[i].second = fn;
		queue_changed_.notify_one();
		return;
	    }
	}
	queue_.push_back(std::make_pair(label, fn));
	queue_changed_.notify_one();
    }

    size_t num_workers() const {
	return num_workers_;
    }

    void add_workers(size_t cnt) {
	boost::lock_guard<boost::mutex> lockit(lock_);
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
    boost::mutex & get_label_mutex(const std::string &label) {
	boost::unique_lock<boost::mutex> lockit(lock_);
	auto it = label_locks_.find(label);
	if (it == label_locks_.end()) {
	    label_locks_[label] = new boost::mutex();
	}
	return *label_locks_[label];
    }
    
    void internal_add_workers(size_t cnt) {
	num_workers_ += cnt;
	for (size_t i = 0; i < cnt; i++) {
	    workers_.push_back(new boost::thread( [&]{ run(); } ));
	}
    }
    
    std::pair<std::function<void ()>, std::string> get_next_function() {
	size_t n = queue_.size();
	for (size_t i = 0; i < n; i++) {
	    if (auto f = queue_[i].second) {
		queue_[i].second = nullptr;
		return std::make_pair(f, queue_[i].first);
	    }
	}
	return std::make_pair(nullptr, "");
    }

    utime::us tick_;
    bool killed_;
    boost::mutex lock_;
    std::vector<std::pair<std::string, std::function<void ()> > > queue_;
    boost::condition_variable queue_changed_;
    std::vector<boost::thread *> workers_;
    size_t num_workers_;
    std::unordered_map<std::string, boost::mutex *> label_locks_;
};
	
}}

#endif
