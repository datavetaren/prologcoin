#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term_tokenizer.hpp>
#include <common/token_chars.hpp>
#include <common/term_parser.hpp>
#include <common/term_emitter.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_simple_parse()
{
    header( "test_simple_parse()" );

    std::stringstream sin("foo(1,2*3+4+5+ +6-(-7),8).");

    heap h;
    term_ops ops;
    term_tokenizer tokenizer(sin);
    term_parser parser(tokenizer, h, ops);

    ext<cell> result = parser.parse();

    std::stringstream sout;
    term_emitter emitter(sout, h, ops);
    emitter.print(result);

    std::string expected = "foo(1, 2*3+4+5+ + 6- - 7, 8)";
    
    std::cout << "IN : " << sin.str() << "\n";
    std::cout << "OUT: " << sout.str() << "\n";
    std::cout << "EXP: " << expected << "\n";

    assert(sout.str() == expected);
}

int main( int argc, char *argv[] )
{
    test_simple_parse();

    return 0;
}
