#pragma once

#ifndef _common_hex_hpp
#define _common_hex_hpp

#include <sstream>
#include <iomanip>

namespace prologcoin { namespace common {

class hex {
public:
    static inline std::string to_string(const uint8_t *bytes, size_t n)
    {
	std::stringstream ss;
	for (size_t i = 0; i < n; i++) {
	    ss << std::hex << std::setfill('0') << std::setw(2) << (((int)bytes[i]) & 0xff);
	}
	return ss.str();
    }
};

}}

#endif
