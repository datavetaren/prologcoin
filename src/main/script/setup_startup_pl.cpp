#include <iostream>
#include <boost/filesystem.hpp>
#include <common/test/test_home_dir.hpp>

int main(int argc, char *argv[])
{
    boost::filesystem::path root(find_home_dir(argv[0]));
    boost::filesystem::path bin_path(argv[1]);

    auto src_file = root / "src" / "main" / "startup.pl";
    auto dst_file = bin_path / "prologcoin-data" / "startup.pl";

    std::cout << "Copy file: " << src_file.string() << std::endl;
    std::cout << "       to: " << dst_file.string() << std::endl;

    try {
      boost::filesystem::create_directories(dst_file.parent_path());
      boost::filesystem::copy_file(src_file, dst_file,
		   boost::filesystem::copy_option::overwrite_if_exists);
    } catch (std::exception &ex) {
      std::cout << "Failed: " << ex.what() << std::endl;
      return 1;
    }

    return 0;
}
