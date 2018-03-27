#include <common/term.hpp>
#include <common/term_match.hpp>

using namespace prologcoin::common;

#include <type_traits>

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_simple_match()
{
    header("test_simple_match");

    // Construct

    term_env env;
    term t = env.new_term( con_cell(",", 2),
		  {con_cell("foo",0),
		   env.new_term(con_cell(",",2),
				{con_cell("bar",0),
				env.new_term(con_cell(",",2),
					     {con_cell("baz",0),
					      con_cell("xyz",0)}
					     )}
				)});

    pattern pat(env);

    term out;
    for (int i = 0; i < 10; i++) {
	static auto p = pat.str( con_cell(",",2),
				 pat.con("foo",0),
				 pat.str(con_cell(",",2),
					 pat.con("bar",0),
					 pat.any(out)));

	bool r = p(env, t);
	std::cout << "Matching: " << r << ": ";
	if (r) {
	    std::cout << env.to_string(out) << std::endl;
	} else {
	    std::cout << "Fail!" << std::endl;
	}

	assert(r);
    }
}

int main(int argc, char *argv[] )
{
    test_simple_match();

    return 0;
}


