#pragma once

#ifndef _node_self_node_hpp
#define _node_self_node_hpp

#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
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

class connection {
private:
    using socket = boost::asio::ip::tcp::socket;
    using io_service = boost::asio::io_service;
public:
    connection(self_node &self);
    ~connection();
    void start();

private:
    void run();
    void read_query_length();
    void process_query();
    void reply_error(const common::term t);
    void reply_ok(const common::term t);
    void reply_answer(const common::term t);

    friend class self_node;

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
    interp::interpreter interpreter_;
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

private:
    void disconnect(connection *conn);
    void run();
    void start_accept();
    io_service & get_io_service() { return ioservice_; }

    using endpoint = boost::asio::ip::tcp::endpoint;
    using acceptor = boost::asio::ip::tcp::acceptor;
    using socket = boost::asio::ip::tcp::socket;
    using socket_base = boost::asio::socket_base;
    using tcp = boost::asio::ip::tcp;

    bool stopped_;
    boost::thread thread_;
    io_service ioservice_;
    endpoint endpoint_;
    acceptor acceptor_;
    socket socket_;
    std::vector<connection *> connections_;
};

inline boost::asio::io_service & connection::get_io_service()
{
     return self_node_.get_io_service();
}

}}

#endif
