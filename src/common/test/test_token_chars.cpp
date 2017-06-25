#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term_tokenizer.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_is_symbol_char()
{
    header( "test_is_symbol_char()" );

    std::string str;
    bool in_seq = false;
    for (int i = 0; i < 500; i++) {
	if (term_tokenizer::is_symbol_char(i)) {
	    if (!in_seq && !str.empty()) {
		str += ", ";
	    }
	    if (!in_seq) {
		str += boost::lexical_cast<std::string>(i);
	    }
	    if (!in_seq &&
		term_tokenizer::is_symbol_char(i+1) &&
		term_tokenizer::is_symbol_char(i+2)) {
		in_seq = true;
	    }
	}
	if (in_seq && !term_tokenizer::is_symbol_char(i)) {
	    str += "..";
	    str += boost::lexical_cast<std::string>(i-1);
	    in_seq = false;
	}
    }
    
    std::cout << "Symbol chars: " + str + "\n";

    assert( str ==
	    "35, 36, 38, 42, 43, 45..47, 58, 60..64, 92, 94, 96, 126, "
	    "160..191, 215, 247");

}

int main( int argc, char *argv[] )
{
    test_is_symbol_char();

    return 0;
}
