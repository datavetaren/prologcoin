#pragma once

#ifndef _node_terminal_hpp
#define _node_terminal_hpp

#include "../node/asio_win32_check.hpp"

#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <string>
#include <iostream>

#include "../common/term_env.hpp"
#include "../common/term_tokenizer.hpp"
#include "../common/term_parser.hpp"

namespace prologcoin { namespace node {

//
// This class provides terminal access to a node programmatically (non-interactive.)
//
class terminal {
private:
    using io_service = boost::asio::io_service;
    using endpoint = boost::asio::ip::tcp::endpoint;
    using socket = boost::asio::ip::tcp::socket;
    using term_env = common::term_env;
    using token_exception = common::token_exception;
    using term_parse_exception = common::term_parse_exception;
    using term = common::term;
    using term_serializer = common::term_serializer;

public:
    terminal(unsigned short port, const std::string &addr = "127.0.0.1");
    ~terminal();

    bool connect();
    void close();

    void lock_text_output_and(std::function<void()> fn);
    void add_text_output(const std::string &line);
    void add_text_output_no_nl(const std::string &line);
    void add_error(const std::string &line);

    std::string flush_text();

    void print_text();
    void purge_text();

    term parse(const std::string &str);

    inline bool execute(const std::string &str)
    { term t = parse(str); if (t != term()) return execute(t); else return false; }

    inline bool execute(term t) { return execute_query(t); }

    inline bool has_more() const { return has_more_; }
    inline bool next() { if (!has_more_) { return false; } else return execute_in_query(";"); }
    bool reset();

    inline term_env & env() { return env_; }

    inline term get_result() const { return result_; }
    inline term get_result_string() const { return result_; }
    inline const std::vector<std::string> & var_names() const { return var_names_; }
    inline term get_result(const std::string &var_name) const {
	auto it = var_result_.find(var_name);
	if (it == var_result_.end()) return term(); else return it->second;
    }

    inline const std::string & get_result_string(const std::string &var_name) const {
	static const std::string empty;
	auto it = var_result_string_.find(var_name);
	if (it == var_result_string_.end()) return empty; else return it->second;
    }

protected:
    inline void set_has_more() { has_more_ = true; }

private:
    void run();

    bool send_query(const common::term t);
    bool send_query(const common::term t, term_env &src_env);
    bool send_buffer(common::term_serializer::buffer_t &buf, size_t n);
    bool send_length(size_t n);
    common::term read_reply();
    bool execute_query(const term query);
    bool execute_in_query(const std::string &cmd);
    bool process_query_reply();

    void error(const std::string &cmd,
	       int column,
	       token_exception *token_ex,
	       term_parse_exception *parse_ex);

    bool connected_;
    term_env env_;

    io_service ioservice_;
    endpoint endpoint_;
    socket socket_;

    term_serializer::buffer_t buffer_;
    term_serializer::buffer_t buffer_len_;
    std::queue<std::string> errors_;

    std::string session_id_;
    bool has_more_;

    term result_;
    std::vector<std::string> var_names_;
    std::unordered_map<std::string, term> var_result_;
    std::unordered_map<std::string, std::string> var_result_string_;

    boost::mutex text_output_queue_mutex_;
    boost::condition_variable text_output_queue_cond_;
    std::queue<std::string> text_output_queue_;
};

}}

#endif
