#include <boost/filesystem.hpp>
#include "global.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

namespace prologcoin { namespace global {

global::global(const std::string &data_dir)
    : data_dir_(data_dir),
      db_heap_dir_((boost::filesystem::path(data_dir_) / "db" / "heap").string()),
      db_closures_dir_((boost::filesystem::path(data_dir_) / "db" / "closures").string()),
      db_symbols_dir_((boost::filesystem::path(data_dir_) / "db" / "symbols").string()),
      db_program_dir_((boost::filesystem::path(data_dir_) / "db" / "program").string()), 
      db_meta_dir_((boost::filesystem::path(data_dir_) / "db" / "meta").string()),
      current_height_(heap_db().total_height()),
      interp_(*this) {
}

void global::erase_db(const std::string &data_dir)
{
    auto dir_path = boost::filesystem::path(data_dir) / "db";
    boost::system::error_code ec;
    boost::filesystem::remove_all(dir_path, ec);
    if (ec) {
        throw global_db_exception( "Failed while attempting to erase all files at " + dir_path.string() + "; " + ec.message());
    }
}

void global::erase_db()
{
    erase_db(data_dir_);
}

void global::increment_height()
{
    interp().commit_heap();
    interp().commit_symbols();
    interp().commit_program();
    current_height_++;
}
    
}}

