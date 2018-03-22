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

namespace prologcoin { namespace node {

class self_node_exception : public std::runtime_error {
public:
    self_node_exception(const std::string &msg)
	: std::runtime_error("self_node_exception: " + msg) { }
};

class self_node;

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

class self_node {
private:
    using io_service = boost::asio::io_service;
    friend class connection;
    friend class address_book_wrapper;

public:
    static const int DEFAULT_PORT = 8783;
    static const size_t MAX_BUFFER_SIZE = 65536;

    self_node();

    address_book_wrapper book() {
	return address_book_wrapper(*this, address_book_);
    }

    inline void set_master_hook(const std::function<void (self_node &)> &hook)
    { master_hook_ = hook; }

    void start();
    void stop();
    void join();
    template<uint64_t C> inline bool join( common::utime::dt<C> t ) {
	return join_us(t);
    }


    void for_each_in_session( const std::function<void (in_session_state *)> &fn);
    in_session_state * new_in_session(in_connection *conn);
    in_session_state * find_in_session(const std::string &id);
    void kill_in_session(in_session_state *sess);
    void in_session_connect(in_session_state *sess, in_connection *conn);

    out_connection * new_out_connection(const ip_service &ip);

private:
    bool join_us(uint64_t microsec);

    static const int TIMER_INTERVAL_SECONDS = 10;

    void disconnect(connection *conn);
    void run();
    void start_accept();
    void start_prune_dead_connections();
    void prune_dead_connections();
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

    bool stopped_;
    boost::thread thread_;
    io_service ioservice_;
    endpoint endpoint_;
    acceptor acceptor_;
    socket socket_;
    strand strand_;
    deadline_timer timer_;

    in_connection *recent_in_connection_;
    std::unordered_set<connection *> in_connections_;
    std::unordered_set<connection *> out_connections_;

    boost::mutex lock_;
    std::unordered_map<std::string, in_session_state *> in_states_;
    std::vector<connection *> closed_;

    address_book address_book_;

    std::function<void (self_node &self)> master_hook_;
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
