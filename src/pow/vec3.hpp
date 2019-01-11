#pragma once

#ifndef _pow_vec3_hpp
#define _pow_vec3_hpp

#include <math.h>
#include <iostream>

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

inline double sqrt(const double v) {
    return ::sqrt(v);
}

inline double inv_sqrt(const double v) {
    return 1.0/::sqrt(v);
}

inline double atan2(const double a, const double b) {
    return ::atan2(a,b);
}

inline double acos(const double a) {
    return ::acos(a);
}

inline double sin(const double x) {
    return ::sin(x);
}

inline double cos(const double x) {
    return ::cos(x);
}
    
template<typename T> class vec3 {
public:
    struct SPHERICAL { };

    typedef T value_t;

    inline vec3() : x_(0), y_(0), z_(0) { }
    inline vec3(const vec3<T> &other) {
	x_ = other.x_;
	y_ = other.y_;
	z_ = other.z_;
    }

    inline vec3(T x, T y, T z) : x_(x), y_(y), z_(z) { }

    inline vec3(SPHERICAL, T radius, T theta, T phi) {
	x_ = radius * sin(theta) * cos(phi);
        y_ = radius * sin(theta) * sin(phi);
	z_ = radius * cos(theta);
    }

    inline void operator = (const vec3<T> &other) {
	x_ = other.x_;
	y_ = other.y_;
	z_ = other.z_;
    }

    inline T x() const {
	return x_;
    }

    inline T y() const {
	return y_;
    }

    inline T z() const {
	return z_;
    }

    inline T r() const {
	return length();
    }

    inline T theta() const {
	return acos(z_ / r());
    }

    inline T phi() const {
	return atan2(y_, x_);
    }

    inline T length() const {
	return sqrt(x_*x_ + y_*y_ + z_*z_);
    }

    inline T inv_length() const {
	return inv_sqrt(x_*x_ + y_*y_ + z_*z_);
    }

    inline vec3<T> operator + (const vec3<T> &other) const {
	return vec3(x_ + other.x_, y_ + other.y_, z_ + other.z_);
    }

    inline vec3<T> operator + (T scalar) const {
	return vec3(x_ + scalar, y_ + scalar, z_ + scalar);
    }

    inline vec3<T> operator - (const vec3<T> &other) const {
	return vec3(x_ - other.x_, y_ - other.y_, z_ - other.z_);
    }
    
    inline vec3<T> operator * (T scalar) const {
	return vec3(x_ * scalar, y_ * scalar, z_ * scalar);
    }

    inline vec3<T> operator / (T scalar) const {
	return vec3(x_ / scalar, y_ / scalar, z_ / scalar);
    }

    inline vec3<T> norm() const {
	return *this * inv_length();
    }

    inline vec3<T> cross(const vec3<T> &other) const {
	return vec3(y_*other.z_ - z_*other.y_,
		    z_*other.x_ - x_*other.z_,
		    x_*other.y_ - y_*other.x_);
    }

    inline T dot(const vec3<T> &other) const {
	return x_*other.x_ + y_*other.y_ + z_*other.z_;
    }
    
    inline void add(const vec3<T> &other) {
	x_ += other.x_;
	y_ += other.y_;
	z_ += other.z_;
    }
    
    friend inline std::ostream & operator << (std::ostream &out, const vec3<T> &v) {
	out << "(" << v.x() << "," << v.y() << "," << v.z() << ")";
	return out;
    }

private:
    T x_, y_, z_;
};

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

#endif
