#pragma once

#ifndef _pow_fxp_hpp
#define _pow_fxp_hpp

#include <math.h>
#include "conv.hpp"

// #define USE_PROLOGCOIN_FXP_ASSERT

#ifdef USE_PROLOGCOIN_FXP_ASSERT
#define PROLOGCOIN_FXP_ASSERT(x) { fxp_assert(x); }
#else
#define PROLOGCOIN_FXP_ASSERT(x)
#endif

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

#ifdef USE_PROLOGCOIN_FXP_ASSERT
static void fxp_assert(bool x) {
    static int cnt = 0;
    cnt++;
    assert(x);
}
#endif

inline int msb(uint64_t v) {
    // Return most signifcant bit.
    int r = 0;
    if (v) {
	if (0xFFFFFFFF00000000 & v) { v>>=(1<<5); r |= (1<<5); }
	if (0x00000000FFFF0000 & v) { v>>=(1<<4); r |= (1<<4); }
	if (0x000000000000FF00 & v) { v>>=(1<<3); r |= (1<<3); }
	if (0x00000000000000F0 & v) { v>>=(1<<2); r |= (1<<2); }
	if (0x000000000000000C & v) { v>>=(1<<1); r |= (1<<1); }
	if (0x0000000000000002 & v) { r |= (1<<0); }
    } else {
	r = -1;
    }
    return r;
}

inline int pos_msb(uint64_t v) {
    if (v >> 63) {
	return msb(~v + 1);
    } else {
	return msb(v);
    }
}

// This is a fixed-point representation of numbers with 16-bit integer and
// 48-bit fraction. This should be suitable for the type of computations we
// perform for the Big Dipper Proof-of-work algorithm.
//
// The trigonometic functions (asin/sin/...) are not really used in the Big Dipper
// in its purest form, but they are convenient to interactively explore the space
// that can be done via the PoW server.
//
class fxp1648 {
public:
    // Convert uint64_t raw value 0..2^64-1 to -0.5..0.5 in fixed point
    // representation.
    inline fxp1648() : data_(0) { }
    inline fxp1648(const fxp1648 &other) = default;

    inline fxp1648(bool, uint64_t val) : data_(val) { }

    inline static fxp1648 raw(uint64_t val) { return fxp1648(true, val); }

    // inline explicit constexpr fxp1648(const uint64_t i) : data_(i) { }

    inline explicit fxp1648(const double v) {
        static const double p2_48 = static_cast<double>(static_cast<uint64_t>(1) << 48);
	bool is_neg = v < 0 ? true : false;
	double v0 = is_neg ? -v : v;
	auto int_part = static_cast<int16_t>(v0);
	auto frac_part = static_cast<uint64_t>((v0 - int_part) * p2_48) & ((static_cast<uint64_t>(1) << 48)-1);
	data_ = (static_cast<uint64_t>(int_part) << 48) | frac_part;
	if (is_neg) {
	    data_ = ~data_ + 1;
	}
    }

    inline explicit fxp1648(size_t v) : data_(v << 48) { }

    inline fxp1648(int i) {
        if (i >= 0) {
            data_ = static_cast<uint64_t>(i) << 48;
	} else {
	    data_ = static_cast<uint64_t>(-i) << 48;
	    data_ = ~data_ + 1;
	}
    }

    inline void write(uint8_t bytes[8]) const {
	bytes[0] = (data_ >> 56) & 0xff;
	bytes[1] = (data_ >> 48) & 0xff;
	bytes[2] = (data_ >> 40) & 0xff;
	bytes[3] = (data_ >> 32) & 0xff;
	bytes[4] = (data_ >> 24) & 0xff;
	bytes[5] = (data_ >> 16) & 0xff;
	bytes[6] = (data_ >>  8) & 0xff;
	bytes[7] = (data_ >>  0) & 0xff;
    }

    inline void read(const uint8_t bytes[8]) {
	data_ = (static_cast<uint64_t>(bytes[0]) << 56) |
	        (static_cast<uint64_t>(bytes[1]) << 48) |
	        (static_cast<uint64_t>(bytes[2]) << 40) |
	        (static_cast<uint64_t>(bytes[3]) << 32) |
	        (static_cast<uint64_t>(bytes[4]) << 24) |
	        (static_cast<uint64_t>(bytes[5]) << 16) |
	        (static_cast<uint64_t>(bytes[6]) << 8) |
	        (static_cast<uint64_t>(bytes[7]) << 0);
    }

    inline void operator = (const double x) {
	data_ = fxp1648(x).raw_value();
    }

    inline uint64_t raw_value() const {
	return data_;
    }

    inline int32_t to_int() const {
	return static_cast<int32_t>(static_cast<int64_t>(data_) >> 48);
    }

    inline bool is_within_range(int32_t v) const {
	return (v >= -32768) && (v <= 32767);
    }

    inline bool overflow_check(int32_t v) const {
	return is_within_range(v);
    }

    static inline fxp1648 eps(bool positive) {
	if (positive) {
	    return fxp1648(true, 1);
	} else {
	    return fxp1648(true, static_cast<uint64_t>(-1));
	}
    }

    static inline fxp1648 min() {
	return fxp1648(true, static_cast<uint64_t>(0x8000000000000000));
    }

    static inline fxp1648 max() {
	return fxp1648(true, static_cast<uint64_t>(0x7fffffffffffffff));
    }

    static inline fxp1648 max_or_min(bool positive) {
	return positive ? max() : min();
    }

    inline bool is_min() const {
	return (*this) == min();
    }

    inline bool is_max() const {
	return (*this) == max();
    }

    inline double to_double() const {
	static const double p2_48 = static_cast<double>(static_cast<uint64_t>(1) << 48);
	uint64_t s = data_;
	int16_t int_part = static_cast<int16_t>(s >> 48);
	uint64_t frac_part = static_cast<uint64_t>(s & ((static_cast<uint64_t>(1) << 48)-1));
        double d = static_cast<double>(int_part) + static_cast<double>(frac_part / p2_48);
        return d;
    }

    inline explicit operator double () const {
	return to_double();
    }

    inline explicit operator uint32_t () const {
	return static_cast<uint32_t>(data_ >> 48);
    }

    inline bool is_negative() const {
	return (data_ >> 63) != 0;
    }

    inline bool is_zero() const {
	return data_ == 0;
    }

    inline fxp1648 half() const {
	return (*this) >> 1;
    }

    inline fxp1648 mul3() const {
	static const fxp1648 limit_max(true,0x2aaaaaaaaaaaaaaa);
	static const fxp1648 limit_min(true,0xd555555555555556);
	PROLOGCOIN_FXP_ASSERT(overflow_check(to_int()*3));
	if ((*this) > limit_max) {
	    return fxp1648::max();
	} else if ((*this) < limit_min) {
	    return fxp1648::min();
	} else {
	    return fxp1648::raw((data_ << 1) + data_);
	}
    }

    inline fxp1648 shift_right_no_saturation(size_t n) const {
	if (n > 63) {
	    return is_negative() ? fxp1648::raw(static_cast<uint64_t>(-1))
		                 : fxp1648::raw(0);
	}
        return fxp1648::raw(static_cast<uint64_t>(static_cast<int64_t>(data_) >> n));
    }

    inline fxp1648 operator >> (size_t n) const {
	if (data_ == 0) {
	    return *this;
	}
	auto v = shift_right_no_saturation(n);
	if (v.raw_value() == 0 || v.raw_value() == static_cast<uint64_t>(-1)) {
	    // Underflow, saturate on positive. Return smallest representable
	    // number.
	    return fxp1648::eps(!is_negative());
	} else {
	    return v;
	}
    }

    inline void operator >>= (size_t n) {
	data_ = ((*this) >> n).raw_value();
    }

    inline fxp1648 operator << (size_t n) const {
	PROLOGCOIN_FXP_ASSERT(overflow_check(to_int()<<1));

	if (is_negative()) {
	    if (is_min() || static_cast<size_t>(62) - msb(~data_ + 1) < n) {
		return fxp1648::min();
	    }
	} else {
	    if (static_cast<size_t>(62) - msb(data_) < n) {
		return fxp1648::max();
	    }
	}
	return fxp1648::raw(data_ << n);
    }

    inline void operator <<= (size_t n) {
	PROLOGCOIN_FXP_ASSERT(overflow_check(to_int()<<1));
	data_ = ((*this) << n).raw_value();
    }

    inline fxp1648 operator + (const fxp1648 other) const {
	PROLOGCOIN_FXP_ASSERT(overflow_check(to_int() + other.to_int()));
	fxp1648 r(true, data_ + other.data_);

	// Overflow cannot happen if only one is negative.
	if (is_negative() == other.is_negative()) {
	    // We changed signs; overflow detected.
	    if (r.is_negative() != is_negative()) {
		return fxp1648::max_or_min(!is_negative());
	    }
	}
	return r;
    }

    inline fxp1648 operator - (const fxp1648 other) const {
	PROLOGCOIN_FXP_ASSERT(overflow_check(to_int() - other.to_int()));
        fxp1648 r(true, data_ - other.data_);

	// Overflow cannot happen if both have the same sign.
	if (is_negative() != other.is_negative()) {
	    // We changed signs; overflow detected.
	    if (r.is_negative() != is_negative()) {
		return fxp1648::max_or_min(!is_negative());
	    }
	}
	return r;
    }
    
    fxp1648 operator * (const int x) const {
	return *this * fxp1648(x);
    }

    inline fxp1648 operator * (const fxp1648 other) const {
	PROLOGCOIN_FXP_ASSERT(overflow_check(to_int() * other.to_int()));

	if (is_zero() || other.is_zero()) {
	    return fxp1648(0);
	}

	bool x_neg = is_negative();
	bool y_neg = other.is_negative();
	uint64_t x = x_neg ? ~data_ + 1 : data_;
	uint64_t y = y_neg ? ~other.data_ + 1 : other.data_;
	bool r_neg = x_neg != y_neg;

	// We multiply 4 32-bit integers.
	// We keep the MSB part (so the least number of bits are)
	uint32_t ah = static_cast<uint32_t>(x >> 32);
	uint32_t al = static_cast<uint32_t>(x);
	uint32_t bh = static_cast<uint32_t>(y >> 32);
	uint32_t bl = static_cast<uint32_t>(y);

	uint64_t p0 = static_cast<uint64_t>(al) * static_cast<uint64_t>(bl);
	uint64_t p1 = static_cast<uint64_t>(al) * static_cast<uint64_t>(bh);
	uint64_t p2 = static_cast<uint64_t>(ah) * static_cast<uint64_t>(bl);
	uint64_t p3 = static_cast<uint64_t>(ah) * static_cast<uint64_t>(bh);

	uint32_t carry = ((p0 >> 32) + static_cast<uint32_t>(p1) + static_cast<uint32_t>(p2)) >> 32;
	uint64_t lo = p0 + (p1 << 32) + (p2 << 32);
	uint64_t hi = p3 + (p2 >> 32) + (p1 >> 32) + carry;

        uint64_t p = (hi << 16) | (lo >> 48);

	if (p >> 63) {
	    // Highest bit set? Then it's an overflow
	    return fxp1648::max_or_min(!r_neg);
	}

	if (p == 0) {
	    return fxp1648::eps(!r_neg);
	}

	if (r_neg) {
	    p = ~p + 1;
	}

	return fxp1648(true, p);
    }


    //
    // Compute the reciprocal (1/x) of number.
    // We use the Newton-Raphson method. First we need to scale number
    // between 0.5 and 1.
    //
    // dp = d * scale2  will become a bit shift positive or negative
    //                  0.5 < dp <= 1
    //
    // Then x := 48/17 − 32/17 * dp
    // do 4 times: x := x + x * (1 - dp * x)
    // rescale x back (inverse of scale2)
    // 
    inline fxp1648 reciprocal() const {
	static const fxp1648 c_48_17(true, static_cast<uint64_t>(0x2d2d2d2d2d2d2));
	static const fxp1648 c_32_17(true, static_cast<uint64_t>(0x1e1e1e1e1e1e1));
	static const fxp1648 c_1(true, static_cast<uint64_t>(0x1000000000000));

	bool is_neg = is_negative();
	uint64_t d = is_neg ? ~data_ + 1 : data_;
	int scale2 = 47 - msb(d);

	fxp1648 dp(true, (scale2 < 0) ? d >> -scale2 : d << scale2);
	fxp1648 x = c_48_17 - c_32_17 * dp;
	x = x + x * (c_1 - dp * x);
	x = x + x * (c_1 - dp * x);
	x = x + x * (c_1 - dp * x);
	x = x + x * (c_1 - dp * x);
	x = x + x * (c_1 - dp * x);
	x = x + x * (c_1 - dp * x);

	if (scale2 > 0) {
	    PROLOGCOIN_FXP_ASSERT((msb(x.data_) + scale2) < 63);

	    // Overflow?
	    if (msb(x.data_) + scale2 >= 63) {
		return fxp1648::max_or_min(!is_neg);
	    }
	}

	uint64_t r = (scale2 < 0) ? x.data_ >> -scale2 : x.data_ << scale2;

	if (r == 0) {
	    // Underflow. Saturate to EPS.
	    return fxp1648::eps(!is_neg);
	}

	if (is_neg) {
	    r = ~r + 1;
	}
	return fxp1648(true, r);
    }

    //
    // inv_sqrt(x) : compute 1 / sqrt(x)
    //
    // (some use the trick using a first approximation via
    //  floating point and the "magic" constant 0x5f3759df, but
    //  we strictly want to avoid any use of floats.)
    //
    // So this is instead plain Newton-Raphson:
    // yn+1 = yn * (3/2 - 1/2 * yn^2)
    //
    // First we scale the value to be between 0.5 < y <= 1 for
    // faster iteration and to avoid overflows.
    //
    // sqrt(x*y) = sqrt(x)*sqrt(y), so the root of the scaling factor
    // can be computed separately.
    inline fxp1648 inv_sqrt() const {

	static const uint64_t INV_SQRT_2N[16+48] = {0xb504f333f9de60, 0x80000000000000, 0x5a827999fcef30, 0x40000000000000, 0x2d413cccfe7798, 0x20000000000000, 0x16a09e667f3bcc, 0x10000000000000, 0xb504f333f9de6, 0x8000000000000, 0x5a827999fcef3, 0x4000000000000, 0x2d413cccfe779, 0x2000000000000, 0x16a09e667f3bc, 0x1000000000000, 0xb504f333f9de, 0x800000000000, 0x5a827999fcef, 0x400000000000, 0x2d413cccfe77, 0x200000000000, 0x16a09e667f3b, 0x100000000000, 0xb504f333f9d, 0x80000000000, 0x5a827999fce, 0x40000000000, 0x2d413cccfe7, 0x20000000000, 0x16a09e667f3, 0x10000000000, 0xb504f333f9, 0x8000000000, 0x5a827999fc, 0x4000000000, 0x2d413cccfe, 0x2000000000, 0x16a09e667f, 0x1000000000, 0xb504f333f, 0x800000000, 0x5a827999f, 0x400000000, 0x2d413cccf, 0x200000000, 0x16a09e667, 0x100000000, 0xb504f333, 0x80000000, 0x5a827999, 0x40000000, 0x2d413ccc, 0x20000000, 0x16a09e66, 0x10000000, 0xb504f33, 0x8000000, 0x5a82799, 0x4000000, 0x2d413cc, 0x2000000, 0x16a09e6, 0x1000000};

	static const uint64_t RESCALE[11] = { 0x0, 0xe8ba2e8ba2e8, 0xd55555555555, 0xc4ec4ec4ec4e, 0xb6db6db6db6d, 0xaaaaaaaaaaaa, 0xa00000000000, 0x969696969696, 0x8e38e38e38e3, 0x86bca1af286b, 0x800000000000 };

	static const uint64_t RESCALE_INV_SQRT[11] = { 0x0, 0xf4161fcec4f0, 0xe9b1e8c246e5, 0xe086dfd59e44, 0xd85c077c225d, 0xd105eb806161, 0xca62c1d6d2da, 0xc457d14846c6, 0xbecfa67baa31, 0xb9b8cfc03f49, 0xb504f333f9de };

	static const fxp1648 c_1(true, static_cast<uint64_t>(0x1000000000000));
	static const fxp1648 c_3_2(true, static_cast<uint64_t>(0x1800000000000));
	static const fxp1648 c_10(true, static_cast<uint64_t>(0xa000000000000));
	static const fxp1648 limit(true, static_cast<uint64_t>(0x40000));

	if (is_negative()) {
	    return fxp1648(0);
	}

	if (is_zero()) {
	    return fxp1648(0);
	}

	if ((*this) < limit) {
	    return fxp1648::max();
	}

	fxp1648 rescale = c_1;
	fxp1648 y = *this;
	uint64_t y_d = y.raw_value();
	int scale2 = 48 - msb(y_d);

	fxp1648 y_s(true, (scale2 < 0) ? y_d >> -scale2 : y_d << scale2);

	// The best result (= least error) is if 1/sqrt(x) is when x is within
	// the range 1..1.10. So let's use a rescaling table to make y_s within
	// this range (and rescale back.)

	int tr = ((y_s - c_1) * c_10).to_int();

	if (tr > 0) {
	    // auto c = (1/10.0)*tr + 1.0;
	    // rescale = fxp1648(1.0/::sqrt(c));
	    // y_s = y_s * fxp1648(1.0/c);
 	    rescale = fxp1648(true, RESCALE_INV_SQRT[tr]);
	    y_s = y_s * fxp1648(true, RESCALE[tr]);
	}

	fxp1648 x2 = y_s.half();

	y_s = y_s * (c_3_2 - (x2 * y_s * y_s));
	y_s = y_s * (c_3_2 - (x2 * y_s * y_s));
	y_s = y_s * (c_3_2 - (x2 * y_s * y_s));
	y_s = y_s * (c_3_2 - (x2 * y_s * y_s));
	y_s = y_s * (c_3_2 - (x2 * y_s * y_s));
	y_s = y_s * (c_3_2 - (x2 * y_s * y_s));
	// y_s = y_s * (c_3_2 - (x2 * y_s * y_s));
	// y_s = y_s * (c_3_2 - (x2 * y_s * y_s));
	// y_s = y_s * (c_3_2 - (x2 * y_s * y_s));
	// std::cout << "y_s " << y_s << std::endl;

	// Scale back (scale2 in -15 .. +47)

	y = y_s;
	while (scale2 > 15) {
	    y = y * fxp1648::raw(INV_SQRT_2N[0]);
	    scale2 -= 15;
	}
	if (scale2 != 0) {
	    y = y * fxp1648::raw(INV_SQRT_2N[-scale2+15]);
	}

	if (rescale != c_1) {
	    y = y * rescale;
	}

	if (y.is_zero()) {
	    y = fxp1648::eps(true);
	}

	return y;
    }

    inline fxp1648 sqrt() const {
	assert(!is_negative());
	if (is_zero()) {
	    return *this;
	}
	return inv_sqrt().reciprocal();
    }

    inline fxp1648 operator / (const fxp1648 other) const {
	if (is_zero()) {
	    // In this case 0/0 becomes 0
	    return *this;
	}
	if (other.is_zero()) {
	    // Saturate to max/min to mimic +inf/-inf.
	    return fxp1648::max_or_min(!is_negative());
	}
	// To avoid overflow errors, we'll find the best match between
	// divisor and dividend.
	auto a = *this;
	auto b = other;
	int mbit = pos_msb(b.raw_value());
	int to_origin = 48 - mbit;

	if (to_origin < 0) {
	    a >>= -to_origin;
	    b >>= -to_origin;
	} else {
	    a <<= to_origin;
	    b <<= to_origin;
	}

	return a * b.reciprocal();
    }

    fxp1648 operator / (const int x) const {
	return *this / fxp1648(x);
    }

    fxp1648 operator / (const size_t x) const {
	return *this / fxp1648(x);
    }

    inline fxp1648 operator -() const {
	return fxp1648(true, ~data_ + 1);
    }

    inline void  operator /= (const fxp1648 other) {
	if (is_zero()) {
	    return;
	}
	auto a = *this;
	auto b = other;
	int mbit = pos_msb(b.raw_value());
	int to_origin = 48 - mbit;
	if (to_origin < 0) {
	    a >>= -to_origin;
	    b >>= -to_origin;
	} else {
	    a <<= to_origin;
	    b <<= to_origin;
	}
	data_ = (a * b.reciprocal()).raw_value();
    }

    inline void operator += (const fxp1648 other) {
	data_ = (*this + other).raw_value();
    }

    inline bool operator == (const fxp1648 other) const {
	return data_ == other.data_;
    }
    inline bool operator != (const fxp1648 other) const {
	return data_ != other.data_;
    }
    inline bool operator < (const fxp1648 other) const {
	return ((data_ - other.data_) >> 63) != 0;
    }
    inline bool operator >= (const fxp1648 other) const {
	return ((data_ - other.data_) >> 63) == 0;
    }
    inline bool operator <= (const fxp1648 other) const {
	return operator < (other) || data_ == other.data_;
    }
    inline bool operator > (const fxp1648 other) const {
	return operator >= (other) && data_ != other.data_;
    }

    friend std::ostream & operator << (std::ostream &out, const fxp1648 v);

    friend fxp1648 sqrt(const fxp1648 v);

private:
    uint64_t data_;
};

inline std::ostream & operator << (std::ostream &out, const fxp1648 v)
{
    out << v.to_double();
    return out;
}

// Support functional form to comply with the use of doubles
inline fxp1648 sqrt(const fxp1648 v) {
    return v.sqrt();
}

inline fxp1648 inv_sqrt(const fxp1648 v) {
    return v.inv_sqrt();
}

// We cheat for the trigonometric functions; they are never used in
// practice, only for debugging (or the interactive mode.) So it's
// ok to use floating point as an interim. The other approach would
// be to implement the cordic algorithms.

inline fxp1648 acos(const fxp1648 v) {
    return fxp1648(::acos(v.to_double()));
}

inline fxp1648 cos(const fxp1648 v) {
    return fxp1648(::cos(v.to_double()));
}

inline fxp1648 sin(const fxp1648 v) {
    return fxp1648(::sin(v.to_double()));
}

inline fxp1648 atan2(const fxp1648 a, const fxp1648 b) {
    return fxp1648(::atan2(a.to_double(), b.to_double()));
}

template<> inline fxp1648 uint64_to_T<fxp1648>(const uint64_t a) {
    static const fxp1648 c_1_2(true, static_cast<uint64_t>(0x800000000000));
    return fxp1648(true, a >> 16) - c_1_2;
}

template<> inline uint64_t T_to_uint64<fxp1648>(const fxp1648 a) {
    static const fxp1648 c_1_2(true, static_cast<uint64_t>(0x800000000000));
    return (a + c_1_2).raw_value() << 16;
}

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

#endif

