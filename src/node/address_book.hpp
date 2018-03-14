#pragma once

#ifndef _node_address_book_hpp
#define _node_address_book_hpp

#include <iostream>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>

namespace prologcoin { namespace node {

//
// This class abstracts IP addresses and normalize them to IPv6.
// We also add a bunch of useful predicates and other information
// extracting properties. If you look carefully at bitcoind you'll
// see that addresses are pretty complicated. Something that you first
// perceive as something easy, is not when you scratch its surface.
//
class ip_address {
public:
    inline ip_address(boost::asio::ip::address_v4 &v4)
    { set_addr(v4); }

    inline ip_address(boost::asio::ip::address_v6 &v6)
    { set_addr(v6); }

    inline ip_address(boost::asio::ip::address &ip)
    { set_addr(ip); }

    ip_address(const std::string &str);

    bool is_v4() const;
    bool is_v6() const;

    bool is_valid() const;
    bool is_local() const;
    bool is_routable() const;
    bool is_tor() const;

    bool is_rfc_1918() const; // IP4 private networks (10.0.0.0/8,
                              // 192.168.0.0/16, 172.16.0.0.12)
    bool is_rfc_2544() const; // IP4 inter-network (192.18.0.0/15)
    bool is_rfc_3927() const; // IP4 autoconfig (169.254.0.0/16)
    bool is_rfc_5737() const; // IP4 documentation (192.0.2.0/24,
                              // 198.51.100.0/24, 203.0.113.0/24)
    bool is_rfc_6598() const; // IP4 ISP-level NAT (100.64.0.0/10)


    bool is_rfc_3849() const; // IP6 documentation (2001:0DB8::/32)
    bool is_rfc_3964() const; // IP6 6to4 tunnelling (2002::/16)
    bool is_rfc_4193() const; // IP6 unique local (FC00::/7)
    bool is_rfc_4380() const; // IP6 Teredo tunnelling (2001::/32)
    bool is_rfc_4843() const; // IP6 ORCHID deprecated (2001:10::/28)
    bool is_rfc_4862() const; // IP6 autoconfig (FF80::/64)
    bool is_rfc_6052() const; // IP6 well known prefix (64:FF9b::/96)
    bool is_rfc_6145() const; // IP6 IP4-translated (::FFFF:0:0:0/96)
    bool is_rfc_7343() const; // IP6 ORCHID2 (2001:20::/28)
    bool is_he_net() const;   // 2001:470::/36

    uint64_t group() const; // Return unique value for network group

    std::string str() const;

    static std::string group_str(uint64_t group);

private:
    inline void set_addr(const boost::asio::ip::address_v4 &v4)
    { memcpy(&addr_[0], &AS_IP4[0], sizeof(AS_IP4));
      auto bb = v4.to_bytes();
      memcpy(at_byte(-4), &bb[0], 4);
    }

    inline void set_addr(const boost::asio::ip::address_v6 &v6)
    { memcpy(at_byte(0), &v6.to_bytes()[0], 16); }

    void set_addr(const boost::asio::ip::address &addr);

    inline const unsigned char * at_byte(int index) const {
	if (index < 0) return &addr_[MAX+index]; else return &addr_[index];
    }

    inline unsigned char * at_byte(int index) {
	if (index < 0) return &addr_[MAX+index]; else return &addr_[index];
    }

    inline unsigned char b(int index) const {
	if (index < 0) return addr_[MAX+index]; else return addr_[index];
    }

    inline uint64_t u(int index) const {
	return static_cast<uint64_t>(b(index));
    }

    static const size_t MAX = 16;
    static const unsigned char AS_IP4 [12];
    unsigned char addr_[MAX];

    static const unsigned char AS_ONION [12];
};

class address_entry {
public:
    address_entry(const std::string &host, int port);
		  
private:
    int port_;
    uint64_t time_;
};

class address_book {
public:
};

}}

#endif

