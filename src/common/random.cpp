#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/random_device.hpp>
#include "random.hpp"

namespace prologcoin { namespace common {

static boost::random::random_device random_;

std::string random::next(size_t entropy_bits)
{
    std::string chars("abcdefghijklmnopqrstuvwxyz"
		      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		      "1234567890");
    boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
    size_t n = (entropy_bits * 1000 + 5953) / 5954;
    std::string s(n, ' ');
    for (size_t i = 0; i < n; ++i) {
	s[i] = chars[index_dist(random_)];
    }
    return s;
}

int random::next_int(int max)
{
    return static_cast<int>(static_cast<unsigned int>(random_()) % max);
}

}}
