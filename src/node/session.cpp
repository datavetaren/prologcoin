#include "../common/random.hpp"
#include "session.hpp"

namespace prologcoin { namespace node {

using namespace prologcoin::common;

in_session_state::in_session_state(self_node *self, in_connection *conn)
  : connection_(conn),
    interp_(self),
    in_query_(false)
{
    id_ = "s" + random::next();
}

bool in_session_state::execute(const term query)
{
    interp_.ensure_initialized();

    query_ = query;
    in_query_ = true;
    bool r = interp_.execute(query);
    if (!r) {
	in_query_ = false;
    }
    return r;
}

}}
