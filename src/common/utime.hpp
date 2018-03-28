#ifndef _common_utime_hpp
#define _common_utime_hpp

#include <stdint.h>
#include <string>

namespace prologcoin { namespace common {

class utime_parse_exception : public std::runtime_error {
public:
    utime_parse_exception(const std::string &msg) :
	runtime_error(msg) { }
};

//
// Generic class of _our_ representation of time. We use milliseconds
// elapsed with Unix epoch.
//
class utime {
public:
    inline utime() : time_(0) { }
    inline utime(uint64_t t) : time_(t) { }

    template<uint64_t DT> class dt {
    public:
	inline dt(uint64_t v) : value_(v) { }
	inline operator uint64_t () const { return value_*DT; }
    protected:
	uint64_t value_;
    };

    struct dd : public dt<86400000000> { dd(uint64_t t) : dt(t) {} };
    struct hh : public dt<3600000000> { hh(uint64_t t) : dt(t) {} };
    struct mm : public dt<60000000> { mm(uint64_t t) : dt(t) {} };
    struct ss : public dt<1000000> { ss(uint64_t t) : dt(t) {} };
    struct ms : public dt<1000> { ms(uint64_t t) : dt(t) {} };
    struct us : public dt<1> { us(uint64_t t) : dt(t) {} };

    static utime now();
    static utime now_seconds();

    template<uint64_t C> inline static void sleep( dt<C> t )
    { utime u = now() + t; sleep_until(u); }

    static void sleep_until(const utime &u);

    inline bool operator == (const utime &other) const {
	return time_ == other.time_;
    }

    inline bool operator != (const utime &other) const {
	return ! operator == (other);
    }
    
    inline bool operator < (const utime &other) const {
	return time_ < other.time_;
    }

    inline bool operator <= (const utime &other) const {
	return time_ <= other.time_;
    }

    inline bool operator > (const utime &other) const {
	return time_ > other.time_;
    }

    inline bool operator >= (const utime &other) const {
	return time_ >= other.time_;
    }

    inline operator uint64_t () const {
	return time_;
    }

    inline utime operator ++ (int) {
	utime prev = *this;
	time_++;
	return prev;
    }

    inline utime & operator ++ () {
	time_++;
	return *this;
    }

    inline void set(const utime &t) {
	time_ = t.time_;
    }

    inline void operator += (uint64_t dt)
    { time_ += dt; }

    inline utime operator + (uint64_t dt) const
    { return utime(time_ + dt); }

    inline utime operator - (const utime &other) const
    { return utime(time_ - other.time_); }

    inline uint64_t in_us() const { return time_; } 
    inline uint64_t in_ms() const { return time_ / 1000; } 
    inline uint64_t in_ss() const { return time_ / 1000000; }
    inline uint64_t in_mm() const { return time_ / 60000000; }
    inline uint64_t in_hh() const { return time_ / 3600000000; }
    inline uint64_t in_dd() const { return time_ / 86400000000; }

    static utime from_string(const std::string &str);

    bool parse(const std::string &str);
    std::string str() const;

private:
    uint64_t time_;
};

}}

#endif
