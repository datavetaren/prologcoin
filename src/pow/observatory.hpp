#pragma once
#ifndef _pow_observatory_hpp
#define _pow_observatory_hpp

#include "galaxy.hpp"
#include "camera.hpp"
#include "siphash.hpp"
#include "dipper_detector.hpp"
#include "checked_cast.hpp"
#include <boost/thread.hpp>
#include <fstream>
#include <math.h>

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

template<size_t N, typename T> class observatory {
public:
    inline observatory(const siphash_keys &keys) : keys_(keys), galaxy_(keys_) { init(); }

    void init(size_t num_stars = 0);

    inline void set_keys(const siphash_keys &keys) {
	keys_ = keys;
	galaxy_.clear();
	init();
    }

    inline const siphash_keys & keys() const {
	return keys_;
    }

    void set_target(const vec3<T> &v, size_t cam_id = 0);
    const vec3<T> & get_target(size_t cam_id = 0) const;
    void set_target(uint64_t nonce_offset, uint32_t nonce, size_t cam_id = 0);

    size_t new_camera();

    inline star get_star(uint32_t id) const {
	uint64_t out[3];
	siphash(keys_, checked_cast<uint64_t>(3*id),
		       checked_cast<uint64_t>(3*id+3), out);
	return star(id, out[0], out[1], out[2]);
    }

    void take_picture(std::vector<projected_star> &stars, size_t cam_id = 0) const;

    bool scan(uint64_t nonce_offset, projected_star &first_visible, std::vector<projected_star> &found, uint32_t &nonce);

    void status() const; 
    void memory() const; 

    size_t num_buckets() const;
    T step_vector_length() const;

private:
    inline galaxy<N, T> & get_galaxy() {
	return galaxy_;
    }

    inline camera<N, T> & get_camera(size_t id = 0) {
	return cameras_[id];
    }

    inline const galaxy<N, T> & get_galaxy() const {
	return galaxy_;
    }

    inline const camera<N, T> & get_camera(size_t id = 0) const {
	return cameras_[id];
    }

    siphash_keys keys_;
    galaxy<N, T> galaxy_;
    std::vector<camera<N, T> > cameras_;
};

template<size_t N, typename T> void observatory<N,T>::init(size_t num_stars)
{
    if (num_stars == 0) {
	galaxy_.init();
    } else {
	galaxy_.init(num_stars);
    }
    cameras_.clear();
    new_camera(); // Default camera id=0
}

template<size_t N, typename T> size_t observatory<N,T>::new_camera() {
    size_t id = cameras_.size();
    camera<N, T> cam(galaxy_, id);
    cameras_.push_back(cam);
    return id;
}

template<size_t N, typename T> size_t observatory<N,T>::num_buckets() const {
    return get_galaxy().num_buckets;
}

template<size_t N, typename T> T observatory<N,T>::step_vector_length() const {
    return get_galaxy().step_vector_length();
}

template<size_t N, typename T> void observatory<N,T>::status() const {
    get_galaxy().status();
}

template<size_t N, typename T> void observatory<N,T>::memory() const {
    get_galaxy().memory();
}


template<size_t N, typename T> void observatory<N,T>::set_target(const vec3<T> &v, size_t cam_id)
{
    get_camera(cam_id).set_target(v);
}

template<size_t N, typename T> void observatory<N,T>::set_target(uint64_t nonce_offset, uint32_t nonce, size_t cam_id)
{
    get_camera(cam_id).set_target(nonce_offset, nonce);
}

template<size_t N, typename T> const vec3<T> & observatory<N,T>::get_target(size_t cam_id) const
{
    return get_camera(cam_id).get_target();
}

template<size_t N, typename T> void observatory<N,T>::take_picture(std::vector<projected_star> &stars, size_t cam_id) const
{
    get_camera(cam_id).take_picture(stars);
}

template<size_t N, typename T> class worker_pool;

template<size_t N, typename T> class worker {
public:
    worker(worker_pool<N,T> &workers);

    worker(const worker &other) = delete;
    void operator = (const worker &other) = delete;

    inline void join() {
	
    }

    inline void kill() {
	killed_ = true;
	set_idle(false);
    }

    inline void wait_idle() const {
    }

    inline void set_idle(bool r) {
	boost::unique_lock<boost::mutex> lockit(idle_lock_);
	idle_ = r;
	idle_cv_.notify_one();
    }

    inline bool is_done() const {
	return found_done_;
    }
    
    inline void set_nonce_range(size_t nonce_offset, size_t nonce_start, size_t nonce_end) {
	nonce_offset_ = nonce_offset;
	nonce_ = nonce_start;
	nonce_start_ = nonce_start;
	nonce_end_ = nonce_end;
	found_done_ = false;
	has_first_visible_ = false;
	first_visible_.clear();
    }

    inline size_t nonce_start() const {
        return nonce_start_;
    }

    inline size_t nonce_end() const {
        return nonce_end_;
    }
  
    inline size_t nonce() const {
	return nonce_;
    }

    inline const projected_star & first_visible() const {
        return first_visible_;
    }

    inline bool has_first_visible() const {
        return has_first_visible_;
    }

    void set_target(uint64_t nonce_offset, uint32_t nonce);
    void take_picture();
    void run();

    inline const std::vector<projected_star> & get_found() const {
	return found_;
    }

private:
    worker_pool<N,T> &workers_;
    std::vector<projected_star> stars_;
    std::vector<projected_star> found_;
    projected_star first_visible_;
    bool has_first_visible_;
    dipper_detector detector_;
    size_t cam_id_;
    bool idle_;
    bool killed_;
    boost::mutex idle_lock_;
    boost::condition_variable idle_cv_;
    size_t nonce_offset_, nonce_, nonce_start_, nonce_end_;
    bool found_done_;
};

template<size_t N, typename T> class worker_pool {
public:
    static const size_t DEFAULT_NUM_WORKERS = 16;

    worker_pool(observatory<N,T> &obs, size_t num_workers = DEFAULT_NUM_WORKERS) : observatory_(obs), num_workers_(num_workers), busy_count_(0) {
	smallest_nonce_ = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < num_workers_; i++) {
	    auto *w = new worker<N,T>(*this);
	    all_workers_.push_back(w);
	    ready_workers_.push_back(w);
	}
	for (auto *w : ready_workers_) {
	    threads_.create_thread( [=]{w->run();} );
	}
    }

    void kill_all_workers() {
	std::vector<worker<N,T> *> all_workers;
	{
	    boost::unique_lock<boost::mutex> lockit(workers_lock_);
	    for (auto *w : all_workers_) {
		all_workers.push_back(w);
	    }
	}
	for (auto *w : all_workers) {
	    w->kill();
	}
    }

    size_t smallest_nonce() {
	boost::unique_lock<boost::mutex> lockit(workers_lock_);
	return smallest_nonce_;
    }

    void found_nonce(size_t nonce) {
	boost::unique_lock<boost::mutex> lockit(workers_lock_);
	if (nonce < smallest_nonce_) {
	    smallest_nonce_ = nonce;
	}
    }
    
    worker<N,T> & find_ready_worker() {
	boost::unique_lock<boost::mutex> lockit(workers_lock_);
	while (busy_count_ == num_workers_) {
	    workers_cv_.wait(lockit);
	}
	worker<N,T> *w = ready_workers_.back();
	ready_workers_.pop_back();
	busy_count_++;
	return *w;
    }

    void wait_until_no_more_busy_workers() {
	{
	    boost::unique_lock<boost::mutex> lockit(workers_lock_);
	    while (busy_count_ != 0) {
		workers_cv_.wait(lockit);
	    }
	}
	threads_.join_all();
    }

    worker<N,T> * find_successful_worker() {
	worker<N,T> *best_worker = nullptr;
	for (auto *worker : all_workers_) {
	    if (worker->is_done()) {
		if (best_worker == nullptr || worker->nonce() < best_worker->nonce()) {
		    best_worker = worker;
		}
	    }
	}
	return best_worker;
    }

    worker<N,T> * find_first_nonce_worker() {
	for (auto *worker : all_workers_) {
	    if (worker->has_first_visible()) {
	        return worker;
	    }
	}
	return nullptr;
    }

    void add_ready_worker(worker<N,T> *w) {
	boost::unique_lock<boost::mutex> lockit(workers_lock_);
	busy_count_--;
	ready_workers_.push_back(w);
	workers_cv_.notify_all();
    }

private:
    observatory<N,T> observatory_;
    size_t num_workers_;
    std::vector<worker<N,T> *> all_workers_;
    std::vector<worker<N,T> *> ready_workers_;
    boost::mutex workers_lock_;
    boost::condition_variable workers_cv_;
    size_t busy_count_;
    boost::thread_group threads_;
    size_t smallest_nonce_;

    friend class worker<N,T>;
};

template<size_t N, typename T> worker<N,T>::worker(worker_pool<N,T> &workers) 
    : workers_(workers), has_first_visible_(false), detector_(stars_), idle_(true), killed_(false), nonce_offset_(0), nonce_(0), nonce_end_(0), found_done_(false) {
    cam_id_ = workers_.observatory_.new_camera();
}

template<size_t N, typename T> void worker<N,T>::set_target(uint64_t nonce_offset, uint32_t nonce) {
    workers_.observatory_.set_target(nonce_offset, nonce, cam_id_);
}

template<size_t N, typename T> void worker<N,T>::take_picture() {
    workers_.observatory_.take_picture(stars_, cam_id_);
}

template<size_t N, typename T> void worker<N,T>::run() {
    for (;;) {
	boost::unique_lock<boost::mutex> lockit(idle_lock_);
	while (idle_ && !killed_) {
	    idle_cv_.wait(lockit);
	}
	if (killed_) {
	    break;
	}
	for (; nonce_ != nonce_end_ && nonce_ < workers_.smallest_nonce(); nonce_++) {
	    set_target(nonce_offset_, nonce_);
	    take_picture();
	    if (nonce_ == 0 && stars_.size() >= 1) {
	        has_first_visible_ = true;
	        first_visible_ = stars_[0];
	    }
	    if (detector_.search(found_)) {
	         workers_.found_nonce(nonce_);
	         found_done_ = true;
	         break;
	    }
	}
	idle_ = true;
	workers_.add_ready_worker(this);
    }
}

template<size_t N, typename T> bool observatory<N,T>::scan(uint64_t nonce_offset, projected_star &first_visible, std::vector<projected_star> &found, uint32_t &nonce_out)
{
    worker_pool<N,T> workers(*this);

    uint32_t nonce = 0, nonce_delta = 100;
    bool first_visible_found = false;

    first_visible.clear();

    for (;;) {
	auto &worker = workers.find_ready_worker();
	if (worker.nonce_start() == 0) {
	    if (worker.has_first_visible()) {
	        first_visible_found = true;
	        first_visible = worker.first_visible();
	    }
	}
	if (worker.is_done()) {
	    workers.add_ready_worker(&worker);
	    break;
	}
	worker.set_nonce_range(nonce_offset, nonce, nonce + nonce_delta);
	nonce += nonce_delta;
	worker.set_idle(false);
    }
    workers.kill_all_workers();
    workers.wait_until_no_more_busy_workers();

    if (!first_visible_found) {
        if (auto *wn = workers.find_first_nonce_worker()) {
	    if (wn->has_first_visible()) {
	        first_visible_found = true;
		first_visible = wn->first_visible();
	    }
	}
    }
    
    auto *w = workers.find_successful_worker();
    if (w) {
	found = w->get_found();
	nonce_out = w->nonce();
    }

    return w != nullptr && first_visible_found;
}    

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

#endif
