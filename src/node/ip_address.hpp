#pragma once

#ifndef _node_ip_address_hpp
#define _node_ip_address_hpp

#include "../common/fast_hash.hpp"
#include "../common/term_serializer.hpp" // buffer_t
#include "../common/utime.hpp"

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
    inline ip_address()
    { memset(addr_, 0, sizeof(addr_)); }

    inline ip_address(const ip_address &other)
    { set_addr(other); }

    inline ip_address(boost::asio::ip::address_v4 &v4)
    { set_addr(v4); }

    inline ip_address(boost::asio::ip::address_v6 &v6)
    { set_addr(v6); }

    inline ip_address(boost::asio::ip::address &ip)
    { set_addr(ip); }

    ip_address(const std::string &str);

    boost::asio::ip::address to_addr() const;

    inline void set_addr(const ip_address &other)
    { set_addr(other.to_bytes(), sizeof(addr_)); }

    inline void set_addr(const boost::asio::ip::address_v4 &v4)
    { set_addr(&v4.to_bytes()[0], 4); }

    inline void set_addr(const boost::asio::ip::address_v6 &v6)
    { set_addr(&v6.to_bytes()[0], 16); }
    
    inline void set_addr( const uint8_t *bytes, size_t n)
    {   assert(n == 4 || n == 16);
	if (n == 4) {
	    memcpy(&addr_[0], &AS_IP4[0], sizeof(AS_IP4));
	}
	memcpy(at_byte(-n), &bytes[0], n);
    }

    inline void set_addr( uint32_t value )
    { uint8_t bytes[4] = { static_cast<uint8_t>((value >> 24) & 0xff),
			   static_cast<uint8_t>((value >> 16) & 0xff),
			   static_cast<uint8_t>((value >> 8) & 0xff),
			   static_cast<uint8_t>(value & 0xff) };
	set_addr(bytes, 4);
    }

    inline const uint8_t * to_bytes() const { return &addr_[0]; }
    inline uint8_t * to_bytes() { return &addr_[0]; }
    inline size_t bytes_size() const { return sizeof(addr_); }

    inline bool operator == (const ip_address &other) const {
	return memcmp(to_bytes(), other.to_bytes(), sizeof(addr_)) == 0;
    }
    inline bool operator != (const ip_address &other) const {
	return memcmp(to_bytes(), other.to_bytes(), sizeof(addr_)) != 0;
    }
    inline bool operator < (const ip_address &other) const {
	return memcmp(to_bytes(), other.to_bytes(), sizeof(addr_)) < 0;
    }
    inline bool operator <= (const ip_address &other) const {
	return memcmp(to_bytes(), other.to_bytes(), sizeof(addr_)) <= 0;
    }
    inline bool operator > (const ip_address &other) const {
	return memcmp(to_bytes(), other.to_bytes(), sizeof(addr_)) > 0;
    }
    inline bool operator >= (const ip_address &other) const {
	return memcmp(to_bytes(), other.to_bytes(), sizeof(addr_)) >= 0;
    }

    bool is_zero() const;  // 0.0.0.0 (or 16 bytes with 0s)

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

    std::string str(size_t maxlen = std::string::npos) const;

    static std::string group_str(uint64_t group);

private:
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

}}

namespace std {
    template<> struct hash<prologcoin::node::ip_address> {
        size_t operator()(const prologcoin::node::ip_address &addr) const {
	    prologcoin::common::fast_hash h;
	    h.update(addr.to_bytes(), 16);
	    return static_cast<uint32_t>(h);
	}
    };
}

#endif

