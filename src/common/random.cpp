#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/random_device.hpp>
#include <limits>
#include "random.hpp"
#include "fast_hash.hpp"

namespace prologcoin { namespace common {

bool random::for_testing_ = false;
static fast_hash testing_rnd_;

static boost::random::random_device random_;

void random::set_for_testing(bool for_testing)
{
    for_testing_ = for_testing;
}

class for_testing_random {
public:
    uint32_t operator () () {
	testing_rnd_ << static_cast<int>(static_cast<uint32_t>(testing_rnd_));
	return static_cast<uint32_t>(testing_rnd_);
    }

    inline const uint32_t min() {
	return std::numeric_limits<uint32_t>::min();
    }
    inline const uint32_t max() {
	return std::numeric_limits<uint32_t>::max();
    }

    typedef uint32_t result_type;

private:
    static fast_hash random_;
};

static for_testing_random for_testing_random_;

std::string random::next(size_t entropy_bits)
{
    std::string chars("abcdefghijklmnopqrstuvwxyz"
		      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		      "1234567890");

    boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);

    size_t n = (entropy_bits * 1000 + 5953) / 5954;
    std::string s(n, ' ');
    for (size_t i = 0; i < n; ++i) {
	if (for_testing_) {
	    s[i] = chars[index_dist(for_testing_random_)];
	} else {
	    s[i] = chars[index_dist(random_)];
	}
    }
    return s;
}

int random::next_int(int max)
{
    if (for_testing_) {
	return static_cast<int>(for_testing_random_() % max);
    } else {
	return static_cast<int>(static_cast<unsigned int>(random_()) % max);
    }
}

}}
