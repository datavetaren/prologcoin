#include <iostream>
#include <common/readline.hpp>

using namespace prologcoin::common;

int main( int argc, char *argv[] )
{
    if (argc == 2) {
	if (strcmp(argv[1], "-readline") == 0) {
	    readline rl;
	    rl.set_accept_ctrl_c(true);

	    bool cont = true;
	    rl.set_callback( [&](readline &rl, int ch){
		    if (ch == 3) {
			cont = false;
		    }
		    return rl.has_standard_handling(ch);
		});

	    while (cont) {
		std::cout << "?- ";
		std::cout.flush();
		std::string line = rl.read();
		std::cout << "\nGOT LINE: " << line << "\n";
		rl.add_history(line);
	    }

	    return 0;
	}
    }
    return 0;
}
