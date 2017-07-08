#include <iostream>
#include <fstream>
#include <iomanip>
#include <assert.h>
#include <common/term_tokenizer.hpp>
#include <common/token_chars.hpp>
#include <common/term_parser.hpp>
#include <common/term_emitter.hpp>

using namespace prologcoin::common;

static std::string home_dir;

static void do_parent(std::string &path)
{
    size_t slashIndex = path.find_last_of("/\\");
    path = path.substr(0, slashIndex);
}

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

    std::stringstream sout2;
    std::stringstream sin2(expected+ ".");
    term_tokenizer tokenizer2(sin2);
    term_parser parser2(tokenizer2, h, ops);
    ext<cell> result2 = parser2.parse();
    term_emitter emitter2(sout2, h, ops);
    emitter2.print(result2);

    std::cout << "2ND: " << sout2.str() << "\n";
    // Should be the same
    assert(sout2.str() == sout.str());
}

//
// Read in my own "Yacc/Bison" like parser generator...
static void test_complicated_parse()
{
    header( "test_complicated_parse()" );

    std::ifstream infile(home_dir + "/src/common/test/test_parser_sample.pl");

    heap h;
    term_ops ops;
    term_tokenizer tokenizer(infile);
    term_parser parser(tokenizer, h, ops);

    std::vector<ext<cell> > clauses;

    int clause_no = 0;
    while (!parser.is_eof()) {
	ext<cell> result = parser.parse();
	if (parser.is_error()) {
	    std::cout << "Parse error at clause " << clause_no << ".\n";
	    assert(!parser.is_error());
	    return;
	}
	clauses.push_back(result);
	clause_no++;
    }

    std::cout << "HEAP IS: "; h.print_status(std::cout); std::cout << "\n";

    term_emitter emitter(std::cout, h, ops);

    parser.for_each_var_name( [&](const ext<cell> &ref,
				  const std::string &name)
			      { emitter.set_var_name(ref, name); } );

    emitter.set_style(term_emitter::STYLE_PROGRAM);

    for (auto clause : clauses) {
	emitter.print(clause);
	emitter.nl();
	emitter.nl();
    }
}

static void find_home_dir(const char *selfpath)
{
    // Current path
    home_dir = selfpath;
    std::replace( home_dir.begin(), home_dir.end(), '\\', '/');

    bool found = false;
    do {
      do_parent(home_dir);
      std::string checkfile = home_dir + "/env/Makefile.main";
      if (auto f = fopen(checkfile.c_str(), "r")) {
   	  fclose(f);
          found = true;
      }
    } while (!found);
}

int main( int argc, char *argv[] )
{
    find_home_dir(argv[0]);   

    test_simple_parse();
    test_complicated_parse();

    return 0;
}
