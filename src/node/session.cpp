#include "../common/random.hpp"
#include "session.hpp"
#include <boost/range/adaptor/reversed.hpp>

namespace prologcoin { namespace node {

using namespace prologcoin::common;

in_session_state::in_session_state(self_node *self, in_connection *conn)
  : self_(self),
    connection_(conn),
    interp_(*this),
    heartbeat_count_(0),
    available_funds_(0)
{
    id_ = "s" + random::next();
}

term in_session_state::query_closure()
{
    return interp_.new_dotted_pair(interp_.query(), interp_.query_var_list());
}

bool in_session_state::execute(const term query)
{
    interp_.ensure_initialized();
    interp_.reset_text_out();
    interp_.set_maximum_cost(available_funds_);
    bool r = interp_.execute(query);
    auto cost = interp_.accumulated_cost();
    if (cost > available_funds_) {
	available_funds_ = 0;
    } else {
	available_funds_ -= cost;
    }
    return r;
}

bool in_session_state::next()
{
    interp_.reset_text_out();
    interp_.set_maximum_cost(available_funds_);
    bool r = interp_.next();
    auto cost = interp_.accumulated_cost();
    if (cost > available_funds_) {
	available_funds_ = 0;
    } else {
	available_funds_ -= cost;
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

void in_session_state::heartbeat()
{
    heartbeat_ = utime::now();
    heartbeat_count_++;
}

}}
