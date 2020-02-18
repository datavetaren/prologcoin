#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <common/random.hpp>
#include <node/self_node.hpp>
#include <node/session.hpp>
#include <wallet/wallet.hpp>
#include <terminal/terminal.hpp>
#include "../../interp/test/test_files_infrastructure.hpp"

using namespace prologcoin::common;
using namespace prologcoin::terminal;
using namespace prologcoin::node;
using namespace prologcoin::wallet;

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
    header( "test_wallet_pl_files" );

    random::set_for_testing(true);
    
    find_home_dir(argv[0]);
    full_mode = is_full(argc, argv);

    const char *name = find_name(argc, argv);

    const std::string dir = "/src/wallet/test/pl_files";

    // First start a node
    self_node self;
    self.start();

    // Connect a terminal to it
    terminal term(terminal::DEFAULT_PORT);
    while (!term.connect()) {
        utime::sleep(utime::ss(1));
    }

    // Connect a wallet to it (with no file path, so information isn't saved anywhere.)
    wallet w;
    w.connect_node(&term);
    w.load();

    if (argc == 2) {
        test_interpreter_files(dir, w.interp(), name);
    } else {
        test_interpreter_files(dir, w.interp());
    }

    self.stop();
    self.join();

    return 0;
}

