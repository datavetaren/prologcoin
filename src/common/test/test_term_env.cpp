#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term_env.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_simple_env()
{
    header( "test_simple_parse()" );

    term_env env;
    std::string sin = "foo(1,2*3+4+5+ +6-(-7),8).";
    auto r = env.parse(sin);
    std::string sout = env.to_string(r);
    std::string expected = "foo(1, 2*3+4+5+ + 6- - 7, 8)";
    
    std::cout << "IN : " << sin << "\n";
    std::cout << "OUT: " << sout << "\n";
    std::cout << "EXP: " << expected << "\n";

    assert(sout == expected);
}

int main( int argc, char *argv[] )
{
    test_simple_env();

    return 0;
}
