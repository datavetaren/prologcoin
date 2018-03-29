#include "utime.hpp"
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <exception>

namespace prologcoin { namespace common {

static const boost::posix_time::ptime EPOCH = boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1));

// Avoid the BOOST posix_time overflow bug by using seconds and
// trimming the end result with microseconds.
// This way we can use older versions of BOOST (tested all this at 1.49)
// This is still not year 2038 safe, but we expect a BOOST upgrade before that.
static uint64_t to_microseconds(boost::posix_time::time_duration dt)
{
   uint64_t sec = static_cast<uint64_t>(dt.total_seconds()) * 1000000;
   uint64_t usec = static_cast<uint64_t>(dt.total_microseconds()) % 1000000;
   return sec + usec;
}

// Avoid the BOOST posix_time overflow bug by using seconds and
// trimming the end result with microseconds.
// This way we can use older versions of BOOST (tested all this at 1.49)
// This is still not year 2038 safe, but we expect a BOOST upgrade before that.
static boost::posix_time::ptime from_microseconds(uint64_t dt)
{
   boost::posix_time::ptime t = EPOCH + boost::posix_time::seconds(dt/1000000);
   t += boost::posix_time::microseconds(dt % 1000000);
   return t;
}
     
utime utime::now()
{
    boost::posix_time::ptime t = boost::posix_time::microsec_clock::universal_time();
    return utime((t - EPOCH).total_microseconds());
}

utime utime::now_seconds()
{
    utime t = utime::now();
    t.set(t.in_us() - t.in_us() % 1000000);
    return t;
}

std::string utime::str() const
{
    auto t = from_microseconds(time_);
    std::string iso = boost::posix_time::to_iso_string(t);
    auto s = iso.substr(0,4) + "." + iso.substr(4,2) + "." + iso.substr(6,2) + "T" + iso.substr(9,2) + ":" + iso.substr(11,2) + ":" + iso.substr(13,2);
    auto r = iso.substr(15);
    if (!r.empty()) {
	s = s + r;
    }
    return s;
}

bool utime::parse(const std::string &str)
{
    std::string s = str;
    if (s.size() >= 19 && s[4] == '.' && s[7] == '.' && s[10] == 'T' &&
	s[13] == ':' && s[16] == ':') {

	bool has_micro = s.size() > 19 && s[19] == '.';

	// Rewrite to ISO standard
	s = s.substr(0,4) + s.substr(5,2) + s.substr(8,2) + 'T' +
	    s.substr(11,2) + s.substr(14,2) + s.substr(17,2) +
	    (has_micro ? s.substr(19) : "");
    }
    try {
	auto t = boost::posix_time::from_iso_string(s);
        auto dt = t - EPOCH;
        time_ = to_microseconds(dt);
	return true;
    } catch (std::exception &ex) {
	return false;
    }
}

void utime::sleep_until(const utime &ut)
{
    uint64_t t0 = now().in_us();
    uint64_t t1 = ut.in_us();
    if (t1 <= t0) {
	return;
    }
    uint64_t delta_us = t1 - t0;
    boost::this_thread::sleep(boost::posix_time::microseconds(delta_us));
}

utime utime::from_string(const std::string &str)
{
    utime u;
    if (u.parse(str)) {
	throw utime_parse_exception( "Couldn't parse utime " + str);
    }
    return u;
}

}}
