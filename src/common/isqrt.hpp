#ifndef _common_isqrt_hpp
#define _common_isqrt_hpp

namespace prologcoin { namespace common {

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

}}

#endif
