#include <common/test/test_home_dir.hpp>
#include <common/utime.hpp>
#include <wallet/wallet.hpp>
#include <terminal/terminal.hpp>
#include <node/self_node.hpp>
#include <boost/algorithm/string.hpp>

using namespace prologcoin::common;
using namespace prologcoin::wallet;
using namespace prologcoin::terminal;
using namespace prologcoin::node;

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

static void test_wallet_tx()
{
    header("test_wallet_tx");

    static struct key_t {
      const char *privkey;
      const char *pubkey;
      const char *address;
    } KEYS[] = {
      // Random key
      {"58'KyPk2t6cc7BBfM5iSpqAnrw5jZejeNcsor7Z2Ctjijkp7GBAGC8d",
       "58'1oPewjwXBpfRpPUZ7s63sQ6Y2KuSHvaQhxAJqQWpUjHGJ",
       "58'19WT1KKq5xNyiPEXFLiNey4WpTfSXVNHUQ"},
      // Key available in wallet
      {"58'KwRcaCKjB4mgCkiDEfnCWvBkyFwRoTmA4LLgoNMLDdY3scKdTJyc",
       "58'1e7d3bYBYcKwYxAYBBy5FaUTE76hw9j6dipaW8tFUrN1D)",
       "58'1NCQMx4LxfhHXLWYWAwk6zYxhN5qvJ614M"},
      // Another random key
      {"58'Kx4LU1ztP84ugymTy3cNMM4957Tm1hyXqdoH2akRjSKtm7DjmAb4",
       "58'1ecAXa5ZNxCBSnHciurXkki3TLy6VBvYWrAhHBJbnDM2Q",
       "58'1DJ5G7DMKsdcZjDJqJprqz3g4caP5ioCU9"},
      {nullptr, nullptr, nullptr}
    };
    
    // First start a node
    self_node self;
    self.start();

    // Connect a terminal to it
    terminal term(terminal::DEFAULT_PORT);
    while (!term.connect()) {
        utime::sleep(utime::ss(1));
    }

    // Connect a wallet to it
    wallet w(wallet_home);
    w.connect_node(&term);
    w.load();

    // Let's create a bunch of rewards in the global state
    // Let the keys rotate
    std::cout << "Adding rewards in global state..." << std::endl;
    for (size_t i = 0, j = 0; i < 1000; i++, j++) {
        if (KEYS[j].address == nullptr) j = 0;
	std::string cmd = "(me:commit(reward(";
	cmd += KEYS[j].address;
	cmd += "))) @ node.";
	std::string result = w.execute(cmd);
	boost::trim(result);
	if (result != "true") std::cout << "Unexpected result: " << result << std::endl;
        assert(result == "true");
    }
    std::cout << "Done" << std::endl;

    // Let's see the frozen closures we have...
    int64_t start_addr = -1;
    bool cont = false;
    do {
        auto query = w.parse("me:query(frozenk(" + boost::lexical_cast<std::string>(start_addr+1) + ",100, X)) @ node.");
	cont = w.execute(query);
	if (cont) {
	     auto lst = w.get_result_term("X");
	     cont = w.env().is_dotted_pair(lst);
	     while (w.env().is_dotted_pair(lst)) {
	         auto c = w.env().arg(lst,0);
	         auto key = static_cast<int_cell &>(c).value();
		 std::cout << key << ", ";
		 start_addr = key + 1;
	         lst = w.env().arg(lst, 1);
	     }
	     std::cout << std::endl;
	}
    } while (cont);
    
    std::cout << std::endl;

    term.close();
    self.stop();
    self.join();
}

int main(int argc, char *argv[])
{
    home_dir = find_home_dir(argv[0]);
    wallet_home = home_dir + "/src/wallet/test/wallet_test.pl";

    test_wallet();
    test_wallet_tx();

    return 0;
}
