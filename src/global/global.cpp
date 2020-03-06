#include <boost/filesystem.hpp>
#include "global.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace global {

global::global(const std::string &data_dir) : data_dir_(data_dir), interp_(*this), current_height_(0) {
    db_heap_dir_ = (boost::filesystem::path(data_dir_) / "db" / "heap").string();
    db_symbols_dir_ = (boost::filesystem::path(data_dir_) / "db" / "symbols").string();

    db_program_dir_ = (boost::filesystem::path(data_dir_) / "db" / "program").string();
    db_meta_dir_ = (boost::filesystem::path(data_dir_) / "db" / "meta").string();  
}

void global::erase_db()
{
    auto dir_path = boost::filesystem::path(data_dir_) / "db";
    boost::system::error_code ec;
    boost::filesystem::remove_all(dir_path, ec);
    if (ec) {
        throw global_db_exception( "Failed while attempting to erase all files at " + dir_path.string() + "; " + ec.message());
    }
}

}}

