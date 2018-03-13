#pragma once

#ifndef _node_connection_hpp
#define _node_connection_hpp

#include "asio_win32_check.hpp"

#include "../common/term.hpp"
#include "../common/term_env.hpp"
#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace prologcoin { namespace node {

class self_node;
class in_session_state;

class connection {
protected:
    using socket = boost::asio::ip::tcp::socket;
    using io_service = boost::asio::io_service;
    using term = prologcoin::common::term;
    using term_env = prologcoin::common::term_env;

public:
    enum connection_type { IN, OUT };

    connection(self_node &self, connection_type type, term_env &env);
    ~connection() = default;

    connection_type type() const { return type_; }

    static void delete_connection(connection *conn);

    inline self_node & node() { return self_node_; }
    inline socket & get_socket() { return socket_; }
    io_service & get_io_service();

    void close();

    inline void prepare_receive() { set_state(RECEIVE_LENGTH); }

    void send_error(const term t);
    void send_ok(const term t);
    void send(const term t);
    term received();

    inline void set_dispatcher( std::function<void ()> dispatcher )
    { dispatcher_ = dispatcher; }

protected:
    enum state {
	IDLE,
	RECEIVE_LENGTH,
	RECEIVE,
	RECEIVED,
	SEND_LENGTH,
	SEND,
	SENT
    };

    inline state get_state() const { return state_; }
    inline void set_state(state state) { state_ = state; }

    void run();

    inline void dispatch() { dispatcher_(); }

private:
    void received_length();

    self_node &self_node_;
    connection_type type_;
    term_env &env_;

    boost::asio::io_service::strand strand_;
    socket socket_;

    state state_;
    size_t received_bytes_;
    size_t receive_length_;
    size_t sent_bytes_;
    size_t send_length_;
    std::vector<uint8_t> buffer_len_;
    std::vector<uint8_t> buffer_;

    std::function<void ()> dispatcher_;
};

class in_connection : public connection {
public:
    in_connection(self_node &self);
    ~in_connection();
    void start();

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

    std::string id_;
    std::unordered_map<prologcoin::common::con_cell,
		       std::function<void(common::term cmd)> > commands_;
    in_session_state *session_;
    term_env env_;
};

}}

#endif

