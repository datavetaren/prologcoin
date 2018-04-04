#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "../node/self_node.hpp"
#include "terminal.hpp"

using namespace prologcoin::node;

static std::string program_name;
static std::string home_dir;
unsigned short port = self_node::DEFAULT_PORT;

static void help()
{
    std::cout << std::endl;
    std::cout << "Usage: " << program_name << " --interactive [--port <number>]" << std::endl;
    std::cout << "  --interactive (non-interactive currently unavailable)" << std::endl;
    std::cout << "  --port <number> (start service on this port, default is " << self_node::DEFAULT_PORT << ")" << std::endl;
    // std::cout << "  --homedir <dir> (location of home directory, default userdir/" << program_name << ")" << std::endl;

    std::cout << std::endl;
    std::cout << "Example: " << program_name << " --interactive --port 8700" << std::endl;
    std::cout << std::endl;
}

static void start()
{
    std::cout << std::endl;

    self_node node(port);

    std::cout << "[" << program_name << " v" << self_node::VERSION_MAJOR << "." << self_node::VERSION_MINOR << "]" << std::endl << std::endl;

    node.start();

    prologcoin::main::terminal term(port);
    if (!term.connect()) {
	std::cout << "Couldn't connect to node." << std::endl;
    } else {
	term.run();
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

    std::string help_opt = get_option(args, "--help");
    if (!help_opt.empty()) {
	help();
	return 0;
    }

    std::string interactive_opt = get_option(args, "--interactive");
    if (interactive_opt.empty()) {
	std::cout << std::endl << program_name << ": --interactive is missing (see --help.)" << std::endl << std::endl;
	return 0;
    }

    std::string port_opt = get_option(args, "--port");
    if (!port_opt.empty()) {
	try {
	    port = boost::lexical_cast<unsigned short>(port_opt);
	} catch (boost::exception &ex) {
	    std::cout << std::endl << program_name << ": erroneous port: " << port_opt << std::endl << std::endl;
	}
    }
    // std::cout << "Dir     : " << home_dir << std::endl;

    start();

    return 0;
}
