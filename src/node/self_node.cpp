#include <boost/asio/placeholders.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "self_node.hpp"
#include "session.hpp"
#include "../common/term_serializer.hpp"
#include "../common/random.hpp"

namespace prologcoin { namespace node {

using namespace prologcoin::common;

self_node::self_node()
    : ioservice_(),
      endpoint_(self_node::tcp::v4(), self_node::DEFAULT_PORT),
      acceptor_(ioservice_, endpoint_),
      socket_(ioservice_),
      strand_(ioservice_),
      timer_(ioservice_, boost::posix_time::seconds(TIMER_INTERVAL_SECONDS)),
      recent_in_connection_(nullptr)
{
}

void self_node::start()
{
    stopped_ = false;

    acceptor_.set_option(acceptor::reuse_address(true));
    acceptor_.set_option(socket_base::enable_connection_aborted(true));
    acceptor_.listen();

    thread_ = boost::thread([&](){ run(); });
}

void self_node::stop()
{
    stopped_ = true;
    ioservice_.stop();
}

void self_node::run()
{
    start_accept();
    start_prune_dead_connections();
    ioservice_.run();
}

void self_node::close(connection *conn)
{
    boost::lock_guard<boost::mutex> guard(lock_);    
    closed_.push_back(conn);
}

void self_node::for_each_in_session(const std::function<void (in_session_state *session)> &fn)
{
    boost::lock_guard<boost::mutex> guard(lock_);

    for (auto p : in_states_) {
	auto *session = p.second;
	fn(session);
    }
}

in_session_state * self_node::new_in_session(in_connection *conn)
{
    auto *ss = new in_session_state(this, conn);

    boost::lock_guard<boost::mutex> guard(lock_);
    in_states_[ss->id()] = ss;
    return ss;
}

in_session_state * self_node::find_in_session(const std::string &id)
{
    boost::lock_guard<boost::mutex> guard(lock_);
    
    auto it = in_states_.find(id);
    if (it == in_states_.end()) {
	return nullptr;
    }

    return it->second;
}

void self_node::in_session_connect(in_session_state *sess, in_connection *conn)
{
    boost::lock_guard<boost::mutex> guard(lock_);

    auto *old_conn = sess->get_connection();

    if (old_conn == conn) {
	return;
    }

    disconnect(old_conn);
    sess->set_connection(conn);
}

out_connection * self_node::new_out_connection(const ip_service &ip)
{
    auto *out = new out_connection(*this, ip);
    out_connections_.insert(out);
    return out;
}

void self_node::kill_in_session(in_session_state *sess)
{
    boost::lock_guard<boost::mutex> guard(lock_);

    in_states_.erase(sess->id());
    delete sess;
}

void self_node::start_accept()
{
    using namespace boost::asio;
    using namespace boost::system;

    in_connection *conn = new in_connection(*this);
    in_connections_.insert(conn);
    recent_in_connection_ = conn;
    acceptor_.async_accept(conn->get_socket(),
		   strand_.wrap(
			   [this](const error_code &){
			       recent_in_connection_->start();
			       start_accept();
			   }));
}

void self_node::start_prune_dead_connections()
{
    using namespace boost::asio;
    using namespace boost::system;

    timer_.async_wait(
	      strand_.wrap(
			   [this](const error_code &) {
			       prune_dead_connections();
			       timer_.expires_from_now(
					 boost::posix_time::seconds(
					    TIMER_INTERVAL_SECONDS));
			       master_hook();
			       start_prune_dead_connections();
			   }));
}

void self_node::prune_dead_connections()
{
    boost::lock_guard<boost::mutex> guard(lock_);
    while (!closed_.empty()) {
	auto *c = closed_.back();
	if (c->type() == connection::IN) {
	    auto *s = reinterpret_cast<in_connection *>(c)->get_session();
	    if (s != nullptr) {
		s->reset_connection();
	    }
	}
	disconnect(c);
	closed_.pop_back();
    }
}

void self_node::join()
{
    thread_.join();
}

bool self_node::join_us(uint64_t us)
{
    return thread_.try_join_for(boost::chrono::microseconds(us));
}

void self_node::disconnect(connection *conn)
{
    if (conn->type() == connection::IN) {
	auto it = std::find(in_connections_.begin(), in_connections_.end(), conn);
	if (it != in_connections_.end()) {
	    in_connections_.erase(it);
	}
    }
    connection::delete_connection(conn);
}

void self_node::master_hook()
{
    if (master_hook_) {
	master_hook_(*this);
    }
}

}}
