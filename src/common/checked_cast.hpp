#pragma once

#ifndef _common_checked_cast_hpp
#define _common_checked_cast_hpp

#include <limits>
#include <sstream>
#include <stdexcept>

namespace prologcoin { namespace common {

class checked_cast_exception : public std::runtime_error {
public:
    checked_cast_exception(const std::string &msg) : runtime_error(msg) { }
};

template<bool,bool> struct checked_compare_ab;

template<> struct checked_compare_ab<false,false> {
   template<typename T, typename U> static inline int compare(T t, U u) {
      if (t < u) {
	 return -1;
      } else if (t > u) {
	 return 1;
      } else {
	 return 0;
      }
   }
};

template<> struct checked_compare_ab<true,true> {
   template<typename T, typename U> static inline int compare(T t, U u) {
      if (t < u) {
	 return -1;
      } else if (t > u) {
	 return 1;
      } else {
	 return 0;
      }
   }
};

template<> struct checked_compare_ab<true,false> {
   template<typename T, typename U> static inline int compare(T t, U u) {
      if (t < 0) {
	 return -1;
      }
      return checked_compare_ab<false,false>::compare(static_cast<typename std::make_unsigned<T>::type>(t), u);
   }
};

template<> struct checked_compare_ab<false,true> {
   template<typename T, typename U> static inline int compare(T t, U u) {
      if (u < 0) {
	 return 1;
      }
      return checked_compare_ab<false,false>::compare(t, static_cast<typename std::make_unsigned<U>::type>(u));
   }
};
   
template<typename T, typename U> inline int checked_compare(T a, U b) {
   return checked_compare_ab<std::is_signed<T>::value, std::is_signed<U>::value>::compare(a,b);
}
   
template<typename T, typename U> inline T checked_cast(U value,
						       T t_min, T t_max)
{
    if (checked_compare(value,t_min) < 0) {
	std::stringstream ss;
	ss << "Value " << value << " was less than minimum " << t_min;
	throw checked_cast_exception(ss.str());
    } else if (checked_compare(value, t_max) > 0) {
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



}}

#endif
