#pragma once

#ifndef _pow_flt_hpp
#define _pow_flt_hpp

#include <math.h>
#include <iomanip>
#include <string.h>
#include "fxp.hpp"

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

// Custom floating point based. We don't support too many operations and
// we reuse the fixed-point implementation under the hood. We need the
// custom floating point to represent numbers between 0..1 with greater
// precision. Remember that PoW target is computed from (1-difficulty)
// multiplied with 2^256-1, so fxp1648 will only let you get 2^208-1 at
// minimum as fxp1648 smallest number is 2^-48.
class flt1648 {
public:
    inline flt1648() : exponent_(0), mantissa_(0) { }
    inline flt1648(const flt1648 &other) : exponent_(other.exponent_),
					   mantissa_(other.mantissa_) { }
    inline flt1648(int32_t value) {
	mantissa_ = fxp1648::raw(static_cast<int64_t>(value));
	exponent_ = 48;
	normalize();
    }

    inline flt1648(int16_t exponent, fxp1648 mantissa)
        : exponent_(exponent), mantissa_(mantissa) { normalize(); }

    inline flt1648 operator + (const flt1648 &other) const {
	// Check distance between exponents
	int32_t diff_exp = exponent() - other.exponent();
	if (diff_exp >= 0) {
	    return flt1648(exponent(),
			   mantissa() + (other.mantissa().shift_right_no_saturation(diff_exp)));
			   
	} else {
	    return flt1648(other.exponent(),
			   (mantissa().shift_right_no_saturation(-diff_exp)) + other.mantissa());
			   
	}
    }

    inline void operator += (const flt1648 &other) {
	*this = *this + other;
    }

    static flt1648 from(int32_t int_part, uint32_t fraction_part) {
	flt1648 f(int_part);
	fxp1648 p;
	while (fraction_part != 0) {
	    p += (fraction_part % 10);
	    p /= 10;
	    fraction_part /= 10;
	}
	return f + flt1648(0,p);
    }

    inline bool operator == (const flt1648 &other) const {
	return mantissa() == other.mantissa() &&
   	       exponent() == other.exponent();
    }

    inline flt1648 operator - (const flt1648 &other) const {
	return (*this) + flt1648(other.exponent(), -other.mantissa());
    }

    inline flt1648 operator * (const flt1648 &other) const {
	return flt1648(exponent() + other.exponent(),
		       mantissa() * other.mantissa());
    }

    inline flt1648 reciprocal() const {
	return flt1648(-exponent(), mantissa().reciprocal());
    }

    inline flt1648 operator / (const flt1648 &other) const {
	return flt1648(exponent() - other.exponent(),
		       mantissa() / other.mantissa());
    }

    inline flt1648 operator << (size_t n) const {
	return flt1648(exponent() + n, mantissa());
    }

    inline flt1648 operator >> (size_t n) const {
	return flt1648(exponent() - n, mantissa());
    }

    inline int16_t exponent() const {
	return exponent_;
    }

    inline fxp1648 mantissa() const {
	return mantissa_;
    }

    // TODO: Fix for negative values
    inline void to_integer(uint8_t *bytes, size_t bytes_len) {
	assert(exponent() >= 0);
	auto bits = mantissa().raw_value();
	int exp_max = exponent();
	auto offset_end = bytes_len*8;
        assert(static_cast<size_t>(exp_max) <= offset_end);
	auto offset = exp_max;
	auto bits_offset = offset % 8;
	auto byte_offset = offset / 8;
	memset(bytes, 0, bytes_len);
	uint8_t bits_mask = ~static_cast<uint8_t>((1 << (8 - bits_offset)) - 1);
	uint8_t bits_out = static_cast<uint8_t>(((bits >> 40) & bits_mask) >> (8 - bits_offset));
	bytes[bytes_len - byte_offset - 1] |= bits_out;
	bits <<= bits_offset;
	byte_offset--;
	while (bits != 0) {
	    bytes[bytes_len - byte_offset - 1] = static_cast<uint8_t>((bits >> 40) & 0xff);
	    byte_offset--;
	    bits <<= 8;
	}
    }

    inline std::string to_integer_string(size_t bytes_len) {
	uint8_t *bytes = new uint8_t[bytes_len];
	to_integer(bytes, bytes_len);
	std::stringstream ss;
	for (size_t i = 0; i < bytes_len; i++) {
	    ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(bytes[i]);
	}
	delete [] bytes;
	return ss.str();
    }

    inline void write(uint8_t bytes[8]) const {
	mantissa().write(bytes);
	bytes[0] = static_cast<uint8_t>((exponent_ >> 8) & 0xff);
	bytes[1] = static_cast<uint8_t>(exponent_ & 0xff);
    }

    inline void read(uint8_t bytes[8]) {
	uint8_t fraction[8];
	memcpy(fraction, bytes, 8);
	fraction[0] = 0;
	fraction[1] = 0;
	mantissa().read(fraction);
	exponent_ = (static_cast<int16_t>(bytes[0]) << 8) |
	             static_cast<int16_t>(bytes[1]);
    }

    inline double to_double() const {
	return ldexp(mantissa().to_double(), exponent());
    }

    friend std::ostream & operator << (std::ostream &out, const flt1648 &v);

private:
    inline void normalize() {
	// If the mantissa is >= 1 or < 0.5  we need to shift the exponent.
	int e = pos_msb(mantissa_.raw_value()) - 47;
	if (e < 0) {
	    mantissa_ <<= -e;
	    exponent_ += e;
	} else {
	    mantissa_ >>= e;
	    exponent_ += e;
	}
    }

    int16_t exponent_;
    fxp1648 mantissa_;
};    

inline std::ostream & operator << (std::ostream &out, const flt1648 &v)
{
    out << v.to_double();
    return out;
}

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

#endif

