#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/utime.hpp>
#include <boost/lexical_cast.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_utime()
{
    header("test_utime");

    utime t;
    std::cout << "Unix epoch: " << t.str() << std::endl;
    assert(t.str() == "1970.01.01T00:00:00");
    utime t_parsed;
    assert(t_parsed.parse(t.str()));
    assert(t == t_parsed);

    t++;
    std::cout << "Unix epoch + 1 microsecond: " << t.str() << std::endl;
    assert(t.str() == "1970.01.01T00:00:00.000001");
    assert(t_parsed.parse(t.str()));
    assert(t == t_parsed);
    t += 999;
    std::cout << "Unix epoch + 1 millisecond: " << t.str() << std::endl;
    assert(t.str() == "1970.01.01T00:00:00.001000");
    t += 999000;
    std::cout << "Unix epoch + 1 second: " << t.str() << std::endl;
    assert(t.str() == "1970.01.01T00:00:01");
    assert(t_parsed.parse(t.str()));
    assert(t == t_parsed);

    // Try similar with other operators
    t = utime();
    t += utime::ms(1);
    std::cout << "Unix epoch + 1 millisecond: " << t.str() << std::endl;
    assert(t.str() == "1970.01.01T00:00:00.001000");
    assert(t_parsed.parse(t.str()));
    assert(t == t_parsed);
    t = utime();
    t += utime::ss(1);
    std::cout << "Unix epoch + 1 second: " << t.str() << std::endl;
    assert(t.str() == "1970.01.01T00:00:01");
    assert(t_parsed.parse(t.str()));
    assert(t == t_parsed);
    t = utime();
    t += utime::mm(1);
    std::cout << "Unix epoch + 1 minute: " << t.str() << std::endl;
    assert(t.str() == "1970.01.01T00:01:00");
    assert(t_parsed.parse(t.str()));
    assert(t == t_parsed);
    t = utime();
    t += utime::hh(1);
    std::cout << "Unix epoch + 1 hour: " << t.str() << std::endl;
    assert(t.str() == "1970.01.01T01:00:00");
    assert(t_parsed.parse(t.str()));
    assert(t == t_parsed);
    t = utime();
    t += utime::dd(1);
    std::cout << "Unix epoch + 1 day: " << t.str() << std::endl;
    assert(t.str() == "1970.01.02T00:00:00");
    assert(t_parsed.parse(t.str()));
    assert(t == t_parsed);
    t = utime();
    t += utime::yy(1);
    std::cout << "Unix epoch + 1 year: " << t.str() << std::endl;
    assert(t.str() == "1971.01.01T00:00:00");
    assert(t_parsed.parse(t.str()));
    assert(t == t_parsed);

   
    utime now = utime::now();
    std::string nowstr = now.str();
    std::cout << "Now: " << nowstr << std::endl;
    assert(t_parsed.parse(now.str()));
    assert(now == t_parsed);
    std::string year_str = nowstr.substr(0,4);
    int year = boost::lexical_cast<int>(year_str);
    assert(year >= 2000 && year < 2100);
    std::cout << "As value: " << static_cast<uint64_t>(now) << "\n";
    std::cout << "Max int : " << static_cast<uint64_t>(pow(2,64-4)) << "\n";
}

static void test_sleep()
{
    header("test_sleep");

    std::cout << "Sleep for 1 second." << std::endl;

    utime::sleep( utime::ss(1) );

    std::cout << "Sleep for 1 second done." << std::endl;
}

int main(int argc, char *argv[])
{
    test_utime();
    test_sleep();

    return 0;
}
