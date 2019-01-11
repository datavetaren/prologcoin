#pragma once

#ifndef _pow_conv_hpp
#define _pow_conv_hpp

#include <limits>

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

// Value between 0..2^63-1 is mapped to -0.5..+0.5
template<typename T> inline T uint64_to_T(uint64_t a);

template<> inline double uint64_to_T<double>(uint64_t a) {
    return static_cast<double>(a) / std::numeric_limits<uint64_t>::max() - 0.5;
}

template<> inline float uint64_to_T<float>(uint64_t a) {
    return static_cast<float>(a) / std::numeric_limits<uint64_t>::max() - 0.5;
}

// Value between -0.5..+0.5 is mapped to 0..2^63-1
template<typename T> inline uint64_t T_to_uint64(T a) {
    return static_cast<uint64_t>((a+T(1)/2)*(std::numeric_limits<uint64_t>::max()));
}

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

#endif
