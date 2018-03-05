#pragma once

#ifndef _node_terminal_hpp
#define _node_terminal_hpp

#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <queue>
#include "../common/term_env.hpp"
#include "../common/readline.hpp"

namespace prologcoin { namespace node {

class terminal {
private:
    using readline = common::readline;
    using term_env = common::term_env;
    using token_exception = common::token_exception;
    using term_parse_exception = common::term_parse_exception;

public:
    terminal();

    void run();

    bool key_callback(int ch);

    void lock_text_output_and(std::function<void()> fn);
    void add_text_output(const std::string &line);

private:
    void start_connector();
    void send_query(const common::term t);
    void send_buffer(common::term_serializer::buffer_t &buf, size_t n);
    void send_length(size_t n);
    common::term read_reply();

    void error(const std::string &cmd,
	       int column,
	       token_exception *token_ex,
	       term_parse_exception *parse_ex);

    std::string prompt_;
    bool stopped_;
    readline readline_;
    term_env env_;

    using io_service = boost::asio::io_service;
    using endpoint = boost::asio::ip::tcp::endpoint;
    using socket = boost::asio::ip::tcp::socket;
    using socket_base = boost::asio::socket_base;
    using tcp = boost::asio::ip::tcp;
    using strand = boost::asio::io_service::strand ;

    io_service ioservice_;
    endpoint endpoint_;
    socket socket_;
    strand strand_;
    boost::thread connector_thread_;
    bool connected_;
    int connect_count_;

    common::term_serializer::buffer_t buffer_;
    common::term_serializer::buffer_t buffer_len_;

    boost::mutex text_output_queue_mutex_;
    boost::condition_variable text_output_queue_cond_;
    std::queue<std::string> text_output_queue_;
};

}}

#endif
