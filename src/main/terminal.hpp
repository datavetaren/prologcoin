#pragma once

#ifndef _main_terminal_hpp
#define _main_terminal_hpp

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
#include "../common/readline.hpp"

namespace prologcoin { namespace main {

//
// This is the main terminal window that acts like a Prolog prompt
// and establishes a direct link to the node on the local machine.
//
class terminal {
private:
    using io_service = boost::asio::io_service;
    using endpoint = boost::asio::ip::tcp::endpoint;
    using socket = boost::asio::ip::tcp::socket;
    using readline = common::readline;
    using term_env = common::term_env;
    using token_exception = common::token_exception;
    using term_parse_exception = common::term_parse_exception;
    using term = common::term;
    using term_serializer = common::term_serializer;

public:
    terminal(unsigned short port);
    ~terminal();

    bool connect();
    void run();

    bool key_callback(int ch);

    void lock_text_output_and(std::function<void()> fn);
    void add_text_output(const std::string &line);
    void add_text_output_no_nl(const std::string &line);
    void add_error(const std::string &line);

private:
    void halt();

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

    std::string prompt_;
    bool stopped_;
    bool connected_;
    readline readline_;
    bool ctrl_c_;
    term_env env_;

    io_service ioservice_;
    endpoint endpoint_;
    socket socket_;

    term_serializer::buffer_t buffer_;
    term_serializer::buffer_t buffer_len_;
    std::queue<std::string> errors_;

    std::string session_id_;
    bool in_query_;

    boost::mutex text_output_queue_mutex_;
    boost::condition_variable text_output_queue_cond_;
    std::queue<std::string> text_output_queue_;
};

}}

#endif
