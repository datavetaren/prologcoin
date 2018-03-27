#include "ip_address.hpp"

namespace prologcoin { namespace node {

const unsigned char ip_address::AS_IP4[12] = {0,0,0,0,0,0,0,0,0,0,0xff,0xff};
const unsigned char ip_address::AS_ONION[12] = {0xFD,0x87,0xD8,0x7E,0xEB,0x43};

void ip_address::set_addr(const boost::asio::ip::address &ip)
{
    if (ip.is_v4()) {
	set_addr(ip.to_v4());
    } else if (ip.is_v6()) {
	set_addr(ip.to_v6());
    } else {
	assert("Unknown IP address" == nullptr);
    }
}

ip_address::ip_address(const std::string &str)
{
    set_addr(boost::asio::ip::address::from_string(str));
}

boost::asio::ip::address ip_address::to_addr() const
{
    if (is_v4()) {
	std::array<unsigned char, 4> arr;
	memcpy(&arr[0], at_byte(-4), arr.size());
	return boost::asio::ip::address_v4(arr);
    } else {
	std::array<unsigned char, 16> arr;
	memcpy(&arr[0], at_byte(0), arr.size());
	return boost::asio::ip::address_v6(arr);
    }
}

bool ip_address::is_zero() const
{
    static const unsigned char ZERO[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    return memcmp(at_byte(0), ZERO, sizeof(ZERO)) == 0;
}

bool ip_address::is_v4() const
{
    return memcmp(at_byte(0), AS_IP4, sizeof(AS_IP4)) == 0;
}

bool ip_address::is_v6() const
{
    return !is_v4() && !is_tor();
}

bool ip_address::is_rfc_1918() const
{
    if (!is_v4()) {
	return false;
    }
    return b(-4) == 10 ||                                // 10.0.0.0/24
   	   (b(-4) == 192 && b(-3) == 168) ||             // 192.168.0.0/16
   	   (b(-4) == 172 && b(-3) >= 16 && b(-3) <= 31); // 172.16.0.0/12
             
}

bool ip_address::is_rfc_2544() const
{
    if (!is_v4()) {
	return false;
    }
    return b(-4) == 192 && (b(-3) == 18 || b(-3) == 19);
}

bool ip_address::is_rfc_3927() const
{
    if (!is_v4()) {
	return false;
    }
    return b(-4) == 169 && b(-3) == 254;
}

bool ip_address::is_rfc_5737() const
{
    if (!is_v4()) {
	return false;
    }
    if (b(-4) == 192 && b(-3) == 0 && b(-2) == 2) {
	return true;
    }
    if (b(-4) == 198 && b(-3) == 51 && b(-2) == 100) {
	return true;
    }
    if (b(-4) == 203 && b(-3) == 0 && b(-2) == 113) {
	return true;
    }
    return false;
}

bool ip_address::is_rfc_6598() const
{
    if (!is_v4()) {
	return false;
    }
    return b(-4) == 100 && b(-3) >= 64 && b(-3) <= 127;
}

bool ip_address::is_rfc_3964() const
{
    return b(-16) == 0x20 && b(-15) == 2;
}

bool ip_address::is_rfc_6052() const
{
    static const unsigned char prefix[12] = {0,0x64,0xFF,0x9B,0,0,0,0,0,0,0,0};
    return memcmp(at_byte(0), &prefix[0], sizeof(prefix)) == 0;
}

bool ip_address::is_rfc_3849() const
{
    static const unsigned char prefix[4] = {0x20,1,0xD,0xB8};
    return memcmp(at_byte(0), &prefix[0], sizeof(prefix)) == 0;
}

bool ip_address::is_rfc_4380() const
{
    static const unsigned char prefix[4] = {0x20,1,0,0};
    return memcmp(at_byte(0), &prefix[0], sizeof(prefix)) == 0;
}

bool ip_address::is_rfc_4862() const
{
    static const unsigned char prefix[8] = {0xFE,0x80,0,0,0,0,0,0};
    return memcmp(at_byte(0), &prefix[0], sizeof(prefix)) == 0;
}

bool ip_address::is_rfc_4193() const
{
    return (b(-16) & 0xFE) == 0xFC;
}

bool ip_address::is_rfc_6145() const
{
    static const unsigned char prefix[12] = {0,0,0,0,0,0,0,0,0xFF,0xFF,0,0};
    return memcmp(at_byte(0), &prefix[0], sizeof(prefix)) == 0;
}

bool ip_address::is_rfc_4843() const
{
    return b(-16) == 0x20 && b(-15) == 1 && b(-14) == 0 &&
	   (b(-13) & 0xF0) == 0x10;
}

bool ip_address::is_rfc_7343() const
{
    return b(-16) == 0x20 && b(-15) == 1 && b(-14) == 0 &&
	   (b(-13) & 0xF0) == 0x20;
}

bool ip_address::is_he_net() const
{
    static const unsigned char prefix[4] = {0x20,1,4,0x70};
    return memcmp(at_byte(0), &prefix[0], sizeof(prefix)) == 0;
}

bool ip_address::is_tor() const
{
    return memcmp(at_byte(0), AS_ONION, sizeof(AS_ONION)) == 0;
}

bool ip_address::is_local() const
{
    if (is_v4()) {
	return b(-4) == 127 || b(-4) == 0;
    } else {
	static const unsigned char local_v6[16]
	    = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	return memcmp(at_byte(0), local_v6, sizeof(local_v6)) == 0;
    }
}

bool ip_address::is_valid() const
{
    static const unsigned char ZERO[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    if (memcmp(at_byte(0), &ZERO[0], sizeof(ZERO)) == 0) {
	return false;
    }

    if (is_rfc_3849()) {
	return false;
    }

    if (is_v4()) {
	if (memcmp(at_byte(-4), &ZERO[0], 4) == 0) {
	    return false;
	}
	static const unsigned char NONE[4] = {0xFF,0xFF,0xFF,0xFF};
	if (memcmp(at_byte(-4), &NONE[0], sizeof(NONE)) == 0) {
	    return false;
	}
    }

    return true;
}

bool ip_address::is_routable() const
{
    return is_valid() &&
	!(is_rfc_1918() || is_rfc_2544() || is_rfc_3927() || is_rfc_4862() ||
	  is_rfc_6598() || is_rfc_5737() || (is_rfc_4193() && !is_tor()) ||
	  is_rfc_4843() || is_rfc_7343() || is_local());
}

uint64_t ip_address::group() const
{
    if (is_local()) {
	return 0xff000000;
    }

    if (!is_routable()) {
	return 0;
    }

    if (is_v4() || is_rfc_6145() || is_rfc_6052()) {
	return (u(-4) << 24) | (u(-3) << 16);
    }
    
    if (is_rfc_3964()) {
	return (u(2) << 24) | (u(3) << 16);
    }

    if (is_rfc_4380()) {
	return (u(2) << 8) | u(1);
    }

    if (is_tor()) {
	return u(6) << 32;
    }

    if (is_he_net()) {
	return (u(0) << 28) | (u(1) << 20) | (u(2) << 12) |
	       (u(3) << 4) | (u(4) >> 4);
    }
    
    // Use first 32-bits
    return (u(0) << 24) | (u(1) << 16) | (u(2) << 8) | u(3);
}

std::string ip_address::str(size_t maxlen) const
{
    std::string s;
    if (is_v4()) {
	unsigned int b1 = b(-4);
	unsigned int b2 = b(-3);
	unsigned int b3 = b(-2);
	unsigned int b4 = b(-1);
	std::stringstream ss;
	ss << b1 << "." << b2 << "." << b3 << "." << b4;
	s = ss.str();
    } else {
	// Let's use boost for now...
	std::array<unsigned char, 16> arr;
	memcpy(&arr[0], at_byte(0), arr.size());
	boost::asio::ip::address_v6 v6(arr);
	s = v6.to_string();
    }

    if (maxlen != std::string::npos && s.size() > maxlen) {
	s = s.substr(s.size()-maxlen);
    }
    return s;
}

std::string ip_address::group_str(uint64_t group)
{
    if ((group >> 32) == 0) {
	std::stringstream ss;
	ss << ((group >> 24) & 0xff) << "."
	   << ((group >> 16) & 0xff) << "."
	   << ((group >> 8) & 0xff) << "."
	   << ((group >> 0) & 0xff);
	return ss.str();
    }

    std::stringstream ss;
    size_t g = 64-16;
    for (size_t i = 0; i < 4; i++) {
	ss << std::hex << ((group >> g) & 0xffff);
	if (i < 3) ss << ":";
    }
    return ss.str();
}

}}
