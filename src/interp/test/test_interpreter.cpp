#include "../interpreter.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_simple_interpreter()
{
    header("test_simple_interpreter()");

    interpreter interp;

    //    term clause1 = interp.env().parse("append([X|Xs],Y,[Z|Zs]) :- append(Xs,Y,Zs).");
    //    term clause2 = interp.env().parse("append([],Zs,Zs).");

    term program = interp.env().parse("[(append([X|Xs],Ys,[Z|Zs]) :- append(Xs,Ys,Zs)), append([], Zs, Zs)].");

    interp.load_program(program);
}

int main( int argc, char *argv[] )
{
    test_simple_interpreter();

    return 0;
}
