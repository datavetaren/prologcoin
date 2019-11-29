#pragma once

#ifndef _node_self_node_hpp
#define _node_self_node_hpp

#include "asio_win32_check.hpp"

#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <string>
#include <ctime>

#include "../interp/interpreter.hpp"
#include "connection.hpp"
#include "address_book.hpp"

namespace prologcoin { namespace node {

class task_execute_query;

class self_node_exception : public std::runtime_error {
public:
    self_node_exception(const std::string &msg)
	: std::runtime_error("self_node_exception: " + msg) { }
};

class self_node;

class address_book_wrapper
{
public:
    address_book_wrapper(address_book_wrapper &&other)
      : self_(other.self_),
	book_(other.book_) { }

    address_book_wrapper(self_node &self, address_book &book);
    ~address_book_wrapper();

    inline address_book & operator ()() { return book_; }

private:
    self_node &self_;
    address_book &book_;
};

class self_node {
private:
    using io_service = boost::asio::io_service;
    using utime = prologcoin::common::utime;
    using term = prologcoin::common::term;
    using term_env = prologcoin::common::term_env;

    friend class connection;
    friend class address_book_wrapper;

public:
    static const int VERSION_MAJOR = 0;
    static const int VERSION_MINOR = 10;

    static const unsigned short DEFAULT_PORT = 8783;
    static const size_t MAX_BUFFER_SIZE = 65536;
    static const size_t DEFAULT_NUM_STANDARD_OUT_CONNECTIONS = 8;
    static const size_t DEFAULT_NUM_VERIFIER_CONNECTIONS = 3;
    static const size_t DEFAULT_NUM_DOWNLOAD_ADDRESSES = 100;
    static const size_t DEFAULT_TTL_SECONDS = 60;
    static const uint64_t DEFAULT_INITIAL_FUNDS = 10000;
    static const uint64_t DEFAULT_MAXIMUM_FUNDS = 10000;
    static const uint64_t DEFAULT_NEW_FUNDS_PER_SECOND = 100;

    self_node(unsigned short port = DEFAULT_PORT);

    inline term_env & env() { return env_; }

    inline const std::string & id() const { return id_; }

    inline unsigned short port() const { return endpoint_.port(); }

    inline void set_name(const std::string &name) { name_ = name; }
    inline const std::string & name() const { return name_; }

    // Must be a Prolog term
    void set_comment(const std::string &str);
    inline term get_comment() const { return comment_; }

    // Funding settings
    inline uint64_t get_initial_funds() const { return initial_funds_; }
    inline void set_initial_funds(uint64_t funds) { initial_funds_ = funds; }
    inline uint64_t get_maximum_funds() const { return maximum_funds_; }
    inline void set_maximum_funds(uint64_t funds) { maximum_funds_ = funds; }
    inline uint64_t new_funds_per_second() const { return new_funds_per_second_; }
    inline void set_new_funds_per_second(uint64_t funds)
    { new_funds_per_second_ = funds; }

    address_book_wrapper book() {
	return address_book_wrapper(*this, address_book_);
    }

    inline void set_master_hook(const std::function<void (self_node &)> &hook)
    { master_hook_ = hook; }

    void start();
    void stop();
    void join();
    template<uint64_t C> inline bool join( common::utime::dt<C> t ) {
	return join_us(t);
    }

    inline uint64_t get_timer_interval_microseconds() const {
	return timer_interval_microseconds_;
    }
    inline uint64_t get_fast_timer_interval_microseconds() const {
	return fast_timer_interval_microseconds_;
    }

    template<uint64_t C> inline void set_time_to_live(utime::dt<C> t)
    { time_to_live_microseconds_ = t; }
    inline uint64_t time_to_live_microseconds() const
    { return time_to_live_microseconds_; }

    // Makes it easier to write fast unit tests that quickly propagate
    // addresses.
    inline bool is_testing_mode() const {
	return testing_mode_;
    }
    inline void set_testing_mode(bool b) {
	testing_mode_ = b;
    }

    template<uint64_t C> inline void set_timer_interval(utime::dt<C> t)
    {
	timer_interval_microseconds_ = t;
	fast_timer_interval_microseconds_ = t / 10;
	timer_.expires_from_now(boost::posix_time::microseconds(
				timer_interval_microseconds_));

    }

    inline size_t get_num_download_addresses() const {
	return num_download_addresses_;
    }

    inline bool is_self(const ip_service &ip) const {
	return self_ips_.find(ip) != self_ips_.end();
    }

    inline void add_self(const ip_service &ip) {
	self_ips_.insert(ip);
    }

    void for_each_in_session( const std::function<void (in_session_state *)> &fn);

    void for_each_in_connection( const std::function<void (in_connection *conn)> &fn);
    void for_each_out_connection( const std::function<void (out_connection *conn)> &fn);
    void for_each_standard_out_connection( const std::function<void (out_connection *conn)> &fn);

    out_connection * find_out_connection(const std::string &where);

    class execute_at_return_t {
    public:
	execute_at_return_t() : result_(), has_more_(false), at_end_(false) { }
	execute_at_return_t(term r) : result_(r), has_more_(false), at_end_(false) { }
	execute_at_return_t(term r, bool has_more, bool at_end, uint64_t cost) : result_(r), has_more_(has_more), at_end_(at_end), cost_(cost) { }
	execute_at_return_t(const execute_at_return_t &other) = default;

	term result() const { return result_; }
	bool failed() const { return result_ == term(); }
	bool has_more() const { return has_more_; }
	bool at_end() const { return at_end_; }
	uint64_t get_cost() const { return cost_; }
    private:
	term result_;
	bool has_more_;
	bool at_end_;
	uint64_t cost_;
    };

    task_execute_query * schedule_execute_new_instance(const std::string &where);    
    task_execute_query * schedule_execute_delete_instance(const std::string &where);    
    task_execute_query * schedule_execute_query(term query, term_env &query_src, const std::string &where);
    task_execute_query * schedule_execute_next(const std::string &where);

    execute_at_return_t schedule_execute_wait_for_result(task_execute_query *task, term_env &query_src);

    bool new_instance_at(term_env &query_src, const std::string &where);
    bool delete_instance_at(term_env &query_src, const std::string &where);
    execute_at_return_t execute_at(term query, term_env &query_src,
				   const std::string &where);

    execute_at_return_t continue_at(term_env &query_src,
				    const std::string &where);

    in_session_state * new_in_session(in_connection *conn);
    in_session_state * find_in_session(const std::string &id);
    void kill_in_session(in_session_state *sess);
    void in_session_connect(in_session_state *sess, in_connection *conn);

    out_connection * new_standard_out_connection(const ip_service &ip);
    out_connection * new_verifier_connection(const ip_service &ip);

    void failed_connection(const ip_service &ip);
    void successful_connection(const ip_service &ip);

    void create_mailbox(const std::string &mailbox_name);

    void send_message(const std::string &mailbox_name,
		      const std::string &from,
		      const std::string &message);

    std::string check_mail();

    class locker;
    friend class locker;

    class locker : public boost::noncopyable {
    public:
	inline locker(self_node &node) : lock_(&node.lock_) { lock_->lock(); }
	inline locker(locker &&other) : lock_(std::move(other.lock_)) { }
	inline ~locker() { lock_->unlock(); }

    private:
	boost::recursive_mutex *lock_;
    };

    inline locker locked() {
	return locker(*this);
    }

private:
    bool join_us(uint64_t microsec);

    static const int DEFAULT_TIMER_INTERVAL_SECONDS = 10;

    void stop_all_connections();
    bool all_connections_closed();
    void disconnect(connection *conn);
    void run();
    void start_accept();
    void start_tick();
    void prune_dead_connections();
    void connect_to(const std::vector<address_entry> &entries);
    void check_out_connections();
    void check_standard_out_connections();
    bool has_standard_out_connection(const ip_service &ip);
    bool recently_failed(const ip_service &ip);
    void check_verifier_connections();
    void close(connection *conn);
    void master_hook();

    io_service & get_io_service() { return ioservice_; }

    using endpoint = boost::asio::ip::tcp::endpoint;
    using acceptor = boost::asio::ip::tcp::acceptor;
    using socket = boost::asio::ip::tcp::socket;
    using strand = boost::asio::io_service::strand;
    using socket_base = boost::asio::socket_base;
    using tcp = boost::asio::ip::tcp;
    using deadline_timer = boost::asio::deadline_timer;

    common::term_env env_;

    std::string id_;
    std::string name_;
    bool stopped_;
    bool flushed_;
    boost::thread thread_;
    
    io_service ioservice_;

    std::vector<boost::thread> workers_;

    endpoint endpoint_;
    acceptor acceptor_;
    socket socket_;
    strand strand_;
    deadline_timer timer_;
    common::term comment_;

    std::unordered_set<ip_service> self_ips_;

    in_connection *recent_in_connection_;
    std::unordered_set<connection *> in_connections_;
    std::unordered_set<connection *> out_connections_;
    std::unordered_set<ip_service> out_standard_ips_;
    std::unordered_map<ip_service, std::pair<utime, size_t> > recently_failed_;
    std::set<std::pair<utime, ip_service> > recently_failed_sorted_;

    boost::recursive_mutex lock_;
    std::unordered_map<std::string, in_session_state *> in_states_;
    std::vector<connection *> closed_;

    address_book address_book_;

    std::function<void (self_node &self)> master_hook_;

    size_t preferred_num_standard_out_connections_;
    size_t preferred_num_verifier_connections_;
    size_t num_standard_out_connections_;
    size_t num_verifier_connections_;

    uint64_t timer_interval_microseconds_;
    uint64_t fast_timer_interval_microseconds_;
    uint64_t time_to_live_microseconds_;
    size_t num_download_addresses_;

    std::map<std::string, std::queue<std::string> > mailbox_;

    bool testing_mode_;

    uint64_t initial_funds_;
    uint64_t maximum_funds_;
    uint64_t new_funds_per_second_;
};

inline address_book_wrapper::address_book_wrapper(self_node &self, address_book &book) : self_(self), book_(book)
{
    self_.lock_.lock();
}

inline address_book_wrapper::~address_book_wrapper()
{
    self_.lock_.unlock();
}

}}

#endif
