#pragma once

#ifndef _pow_star_hpp
#define _pow_star_hpp

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

//
// A star at coordinates (x,y,z) with id represented in space of
// uint64_t, so {x,y,z} in [0..2^64-1]. These can be converted into
// normalized coordinates -0.5..0.5 ("length 1").
//

class star {
public:
    star() = default;
    star(uint32_t id, uint64_t x, uint64_t y, uint64_t z) : id_(id), x_(x), y_(y), z_(z) { }

    typedef uint32_t index_type;

    inline uint32_t id() const { return static_cast<uint32_t>(id_); }
    inline uint64_t x() const { return x_; }
    inline uint64_t y() const { return y_; }
    inline uint64_t z() const { return z_; }

    inline bool operator == (const star &other) const {
	return id_ == other.id_ &&
	       x_ == other.x_ &&
	       y_ == other.y_ &&
	       z_ == other.z_;
    }

private:
    uint64_t id_, x_, y_, z_;
};

inline std::ostream & operator << (std::ostream &out, const star &s) {
    return out << "star(" << s.id() << "," << s.x() << "," << s.y() << "," << s.z() << ")";
}

class projected_star {
public:
    projected_star() = default;

    projected_star(uint32_t id, uint32_t x, uint32_t y, uint32_t r) : id_(id), x_(x), y_(y), r_(r) {
    }

    inline void clear() { id_ = 0; x_ = 0; y_ = 0; r_ = 0; }

    inline uint32_t id() const { return id_; }
    inline uint32_t x() const { return x_; }
    inline uint32_t y() const { return y_; }
    inline uint32_t r() const { return r_; }

    typedef uint32_t id_type;

private:
    uint32_t id_;
    uint32_t x_, y_, r_;
};

inline std::ostream & operator << (std::ostream &out, const projected_star &s) {
    return out << "projected_star(" << s.id() << "," << s.x() << "," << s.y() << "," << s.r() << ")";
}

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

#endif
