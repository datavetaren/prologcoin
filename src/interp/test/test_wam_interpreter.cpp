#include "../../common/term_tools.hpp"
#include "../wam_interpreter.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

namespace prologcoin { namespace interp {

class test_wam_interpreter
{
public:
    test_wam_interpreter() { }

    void test_flatten();

private:
    wam_interpreter interp;
};

}}

void test_wam_interpreter::test_flatten()
{
    term t = interp.parse("f(g(y,12,h(k),i(2)),m(X)).");
    auto fl = interp.flatten(t);
    interp.print_prims(fl);
}

static void test_flatten()
{
    header("test_flatten");

    test_wam_interpreter wam;
    wam.test_flatten();
}

static void test_instruction_sequence()
{
    header("test_instruction_sequence");

    wam_interpreter interp;
    wam_instruction_sequence instrs(interp);
    instrs.add(wam_instruction<PUT_VARIABLE_X>(1, 2));
    instrs.add(wam_instruction<PUT_VARIABLE_X>(3, 4));
    instrs.add(wam_instruction<PUT_VARIABLE_Y>(5, 6));
    instrs.add(wam_instruction<PUT_VALUE_X>(6, 7));
    instrs.add(wam_instruction<PUT_VALUE_Y>(8, 9));
    instrs.add(wam_instruction<PUT_UNSAFE_VALUE_Y>(10, 11));
    instrs.print(std::cout);
}

int main( int argc, char *argv[] )
{
    test_flatten();

    test_instruction_sequence();

    return 0;
}
