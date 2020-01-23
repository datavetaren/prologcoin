#include <iostream>
#include <boost/filesystem.hpp>

#define TEST_INTERPRETER_FILES_UNSUED 1

#include "test_files_infrastructure.hpp"
#include "../interpreter.hpp"

using namespace prologcoin::interp;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

int main( int argc, char *argv[] )
{
    header( "test_load_save" );

    find_home_dir(argv[0]);

    const std::string dir = "/src/interp/test/pl_files";

    const std::string filename = "flightconn.pl";

    const std::string &home_dir = find_home_dir();
    auto src_file0 = boost::filesystem::path(home_dir) / dir / filename;
    auto src_file = boost::filesystem::path(home_dir) / "bin" / "test" /"interp" / (filename + ".load");
    auto gold_file = boost::filesystem::path(home_dir) / dir / (filename + ".save.gold");

    try {
      boost::filesystem::create_directories(src_file.parent_path());
      boost::filesystem::copy_file(src_file0, src_file,
		   boost::filesystem::copy_option::overwrite_if_exists);
    } catch (std::exception &ex) {
	std::cout << "Error: " << ex.what() << std::endl;
	return 1;
    }
    
    interpreter interp;
    interp.enable_file_io();
    interp.setup_standard_lib();
    std::ifstream ifs(src_file.string());
    interp.load_program(ifs);
    ifs.close();

    auto dst_file = boost::filesystem::path(home_dir) / "bin" / "test" / "interp" / (filename + ".save");
    std::ofstream ofs(dst_file.string());
    interp.save_program(interp.current_module(), ofs);
    ofs.close();

    // Compare with gold file
    assert(compare_files(dst_file.string(), gold_file.string()));

    return 0;
}
