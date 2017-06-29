#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term_tokenizer.hpp>
#include <common/token_chars.hpp>
#include <common/term_parser.hpp>

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

    std::stringstream ss("foo(1,2,3).");

    heap h;
    term_ops ops;
    term_tokenizer tokenizer(ss);
    term_parser parser(tokenizer, h, ops);
    parser.set_debug(true);

    /*
    parser.process_next();
    parser.process_next();
    parser.process_next();
    parser.process_next();
    */
}

int main( int argc, char *argv[] )
{
    test_simple_parse();

    return 0;
}
