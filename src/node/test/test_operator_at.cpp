#include <iostream>

#include <common/utime.hpp>
#include "setup_nodes.hpp"

using namespace prologcoin::common;
using namespace prologcoin::node;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_operator_at_simple()
{
    header("test_operator_at_simple");

    term_env env;

    setup_nodes network( { { "apple", 8000 },
		           { "pear", 8001 },
		           { "banana", 8002 } } );

    network.start();

    // Construct a query: member(X, [1,2,3,4]) @ pear from 'apple'
    auto tm = network.new_terminal("apple");

    std::string q1 = "member(X, [1,2,3,4]) @ pear.";
    std::cout << "@apple: testing query: " << q1 << std::endl;
    {
	bool r = tm->execute(q1);
	assert(r);
	network.check_result(tm, { "X = 1", "X = 2", "X = 3", "X = 4" } );
    }

    std::string q2 = "member(X, [1,2,3]) @ pear, member(Y, [a,b,c]) @ banana.";
    std::cout << "@apple: testing query: " << q2 << std::endl;
    {
	bool r = tm->execute(q2);
	assert(r);
	network.check_result(tm, { 
		                   "X = 1, Y = a", 
		                   "X = 1, Y = b",
		                   "X = 1, Y = c",
		                   "X = 2, Y = a", 
		                   "X = 2, Y = b",
		                   "X = 2, Y = c",
		                   "X = 3, Y = a", 
		                   "X = 3, Y = b",
			           "X = 3, Y = c" } );
    }


    std::string q3 = "member(X, [1,2,3]) @ pear, member(Y, [a,b,c]) @ banana, member(Z, [q,w]).";
    std::cout << "@apple: testing query: " << q3 << std::endl;
    {
	bool r = tm->execute(q3);
	assert(r);
	network.check_result(tm, { 
		                   "X = 1, Y = a, Z = q", 
		                   "X = 1, Y = a, Z = w", 
			           "X = 1, Y = b, Z = q",
			           "X = 1, Y = b, Z = w",
		   	           "X = 1, Y = c, Z = q",
		   	           "X = 1, Y = c, Z = w",
		                   "X = 2, Y = a, Z = q", 
		                   "X = 2, Y = a, Z = w", 
		                   "X = 2, Y = b, Z = q",
		                   "X = 2, Y = b, Z = w",
		                   "X = 2, Y = c, Z = q",
		                   "X = 2, Y = c, Z = w",
		                   "X = 3, Y = a, Z = q", 
		                   "X = 3, Y = a, Z = w", 
		                   "X = 3, Y = b, Z = q",
		                   "X = 3, Y = b, Z = w",
			           "X = 3, Y = c, Z = q",
			           "X = 3, Y = c, Z = w" });
    }

    // Testing different order of backtracking
    std::string q4 = "member(Z, [q,w]), member(X, [1,2,3]) @ pear, member(Y, [a,b,c]) @ banana.";
    std::cout << "@apple: testing query: " << q4 << std::endl;
    {
	bool r = tm->execute(q4);
	assert(r);
	network.check_result(tm, { 
		                   "Z = q, X = 1, Y = a",
			           "Z = q, X = 1, Y = b",
		   	           "Z = q, X = 1, Y = c",
		                   "Z = q, X = 2, Y = a", 
		                   "Z = q, X = 2, Y = b",
		                   "Z = q, X = 2, Y = c",
		                   "Z = q, X = 3, Y = a", 
		                   "Z = q, X = 3, Y = b",
			           "Z = q, X = 3, Y = c",
		                   "Z = w, X = 1, Y = a",
			           "Z = w, X = 1, Y = b",
		   	           "Z = w, X = 1, Y = c",
		                   "Z = w, X = 2, Y = a", 
		                   "Z = w, X = 2, Y = b",
		                   "Z = w, X = 2, Y = c",
		                   "Z = w, X = 3, Y = a", 
		                   "Z = w, X = 3, Y = b",
			           "Z = w, X = 3, Y = c"
			         });
    }

    tm->close();
    
    // Wait for one second
    utime::sleep(utime::ss(1));

    network.stop();
}

int main(int argc, char *argv[])
{
    test_operator_at_simple();

    return 0;
}
