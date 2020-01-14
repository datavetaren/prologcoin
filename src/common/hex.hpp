#pragma once

#ifndef _common_hex_hpp
#define _common_hex_hpp

#include <cassert>
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

    static inline void from_string(const std::string &str, uint8_t *bytes, size_t bytes_len) {
      size_t n = str.size();
      assert(2*bytes_len == n);
      size_t j = 0;
      for (size_t i = 0; i < n; i += 2, j++) {
	  char nibble_hi = tolower(str[i]), nibble_lo = tolower(str[i+1]);

	  if (nibble_hi >= '0' && nibble_hi <= '9') nibble_hi -= '0';
	  else if (nibble_hi >= 'a' && nibble_hi <= 'f') nibble_hi -= 'a' + 10;
	  else nibble_hi = 0;

	  if (nibble_lo >= '0' && nibble_lo <= '9') nibble_lo -= '0';
	  else if (nibble_lo >= 'a' && nibble_lo <= 'f') nibble_lo -= 'a' + 10;
	  else nibble_lo = 0;

	  bytes[j] = (static_cast<uint8_t>(nibble_hi) << 4) | static_cast<uint8_t>(nibble_lo);
      }
    }
};

}}

#endif
