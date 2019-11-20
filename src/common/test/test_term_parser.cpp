#include <iostream>
#include <fstream>
#include <iomanip>
#include <assert.h>
#include <common/term_tokenizer.hpp>
#include <common/token_chars.hpp>
#include <common/term_parser.hpp>
#include <common/term_emitter.hpp>
#include "test_home_dir.hpp"

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

    term result = parser.parse();

    std::stringstream sout;
    term_emitter emitter(sout, h, ops);
    emitter.print(result);

    std::string expected = "foo(1, 2*3+4+5+6- -7, 8)";
    
    std::cout << "IN : " << sin.str() << "\n";
    std::cout << "OUT: " << sout.str() << "\n";
    std::cout << "EXP: " << expected << "\n";

    assert(sout.str() == expected);

    std::stringstream sout2;
    std::stringstream sin2(expected+ ".");
    term_tokenizer tokenizer2(sin2);
    term_parser parser2(tokenizer2, h, ops);
    term result2 = parser2.parse();
    term_emitter emitter2(sout2, h, ops);
    emitter2.print(result2);

    std::cout << "2ND: " << sout2.str() << "\n";
    // Should be the same
    assert(sout2.str() == sout.str());
}

static void skip_whitespace(term_tokenizer &tokenizer)
{
    while (tokenizer.has_more_tokens()) {
	auto &token = tokenizer.peek_token();
	if (token.type() != term_tokenizer::TOKEN_LAYOUT_TEXT) {
	    return;
	}
	tokenizer.consume_token();
    }
}

//
// Read in my own "Yacc/Bison" like parser generator...
//
static void test_complicated_parse()
{
    header( "test_complicated_parse()" );

    const std::string &home_dir = find_home_dir();

    std::ifstream infile(home_dir + "/src/common/test/test_parser_sample.pl");

    heap h;
    term_ops ops;
    term_tokenizer tokenizer(infile);
    term_parser parser(tokenizer, h, ops);

    // parser.set_debug(true);

    std::vector<term> clauses;

    int clause_no = 0;
    while (!parser.is_eof()) {
	term result;
	try {
	    result = parser.parse();
	} catch (std::runtime_error &ex) {
	    if (parser.is_error()) {
		std::cout << "Parse error at clause " << clause_no << ".\n";
		assert(!parser.is_error());
		throw;
	    }
	}
	clauses.push_back(result);
	clause_no++;
    }

    std::cout << "HEAP IS: "; h.print_status(std::cout); std::cout << "\n";

    std::stringstream reemit;

    term_emitter emitter(reemit, h, ops);

    parser.for_each_var_name( [&](const term &ref,
				  const std::string &name)
			      { emitter.set_var_name(ref, name); } );

    emitter.options().set(emitter_option::EMIT_PROGRAM);

    for (auto clause : clauses) {
	emitter.print(clause);
	emitter.nl();
	emitter.nl();
    }

    // Write entire 'reemit' to file for debug
    std::ofstream outfile(home_dir + "/bin/test/common/test_parser_sample.pl.cmp");
    outfile << reemit.str() << "\n";

    // We now have everything remitten in 'reemit'.
    // Let's compare using original file
    {
	std::ifstream infile(home_dir + "/src/common/test/test_parser_sample.pl");

	term_tokenizer orig_tokens(infile);
	term_tokenizer cmp_tokens(reemit);

	skip_whitespace(orig_tokens);
	skip_whitespace(cmp_tokens);

	while (orig_tokens.has_more_tokens() && cmp_tokens.has_more_tokens()) {
	    auto &orig_token = orig_tokens.peek_token();
	    auto &cmp_token = cmp_tokens.peek_token();

	    if (orig_token.lexeme() != cmp_token.lexeme()) {
		std::cout << "Difference found at " << orig_token.pos().str() << "\n";
		std::cout << token_chars::escape_pretty(orig_token.lexeme()) << " != " << token_chars::escape_pretty(cmp_token.lexeme()) << "\n";
	    }

	    assert(orig_token.lexeme() == cmp_token.lexeme());

	    orig_tokens.consume_token();
	    cmp_tokens.consume_token();

	    skip_whitespace(orig_tokens);
	    skip_whitespace(cmp_tokens);
	}

	// EOF should happen for both.
	assert(orig_tokens.has_more_tokens() ==
	       cmp_tokens.has_more_tokens());

    }
}

static void test_bignum_parse()
{
    header( "test_bignum_parse()" );

    std::string expect58 = "4atLG7Hb9u2NH7HrRBedKHJ5hQ3z4QQcEWA3b8ACU";

    std::stringstream sin("foo(58'" + expect58 + ").");
    heap h;
    term_ops ops;
    term_tokenizer tokenizer(sin);
    term_parser parser(tokenizer, h, ops);
    term result = parser.parse();

    assert(result.tag() == tag_t::STR);
    assert(h.functor(result) == con_cell("foo",1));
    term arg = h.arg(result,0);
    assert(arg.tag() == tag_t::BIG);

    std::string actual58 = h.big_to_string(arg, 58);
    std::cout << "Actual: " << actual58 << std::endl;
    std::cout << "Expect: " << expect58 << std::endl;
    assert(actual58 == expect58);
    
    std::string expect10 = "123456789123456789123456789123456789123456789123456789123456789123456789";
    std::string actual10 = h.big_to_string(arg, 10);

    std::cout << "Actual: " << actual10 << std::endl;
    std::cout << "Expect: " << expect10 << std::endl;
    assert(actual10 == expect10);

    std::string expect16 = "11E3444E07186473F6C29BFB5CD699549E6C50200673C72870B684045F15";
    std::string actual16 = h.big_to_string(arg, 16, true);
    std::cout << "Actual: " << actual16 << std::endl;
    std::cout << "Expect: " << expect16 << std::endl;
    assert(actual16 == expect16);

}

int main( int argc, char *argv[] )
{
    find_home_dir(argv[0]);   

    test_simple_parse();
    test_complicated_parse();
    test_bignum_parse();

    return 0;
}
