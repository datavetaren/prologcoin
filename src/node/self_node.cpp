#include "asio_win32_check.hpp"
#include <boost/asio/placeholders.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/chrono.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "../common/term_serializer.hpp"
#include "../common/random.hpp"
#include "self_node.hpp"
#include "session.hpp"
#include "address_verifier.hpp"
#include "address_downloader.hpp"
#include "task_execute_query.hpp"

namespace prologcoin { namespace node {

using namespace prologcoin::common;

self_node::self_node(unsigned short port)
    : name_("noname"),
      ioservice_(),
      endpoint_(self_node::tcp::v4(), port),
      acceptor_(ioservice_, endpoint_),
      socket_(ioservice_),
      strand_(ioservice_),
      timer_(ioservice_),
      comment_(env_.empty_list()),
      recent_in_connection_(nullptr),
      preferred_num_standard_out_connections_(DEFAULT_NUM_STANDARD_OUT_CONNECTIONS),
      preferred_num_verifier_connections_(DEFAULT_NUM_VERIFIER_CONNECTIONS),
      num_standard_out_connections_(0),
      num_verifier_connections_(0),
      num_download_addresses_(DEFAULT_NUM_DOWNLOAD_ADDRESSES),
      testing_mode_(false),
      initial_funds_(DEFAULT_INITIAL_FUNDS),
      maximum_funds_(DEFAULT_MAXIMUM_FUNDS),
      new_funds_per_second_(DEFAULT_NEW_FUNDS_PER_SECOND)
{
    set_timer_interval(utime::ss(DEFAULT_TIMER_INTERVAL_SECONDS));
    set_time_to_live(utime::ss(DEFAULT_TTL_SECONDS));
    id_ = random::next();
}

void self_node::start()
{
    stopped_ = false;
    flushed_ = false;

    acceptor_.set_option(acceptor::reuse_address(true));
    acceptor_.set_option(socket_base::enable_connection_aborted(true));
    acceptor_.listen();

    thread_ = boost::thread([&](){ run(); });
}

void self_node::stop()
{
    stop_all_connections();
}

void self_node::run()
{
    start_accept();
    start_tick();

    for (int i = 0; i < 2; i++) {
    	workers_.push_back(boost::thread([this]() { ioservice_.run(); } ));
    }
}

void self_node::close(connection *conn)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);    
    closed_.push_back(conn);
}

void self_node::stop_all_connections()
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    if (stopped_) {
	return;
    }

    stopped_ = true;
    boost::system::error_code ec;
    acceptor_.cancel(ec);
    acceptor_.close();

    recent_in_connection_->stop();
    disconnect(recent_in_connection_);
    for (auto *conn : in_connections_) {
	conn->stop();
    }
    for (auto *conn : out_connections_) {
	conn->stop();
    }
}

bool self_node::all_connections_closed()
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);
    for (auto *conn : in_connections_) {
	if (!conn->is_closed()) {
	    return false;
	}
    }
    for (auto *conn : out_connections_) {
	if (!conn->is_closed()) {
	    return false;
	}
    }
    return true;
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

void self_node::for_each_out_connection(const std::function<void (out_connection *out)> &fn)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    for (auto *conn : out_connections_) {
	auto *out_conn = reinterpret_cast<out_connection *>(conn);
	fn(out_conn);
    }
}

void self_node::for_each_standard_out_connection(const std::function<void (out_connection *out)> &fn)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    for (auto *conn : out_connections_) {
	auto *out_conn = reinterpret_cast<out_connection *>(conn);
	if (out_conn->out_type() == out_connection::STANDARD) {
	    fn(out_conn);
	}
    }
}

void self_node::for_each_in_connection(const std::function<void (in_connection *out)> &fn)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    for (auto *conn : in_connections_) {
	auto *in_conn = reinterpret_cast<in_connection *>(conn);
	fn(in_conn);
    }
}

out_connection * self_node::find_out_connection(const std::string &where)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    for (auto *conn : out_connections_) {
	auto *out = reinterpret_cast<out_connection *>(conn);
	if (out->out_type() == out_connection::STANDARD) {
	    if (out->name() == where) {
		return out;
	    }
	}
    }
    return nullptr;
}

task_execute_query * self_node::schedule_execute_new_instance(const std::string &where)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);
    auto *out = find_out_connection(where);
    if (out == nullptr) {
	return nullptr;
    }
    auto *task = new task_execute_query(*out, task_execute_query::new_instance());
    out->schedule(task);
    return task;
}

task_execute_query * self_node::schedule_execute_delete_instance(const std::string &where)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);
    auto *out = find_out_connection(where);
    if (out == nullptr) {
	return nullptr;
    }
    auto *task = new task_execute_query(*out, task_execute_query::delete_instance());
    out->schedule(task);
    return task;
}

task_execute_query * self_node::schedule_execute_query(term query, term_env &query_src, const std::string &where)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);
    auto *out = find_out_connection(where);
    if (out == nullptr) {
	return nullptr;
    }
    auto *task = new task_execute_query(*out, query, query_src);
    out->schedule(task);
    return task;
}

task_execute_query * self_node::schedule_execute_next(const std::string &where)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);
    auto *out = find_out_connection(where);
    if (out == nullptr) {
	return nullptr;
    }
    auto *task = new task_execute_query(*out, task_execute_query::do_next());
    out->schedule(task);
    return task;
}

self_node::execute_at_return_t self_node::schedule_execute_wait_for_result(task_execute_query *task, term_env &query_src)
{
    task->wait_for_result();
    if (task->failed()) {
	return execute_at_return_t();
    }

    term result_term = task->get_result();
    bool has_more = task->has_more();
    bool at_end = task->at_end();
    // Copy this result to the right environment
    uint64_t cost = task->get_cost();

    uint64_t cost_tmp = 0;
    term result_copy = query_src.copy(result_term, task->env(), cost_tmp);
    task->consume_result();

    return execute_at_return_t(result_copy, has_more, at_end, cost);
}

self_node::execute_at_return_t self_node::execute_at(term query, term_env &query_src, const std::string &where)
{
    auto *task = schedule_execute_query(query, query_src, where);
    if (task == nullptr) {
	// TODO: Throw an exception instead
	return execute_at_return_t();
    }
    return schedule_execute_wait_for_result(task, query_src);
}

self_node::execute_at_return_t self_node::continue_at(term_env &query_src, const std::string &where)
{
    auto *task = schedule_execute_next(where);
    if (task == nullptr) {
	// TODO: Throw an exception instead
	return execute_at_return_t();
    }
    return schedule_execute_wait_for_result(task, query_src);
}

bool self_node::new_instance_at(term_env &query_src, const std::string &where)
{
    auto *task = schedule_execute_new_instance(where);
    if (task == nullptr) {
	// TODO: Throw an exception instead
	return false;
    }
    auto r = schedule_execute_wait_for_result(task, query_src);
    return !r.failed();
}

bool self_node::delete_instance_at(term_env &query_src, const std::string &where)
{
    auto *task = schedule_execute_delete_instance(where);
    if (task == nullptr) {
	// TODO: Throw an exception instead
	return false;
    }
    auto r = schedule_execute_wait_for_result(task, query_src);
    return !r.failed();
}

in_session_state * self_node::new_in_session(in_connection *conn)
{
    auto *ss = new in_session_state(this, conn);
    ss->set_available_funds( get_initial_funds() );
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
    task_address_downloader *task = new task_address_downloader(*out);
    out->schedule(task);
    return out;
}

bool self_node::has_standard_out_connection(const ip_service &ip)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    return out_standard_ips_.find(ip) != out_standard_ips_.end();
}

bool self_node::recently_failed(const ip_service &ip)
{
    auto it = recently_failed_.find(ip);
    if (it == recently_failed_.end()) {
	return false;
    }
    auto p = it->second;
    utime t0 = utime::now();
    auto t1 = p.first;
    auto cnt = p.second;
    uint64_t span = 60;
    // First three attemptes all acceptable within next minute
    // Then we try next hour.
    switch (cnt) {
    case 0: case 1: case 2: case 3: span = 60; break;
    default: span = 3600; break;
    }
    auto diff = (t0 - t1).in_ss();
    return diff < span;
}

out_connection * self_node::new_verifier_connection(const ip_service &ip)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    auto *out = new out_connection(*this, out_connection::VERIFIER, ip);
    task_address_verifier *task = new task_address_verifier(*out);
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
			       if (!stopped_) {
				   recent_in_connection_->start();
				   start_accept();
			       }
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
	    case out_connection::STANDARD: num_standard_out_connections_--;
		break;
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
	if (!has_standard_out_connection(e) && !recently_failed(e)) {
	    new_standard_out_connection(e);
	}
    }
}

void self_node::check_standard_out_connections()
{
    if (stopped_ ||
	num_standard_out_connections_ >= preferred_num_standard_out_connections_) {
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
    if (stopped_ ||
	num_verifier_connections_ >= preferred_num_verifier_connections_) {
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

    if (stopped_) {
	return;
    }

    check_standard_out_connections();
    check_verifier_connections();
}

void self_node::failed_connection(const ip_service &ip)
{
    auto &p = recently_failed_[ip];
    recently_failed_sorted_.erase(std::make_pair(p.first, ip));
    p.first = utime::now();
    p.second++;
    recently_failed_sorted_.insert(std::make_pair(p.first, ip));

    //
    // If the recently failed set is too big, then prune the oldest ones.
    // (Never remove more than 10 at a time. Shouldn't happen in practice
    //  but as a double safety check.)
    size_t cnt = 0;
    while (cnt < 10 &&
	recently_failed_.size()>2*preferred_num_standard_out_connections_) {
	auto oldest = *recently_failed_sorted_.begin();
	recently_failed_sorted_.erase(oldest);
	recently_failed_.erase(oldest.second);
	cnt++;
    }
}

void self_node::successful_connection(const ip_service &ip)
{
    auto it = recently_failed_.find(ip);
    if (it == recently_failed_.end()) {
	return;
    }
    auto &p = it->second;
    recently_failed_sorted_.erase(std::make_pair(p.first, ip));
    recently_failed_.erase(it);
}

void self_node::join()
{
    using namespace boost::system;

    stop_all_connections();
    for (size_t i = 0; i < 100 && !all_connections_closed(); i++) {
	prune_dead_connections();
	utime::sleep(utime::us(fast_timer_interval_microseconds_));
    }
    timer_.expires_from_now(boost::posix_time::microseconds(
		    get_fast_timer_interval_microseconds()));
    
    timer_.async_wait(
         strand_.wrap(
		 [this](const error_code &ec) {
		     this->flushed_ = true;
		 }));
    while (!flushed_) {
	utime::sleep(utime::us(get_fast_timer_interval_microseconds()));
    }
    ioservice_.stop();
    thread_.join();
    while (!workers_.empty()) {
	auto &worker = workers_.back();
	worker.join();
	workers_.pop_back();
    }
}

bool self_node::join_us(uint64_t us)
{
    return thread_.timed_join(boost::posix_time::microseconds(us));
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

void self_node::create_mailbox(const std::string &mailbox_name)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    mailbox_[mailbox_name] = std::queue<std::string>();
}

void self_node::send_message(const std::string &mailbox_name,
			     const std::string &from,
			     const std::string &message)
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    auto it = mailbox_.find(mailbox_name);
    if (it == mailbox_.end()) {
	// Unknown mailbox. Just drop the message silently.
	return;
    }
    auto &q = it->second;
    std::string msg = (from.empty() ? "" : "@") + from + ": ";
    msg += message;
    q.push(msg);
}

// Flush messages, move them to text_out (with mailbox_name as prefix)
std::string self_node::check_mail()
{
    boost::lock_guard<boost::recursive_mutex> guard(lock_);

    std::string s;

    for (auto &p : mailbox_) {
	auto &name = p.first;
	auto &msgs = p.second;
	while (!msgs.empty()) {
	    const std::string &msg = msgs.front();
	    s  += name + ": " + msg;
	    if (!boost::ends_with(msg, "\n")) {
		s += "\n";
	    }
	    msgs.pop();
	}
    }

    return s;
}

}}
