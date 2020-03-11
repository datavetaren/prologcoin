#include <boost/filesystem.hpp>
#include "global.hpp"

extern "C" void DebugBreak();

using namespace prologcoin::common;

namespace prologcoin { namespace global {

global::global(const std::string &data_dir)
    : data_dir_(data_dir),
      current_height_(0),
      db_heap_dir_((boost::filesystem::path(data_dir_) / "db" / "heap").string()),
      db_closures_dir_((boost::filesystem::path(data_dir_) / "db" / "closures").string()),
      db_symbols_dir_((boost::filesystem::path(data_dir_) / "db" / "symbols").string()),
      db_program_dir_((boost::filesystem::path(data_dir_) / "db" / "program").string()), 
      db_meta_dir_((boost::filesystem::path(data_dir_) / "db" / "meta").string()),
      block_flusher_(),
      block_cache_(BLOCK_CACHE_SIZE / heap_block::MAX_SIZE / sizeof(cell)),
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

void global::commit_heap()
{
    std::vector<heap_block*> blocks;
    for (auto &e : modified_blocks_) {
        auto *block = e.second;
	blocks.push_back(block);
    }
    std::sort(blocks.begin(), blocks.end(),
	   [](heap_block *a, heap_block *b) { return a->index() < b->index();});
    for (auto *block : blocks) {
	heap_block_as_custom_data view(*block);
	view.write();
	// Write on disk and move block to block cache
	heap_db().update(current_height(), block->index(),
			 view.custom_data(), view.custom_data_size());
	block->clear_changed();
	block_cache_.insert(block->index(), block);
    }
    modified_blocks_.clear();
}

void global::increment_height()
{
    commit_heap();
    current_height_++;
}
    
}}

