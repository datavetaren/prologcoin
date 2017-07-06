#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term_env.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_simple_env()
{
    header( "test_simple_parse()" );

    term_env env;
    std::string sin = "foo(1,2*3+4+5+ +6-(-7),8).";
    auto r = env.parse(sin);
    std::string sout = env.to_string(r);
    std::string expected = "foo(1, 2*3+4+5+ + 6- - 7, 8)";
    
    std::cout << "IN : " << sin << "\n";
    std::cout << "OUT: " << sout << "\n";
    std::cout << "EXP: " << expected << "\n";

    assert(sout == expected);
}

static void test_named_vars()
{
    header( "test_named_vars()" );

    term_env env;
    std::string sin = "foo(X, B, bar(Y)).";
    std::string exp = sin.substr(0, sin.size()-1);
    auto r = env.parse(sin);
    std::string sout = env.to_string(r);
    
    std::cout << "IN : " << sin << "\n";
    std::cout << "OUT: " << sout << "\n";
    std::cout << "EXP: " << exp << "\n";

    assert( sout == exp);
}

static void test_unification()
{
    header( "test_unification()" );

    term_env env;

    std::string sin1 = "foo(X, B, bar(Y)).";
    std::string sin2 = "foo(baz(Q), yes, bar(B)).";
    std::string exp = "foo(baz(Q), yes, bar(Y))";

    auto r1 = env.parse(sin1);
    auto r2 = env.parse(sin2);

    std::string sout1 = env.to_string(r1);
    std::string sout2 = env.to_string(r2);

    std::cout << "Parsed1: " << sout1 << "\n";
    std::cout << "Parsed2: " << sout2 << "\n";

    // Now unify these two terms

    assert( env.unify(r1, r2) );

    std::string rs1 = env.to_string(r1);
    std::string rs2 = env.to_string(r2);

    std::cout << "Unified!\n";
    std::cout << "Result1: " << rs1 << "\n";
    std::cout << "Result2: " << rs2 << "\n";

    assert(rs1 == rs2);

    std::cout << "Expect : " << exp << "\n";

    assert(rs1 == exp);
}

static void test_failed_unification()
{
    header( "test_failed_unification()" );

    term_env env;

    std::string sin1 = "foo(X, X, bar(Y)).";
    std::string sin2 = "foo(baz(Q), yes, bar(B)).";

    auto r1 = env.parse(sin1);
    auto r2 = env.parse(sin2);

    std::string sout1 = env.to_string(r1);
    std::string sout2 = env.to_string(r2);

    std::cout << "Parsed1: " << sout1 << "\n";
    std::cout << "Parsed2: " << sout2 << "\n";

    std::cout << "Status : " << env.status() << "\n";
    
    bool r = env.unify(r1,r2);

    std::cout << "Unify should fail: " << r << "\n";
    assert(!r);

    std::cout << "Status : " << env.status() << "\n";

    auto rs1 = env.to_string(r1);
    std::cout << "Result1: " << rs1 << "\n";

    auto rs2 = env.to_string(r2);
    std::cout << "Result2: " << rs2 << "\n";

    std::cout << "The results shouldn't have changed...\n";

    assert(rs1 == sout1);
    assert(rs2 == sout2);

    std::cout << "Stack and trail should be zero...\n";
    std::cout << "Status : " << env.status() << "\n";

    assert(env.stack_size() == 0);
    assert(env.trail_size() == 0);
}

int main( int argc, char *argv[] )
{
    test_simple_env();
    test_named_vars();
    test_unification();
    test_failed_unification();

    return 0;
}
