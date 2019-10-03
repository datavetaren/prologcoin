#include <iostream>
#include <sstream>
#include <assert.h>
#include <vector>
#include <cmath>
#include "../galaxy.hpp"
#include "../camera.hpp"
#include "../dipper_detector.hpp"
#include "../observatory.hpp"

using namespace prologcoin::pow;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_uint64_t()
{
    header("test_uint64_t");

    uint64_t maximum = std::numeric_limits<uint64_t>::max();
    uint64_t minimum = 0;

    std::cout << "Max UINT64 to DOUBLE: " << uint64_to_T<double>(maximum) << std::endl;
    std::cout << "Min UINT64 to DOUBLE: " << uint64_to_T<double>(minimum) << std::endl;
    std::cout << "1 << 63: " << uint64_to_T<double>(static_cast<uint64_t>(1)<<63) << std::endl;
    std::cout << "1 << 62: " << uint64_to_T<double>(static_cast<uint64_t>(1)<<62) << std::endl;
    std::cout << "1 << 61: " << uint64_to_T<double>(static_cast<uint64_t>(1)<<61) << std::endl;

    double x_min = 1000.0;
    double x_max = -1000.0;

    const char *msg = "hello42";
    siphash_keys keys(msg, strlen(msg));

    for (size_t i = 0; i < 10000; i++) {
	uint64_t x64 = 0;
	siphash_1(keys, i, x64);
	if (x64 & (static_cast<uint64_t>(1) << 63)) {
	    double x = uint64_to_T<double>(x64);
	    if (x < x_min) {
		x_min = x;
	    }
	    if (x > x_max) {
		x_max = x;
	    }
	}
    }

    std::cout << "x_min: " << x_min << " x_max: " << x_max << std::endl;
}

#if 0
static void test_galaxy()
{
    header("test_galaxy");

    const char *msg = "hello42";
    
    siphash_keys keys(msg, strlen(msg));

    galaxy<7> gal(keys);
    gal.init(1000000);
    gal.check();

    camera<7> cam(gal, 0);
    cam.set_target(vec3(vec3::SPHERICAL(), 0.1, M_PI/2, M_PI/2));
    std::vector<projected_star> stars;
    cam.take_picture(stars);
}
#endif

#if 0
static void test_search()
{
    header("test_search");

    const char *msg = "hello42";

    siphash_keys keys(msg, strlen(msg));

    galaxy<7> gal(keys);
    gal.init(10000000);
    gal.check();

    camera<7> cam(gal, 0);
    cam.set_target(vec3(vec3::SPHERICAL(), 0.1, M_PI/2, M_PI/2));
    std::vector<projected_star> stars;
    cam.take_picture(stars);

    // Detect single picture
    dipper_detector detector(stars);
    std::vector<projected_star> found;
    detector.search(found, 10);
}
#endif

static void test_scan()
{
    header("test_scan");

    char msg[8] = "hello42";
    siphash_keys keys(msg, strlen(msg));

    observatory<8,double> obs(keys);
    obs.status();

    for (int i = 0; i < 100; i++) {
	std::vector<projected_star> found;
	uint32_t nonce = 0;
	projected_star first_visible;
	obs.scan(i, first_visible, found, nonce);
    }
}

static void test_simple()
{
    header("test_simple");

    char msg[8] = "hello42";

    siphash_keys keys(msg, strlen(msg));
    observatory<7,double> obs(keys);
    obs.status();
}

int main(int argc, char *argv[])
{
    test_uint64_t();
    test_simple();
    // test_galaxy();
    // test_search();
    if (argc == 2 && strcmp(argv[1], "--scan") == 0) {
	test_scan();
    }

    return 0;
}
