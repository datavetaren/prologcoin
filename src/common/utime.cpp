#include "utime.hpp"
#include <boost/chrono.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <exception>

namespace prologcoin { namespace common {

static const boost::posix_time::ptime EPOCH = boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1));

utime utime::now()
{
    boost::posix_time::ptime t = boost::posix_time::microsec_clock::local_time();
    return utime((t - EPOCH).total_microseconds());
}

std::string utime::str() const
{
    auto t = EPOCH + boost::posix_time::microseconds(time_);
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
	time_ = (t - EPOCH).total_microseconds();
	return true;
    } catch (std::exception &ex) {
	return false;
    }
}

}}
