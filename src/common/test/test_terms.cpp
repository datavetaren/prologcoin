#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term.hpp>
#include <common/term_ops.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_ref_cells()
{
    header( "test_ref_cells()" );

    ref_cell a( 10 );
    ref_cell b( 42 );
    ref_cell c( 4711 );

    std::cout << " a.str(): " << a.str() << "\n";
    std::cout << " b.str(): " << b.str() << "\n";
    std::cout << " c.str(): " << c.str() << "\n";

    assert( a.index() == 10 );
    assert( b.index() == 42 );
    assert( c.index() == 4711 );
}

static void test_con_cells()
{
    header( "test_con_cells()" );

    con_cell foo3( "foo", 3 );
    con_cell bars2( "bars", 2 );
    con_cell dot1( ".", 1 );
    con_cell foot5( "foot", 5 );
    con_cell foobar6( "foobar", 6 );

    std::cout << " foo3.str()    : " << foo3.str() << "\n";
    assert(foo3.name_and_arity() == "foo/3");

    std::cout << " bars2.str()   : " << bars2.str() << "\n";
    assert(bars2.name_and_arity() == "bars/2");

    std::cout << " dot1.str()    : " << dot1.str() << "\n";
    assert(dot1.name_and_arity() == "./1");

    std::cout << " foot5.str()   : " << foot5.str() << "\n";
    assert(foot5.name_and_arity() == "foot/5");

    std::cout << " foobar6.str() : " << foobar6.str() << "\n";
    assert(foobar6.name_and_arity() == "foobar/6");
}

static void test_int_cells()
{
    header( "test_int_cells()" );

    int_cell a( 123 );
    int_cell b( 456 );

    int_cell r1 = a + b;
    int_cell r2 = a - b;
    int_cell r3 = a * b;
    int_cell r4 = b / a;

    std::cout << "---- Result : " << (int64_t)r1 << "\n";
    assert(r1 == 579);

    std::cout << "---- Result : " << (int64_t)r2 << "\n";
    assert(r2 == -333);

    std::cout << "---- Result : " << (int64_t)r3 << "\n";
    assert(r3 == 56088);

    std::cout << "---- Result : " << (int64_t)r4 << "\n";
    assert(r4 == 3);

    std::cout << "---- r1 String : " << r1.str() << "\n";

    int_cell actual_maximum = int_cell::max();
    int64_t expect_maximum =  static_cast<int64_t>((static_cast<uint64_t>(1) << (63 - cell::TAG_SIZE_BITS)) - 1);
    std::cout << "Actual maximum: " << actual_maximum.value() << std::endl;
    std::cout << "Expect maximum: " << expect_maximum << std::endl;
    assert(actual_maximum == int_cell(expect_maximum));
    assert(actual_maximum.value() == expect_maximum);

    int_cell actual_minimum = int_cell::min();
    int64_t expect_minimum =  static_cast<int64_t>(~((static_cast<uint64_t>(1) << (63 - cell::TAG_SIZE_BITS)) - 1));
    std::cout << "Actual minimum: " << actual_minimum.value() << std::endl;
    std::cout << "Expect minimum: " << expect_minimum << std::endl;
    assert(actual_minimum == int_cell(expect_minimum));
    assert(actual_minimum.value() == expect_minimum);
}    

static void test_heap_simple()
{
    header( "test_heap_simple()" );

    heap h;

    auto cp = h.new_str( con_cell("foo", 3) );
    h.set_arg(cp, 0, h.arg(cp, 0));
    h.set_arg(cp, 1, int_cell(4711));
    h.set_arg(cp, 2, con_cell("bar",0));

    auto big = h.new_big( 256 ); // 256 bits
    boost::multiprecision::cpp_int val("123456789123456789123456789123456789123456789123456789123456789123456789");
    h.set_big(big, val);

    auto big2 = h.new_big( 128 ); // 128 bits
    boost::multiprecision::cpp_int val2("12345678912345678912345678912345");
    h.set_big(big2, val2);

    h.print(std::cout);
    h.print_status(std::cout);

    (void)cp;
}

static void test_term_ops()
{
    header( "test_term_ops()" );
    
    term_ops ops;

    // ops.put( "foobar", 377, term_ops::XFY );
    // ops.put( "div", 313, term_ops::XFY );

    ops.print(std::cout);
}

static void test_term_big()
{
    header( "test_term_big()" );

    heap h;
    cell big = h.new_big( 256 );
    std::string val10 = "123456789123456789123456789123456789123456789123456789123456789123456789";
    std::string val16 = "11E3444E07186473F6C29BFB5CD699549E6C50200673C72870B684045F15";
    std::string val58 = "4atLG7Hb9u2NH7HrRBedKHJ5hQ3z4QQcEWA3b8ACU";

    boost::multiprecision::cpp_int val(val10);
    h.set_big(big, val);

    std::string val10_cmp = h.big_to_string(big, 10);
    std::cout << "Actual: " << val10_cmp << std::endl;
    std::cout << "Expect: " << val10 << std::endl;
    assert(val10 == val10_cmp);

    std::string val16_cmp = h.big_to_string(big, 16);
    std::cout << "Actual: " << val16_cmp << std::endl;
    std::cout << "Expect: " << val16 << std::endl;
    assert(val16 == val16_cmp);

    std::string val58_cmp = h.big_to_string(big, 58);
    std::cout << "Actual: " << val58_cmp << std::endl;
    std::cout << "Expect: " << val58 << std::endl;
    assert(val58 == val58_cmp);
}

int main(int argc, char *argv[])
{
    test_ref_cells();
    test_con_cells();
    test_int_cells();

    test_heap_simple();

    test_term_ops();

    test_term_big();

    return 0;
}
