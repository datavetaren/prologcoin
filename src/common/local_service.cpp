#include "local_service.hpp"

namespace prologcoin { namespace common {

void local_service::run() {
    boost::chrono::duration<uint64_t, boost::micro> timeout(tick_);
    while (!killed_) {
	std::function<void()> f;
	std::string label;
	{
	    boost::unique_lock<boost::mutex> lockit(lock_);
	    queue_changed_.wait_for(lockit, timeout);
	    std::tie(f,label) = get_next_function();
	}
	if (f) {
	    if (!label.empty()) {
		auto &l = get_label_mutex(label);
		boost::lock_guard<boost::mutex> lockit(l);
		f();
	    } else {
		f();
	    }
	}
    }
}

}}
