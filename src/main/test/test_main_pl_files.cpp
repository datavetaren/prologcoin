#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <common/random.hpp>
#include <node/self_node.hpp>
#include <node/session.hpp>
#include <main/meta_interpreter.hpp>
#include <terminal/terminal.hpp>
#include "../../interp/test/test_files_infrastructure.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;
using namespace prologcoin::terminal;
using namespace prologcoin::node;
using namespace prologcoin::main;
using namespace prologcoin::global;

std::string test_dir;

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
    header( "test_main_pl_files" );

    random::set_for_testing(true);
    
    std::string home_dir = find_home_dir(argv[0]);
    test_dir = (boost::filesystem::path(home_dir) / "bin" / "test" / "main").string();
    
    full_mode = is_full(argc, argv);

    const char *name = find_name(argc, argv);

    const std::string dir = "/src/main/test/pl_files";

    test_interpreter_files<meta_interpreter>(
	      dir,
	      [&](meta_interpreter &mi){mi.set_home_dir(test_dir);},
	      [&](const std::string &cmd) { return false; },
	      name);

    return 0;
}

