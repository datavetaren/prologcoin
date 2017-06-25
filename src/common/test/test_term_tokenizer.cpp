#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term_tokenizer.hpp>
#include <common/token_chars.hpp>

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

void test_token_names()
{
    std::string s = "this is a test'\\^?\\^Z\\^a'\t\n+=/*bla/* ha */ xx *q*/\001%To/*themoon\xf0\n'foo'!0'a0'\\^g4242 42.4711 42e3 47.11e-12Foo_Bar\"string\"\"\\^g\" _Baz__ 'bar\x55'[;].";
    std::stringstream ss(s);
    term_ops ops;

    term_tokenizer tt(ss, ops);

    int cnt = 0;
    while (tt.has_more_tokens()) {
	auto tok = tt.next_token();
	std::cout << "NEXT TOKEN: " << token_chars::escape(tok.lexeme()) << "\n";
	cnt++;
	if (cnt > 100) {
	    break;
	}
    }
}

int main( int argc, char *argv[] )
{
    test_is_symbol_char();
    test_token_names();

    return 0;
}
