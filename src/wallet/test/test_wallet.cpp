#include <common/test/test_home_dir.hpp>
#include <common/utime.hpp>
#include <wallet/wallet.hpp>
#include <terminal/terminal.hpp>

using namespace prologcoin::common;
using namespace prologcoin::wallet;
using namespace prologcoin::terminal;

std::string home_dir;
std::string wallet_home;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_wallet()
{
    header("test_wallet");
    
    std::cout << "Wallet file: " << wallet_home << std::endl;

    wallet w(wallet_home);
    w.load();

    w.print();
    
    std::cout << "WE GOT: " << w.execute("wallet:pubkey(1, X).") << std::endl;
}

int main(int argc, char *argv[])
{
    home_dir = find_home_dir(argv[0]);
    wallet_home = home_dir + "/src/wallet/test/wallet_test.pl";

    test_wallet();

    return 0;
}
