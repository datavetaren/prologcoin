#include "../../interp/test/test_files_infrastructure.hpp"
#include "../builtins.hpp"
#include "../../common/random.hpp"

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

    find_home_dir(argv[0]);
    full_mode = is_full(argc, argv);

    // Make random deterministic (for testing only)
    random::set_for_testing(true);
    
    const char *name = find_name(argc, argv);

    const std::string dir = "/src/ec/test/pl_files";

    if (argc == 2) {
	test_interpreter_files(dir, [](interpreter &i){prologcoin::ec::builtins::load(i);}, name);
    } else {
	test_interpreter_files(dir, [](interpreter &i){prologcoin::ec::builtins::load(i);});
    }

    return 0;
}

