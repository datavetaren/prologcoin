#include <boost/filesystem.hpp>
#include <common/random.hpp>
#include <common/test/test_home_dir.hpp>
#include <common/term_tools.hpp>
#include <global/global.hpp>

using namespace prologcoin::common;
using namespace prologcoin::global;

std::string home_dir;
std::string test_dir;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_global_basic()
{
    header("test_global_basic");

    global g(test_dir);

    std::cout << "Data directory: " << test_dir << std::endl;
    
    std::cout << "STATUS: " << g.env().status() << std::endl;
}

int main(int argc, char *argv[])
{
    home_dir = find_home_dir(argv[0]);
    test_dir = home_dir;
    test_dir = (boost::filesystem::path(test_dir) / "bin" / "test" / "global" / "triedb").string();

    random::set_for_testing(true);
  
    test_global_basic();
    return 0;
}
