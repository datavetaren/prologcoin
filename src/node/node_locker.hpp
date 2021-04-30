#pragma once

#ifndef _node_node_locker_hpp
#define _node_node_locker_hpp

#include <boost/thread/recursive_mutex.hpp>

namespace prologcoin { namespace node {

class self_node;

class node_locker : public boost::noncopyable {
public:
    node_locker(self_node &node);
    node_locker(node_locker &&other);
    ~node_locker();

private:
    boost::recursive_mutex *lock_;
};

}}

#endif
