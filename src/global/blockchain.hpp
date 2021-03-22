#pragma once

#ifndef _global_blockchain_hpp
#define _global_blockchain_hpp

#include "meta_entry.hpp"
#include <unordered_map>

namespace prologcoin { namespace global {

class blockchain {
public:
    static const uint64_t VERSION = 1;
    
    blockchain(const std::string &data_dir);

    void init();

    static void erase_all(const std::string &data_dir);

    void flush_db();

    void set_version(uint64_t ver) {
	version_ = ver;
    }
    void set_nonce(uint64_t nonce) {
	nonce_ = nonce;
    }
    void set_time(common::utime &t) {
	time_ = t;
    }
    
    void advance();
    void update_tip();
    void add_meta_entry(meta_entry &e);

    inline db::triedb & get_db_instance(std::unique_ptr<db::triedb> &var,
					const std::string &dir) const {
        if (var.get() == nullptr) {
	    var = std::unique_ptr<db::triedb>(new db::triedb(dir));
        }
	return *var.get();
    }

    inline db::triedb & meta_db() {
        return get_db_instance(db_meta_, db_meta_dir_);
    }
    inline db::triedb & goal_blocks_db() {
	return get_db_instance(db_goal_blocks_, db_goal_blocks_dir_);
    }
    inline db::triedb & heap_db() {
	return get_db_instance(db_heap_, db_heap_dir_);
    }
    inline db::triedb & closures_db() {
        return get_db_instance(db_closures_, db_closures_dir_);
    }
    inline const db::triedb & closures_db() const {
        return get_db_instance(db_closures_, db_closures_dir_);
    }    
    inline db::triedb & symbols_db() {
        return get_db_instance(db_symbols_, db_symbols_dir_);      
    }

    inline db::triedb & program_db() {
        return get_db_instance(db_program_, db_program_dir_);
    }
    inline const db::triedb & program_db() const {
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

    inline db_root_id goal_blocks_root() const {
	return tip_.get_root_id_goal_blocks();
    }

    inline meta_entry & tip() {
	return tip_;
    }

    inline const meta_entry & tip() const {
	return tip_;
    }

    inline void set_tip(const meta_entry &e) {
	tip_ = e;
    }

    std::set<meta_id> find_entries(size_t height, const uint8_t *prefix, size_t prefix_len);
    std::set<meta_id> find_entries(const uint8_t *prefix, size_t prefix_len);

    inline const meta_entry * get_meta_entry(const meta_id &id) {
	auto it = chains_.find(id);
	if (it == chains_.end()) {
	    return nullptr;
	}
	return &(it->second);
    }

    inline const meta_id & previous(const meta_id &id) {
	static meta_id NONE;
	auto e = get_meta_entry(id);
	if (!e) {
	    return NONE;
	}
	return e->get_previous_id();
    }
    
    inline std::set<meta_id> follows(const meta_id &id) {
	static std::set<meta_id> EMPTY_SET;
	
	auto entry = get_meta_entry(id);
	if (entry == nullptr) {
	    return EMPTY_SET;
	}
	std::set<meta_id> s;
	for (auto next_id : at_height_[entry->get_height()+1]) {
	    if (auto next = get_meta_entry(next_id)) {
		if (next->get_previous_id() == id) {
		    s.insert(next_id);
		}
	    }
	}
	return s;
    }

    inline meta_id genesis() {
	auto it = at_height_.find(0);
	if (it == at_height_.end()) {
	    return meta_id();
	}
	auto &ids = it->second;
	if (ids.empty()) {
	    return meta_id();
	}
	return *ids.begin();
    }

    inline size_t num_symbols() {
	return symbols_db().num_entries(tip().get_root_id_symbols());
    }

private:
    void update_meta_id();

    std::string data_dir_;

    std::string db_meta_dir_;
    std::string db_goal_blocks_dir_;
    std::string db_heap_dir_;
    std::string db_closures_dir_;
    std::string db_symbols_dir_;  
    std::string db_program_dir_;

    mutable std::unique_ptr<db::triedb> db_meta_;
    mutable std::unique_ptr<db::triedb> db_goal_blocks_;
    mutable std::unique_ptr<db::triedb> db_heap_;
    mutable std::unique_ptr<db::triedb> db_closures_;
    mutable std::unique_ptr<db::triedb> db_symbols_;
    mutable std::unique_ptr<db::triedb> db_program_;

    meta_entry tip_;
    std::unordered_map<size_t, std::set<meta_id> > at_height_;
    std::map<meta_id, meta_entry> chains_;

    uint64_t version_;
    uint64_t nonce_;
    common::utime time_;
};

}}

#endif
