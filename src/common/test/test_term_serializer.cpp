#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term_env.hpp>
#include <common/term_serializer.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_term_serializer()
{
    header( "test_term_serializer()" );

    term_env env;
    term t = env.parse("foo(1, bar(kallekula, [1,2,baz]), Foo, kallekula, world, test4711, Foo, Bar).");

    auto str1 = env.to_string(t);

    std::cout << "WRITE TERM: " << str1 << "\n";

    term_serializer ser(env);
    term_serializer::buffer_t buf;
    ser.write(buf, t);

    term_env env2;
    term_serializer ser2(env2);
    term t2 = ser2.read(buf);
    
    auto str2 = env2.to_string(t2);

    std::cout << "READ TERM:  " << str2 << "\n";
    
    assert(str1 == str2);
}

int main( int argc, char *argv[] )
{
    test_term_serializer();

    return 0;
}
