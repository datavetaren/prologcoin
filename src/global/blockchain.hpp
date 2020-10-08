#pragma once

#ifndef _global_blockchain_hpp
#define _global_blockchain_hpp

#include "meta_entry.hpp"
#include <unordered_map>

namespace prologcoin { namespace global {

class blockchain {
public:
    blockchain(const std::string &data_dir);

    void init();

    static void erase_all(const std::string &data_dir);

    void flush_db();

    void increment_height();

    inline db::triedb & get_db_instance(std::unique_ptr<db::triedb> &var,
					const std::string &dir) {
        if (var.get() == nullptr) {
	    var = std::unique_ptr<db::triedb>(new db::triedb(dir));
        }
	return *var.get();
    }

    void update_meta_id();

    inline db::triedb & meta_db() {
        return get_db_instance(db_meta_, db_meta_dir_);
    }
    inline db::triedb & blocks_db() {
	return get_db_instance(db_blocks_, db_blocks_dir_);
    }
    inline db::triedb & heap_db() {
	return get_db_instance(db_heap_, db_heap_dir_);
    }
    inline db::triedb & closures_db() {
        return get_db_instance(db_closures_, db_closures_dir_);
    }
    inline db::triedb & symbols_db() {
        return get_db_instance(db_symbols_, db_symbols_dir_);      
    }
    inline db::triedb & program_db() {
        return get_db_instance(db_program_, db_program_dir_);
    }

    inline db_root_id heap_root() const {
	return tip_.get_root_id_heap();
    }

    inline db_root_id closures_root() const {
	return tip_.get_root_id_closures();
    }  

    inline db_root_id symbols_root() const {
	return tip_.get_root_id_symbols();
    }

    inline db_root_id program_root() const {
	return tip_.get_root_id_program();
    }

    inline meta_entry & tip() {
	return tip_;
    }

    inline const meta_entry & tip() const {
	return tip_;
    }

    inline size_t num_symbols() {
	return symbols_db().num_entries(tip().get_root_id_symbols());
    }

private:
    std::string data_dir_;

    std::string db_meta_dir_;
    std::string db_blocks_dir_;
    std::string db_heap_dir_;
    std::string db_closures_dir_;
    std::string db_symbols_dir_;  
    std::string db_program_dir_;

    std::unique_ptr<db::triedb> db_meta_;
    std::unique_ptr<db::triedb> db_blocks_;
    std::unique_ptr<db::triedb> db_heap_;
    std::unique_ptr<db::triedb> db_closures_;
    std::unique_ptr<db::triedb> db_symbols_;
    std::unique_ptr<db::triedb> db_program_;

    meta_entry tip_;
    std::unordered_map<meta_id, meta_entry> chains_;
};

}}

#endif
