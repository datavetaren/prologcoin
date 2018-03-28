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
    ~connection() = default;

    connection_type type() const { return type_; }

    static void delete_connection(connection *conn);

    inline self_node & self() { return self_node_; }
    inline socket & get_socket() { return socket_; }
    io_service & get_io_service();

    void close();

    inline void prepare_receive() { set_state(RECEIVE_LENGTH); }
    inline void prepare_send() { set_state(SEND_LENGTH); }

    void send_error(const term t);
    void send_ok(const term t);
    void send(const term t);
    term received();

    inline void set_dispatcher( std::function<void ()> dispatcher )
    { dispatcher_ = dispatcher; }

    inline bool auto_send() const
    { return auto_send_; }
    inline void set_auto_send(bool auto_send)
    { auto_send_ = auto_send; }

protected:
    enum state {
	IDLE,
	RECEIVE_LENGTH,
	RECEIVE,
	RECEIVED,
	SEND_LENGTH,
	SEND,
	SENT,
	KILLED
    };

    inline state get_state() const { return state_; }
    inline void set_state(state state) { state_ = state; }

    void start();
    void run();

    inline void dispatch() { dispatcher_(); }

    boost::asio::io_service::strand strand() { return strand_; }

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
};

class in_connection : public connection {
public:
    in_connection(self_node &self);
    ~in_connection();

private:
    common::con_cell get_state_atom();
    void setup_commands();
    in_session_state * get_session(const term id_term);
    inline in_session_state * get_session() { return session_; }

    void on_state();

    void command_new(const term cmd);
    void command_connect(const term cmd);
    void command_kill(const term cmd);
    void command_next(const term cmd);
    void process_command(const term cmd);
    void process_query();
    void process_query_reply();
    void process_execution(const term cmd, bool in_query);

    void reply_error(const common::term t);
    void reply_ok(const common::term t);

    friend class self_node;

    std::unordered_map<prologcoin::common::con_cell,
		       std::function<void(common::term cmd)> > commands_;
    in_session_state *session_;
    term_env env_;
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

    inline term_env & env() { return env_; }

    inline void set_use_heartbeat(bool b) { use_heartbeat_ = b; }

    out_task create_heartbeat_task();
    out_task create_init_connection_task();

    inline void schedule(out_task &task) {
	reschedule(task, utime::us(0));
    }

    template<uint64_t C> inline void reschedule(out_task &task, utime::dt<C> dt)
    { reschedule(task, utime::now()+dt); }

    inline void reschedule(out_task &task, utime t)
    {
        // If task issues a reschedule on SEND, which normally it doesn't
        // as it doesn't pop the work queue, then we'll pop the work queue
        // to remove it first.
        if (task.get_state() == out_task::SEND) {
  	    work_.pop();
        }

	task.set_when(t);
	if (t > last_in_work_) {
	    last_in_work_ = t;
	}
	work_.push(task);
    }

    inline void reschedule_last(out_task &task)
    { if (work_.empty()) {
          schedule(task);
      } else {
          reschedule(task, last_in_work_+utime::us(1));
      }
    }

    inline bool is_connected() const
    { return connected_; }

protected:
    void idle_state();
    void error(const std::string &msg);

private:
    void handle_heartbeat_task(out_task &task);
    static void handle_heartbeat_task_fn(out_task &task);

    void handle_init_connection_task(out_task &task);
    static void handle_init_connection_task_fn(out_task &task);

    void send_next_task();
    void on_state();

    void reply_error(const common::term t);
    void reply_ok(const common::term t);

    friend class self_node;

    out_type_t out_type_;
    ip_service ip_;
    std::string id_;
    bool use_heartbeat_;
    bool connected_;
    term_env env_;
    std::priority_queue<out_task> work_;
    utime last_in_work_;
};

}}

#endif

