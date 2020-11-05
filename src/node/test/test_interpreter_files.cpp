#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <node/self_node.hpp>
#include <node/session.hpp>
#include "../../interp/test/test_files_infrastructure.hpp"

using namespace prologcoin::node;
using namespace prologcoin::global;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}


static bool is_full(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++) {
	if (strcmp(argv[i], "-full") == 0) {
	    return true;
	}
    }
    return false;
}

static const char * find_name(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-",1) != 0) {
	    return argv[i];
        }
    }
    return nullptr;
}

int main( int argc, char *argv[] )
{
    header( "test_interpreter_files" );

    std::string home_dir = find_home_dir(argv[0]);
    full_mode = is_full(argc, argv);

    const char *name = find_name(argc, argv);

    const std::string dir = "/src/node/test/pl_files";

    std::string test_dir = (boost::filesystem::path(home_dir) / "bin" / "test" / "node" / "triedb").string();

    global::erase_db(test_dir);
    
    self_node node(test_dir);
    auto state = node.new_in_session(nullptr, true);
    auto &interp = state->interp();
    interp.ensure_initialized();
    interp.told_standard_output(); // Use normal std::cout as output

    test_interpreter_files(dir, interp, [](const std::string &){return false;}, name);

    return 0;
}

