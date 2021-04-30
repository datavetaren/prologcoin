#include "node_locker.hpp"
#include "self_node.hpp"

namespace prologcoin { namespace node {

node_locker::node_locker(self_node &node) : lock_(&node.lock_)
{ lock_->lock(); }
	
node_locker::node_locker(node_locker &&other) : lock_(std::move(other.lock_))
{ }

node_locker::~node_locker() { lock_->unlock(); }

}}
