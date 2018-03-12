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

namespace prologcoin { namespace node {

class self_node_exception : public std::runtime_error {
public:
    self_node_exception(const std::string &msg)
	: std::runtime_error("self_node_exception: " + msg) { }
};

class self_node;
class session_state;

class connection {
private:
    using term = prologcoin::common::term;
    using socket = boost::asio::ip::tcp::socket;
    using io_service = boost::asio::io_service;
public:
    connection(self_node &self);
    ~connection();
    void start();

private:
    common::con_cell get_state_atom();
    void setup_commands();
    session_state * get_session(const term id_term);
    inline session_state * get_session() { return session_; }
    void command_new(const term cmd);
    void command_connect(const term cmd);
    void command_kill(const term cmd);
    void command_next(const term cmd);
    void process_command(const term cmd);
    void process_query();
    void process_query_reply();
    void process_execution(const term cmd, bool in_query);

    void run();
    void close();
    void read_query_length();
    void reply_error(const common::term t);
    void reply_ok(const common::term t);
    void reply_answer(const common::term t);

    friend class self_node;

    inline self_node & node() { return self_node_; }
    inline socket & get_socket() { return socket_; }
    io_service & get_io_service();

    self_node &self_node_;
    boost::asio::io_service::strand strand_;
    socket socket_;

    enum state {
	READ_QUERY_LENGTH,
	READ_QUERY,
	REPLY_WITH_ANSWER_LENGTH,
	REPLY_WITH_ANSWER
    };

    state state_;
    size_t read_bytes_;
    size_t query_length_;
    size_t write_bytes_;
    size_t reply_length_;
    std::vector<uint8_t> buffer_len_;
    std::vector<uint8_t> buffer_;
    std::string id_;
    common::term_env env_;
    std::unordered_map<prologcoin::common::con_cell,
		       std::function<void(common::term cmd)> > commands_;
    session_state *session_;
};

class session_state {
public:
    session_state(connection *conn);

    inline const std::string & id() const { return id_; }
    inline common::term_env & env() { return interp_; }

    inline connection * get_connection() { return connection_; }
    inline void set_connection(connection *conn) { connection_ = conn; }
    inline void reset_connection() { connection_ = nullptr; }

    bool execute(const common::term query);

    inline void set_query(const common::term query)
    {
	query_ = query;
        auto vars = env().find_vars(query_);
	vars_ = env().empty_list();
	for (auto v = vars.rbegin(); v != vars.rend(); ++v) {
	    vars_ = env().new_dotted_pair(
			  env().new_term(common::con_cell("=",2),
				 {env().functor(v->first,0), v->second}),
		  vars_);
	}
    }

    inline common::term query() const { return query_; }
    inline common::term query_vars()
    { return vars_; }
    
    inline common::term get_result() { return interp_.get_result_term(); }

    inline bool in_query() const { return in_query_; }

    inline bool has_more() const { return in_query() && interp_.has_more(); }

    inline bool next() {
	bool r = interp_.next();
	if (!r) {
	    in_query_ = false;
	}
	return r;
    }

private:
    std::string id_;
    connection *connection_;
    interp::interpreter interp_;
    bool interp_initialized_;
    common::term query_;
    bool in_query_;
    common::term vars_;
};

class self_node {
private:
    using io_service = boost::asio::io_service;
    friend class connection;

public:
    static const int DEFAULT_PORT = 8783;
    static const size_t MAX_BUFFER_SIZE = 65536;

    self_node();

    void start();
    void stop();
    void join();

    session_state * new_session(connection *conn);
    session_state * find_session(const std::string &id);
    void kill_session(session_state *sess);
    void session_connect(session_state *sess, connection *conn);

private:
    static const int TIMER_INTERVAL_SECONDS = 10;

    void disconnect(connection *conn);
    void run();
    void start_accept();
    void start_prune_dead_connections();
    void prune_dead_connections();
    void close(connection *conn);
    io_service & get_io_service() { return ioservice_; }

    using endpoint = boost::asio::ip::tcp::endpoint;
    using acceptor = boost::asio::ip::tcp::acceptor;
    using socket = boost::asio::ip::tcp::socket;
    using strand = boost::asio::io_service::strand;
    using socket_base = boost::asio::socket_base;
    using tcp = boost::asio::ip::tcp;
    using deadline_timer = boost::asio::deadline_timer;

    bool stopped_;
    boost::thread thread_;
    io_service ioservice_;
    endpoint endpoint_;
    acceptor acceptor_;
    socket socket_;
    strand strand_;
    deadline_timer timer_;

    connection *recent_connection_;
    std::unordered_set<connection *> connections_;

    boost::mutex lock_;
    std::unordered_map<std::string, session_state *> states_;
    std::vector<connection *> closed_;
};

inline boost::asio::io_service & connection::get_io_service()
{
     return self_node_.get_io_service();
}

}}

#endif
