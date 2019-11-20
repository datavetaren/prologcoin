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

    std::string s = "this is a test'\\^?\\^Z\\^a'\t\n\t+=/*bla/* ha */ xx *q*/\001%To/*themoon\xf0\n'foo'!0'a0'\\^g4242 42.4711 42e3 47.11e-12Foo_Bar\"string\"\"\\^g\" _Baz__ 'bar\x55'[;].";

    std::string expected[] = { "token<NAME>[this]@(L1,C1)",
			       "token<LAYOUT_TEXT>[\\x20]@(L1,C5)",
			       "token<NAME>[is]@(L1,C6)",
			       "token<LAYOUT_TEXT>[\\x20]@(L1,C8)",
			       "token<NAME>[a]@(L1,C9)",
			       "token<LAYOUT_TEXT>[\\x20]@(L1,C10)",
			       "token<NAME>[test]@(L1,C11)",
			       "token<NAME>[\\x7f\\x1a\\x01]@(L1,C15)",
			       "token<LAYOUT_TEXT>[\\x09\\x0a\\x09]@(L1,C26)",
			       "token<NAME>[+=]@(L2,C8)",
			       "token<LAYOUT_TEXT>[/*bla/*\\x20ha\\x20*/\\x20xx\\x20*q*/\\x01%To/*themoon\\xf0\\x0a]@(L2,C10)",
			       "token<NAME>[foo]@(L3,C1)",
			       "token<NAME>[!]@(L3,C6)",
			       "token<NATURAL_NUMBER>[97]@(L3,C7)",
			       "token<NATURAL_NUMBER>[7]@(L3,C10)",
			       "token<NATURAL_NUMBER>[4242]@(L3,C15)",
			       "token<LAYOUT_TEXT>[\\x20]@(L3,C19)",
			       "token<UNSIGNED_FLOAT>[42.4711]@(L3,C20)",
			       "token<LAYOUT_TEXT>[\\x20]@(L3,C27)",
			       "token<UNSIGNED_FLOAT>[42e3]@(L3,C28)",
			       "token<LAYOUT_TEXT>[\\x20]@(L3,C32)",
			       "token<UNSIGNED_FLOAT>[47.11e-12]@(L3,C33)",
			       "token<VARIABLE>[Foo_Bar]@(L3,C42)",
			       "token<STRING>[string\\x22\\x07]@(L3,C49)",
			       "token<LAYOUT_TEXT>[\\x20]@(L3,C62)",
			       "token<VARIABLE>[_Baz__]@(L3,C63)",
			       "token<LAYOUT_TEXT>[\\x20]@(L3,C69)",
			       "token<NAME>[barU]@(L3,C70)",
			       "token<PUNCTUATION_CHAR>[[]@(L3,C76)",
			       "token<NAME>[;]@(L3,C77)",
			       "token<PUNCTUATION_CHAR>[]]@(L3,C78)",
			       "token<FULL_STOP>[.]@(L3,C79)" };    

    std::stringstream ss(s, (std::stringstream::in | std::stringstream::binary));
    term_tokenizer tt(ss);

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

static void test_negative_tokens()
{
    header( "test_negative_tokens()" );

    struct entry { std::string str;
                   token_exception *exc;
    };

    auto p = [](int line, int col) { return token_position(line,col); };

    entry table[] =
     {  { "'foo",     new token_exception_unterminated_quoted_name("",p(1,1)) }
         ,{ "'esc\\",   new token_exception_unterminated_escape("",p(1,1)) }
	 ,{ "'esc\\x",  new token_exception_unterminated_escape("",p(1,1)) }
	 ,{ "'esc\\x3", new token_exception_unterminated_escape("",p(1,1)) }
	 ,{ "'esc\\^",  new token_exception_unterminated_escape("",p(1,1)) }
	 ,{ "'esc\\^\t",  new token_exception_control_char("",p(1,1)) }
 	 ,{ "'esc\\xg",  new token_exception_hex_code("", p(1,1)) }
	 ,{ "0'",       new token_exception_no_char_code("", p(1,1)) }
	 ,{ "11'",      new token_exception_missing_number_after_base("", p(1,1)) }
	 ,{ "1.x",      new token_exception_missing_decimal("", p(1,1)) }
	 ,{ "1.e",     new token_exception_missing_decimal("", p(1,1)) }
 	 ,{ "1ex",      new token_exception_missing_exponent("", p(1,1)) }
 	 ,{ "1e+",     new token_exception_missing_exponent("", p(1,1)) }
	 ,{ "1e-",     new token_exception_missing_exponent("", p(1,1)) }
	 ,{ "2E-",     new token_exception_missing_exponent("", p(1,1)) }
	 ,{ "\"foo",  new token_exception_unterminated_string("", p(1,1)) }
      };

    for (auto e : table) {
        std::stringstream ss(e.str);
        term_tokenizer tt(ss);

	try {
	    std::cout << "Testing token: " << e.str << "\n";
	    tt.next_token();
	    std::cout << " (Expected exception '" << typeid(e.exc).name() << "' not thrown)\n";
	    assert(false);
	} catch (token_exception &exc) {
  	    std::string actual = typeid(exc).name();
	    std::string expected = typeid(*e.exc).name();
	    std::cout << "  Thrown: " << actual << "\n";
	    if (actual != expected) {
	        std::cout << " (Expected exception '" << expected
		  	  << "' but got '" << actual << "'\n";
	        assert(false);
	    }
	    if (exc.pos() != e.exc->pos()) {
	        std::cout << " (Expected position " << e.exc->pos().str()
		          << " but got " << exc.pos().str() << ")\n";
	        assert(false);
	    }
	    delete e.exc; // Free memory (good for valgrind)
	}
    }
}

int main( int argc, char *argv[] )
{
    test_is_symbol_char();
    test_tokens();
    test_negative_tokens();

    return 0;
}
