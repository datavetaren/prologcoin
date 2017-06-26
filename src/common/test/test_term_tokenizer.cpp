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

static void test_tokens()
{
    header( "test_tokens()" );

    std::string s = "this is a test'\\^?\\^Z\\^a'\t\n+=/*bla/* ha */ xx *q*/\001%To/*themoon\xf0\n'foo'!0'a0'\\^g4242 42.4711 42e3 47.11e-12Foo_Bar\"string\"\"\\^g\" _Baz__ 'bar\x55'[;].";

    std::string expected[] = { "token<NAME>[this]",
			       "token<LAYOUT_TEXT>[\\x20]",
			       "token<NAME>[is]",
			       "token<LAYOUT_TEXT>[\\x20]",
			       "token<NAME>[a]",
			       "token<LAYOUT_TEXT>[\\x20]",
			       "token<NAME>[test]",
			       "token<NAME>[\\x7f\\x1a\\x01]",
			       "token<LAYOUT_TEXT>[\\x09\\x0a]",
			       "token<NAME>[+=]",
			       "token<LAYOUT_TEXT>[/*bla/*\\x20ha\\x20*/\\x20xx\\x20*q*/\\x01%To/*themoon\\xf0\\x0a]",
			       "token<NAME>[foo]",
			       "token<NAME>[!]",
			       "token<NATURAL_NUMBER>[97]",
			       "token<NATURAL_NUMBER>[7]",
			       "token<NATURAL_NUMBER>[4242]",
			       "token<LAYOUT_TEXT>[\\x20]",
			       "token<UNSIGNED_FLOAT>[42.4711]",
			       "token<LAYOUT_TEXT>[\\x20]",
			       "token<UNSIGNED_FLOAT>[42e3]",
			       "token<LAYOUT_TEXT>[\\x20]",
			       "token<UNSIGNED_FLOAT>[47.11e-12]",
			       "token<VARIABLE>[Foo_Bar]",
			       "token<STRING>[string\\x22\\x07]",
			       "token<LAYOUT_TEXT>[\\x20]",
			       "token<VARIABLE>[_Baz__]",
			       "token<LAYOUT_TEXT>[\\x20]",
			       "token<NAME>[barU]",
			       "token<PUNCTUATION_CHAR>[[]",
			       "token<NAME>[;]",
			       "token<PUNCTUATION_CHAR>[]]",
			       "token<FULL_STOP>[.]" };

    std::stringstream ss(s, (std::stringstream::in | std::stringstream::binary));
    term_ops ops;

    term_tokenizer tt(ss, ops);

    int cnt = 0;
    while (tt.has_more_tokens()) {
	auto tok = tt.next_token();
	std::cout << tok.str() << "\n";
	if (tok.str() != expected[cnt]) {
	  std::cout << "Expected token: " << expected[cnt] << "\n";
	  std::cout << "But got       : " << tok.str() << "\n";
	}
	assert( tok.str() ==  expected[cnt] );
	cnt++;
    }
}

int main( int argc, char *argv[] )
{
    test_is_symbol_char();
    test_tokens();

    return 0;
}
