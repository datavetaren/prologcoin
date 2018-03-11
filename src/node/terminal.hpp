#pragma once

#ifndef _node_terminal_hpp
#define _node_terminal_hpp

// terminal
//
// The purpose of the terminal is mostly a debugging tool. It provides
// a command line interface to a running node. Basically, it will be the
// same protocol that nodes use when they communicate with each other.
//
// It's not a Prolog interpreter itself, but it uses terms (and term
// serialization) as data structures. The terminal doesn't have a
// state itself.
//
// Normally, the runnng node is in a "execute next command" state, waiting
// for a Prolog query. But if the query has more solutions (as part of
// interactive backtracking), then the state goes into a "continuation"
// mode. That's when the terminal window changes its "state" and waits
// for a ";" or a "\n" character to determine whether to query for the
// next solution or stop. However, behind the scenes it'll issue a
// "cont." or "stop." command back to the running node.
// This is basically, the only protocol we have.
//

#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <queue>
#include "../common/term_env.hpp"
#include "../common/readline.hpp"
#include "../common/term_serializer.hpp"

namespace prologcoin { namespace node {

class terminal;

class session {
private:
    using term_env = prologcoin::common::term_env;
    using io_service = boost::asio::io_service;
    using endpoint = boost::asio::ip::tcp::endpoint;
    using socket = boost::asio::ip::tcp::socket;

public:
    session(terminal &term, io_service &ios, const endpoint &ep, socket &sock);
    ~session();

    inline terminal & get_terminal() { return terminal_; }
    inline term_env & env() { return env_; }
    inline endpoint & get_endpoint() { return endpoint_; }

    void start(endpoint &ep);
    void send_query(const common::term t);
    void send_query(const common::term t, term_env &src_env);
    void send_buffer(common::term_serializer::buffer_t &buf, size_t n);
    void send_length(size_t n);
    common::term read_reply();
    inline bool has_errors() const { return !errors_.empty(); }
    inline std::string next_error()
        { std::string err = errors_.front(); errors_.pop(); return err; }
    inline const std::string & id() const { return id_; }
    inline void set_id(const std::string &id) { id_ = id; }

private:
    using socket_base = boost::asio::socket_base;
    using tcp = boost::asio::ip::tcp;
    using strand = boost::asio::io_service::strand;
    using term_serializer = prologcoin::common::term_serializer;

    void add_error(const std::string &msg);

    std::string id_;

    terminal &terminal_;
    io_service &ioservice_;
    endpoint endpoint_;
    socket socket_;
    strand strand_;
    bool connected_;

    term_env env_;

    term_serializer::buffer_t buffer_;
    term_serializer::buffer_t buffer_len_;

    std::queue<std::string> errors_;
};

class terminal {
private:
    using io_service = boost::asio::io_service;
    using readline = common::readline;
    using term_env = common::term_env;
    using token_exception = common::token_exception;
    using term_parse_exception = common::term_parse_exception;
    using term = common::term;

public:
    terminal();
    ~terminal();

    void run();

    bool key_callback(int ch);

    void lock_text_output_and(std::function<void()> fn);
    void add_text_output(const std::string &line);
    void add_text_output_no_nl(const std::string &line);
    void add_error(const std::string &line);

    void register_session(session *s);
    void unregister_session(session *s);

private:
    friend class session;

    inline void set_prompt(const std::string &prompt)
       { prompt_ = prompt; }

    session * get_session(const term cmd);

    void command_help(const term cmd);
    void command_quit(const term cmd);
    void command_list(const term cmd);
    void command_new(const term cmd);
    void command_connect(const term cmd);
    void command_kill(const term cmd);
    void command_close(const term cmd);

    session * new_session(const term host_port);
    bool process_errors(session *s);
    void process_query_reply();

    void setup_commands();
    void execute_command(const term cmd);
    void execute_query(const term query);
    void execute_in_query(const std::string &cmd);

    void error(const std::string &cmd,
	       int column,
	       token_exception *token_ex,
	       term_parse_exception *parse_ex);

    inline void clear_in_query() { in_query_ = false; }

    std::string prompt_;
    bool stopped_;
    readline readline_;
    bool ctrl_c_;
    term_env env_;

    io_service ioservice_;

    boost::mutex text_output_queue_mutex_;
    boost::condition_variable text_output_queue_cond_;
    std::queue<std::string> text_output_queue_;

    std::unordered_map<std::string, session *> sessions_;
    std::unordered_map<common::con_cell, std::function<void(const term t)> > commands_;

    session *current_session_;
    bool in_query_;
};

}}

#endif
