#include "triedb.hpp"
#include <boost/filesystem/path.hpp>

namespace prologcoin { namespace db {

triedb::triedb(const std::string &dir_path) : dir_path_(dir_path) {
}

std::string triedb::roots_file_path() const {
    auto file_path = boost::filesystem::path(dir_path_) / "roots.bin";
    return file_path.string();
}
    
}}
