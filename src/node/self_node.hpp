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
#include "../global/global.hpp"
#include "../terminal/terminal.hpp"

namespace prologcoin { namespace node {

class out_task;
class task_execute_query;
class sync;

class self_node_exception : public std::runtime_error {
public:
    self_node_exception(const std::string &msg)
	: std::runtime_error("self_node_exception: " + msg) { }
};

class self_node;
class local_interpreter;

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

class node_delayed_t : public interp::interpreter_base::delayed_t {
public:
    node_delayed_t(interp::interpreter_base *i, common::term q, common::term e) : delayed_t(i,q,e,nullptr), task_(nullptr) { }
    virtual ~node_delayed_t();
    void set_task(task_execute_query *task) { task_ = task; }
private:
    task_execute_query *task_;
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

    static const unsigned short DEFAULT_PORT = prologcoin::terminal::terminal::DEFAULT_PORT;
    static const size_t MAX_BUFFER_SIZE = prologcoin::terminal::terminal::MAX_BUFFER_SIZE;
    static const size_t DEFAULT_NUM_STANDARD_OUT_CONNECTIONS = 8;
    static const size_t DEFAULT_NUM_VERIFIER_CONNECTIONS = 3;
    static const size_t DEFAULT_NUM_DOWNLOAD_ADDRESSES = 100;
    static const size_t DEFAULT_TTL_SECONDS = 60;
    static const uint64_t DEFAULT_INITIAL_FUNDS = 10000;
    static const uint64_t DEFAULT_MAXIMUM_FUNDS = 10000;
    static const uint64_t DEFAULT_NEW_FUNDS_PER_SECOND = 100;

    self_node(const std::string &data_dir, unsigned short port = DEFAULT_PORT);
    ~self_node();

    inline term_env & env() { return env_; }

    inline global::global & global() { return global_; }
    inline const global::global & global() const { return global_; }    

    inline void erase_db(const std::string &data_dir) { global::global::erase_db(data_dir); }

    inline bool is_grant_root_for_local() const { return grant_root_for_local_; }
    inline void set_grant_root_for_local(bool b) { grant_root_for_local_ = b; }

    inline const std::string & id() const { return id_; }

    inline boost::asio::ip::address address() { return endpoint_.address(); }
    inline unsigned short port() const { return endpoint_.port(); }

    inline void set_name(const std::string &name) { name_ = name; }
    inline const std::string & name() const { return name_; }

    inline const std::string & data_directory() const { return data_dir_; }

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
    void start_sync();
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

    task_execute_query * schedule_execute_new_instance(const std::string &where);    
    task_execute_query * schedule_execute_delete_instance(const std::string &where);
    
    task_execute_query * schedule_execute_query(term query, node_delayed_t *delayed, term_env &query_src, const std::string &where, interp::remote_execute_mode mode);
    task_execute_query * schedule_execute_next(const std::string &where, term_env &query_src, interp::remote_execute_mode mode);

    interp::remote_return_t schedule_execute_wait_for_result(task_execute_query *task, term_env &query_src);

    void add_waiting(out_task *task);

    bool new_instance_at(term_env &query_src, const std::string &where);
    bool delete_instance_at(term_env &query_src, const std::string &where);

    interp::remote_return_t execute_at(term query,
				       term else_do,
				       interp::interpreter_base &query_interp,
				       const std::string &where,
				       interp::remote_execute_mode mode,
				       size_t timeout);

    interp::remote_return_t continue_at(term query,
					term else_do,
					interp::interpreter_base &query_interp,
					const std::string &where,
					interp::remote_execute_mode mode,
					size_t timeout);

    in_session_state * new_in_session(in_connection *conn, bool is_root);
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

    inline void add_parallel(task_execute_query *t) {
	parallel_.push_back(t);
    }
    
    inline void notify_parallel() {
	boost::unique_lock<boost::mutex> lockit(parallel_changed_lock_);
	parallel_changed_.notify_one();
    }

    bool wait_parallel_us(uint64_t timeout_microsec);

    std::vector<task_execute_query *> & all_parallel() {
	return parallel_;
    }

    bool check_pow() const {
	return global().check_pow();
    }
    
    void set_check_pow(bool b) {
	global().set_check_pow(b);
    }

    void change_connection_name(const std::string &old, const std::string &name);
    bool is_unique_connection_name(const std::string &name) {
	auto it =  named_out_connections_.find(name);
	if (it == named_out_connections_.end()) {
	    return false;
	}
	return it->second == 1;
    }

    bool is_sync_complete() {
	boost::lock_guard<boost::recursive_mutex> guard(lock_);	
	return sync_complete_;
    }

    void set_sync_complete(bool b) {
	boost::lock_guard<boost::recursive_mutex> guard(lock_);
	sync_complete_ = b;
    }

    void set_sync_init(const std::string &str) {
	boost::lock_guard<boost::recursive_mutex> guard(lock_);	
	sync_init_ = str;
    }

    std::string get_sync_init() {
	boost::lock_guard<boost::recursive_mutex> guard(lock_);	
	return sync_init_;
    }

    std::string get_sync_mode() {
	boost::lock_guard<boost::recursive_mutex> guard(lock_);
	return sync_mode_;
    }
    
    void set_sync_mode(const std::string &mode) {
	boost::lock_guard<boost::recursive_mutex> guard(lock_);	
	sync_mode_ = mode;
    }

    size_t get_syncing_meta_block() {
	boost::lock_guard<boost::recursive_mutex> guard(lock_);
	return syncing_meta_block_;
    }

    void set_syncing_meta_block(size_t m) {
	boost::lock_guard<boost::recursive_mutex> guard(lock_);
	syncing_meta_block_ = m;
    }    

private:
    bool join_us(uint64_t microsec);

    static const int DEFAULT_TIMER_INTERVAL_MILLISECONDS = 10000;

    void stop_all_connections();
    bool all_connections_closed();
    void disconnect(connection *conn);
    void run();
    void start_accept();
    void start_tick();
    void stop_sync();
    void process_waiting_tasks();
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
    std::unordered_map<std::string, size_t> named_out_connections_;
    std::unordered_set<ip_service> out_standard_ips_;
    std::unordered_map<ip_service, std::pair<utime, size_t> > recently_failed_;
    std::set<std::pair<utime, ip_service> > recently_failed_sorted_;
    boost::recursive_mutex waiting_tasks_lock_;
    std::vector<out_task *> waiting_tasks_;

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

    bool grant_root_for_local_;

    std::string data_dir_;
  
    // This is where the consensus is stored
    global::global global_;

    // Tracking outgoing parallel tasks
    std::vector<task_execute_query *> parallel_;
    boost::mutex parallel_changed_lock_;
    boost::condition_variable parallel_changed_;

    sync *sync_{nullptr};
    bool sync_complete_{false};
    std::string sync_init_; // Source code before running sync thread
                            // Useful for testing
    std::string sync_mode_;
    size_t syncing_meta_block_{0};
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
