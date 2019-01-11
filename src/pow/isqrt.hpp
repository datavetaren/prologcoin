#pragma once

#ifndef _pow_isqrt_hpp
#define _pow_isqrt_hpp

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

template<typename T> static inline T isqrt(T val) {
    T temp, g=0, b = 0x8000, bshft = 15;
    do {
	if (val >= (temp = (((g << 1) + b)<<bshft--))) {
	    g += b;
	    val -= temp;
	}
    } while (b >>= 1);
    return g;
}

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

#endif
