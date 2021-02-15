#pragma once

#ifndef _common_spinlock_hpp
#define _common_spinlock_hpp

#include <boost/atomic.hpp>

namespace prologcoin { namespace common {

class spinlock {
public:
    spinlock() : state_(false) { }
    void lock() { while (state_.exchange(true, boost::memory_order_acquire) == true) { } }
    void unlock() { state_.exchange(false, boost::memory_order_release); }
private:
    boost::atomic<bool> state_;
};

}}

#endif

