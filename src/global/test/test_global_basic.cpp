#include <common/term_tools.hpp>
#include <global/global.hpp>

using namespace prologcoin::common;
using namespace prologcoin::global;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_global_basic()
{
    header("test_global_basic");

    global g;
    std::cout << "STATUS: " << g.env().status() << std::endl;
}

int main(int argc, char *argv[])
{
    test_global_basic();
    return 0;
}
