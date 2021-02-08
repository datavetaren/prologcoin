#include "local_service.hpp"

namespace prologcoin { namespace common {

void local_service::run() {
    boost::chrono::duration<uint64_t, boost::micro> timeout(tick_);
    while (!killed_) {
	std::function<void()> f;
	{
	    boost::unique_lock<boost::mutex> lockit(lock_);
	    queue_changed_.wait_for(lockit, timeout);
	    f = get_next_function();
	}
	if (f) f();
    }
}

}}
