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

int main( int argc, char *argv[] )
{
    test_flatten();

    return 0;
}
