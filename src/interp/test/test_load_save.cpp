#include <iostream>
#include <boost/filesystem.hpp>
#include "../../common/test/test_home_dir.hpp"
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

    const std::string &home_dir = find_home_dir();
    auto src_file0 = boost::filesystem::path(home_dir) / dir / "/ex_99_bitone_full.pl";
    auto src_file = boost::filesystem::path(home_dir) / "/bin/test/interp/ex_99_load.pl";

    try {
      boost::filesystem::create_directories(src_file.parent_path());
      boost::filesystem::copy_file(src_file0, src_file,
		   boost::filesystem::copy_option::overwrite_if_exists);
    } catch (std::exception &ex) {
	std::cout << "Error: " << ex.what() << std::endl;
	return 1;
    }
    
    interpreter interp;
    std::ifstream ifs(src_file.string());
    interp.load_program(ifs);
    ifs.close();

    auto dst_file = boost::filesystem::path(home_dir) / "/bin/test/interp/ex_99_save.pl";
    std::ofstream ofs(dst_file.string());
    interp.save_program(interp.current_module(), ofs);
    ofs.close();

    return 0;
}
