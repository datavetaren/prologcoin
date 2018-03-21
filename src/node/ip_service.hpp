#pragma once

#ifndef _node_ip_service_hpp
#define _node_ip_service_hpp

#include "ip_address.hpp"

namespace prologcoin { namespace node {

//
// This class abstracts IP addresses and normalize them to IPv6.
// We also add a bunch of useful predicates and other information
// extracting properties. If you look carefully at bitcoind you'll
// see that addresses are pretty complicated. Something that you first
// perceive as something easy, is not when you scratch its surface.
//
class ip_service : public ip_address {
public:
    inline ip_service() : ip_address(), port_(0) { }
    inline ip_service(const ip_service &other)
        : ip_address(other), port_(other.port_) { }

    inline const ip_address & addr() const { return *this; }

    inline unsigned short port() const { return port_; }
    inline void set_port(unsigned short port) { port_ = port; }

    inline ip_service(const ip_address &ip, unsigned short port)
	: ip_address(ip), port_(port) { }

    inline ip_service(boost::asio::ip::address_v4 &v4, unsigned short port)
	: ip_address(v4), port_(port) { }

    inline ip_service(boost::asio::ip::address_v6 &v6, unsigned short port)
	: ip_address(v6), port_(port) { }

    inline ip_service(boost::asio::ip::address &ip, unsigned short port)
        : ip_address(ip), port_(port) { }

    inline ip_service(const std::string &str, unsigned short port)
	: ip_address(str), port_(port) { }

    inline bool operator == (const ip_service &other) const
        { return ip_address::operator ==(other) && port_ == other.port_; }
    inline bool operator != (const ip_service &other) const
        { return ! operator == (other); }

private:
    unsigned short port_;
};

}}

#endif

