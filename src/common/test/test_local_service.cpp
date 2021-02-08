#include <common/local_service.hpp>
#include <iostream>
#include <assert.h>
#include <string>
#include <common/utime.hpp>
#include <common/random.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_local_service()
{
    header("test_local_service");

    local_service ls;

    ls.add_workers(8);

    boost::mutex lock;
    const size_t n = 128;
    uint32_t result[n];
    size_t cnt = 0;
    for (size_t i = 0; i < n; i++) {
	ls.add( [i,&cnt,&lock,&result](){ utime::sleep( utime::ms( random::next_int(100) ) );
		       boost::unique_lock<boost::mutex> lock_it(lock);
		       result[i] = i; cnt++;
	             } );
    }

    while (cnt < n) {
	size_t cnt1;
	{
	    boost::unique_lock<boost::mutex> lock_it(lock);
	    cnt1 = cnt;
	}
	std::cout << "Progress: cnt=" << cnt1 << std::endl;
	utime::sleep( utime::ss(1) );
    }
    std::cout << "Progress: cnt=" << cnt << std::endl;

    std::cout << "Kill" << std::endl;

    ls.kill();
    ls.join();

    std::cout << "Check result" << std::endl;
    for (size_t i = 0; i < n; i++) {
	assert(result[i] == i);
    }
    std::cout << "Done" << std::endl;
}

int main( int argc, char *argv[] )
{
    random::set_for_testing(true);

    test_local_service();

    return 0;
}

