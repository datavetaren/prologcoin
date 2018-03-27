#include "asio_win32_check.hpp"
#include <boost/asio/placeholders.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "self_node.hpp"
#include "session.hpp"
#include "address_verifier.hpp"
#include "../common/term_serializer.hpp"
#include "../common/random.hpp"

namespace prologcoin { namespace node {

using namespace prologcoin::common;

self_node::self_node(unsigned short port)
    : ioservice_(),
      endpoint_(self_node::tcp::v4(), port),
      acceptor_(ioservice_, endpoint_),
      socket_(ioservice_),
      strand_(ioservice_),
      timer_(ioservice_),
      recent_in_connection_(nullptr),
      preferred_num_standard_out_connections_(DEFAULT_NUM_STANDARD_OUT_CONNECTIONS),
      preferred_num_verifier_connections_(DEFAULT_NUM_VERIFIER_CONNECTIONS),
      num_standard_out_connections_(0),
      num_verifier_connections_(0)
{
    set_timer_interval(utime::ss(DEFAULT_TIMER_INTERVAL_SECONDS));
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
    start_tick();
    ioservice_.run();
}

void self_node::close(connection *conn)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);    
    closed_.push_back(conn);
}

void self_node::set_comment(const std::string &str)
{
    comment_ = env_.parse(str);
}

void self_node::for_each_in_session(const std::function<void (in_session_state *session)> &fn)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    for (auto p : in_states_) {
	auto *session = p.second;
	fn(session);
    }
}

in_session_state * self_node::new_in_session(in_connection *conn)
{
    auto *ss = new in_session_state(this, conn);

    boost::lock_guard<boost::recursive_mutex> guard(lock_);
    in_states_[ss->id()] = ss;
    return ss;
}

in_session_state * self_node::find_in_session(const std::string &id)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);
    
    auto it = in_states_.find(id);
    if (it == in_states_.end()) {
	return nullptr;
    }

    return it->second;
}

void self_node::in_session_connect(in_session_state *sess, in_connection *conn)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    auto *old_conn = sess->get_connection();

    if (old_conn == conn) {
	return;
    }

    disconnect(old_conn);
    sess->set_connection(conn);
}

out_connection * self_node::new_standard_out_connection(const ip_service &ip)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    auto *out = new out_connection(*this, out_connection::STANDARD, ip);
    out_connections_.insert(out);
    out_standard_ips_.insert(ip);
    num_standard_out_connections_++;
    return out;
}

bool self_node::has_standard_out_connection(const ip_service &ip)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    return out_standard_ips_.find(ip) != out_standard_ips_.end();
}

out_connection * self_node::new_verifier_connection(const ip_service &ip)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    auto *out = new out_connection(*this, out_connection::VERIFIER, ip);
    address_verifier task(*out);
    out->schedule(task);
    out_connections_.insert(out);
    num_verifier_connections_++;
    return out;
}

void self_node::kill_in_session(in_session_state *sess)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

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

void self_node::start_tick()
{
    using namespace boost::asio;
    using namespace boost::system;

    timer_.async_wait(
	      strand_.wrap(
			   [this](const error_code &) {
			       prune_dead_connections();
			       check_out_connections();
			       master_hook();
			       timer_.expires_from_now(
					 boost::posix_time::microseconds(
					    timer_interval_microseconds_));
			       start_tick();
			   }));
}

void self_node::prune_dead_connections()
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);
    while (!closed_.empty()) {
	auto *c = closed_.back();
	if (c->type() == connection::CONNECTION_IN) {
	    auto *s = reinterpret_cast<in_connection *>(c)->get_session();
	    if (s != nullptr) {
		s->reset_connection();
	    }
	} else {
	    assert(c->type() == connection::CONNECTION_OUT);
	    auto *out = reinterpret_cast<out_connection *>(c);
	    switch (out->out_type()) {
	    case out_connection::STANDARD: num_standard_out_connections_--; break;
	    case out_connection::VERIFIER: num_verifier_connections_--; break;
	    }
	}
	disconnect(c);
	closed_.pop_back();
    }
}

void self_node::connect_to(const std::vector<address_entry> &entries)
{
    for (auto &e : entries) {
	if (!has_standard_out_connection(e)) {
	    new_standard_out_connection(e);
	}
    }
}

void self_node::check_standard_out_connections()
{
    if (num_standard_out_connections_ >= preferred_num_standard_out_connections_) {
	return;
    }

    size_t remaining = preferred_num_standard_out_connections_ - num_standard_out_connections_;
    size_t num_top10 = (remaining + 1) / 2;
    size_t num_bot90 = remaining - num_top10;
    auto top10 = address_book_.get_randomly_from_top_10_pt(num_top10);
    auto bot90 = address_book_.get_randomly_from_bottom_90_pt(num_bot90);
    // std::cout << "Top10%: n=" << top10.size() << " bot90%%: n=" << bot90.size() << std::endl;
    connect_to(top10);
    connect_to(bot90);
}

void self_node::check_verifier_connections()
{
    if (num_verifier_connections_ >= preferred_num_verifier_connections_) {
	return;
    }

    size_t remaining = preferred_num_verifier_connections_ - num_verifier_connections_;
    auto unverified = address_book_.get_randomly_from_unverified(remaining);
    for (auto &addr : unverified) {
	auto *out = new_verifier_connection(addr);
	out->set_use_heartbeat(false);
    }
}

//
// Check if number of outgoing connections is too low.
// If so, up to the remaining connections, take half from top 10%
// addresses and half from bottom 90%.
//
void self_node::check_out_connections()
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    check_standard_out_connections();
    check_verifier_connections();
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
    auto &conn_set = (conn->type() == connection::CONNECTION_IN) ? in_connections_ : out_connections_;
    auto it = std::find(conn_set.begin(), conn_set.end(), conn);
    if (it != conn_set.end()) {
	conn_set.erase(it);
    }
    if (conn->type() == connection::CONNECTION_OUT) {
	auto *out_conn = reinterpret_cast<out_connection *>(conn);
	if (out_conn->out_type() == out_connection::STANDARD) {
	    out_standard_ips_.erase(out_conn->ip());
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
