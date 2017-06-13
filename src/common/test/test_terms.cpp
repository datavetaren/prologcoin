#include <iostream>
#include <iomanip>
#include <assert.h>
#include <common/term.hpp>

using namespace prologcoin::common;

void header( const std::string &str )
{
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
}

void test_ref_cells()
{
    header( "Cells / REF" );

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

void test_con_cells()
{
    header( "Cells / CON" );

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

void test_int_cells()
{
    header( "Cells / INT" );

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
}    


int main(int argc, char *argv[])
{
    test_ref_cells();
    test_con_cells();
    test_int_cells();
}
