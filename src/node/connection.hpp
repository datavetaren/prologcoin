#pragma once

#ifndef _node_connection_hpp
#define _node_connection_hpp

#include "asio_win32_check.hpp"

#include <queue>
#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include "../common/term.hpp"
#include "../common/term_env.hpp"
#include "../common/utime.hpp"
#include "ip_address.hpp"
#include "ip_service.hpp"
#include "task.hpp"

namespace prologcoin { namespace node {

class self_node;
class in_session_state;
class task_reset;

class reason_t {
public:
    enum enum_t { ERROR_CANNOT_CONNECT,
		  ERROR_FAIL_CONNECT,
		  ERROR_UNRECOGNIZED,
		  ERROR_SELF,
		  ERROR_VERSION };
    inline reason_t(const reason_t &other) : e_(other.e_) { }
    inline reason_t(const enum_t e) : e_(e) { }

    inline operator enum_t () const { return e_; }

    inline bool operator == (const reason_t &other) {
	return e_ == other.e_;
    }
    inline bool operator != (const reason_t &other) {
	return ! operator == (other);
    }

    inline std::string str() const {
	switch (e_) {
	case ERROR_CANNOT_CONNECT: return "ERROR_CANNOT_CONNECT";
	case ERROR_FAIL_CONNECT: return "ERROR_FAIL_CONNECT";
	case ERROR_UNRECOGNIZED: return "ERROR_UNRECOGNIZED";
	case ERROR_SELF: return "ERROR_SELF";
	case ERROR_VERSION: return "ERROR_VERSION";
	default: return "???";
	}
    }
private:
    enum_t e_;
};

class connection {
protected:
    using socket = boost::asio::ip::tcp::socket;
    using io_service = boost::asio::io_service;
    using deadline_timer = boost::asio::deadline_timer;
    using term = prologcoin::common::term;
    using term_env = prologcoin::common::term_env;

public:
    enum connection_type { CONNECTION_IN, CONNECTION_OUT };

    connection(self_node &self, connection_type type, term_env &env);
    ~connection();

    connection_type type() const { return type_; }

    static void delete_connection(connection *conn);

    inline self_node & self() { return self_node_; }
    inline socket & get_socket() { return socket_; }
    io_service & get_io_service();

    void stop();
    inline bool is_stopped() const { return stopped_; }

    void close();
    inline bool is_closed() const { return get_state() == STATE_CLOSED; }

    inline void prepare_receive() { set_state(STATE_RECEIVE_LENGTH); }
    inline void prepare_send() { set_state(STATE_SEND_LENGTH); }

    void send_error(const term t);
    void send_ok(const term t);
    void send(const term t);
    term received();
    term received(term_env &env);

    inline void set_dispatcher( std::function<void ()> dispatcher )
    { dispatcher_ = dispatcher; }

    inline bool auto_send() const
    { return auto_send_; }
    inline void set_auto_send(bool auto_send)
    { auto_send_ = auto_send; }

protected:
    enum state {
	STATE_IDLE,
	STATE_RECEIVE_LENGTH,
	STATE_RECEIVE,
	STATE_RECEIVED,
	STATE_SEND_LENGTH,
	STATE_SEND,
	STATE_SENT,
	STATE_ERROR,
	STATE_KILLED,
	STATE_CLOSED
    };

    inline state get_state() const { return state_; }
    inline void set_state(state state) { state_ = state; }
    inline void set_state_error(const std::string &info) {
	state_ = STATE_ERROR;
	last_error_ = info;
    }

    void start();
    void run();

    inline void dispatch() { dispatcher_(); }

    boost::asio::io_service::strand strand() { return strand_; }

    void trigger_now();

    const std::string & last_error() const { return last_error_; }

private:
    bool received_length();

    self_node &self_node_;
    connection_type type_;
    term_env &env_;

    boost::asio::io_service::strand strand_;
    socket socket_;
    deadline_timer timer_;

    state state_;
    size_t received_bytes_;
    size_t receive_length_;
    size_t sent_bytes_;
    size_t send_length_;
    std::vector<uint8_t> buffer_len_;
    std::vector<uint8_t> buffer_;

    std::function<void ()> dispatcher_;
    bool auto_send_;
    bool stopped_;
    std::string last_error_;
};

class in_connection : public connection {
public:
    in_connection(self_node &self);
    ~in_connection();

    inline const std::string & name() const { return name_; }

private:
    std::string to_error_message(const std::vector<std::string> &msgs);
    std::string to_error_message(const common::token_exception &ex);
    std::string to_error_message(const common::term_parse_exception &ex);

    common::con_cell get_state_atom();
    void setup_commands();
    in_session_state * get_session(const term id_term);
    inline in_session_state * get_session() { return session_; }

    void on_state();

    void command_new(const term cmd);
    void command_connect(const term cmd);
    void command_name(const term cmd);
    void command_kill(const term cmd);
    void command_next(const term cmd);
    void command_delete_instance(const term cmd);
    void command_reset(const term cmd);
    void command_local_reset(const term cmd);
    void process_command(const term cmd);
    void process_query();
    void process_query_reply();
    void process_execution(const term cmd, bool in_query);

    void reply_exception(const std::string &msg);
    void reply_error(const common::term t);
    void reply_ok(const common::term t);

    friend class self_node;

    std::unordered_map<prologcoin::common::con_cell,
		       std::function<void(common::term cmd)> > commands_;
    in_session_state *session_;
    term_env env_;
    std::string name_;
};

//
// This is an outgoing connection. Its purpose is to connect to other
// nodes and communicate with them. Note that this means running a
// Prolog engine at the remote end.
//
class out_connection;

class out_connection : public connection {
public:
    using utime = prologcoin::common::utime;
    enum out_type_t { STANDARD, VERIFIER };

    out_connection(self_node &self, out_type_t t, const ip_service &ip);
    ~out_connection();

    inline out_type_t out_type() const { return out_type_; }
    inline const ip_service & ip() const { return ip_; }
    inline const std::string & name() const { return name_; }
    inline const std::string & id() const { return id_; }

    inline bool sent_my_name() const { return sent_my_name_; }
    inline void set_sent_my_name() { sent_my_name_ = true; }


    inline void set_id(const std::string &id) { id_ = id; }
    inline void set_name(const std::string &name) { name_ = name; }

    inline term_env & env() { return env_; }

    inline bool use_heartbeat() const { return use_heartbeat_; }
    inline void set_use_heartbeat(bool b) { use_heartbeat_ = b; }

    out_task * create_heartbeat_task();
    out_task * create_publish_task();
    out_task * create_info_task();
    out_task * create_init_connection_task();
    task_reset * create_reset_task();

    inline void schedule(out_task *task) { reschedule_next(task); }

    template<uint64_t C> inline void reschedule(out_task *task, utime::dt<C> dt)
    { reschedule(task, utime::now()+dt); }

    inline void reschedule_next(out_task *task)
    { reschedule(task, 0); }

    void reschedule(out_task *task, utime t);

    inline void reschedule_last(out_task *task)
    { if (work_.empty()) {
          schedule(task);
      } else {
	  utime next_t = last_in_work_+utime::us(1);
          reschedule(task, next_t);
      }
    }

    inline bool is_connected() const { return connected_; }
    inline void set_connected(bool b) { connected_ = b; }

    void print_task_queue() const;

    void error(const reason_t &reason, const std::string &msg);

protected:
    void idle_state();

private:
    void handle_publish_task(out_task &task);
    void handle_info_task(out_task &task);
    static void handle_publish_task_fn(out_task &task);
    static void handle_info_task_fn(out_task &task);

    void handle_init_connection_task(out_task &task);
    static void handle_init_connection_task_fn(out_task &task);

    void send_next_task();
    void on_state();

    void reply_error(const common::term t);
    void reply_ok(const common::term t);

    friend class self_node;
    friend class out_task;

    out_type_t out_type_;
    ip_service ip_;
    std::string id_;
    std::string name_;
    bool init_in_progress_;
    bool use_heartbeat_;
    bool connected_;
    bool sent_my_name_;
    term_env env_;
    boost::recursive_mutex work_lock_;
    // boost::condition_variable work_cv_;
    std::priority_queue<out_task *, std::vector<out_task *>, std::function<bool (const out_task *t1, const out_task *t2)> > work_;
    utime last_in_work_;
};

}}

#endif

