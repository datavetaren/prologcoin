#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term_env.hpp>
#include <common/term_ops.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_simple_env()
{
    header( "test_simple_env()" );

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

static void test_unify_append()
{
    header( "test_unify_append()" );

    term_env env;

    std::string s1 = "append([1,2,3], [4,5,6], Z).";
    std::string s2 = "append([X|Xs], Ys, [X|Zs]).";

    auto t1 = env.parse(s1);
    auto t2 = env.parse(s2);

    std::cout << "Parsed1: " << env.to_string(t1) << "\n";
    std::cout << "Parsed2: " << env.to_string(t2) << "\n";

    assert( env.unify(t1, t2) );

    auto rs1 = env.to_string(t1);
    auto rs2 = env.to_string(t2);

    std::cout << "Unify1   : " << rs1 << "\n";
    std::cout << "Unify2   : " << rs2 << "\n";

    assert(rs1 == rs2);

    std::string expected = "append([1,2,3], [4,5,6], [1|Zs])";
    std::cout << "Expected : " << expected << "\n";

    assert(rs1 == expected);
}

static void test_copy_term()
{
    header( "test_copy_term()" );

    term_env env;

    std::string s = "foo(X, 42, X, bar(Y)).";
    auto t1 = env.parse(s);

    std::string s1 = env.to_string(t1);

    std::cout << "Copy term: " << s1 << "\n";

    assert(s1 == s.substr(0, s.length()-1));

    auto t2 = env.copy(t1);

    auto s2 = env.to_string(t2);

    std::cout << "Result   : " << s2 << "\n";

    assert(s2 != s1);

    bool r = false;
    assert( r = env.unify(t1, t2) );

    std::cout << "Unified " << r << "\n";

    s2 = env.to_string(t2);

    std::cout << "Unified : " << s2 << "\n";

    std::cout << "Expected: " << s1 << "\n";
    assert( s1 == s2 );
}

static void test_dfs_iterator()
{
    header( "test_dfs_iterator()" );

    term_env env;

    std::string s = "foo(bar(1,baz(A,2),fun),[1,2,3],end).";
    auto t = env.parse(s);

    std::string expected [] =
	{ "1", "A", "2", "baz(A, 2)", "fun", "bar(1, baz(A, 2), fun)",
	  "1", "2", "3", "[]", "[3]", "[2,3]", "[1,2,3]", "end",
	  "foo(bar(1, baz(A, 2), fun), [1,2,3], end)" };

    auto it_expected = std::begin(expected);

    for (auto tt : env.iterate_over(t)) {
	std::string actual = env.to_string(tt);
	std::string expect = *it_expected;
	++it_expected;
	std::cout << "Actual: " << actual << "\n";
	std::cout << "Expect: " << expect << "\n";
	assert(actual == expect);
    }
    std::cout << "Expect no more...\n";
    assert(it_expected == std::end(expected));
}

static void test_copy_term_heaps()
{
    header( "test_copy_term_heaps()" );

    term_env src_env;
    term_env dst_env;

    std::string s = "foo(bar(1,baz(X,Y,2),fun),[1,2,3,X],end).";
    auto t_src = src_env.parse(s);

    auto src_str = src_env.to_string(t_src);
    std::cout << "Source     : " << src_str << "\n";

    auto t_dst = dst_env.copy(t_src, src_env);

    std::cout << "Destination: " << dst_env.to_string(t_dst) << "\n";

    auto t_src2 = src_env.copy(t_dst, dst_env);

    std::cout << "Back       : " << src_env.to_string(t_src2) << "\n";

    src_env.unify(t_src, t_src2);

    auto unify_str = src_env.to_string(t_src2);

    std::cout << "Unify      : " << unify_str << "\n";

    assert(src_str == unify_str);
}

int main( int argc, char *argv[] )
{
    test_simple_env();
    test_named_vars();
    test_unification();
    test_failed_unification();
    test_unify_append();
    test_copy_term();
    test_dfs_iterator();
    test_copy_term_heaps();

    return 0;
}



