#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "../node/self_node.hpp"
#include "interactive_prompt.hpp"
#include "../common/readline.hpp"
#include "../wallet/wallet.hpp"

using namespace prologcoin::node;
using namespace prologcoin::wallet;

static std::string program_name;
static std::string home_dir;
unsigned short port = self_node::DEFAULT_PORT;
static std::string name;
static std::string dir;
static bool is_wallet = false;

static void help()
{
    std::cout << std::endl;
    std::cout << "Usage: " << program_name << " --interactive(-wallet) [--port <number>]" << std::endl;
    std::cout << "  --interactive (non-interactive currently unavailable)" << std::endl;
    std::cout << "  --interactive-wallet (will start both the node and a walet " << std::endl;
    std::cout << "                        connecting to that node.)" << std::endl;
    std::cout << "  --port <number> (start service on this port, default is " << self_node::DEFAULT_PORT << ")" << std::endl;
    std::cout << "  --name <string> (set friendly name on node, default is noname)" << std::endl;
    std::cout << "  --datadir <dir> (location of data directory)" << std::endl;

    std::cout << std::endl;
    std::cout << "Example: " << program_name << " --interactive --port 8700" << std::endl;
    std::cout << std::endl;
}

static void start()
{
    std::cout << std::endl;

    self_node node(port);
    std::shared_ptr<wallet> wallet_ptr;

    if (!name.empty()) {
	node.set_name(name);
    }

    std::cout << "[" << program_name << " v" << self_node::VERSION_MAJOR << "." << self_node::VERSION_MINOR << "]" << std::endl;
    std::cout << "Data directory: " << dir << std::endl;
    std::cout << std::endl;

    node.set_data_directory(dir);
    
    node.start();

    prologcoin::main::interactive_prompt prompt;
    if (!prompt.connect_node(port)) {
        std::cout << "Couldn't connect to node." << std::endl;
	return;
    }

    if (is_wallet) {
        std::string wallet_file = dir;
        wallet_file += boost::filesystem::path::preferred_separator;
	wallet_file += "wallet.pl";
	std::cout << "[In wallet mode using: " << wallet_file << "]" << std::endl;
        wallet_ptr = std::shared_ptr<wallet>(new wallet(wallet_file));
	prompt.connect_wallet(wallet_ptr);
	wallet_ptr->connect_node(prompt.node_terminal());
    }
    
    prompt.run();

    if (is_wallet) {
	wallet_ptr.reset();
    }

    node.stop();
    node.join();
}

typedef std::vector<std::string> args_t;

const std::string & get_option(args_t &args, const std::string &name)
{
    using namespace boost;

    static const std::string empty;
    static const std::string tr("true");
    for (size_t i = 0; i < args.size(); i++) {
	if (args[i] == name) {
	    if (i == args.size()-1 || boost::starts_with(args[i+1], "--")) {
		return tr;
	    } else {
		return args[i+1];
	    }
	}
    }
    return empty;
}

int main(int argc, char *argv[])
{
    auto path = boost::filesystem::path(argv[0]);
    program_name = path.filename().string();
    home_dir = path.parent_path().string();

    if (argc <= 1) {
	help();
	return 0;
    }

    args_t args;
    for (int i = 0; i < argc; i++) {
	args.push_back(argv[i]);
    }

    std::string checkkey_opt = get_option(args, "--check_key");
    if (!checkkey_opt.empty()) {
        prologcoin::common::readline::check_key();
	return 0;
    }
    
    std::string help_opt = get_option(args, "--help");
    if (!help_opt.empty()) {
	help();
	return 0;
    }

    std::string interactive_opt = get_option(args, "--interactive");
    std::string interactive_wallet_opt = get_option(args, "--interactive-wallet");
    
    if (interactive_wallet_opt.empty() && interactive_opt.empty()) {
	std::cout << std::endl << program_name << ": --interactive or --interactive-wallet is missing (see --help.)" << std::endl << std::endl;
	return 0;
    }

    is_wallet = !interactive_wallet_opt.empty();

    std::string port_opt = get_option(args, "--port");
    if (!port_opt.empty()) {
	try {
	    port = boost::lexical_cast<unsigned short>(port_opt);
	} catch (boost::exception &ex) {
	    std::cout << std::endl << program_name << ": erroneous port: " << port_opt << std::endl << std::endl;
	}
    }

    std::string name_opt = get_option(args, "--name");
    if (!name_opt.empty()) {
	name = name_opt;
    }

    auto bdir = boost::filesystem::path(home_dir);
    bdir /= "prologcoin-data";
    dir = bdir.string();
    std::string dir_opt = get_option(args, "--dir");
    if (!dir_opt.empty()) {
        dir = dir_opt;
    }

    start();

    return 0;
}
