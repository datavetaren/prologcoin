#pragma once

#ifndef _global_global_hpp
#define _global_global_hpp

#include "../common/term_env.hpp"
#include "../common/term_serializer.hpp"
#include "../db/triedb.hpp"
#include "../db/util.hpp"
#include "global_interpreter.hpp"
#include <unordered_map>

namespace prologcoin { namespace global {

class global_exception : public std::runtime_error {
public:
    global_exception(const std::string &msg) : std::runtime_error(msg) { }
};

class global_db_exception : public global_exception {
public:
    global_db_exception(const std::string &msg) : global_exception(msg) { }
};
    
class heap_block_as_custom_data {
public:
    heap_block_as_custom_data(common::heap_block &blk) : block_(blk), custom_data_owner_(true), custom_data_(nullptr), custom_data_size_(0) { }
    heap_block_as_custom_data(common::heap_block &blk, uint8_t *custom_data, size_t custom_data_size) : block_(blk), custom_data_owner_(false), custom_data_(custom_data), custom_data_size_(custom_data_size)  { }
    ~heap_block_as_custom_data() { if (custom_data_owner_ && custom_data_) delete [] custom_data_; }

    inline const uint8_t * custom_data() const { return custom_data_; }
    inline size_t custom_data_size() const { return custom_data_size_; }
  
    inline void write() {
        auto n = block_.size();
        custom_data_size_ = sizeof(common::cell)*block_.size();
        if (custom_data_ == nullptr) custom_data_ = new uint8_t[custom_data_size_];
	auto *dst = custom_data_;
	const common::cell *src = block_.cells();
	for (size_t i = 0; i < n; i++, dst += sizeof(uint64_t), src++) {
	    db::write_uint64(dst, static_cast<uint64_t>(src->raw_value()));
	}
    }

    inline void read() {
        auto n = custom_data_size_ / sizeof(common::cell);
	common::cell *dst = block_.cells();
	const uint8_t *src = custom_data_;
	for (size_t i = 0; i < n; i++, dst++, src += sizeof(uint64_t)) {
	    *dst = common::cell(db::read_uint64(src));
	}
	block_.trim(n);
    }
    
private:
    common::heap_block &block_;
    bool custom_data_owner_;
    uint8_t *custom_data_;
    size_t custom_data_size_;
};

//
// global. This class captures the global state that everybody shares
// in the network.
class global {
private:
    using term_env = prologcoin::common::term_env;
    using term = prologcoin::common::term;
    using buffer_t = prologcoin::common::term_serializer::buffer_t;

    static const size_t MB = 1024*1024;
    static const size_t GB = 1024*MB;

    static const size_t BLOCK_CACHE_SIZE = 4*GB;

public:
    global(const std::string &data_dir);

    static inline common::heap_block & call_get_heap_block(common::heap &h, void *context, size_t block_index)
    {
        auto *g = reinterpret_cast<global *>(context);
	return g->get_heap_block(h, block_index);
    }

    static inline void call_modified_heap_block(common::heap_block &block, void *context)
    {
        auto *g = reinterpret_cast<global *>(context);
	return g->modified_heap_block(block);
    }

    inline void modified_heap_block(common::heap_block &block)
    {
        // Won't delete block because of "changed" flag
        block_cache_.erase(block.index());

	// Reinsert this heap block in modified blocks
        modified_blocks_.insert(std::make_pair(block.index(), &block));
    }

    inline common::heap_block & get_heap_block(common::heap &h, size_t block_index)
    {
        if (h.get_head_block() != nullptr &&
	    h.get_head_block()->index() == block_index) {
	    return *h.get_head_block();
        }
        if (block_index == common::heap::NEW_BLOCK) {
  	    size_t new_block_index = h.num_blocks() + 1;
  	    auto *new_block = new common::heap_block(h, new_block_index);
	    modified_blocks_.insert(std::make_pair(new_block_index, new_block));
	    h.set_head_block(new_block);
	    return *new_block;
        }
        auto it = modified_blocks_.find(block_index);
	if (it != modified_blocks_.end()) {
	    return *it->second;
	}
	auto *found = block_cache_.find(block_index);
	if (found != nullptr) {
	    return **found;
	}
        auto *leaf = heap_db().find(current_height(), block_index);
	common::heap_block *block = new common::heap_block(h, block_index);
	heap_block_as_custom_data view(*block, leaf->custom_data(),
				       leaf->custom_data_size());
	view.read();
	block_cache_.insert(block_index, block);
	return *block;
    }

    // All modified (pending) heap blocks are written
    void commit_heap();
    void erase_db();

    static void erase_db(const std::string &data_dir);
  
    inline term_env & env() { return interp_; }
    inline void set_naming(bool b) { interp_.set_naming(b); }

    inline static void setup_consensus_lib(interp::interpreter &interp) {
        global_interpreter::setup_consensus_lib(interp);
    }

    inline void reset() {
        return interp_.reset();
    }
  
    inline bool execute_goal(term t, bool and_undo) {
        return interp_.execute_goal(t, and_undo);
    }
    inline bool execute_goal(buffer_t &buf, bool and_undo) {
        return interp_.execute_goal(buf, and_undo);
    }
    inline void execute_cut() {
        interp_.execute_cut();
    }
    inline bool is_clean() const {
        bool r = interp_.is_empty_stack() && interp_.is_empty_trail();
	return r;
    }
    inline size_t heap_size() const {
        return interp_.heap_size();
    }
    inline size_t stack_size() const {
        return interp_.stack_size();
    }
    inline size_t trail_size() const {
        return interp_.trail_size();
    }

    inline size_t current_height() const {
        return current_height_;
    }

    void increment_height();

    inline void set_height(size_t h) {
        current_height_ = h;
    }

    inline global_interpreter & interp() {
        return interp_;
    }

    inline db::triedb & get_db_instance(std::unique_ptr<db::triedb> &var,
					const std::string &dir) {
        if (var.get() == nullptr) {
	    var = std::unique_ptr<db::triedb>(new db::triedb(dir));
        }
	return *var.get();
    }
  
private:
    std::string data_dir_;
    size_t current_height_;

    inline db::triedb & heap_db() {
        return get_db_instance(db_heap_, db_heap_dir_);
    }
    inline db::triedb & closures_db() {
        return get_db_instance(db_heap_, db_closures_dir_);
    }
    inline db::triedb & symbols_db() {
        return get_db_instance(db_symbols_, db_symbols_dir_);      
    }
    inline db::triedb & program_db() {
        return get_db_instance(db_program_, db_program_dir_);
    }
    inline db::triedb & meta_db() {
        return get_db_instance(db_meta_, db_meta_dir_);      
    }

    std::string db_heap_dir_;
    std::string db_closures_dir_;
    std::string db_symbols_dir_;  
    std::string db_program_dir_;
    std::string db_meta_dir_;  
  
    std::unique_ptr<db::triedb> db_heap_;
    std::unique_ptr<db::triedb> db_closures_;
    std::unique_ptr<db::triedb> db_symbols_;
    std::unique_ptr<db::triedb> db_program_;
    std::unique_ptr<db::triedb> db_meta_;

    std::unordered_map<size_t, common::heap_block *> modified_blocks_;

    struct block_flusher {
        void evicted(size_t, common::heap_block *block) {
	    if (!block->has_changed() && !block->is_head_block()) {
	        delete block;
	    }
        }
    };
    block_flusher block_flusher_;
    common::lru_cache<size_t, common::heap_block *, block_flusher> block_cache_;
  
    global_interpreter interp_;
};

}}

#endif
