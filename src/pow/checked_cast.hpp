#pragma once

#ifndef _pow_checked_cast_hpp
#define _pow_checked_cast_hpp

#include <limits>
#include <sstream>
#include <stdexcept>

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

class checked_cast_exception : public std::runtime_error {
public:
    checked_cast_exception(const std::string &msg) : runtime_error(msg) { }
};

template<typename T, typename U> inline T checked_cast(U value,
						       T t_min, T t_max)
{
    if (value < t_min) {
	std::stringstream ss;
	ss << "Value " << value << " was less than minimum " << t_min;
	throw checked_cast_exception(ss.str());
    } else if (value > t_max) {
	std::stringstream ss;
	ss << "Value " << value << " was greater than maximum " << t_max;
	throw checked_cast_exception(ss.str());
    } else {
	return static_cast<T>(value);
    }
}

template<typename T, typename U> inline T checked_cast(U value)
{
    return checked_cast<T>(value,
			   std::numeric_limits<T>::min(),
			   std::numeric_limits<T>::max());
}

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

#endif
