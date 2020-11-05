#include "../common/random.hpp"
#include "session.hpp"
#include "self_node.hpp"
#include <boost/range/adaptor/reversed.hpp>

namespace prologcoin { namespace node {

using namespace prologcoin::common;

in_session_state::in_session_state(self_node *self, in_connection *conn, bool is_root)
  : self_(self),
    connection_(conn),
    interp_(*this),
    heartbeat_count_(0),
    available_funds_(0),
    is_root_(is_root)
{
    id_ = "s" + random::next();
}

term in_session_state::query_closure()
{
    return interp_.new_dotted_pair(interp_.query(), interp_.query_var_list());
}

bool in_session_state::execute(const term query)
{
    using namespace prologcoin::interp;

    interp_.ensure_initialized();
    interp_.reset_text_out();
    interp_.set_maximum_cost(available_funds_);
    bool r = false;
    try {
	r = interp_.execute(query);
	interp_.flush_standard_output();
	auto cost = interp_.accumulated_cost();
	if (cost > available_funds_) {
	    available_funds_ = 0;
	} else {
	    available_funds_ -= cost;
	}
    } catch (const interpreter_exception_out_of_funds &ex) {
	interp_.flush_standard_output();
	available_funds_ = 0;
	throw ex;
    }
    return r;
}

bool in_session_state::next()
{
    using namespace prologcoin::interp;

    interp_.reset_text_out();
    interp_.set_maximum_cost(available_funds_);
    bool r = false;
    try {
	r = interp_.next();
	interp_.flush_standard_output();
	auto cost = interp_.accumulated_cost();
	if (cost > available_funds_) {
	    available_funds_ = 0;
	} else {
	    available_funds_ -= cost;
	}
    } catch (const interpreter_exception_out_of_funds &ex) {
	interp_.flush_standard_output();
	available_funds_ = 0;
	throw ex;
    }
    return r;
}

bool in_session_state::at_end()
{
    return !interp_.has_more() && interp_.is_instance();
}

void in_session_state::delete_instance()
{
    interp_.delete_instance();
}

bool in_session_state::reset()
{
    return interp_.reset();
}

void in_session_state::local_reset()
{
    interp_.local_reset();
}

void in_session_state::add_funds(uint64_t dfunds)
{
    size_t max_funds = self().get_maximum_funds();
    if (max_funds == std::numeric_limits<uint64_t>::max()) {
	available_funds_ = max_funds;	
	return;
    }
    size_t limit = max_funds - dfunds;
    if (available_funds_ >= limit) {
	available_funds_ = max_funds;
	return;
    }
    available_funds_ += dfunds;
}

void in_session_state::heartbeat()
{
    utime t0 = utime::now();
    auto dt = (t0 - heartbeat_).in_us();
    auto max_dt = 2*self().get_timer_interval_microseconds();
    if (dt > max_dt) {
	dt = max_dt;
    }
    auto sec = dt / 1000000;
    auto new_funds_per_sec = self().new_funds_per_second();
    uint64_t new_funds = 0;
    if (new_funds_per_sec != std::numeric_limits<uint64_t>::max()) {
	new_funds = self().new_funds_per_second() * sec;
    } else {
	new_funds = std::numeric_limits<uint64_t>::max();
    }
    add_funds(new_funds);

    heartbeat_ = utime::now();
    heartbeat_count_++;
}

}}
