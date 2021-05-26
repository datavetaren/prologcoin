#pragma once

#ifndef _global_global_hpp
#define _global_global_hpp

#include "../common/term_env.hpp"
#include "../common/term_serializer.hpp"
#include "../common/fast_hash.hpp"
#include "../common/checked_cast.hpp"
#include "../db/triedb.hpp"
#include "../db/util.hpp"
#include "global_interpreter.hpp"
#include "blockchain.hpp"
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

class global_db_no_interpreter_exception : public global_db_exception {
public:
    global_db_no_interpreter_exception(const std::string &msg) : global_db_exception(msg) {
    }
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

    bool pow_check_{true};

public:
    static const size_t BLOCK_CACHE_SIZE = 4*GB;

    inline const std::string & data_dir() { return data_dir_; }

    inline bool check_pow() const {
	return pow_check_;
    }
    
    inline void set_check_pow(bool b) {
	pow_check_ = b;
    }
    
    global(const std::string &data_dir);

    // Reinitializes interpreter from a different state
    bool set_tip(const meta_id &id);

    const meta_id & tip_id() const {
	return blockchain_.tip().get_id();
    }

    void total_reset();

    void go_debug();
  
    static void erase_db(const std::string &data_dir);
  
    inline term_env & env() { return *interp_; }
    inline void set_naming(bool b) { interp_->set_naming(b); }

    inline static void setup_consensus_lib(interp::interpreter &interp) {
        global_interpreter::setup_consensus_lib(interp);
    }

    inline bool has_interp() const {
	return interp_ != nullptr;
    }

    inline void reset() {
	if (interp_) return interp_->reset();
    }

    inline void check_interp() const {
	if (!interp_) {
	    throw global_db_no_interpreter_exception( "No global interpreter available due to missing state");
	}
    }

    inline bool execute_goal(term t) {
	check_interp();
        return interp_->execute_goal(t);
    }

    inline bool execute_goal(buffer_t &buf, bool silent) {
	check_interp();
        return interp_->execute_goal(buf, silent);
    }
    
    inline bool execute_goal_silent(const buffer_t &buf) {
	check_interp();
        return interp_->execute_goal(const_cast<buffer_t &>(buf), true);
    }

    void init_empty_goals();

    void setup_commit(const buffer_t &buf);
    bool execute_commit(const buffer_t &buf);

    inline void execute_cut() {
	check_interp();
        interp_->execute_cut();
    }
    inline bool is_clean() const {
	check_interp();
        bool r = interp_->is_empty_stack() && interp_->is_empty_trail();
	return r;
    }
    inline size_t heap_size() const {
	check_interp();
        return interp_->heap_size();
    }
    inline size_t stack_size() const {
	check_interp();
        return interp_->stack_size();
    }
    inline size_t trail_size() const {
	check_interp();
        return interp_->trail_size();
    }

    size_t current_height() const;
    size_t max_height() const;
    void increment_height();

    void advance() {
	check_interp();
	increment_height();
    }

    void discard() {
	check_interp();
	interp_->discard_changes();
    }

    blockchain & get_blockchain() {
	return blockchain_;
    }

    inline global_interpreter & interp() {
	check_interp();	
        return *interp_;
    }

    common::heap_block * db_get_heap_block(size_t block_index) {
	auto leaf = blockchain_.heap_db().find( blockchain_.heap_root(),
						block_index );
	common::heap_block *block = new common::heap_block(env(), block_index);
	custom_data_to_heap_block(leaf->custom_data(),
				  leaf->custom_data_size(), *block);
	return block;
    }

    void db_set_heap_block(size_t block_index, common::heap_block *block) {
	uint8_t custom_data[common::heap_block::MAX_SIZE*sizeof(common::cell)];
	size_t custom_data_size = 0;
	heap_block_to_custom_data(*block, custom_data, custom_data_size);
	blockchain_.heap_db().update(blockchain_.heap_root(), block_index,
				     custom_data, custom_data_size);
    }  

    //
    // Symbols are stored as
    //     symbol_id         => symbol_name
    // AND hash(symbol_name) | 0x=> symbol_id
    //
    // Number of symbols = num entries / 2
    //

    size_t db_num_symbols() {
	if (blockchain_.symbols_db().is_empty()) {
	    return 0;
	}
	if (blockchain_.symbols_root().is_zero()) {
	    return 0;
	}
	return common::checked_cast<size_t>(blockchain_.symbols_db().num_entries(blockchain_.symbols_root())) / 2;
    }

    size_t num_symbols() {
	return interp().num_symbols();
    }

    size_t db_symbol_entry_to_custom_data(uint8_t *buffer,
					  size_t symbol_index,
					  const std::string &symbol_name) {
	uint8_t *p = buffer;
	db::write_uint32(p, common::checked_cast<uint32_t>(symbol_index));
	p += sizeof(uint32_t);
	db::write_uint32(p, common::checked_cast<uint32_t>(symbol_name.size()));
	p += sizeof(uint32_t);
	memcpy(p, symbol_name.c_str(), symbol_name.size());
	return 2*sizeof(uint32_t)+symbol_name.size();
    }

    std::pair<size_t, std::string> db_custom_data_to_symbol_entry(const uint8_t *buffer) {
	const uint8_t *p = buffer;
	size_t symbol_index = db::read_uint32(p); p += sizeof(uint32_t);
	size_t str_len = db::read_uint32(p); p += sizeof(uint32_t);
	assert(str_len < common::con_cell::MAX_NAME_LENGTH);
	std::string symbol_name(reinterpret_cast<const char *>(p), str_len);
	return std::make_pair(symbol_index, symbol_name);
    }

    size_t db_symbol_entry_to_custom_data(size_t symbol_index,
					  const std::string &symbol_name,
					  uint8_t *buffer) {
	uint8_t *p = buffer;
	db::write_uint32(p, common::checked_cast<uint32_t>(symbol_index));
	p += sizeof(uint32_t);
	db::write_uint32(p, common::checked_cast<uint32_t>(symbol_name.size()));
	p += sizeof(uint32_t);
	memcpy(p, symbol_name.c_str(), symbol_name.size());
	return 2*sizeof(uint32_t) + symbol_name.size();
    }

    std::string db_get_symbol_name(size_t index) {
	auto leaf = blockchain_.symbols_db().find(
			  blockchain_.symbols_root(), index );
	if (leaf == nullptr) {
	    return "";
	}
	size_t symbol_index;
	std::string symbol_name;
	std::tie(symbol_index, symbol_name) = db_custom_data_to_symbol_entry(leaf->custom_data());
	assert(symbol_index == index);
	return symbol_name;
    }

    size_t db_get_symbol_index(const std::string &name) {
	if (blockchain_.symbols_root().is_zero()) {
	    return 0;
	}
	// Maximum 10 hash collisions before we give up
	for (size_t i = 0; i < 10; i++) {
	    common::fast_hash h;
	    h << name << i;
	    uint32_t key = (h.finalize() & ~(1 << 29)) | (1 << 29);
	    auto leaf = blockchain_.symbols_db().find(
				      blockchain_.symbols_root(), key);
	    if (leaf == nullptr) {
		return 0;
	    }
	    size_t symbol_index;
	    std::string symbol_name;
	    std::tie(symbol_index, symbol_name) = db_custom_data_to_symbol_entry(leaf->custom_data());
	    if (symbol_name == name) {
		return symbol_index;
	    }
	}
	return 0;
    }

    bool db_set_symbol_index(size_t index, const std::string &name) {
	// Maximum 10 hash collisions before we give up
	size_t i;
	uint32_t key;
	for (i = 0; i < 10; i++) {
	    common::fast_hash h;
	    h << name << i;
	    key = (h.finalize() & ~(1 << 29)) | (1 << 29);
	    auto leaf = blockchain_.symbols_db().find(
				      blockchain_.symbols_root(), key);
	    if (leaf == nullptr) {
		break;
	    }
	    size_t n = leaf->custom_data_size();
	    const uint8_t *p = leaf->custom_data();
	    p += sizeof(uint32_t); /* Skip index */
	    size_t str_len = db::read_uint32(p); p += sizeof(uint32_t);
	    assert(n == 2*sizeof(uint32_t)+str_len);
	    std::string str(reinterpret_cast<const char *>(p), str_len);
	    if (name == str) {
		break;
	    }
	}
	if (i == 10) {
	    // We failed (because too many hash collisions)
	    return false;
	}
	uint8_t custom_data[common::con_cell::MAX_NAME_LENGTH+2*sizeof(uint32_t)];
	size_t custom_data_size = db_symbol_entry_to_custom_data(index, name,
								 custom_data);
	blockchain_.symbols_db().update(
		blockchain_.symbols_root(), key,
		custom_data, custom_data_size);

	blockchain_.symbols_db().update(
		blockchain_.symbols_root(), index,
		custom_data, custom_data_size);

	return true;
    }

    size_t db_num_predicates() const {
	if (blockchain_.program_db().is_empty()) {
	    return 0;
	}
	if (blockchain_.program_root().is_zero()) {
	    return 0;
	}
	return common::checked_cast<size_t>(blockchain_.program_db().num_entries(blockchain_.program_root()));
    }

    size_t num_predicates() {
	check_interp();
	return interp().num_predicates();
    }

    size_t db_num_frozen_closures() const {
	if (blockchain_.closure_db().is_empty()) {
	    return 0;
	}
	return common::checked_cast<size_t>(blockchain_.closure_db().num_entries(blockchain_.closure_root()));
    }

    size_t num_frozen_closures() {
	check_interp();
	return interp().num_frozen_closures();
    }

    bool db_get_predicate(const interp::qname &qn,
			  std::vector<common::term> &clauses);

    bool db_set_predicate(const interp::qname &qn,
			  const std::vector<common::term> &clauses);

    //
    // Closures
    //

    term db_get_closure(size_t addr) {
	auto leaf = blockchain_.closure_db().find(blockchain_.closure_root(),
						   addr);
	if (leaf == nullptr) {
	    return common::heap::EMPTY_LIST;
	}
	assert(leaf->custom_data_size() == sizeof(uint64_t));
	return common::cell(db::read_uint64(leaf->custom_data()));
    }

    void db_remove_closure(size_t addr) {
	blockchain_.closure_db().remove(blockchain_.closure_root(), addr);
    }

    void db_set_closure(size_t addr, term t) {
	uint8_t buffer[sizeof(uint64_t)];
	db::write_uint64(buffer, t.raw_value());
	blockchain_.closure_db().update(blockchain_.closure_root(), addr,
					 buffer, sizeof(buffer));
    }

    //
    // Goal block (these corresponds to "blocks" in traditional
    // cryptocurrencies.) I renamed them to goals to match the
    // terminology in Prolog. These are the things that we append
    // to the query.
    //
    
    term db_get_block(common::term_env &dst, const meta_id &id, bool raw);
    term db_get_block(common::term_env &dst, const meta_entry &e, bool raw);
    void db_set_block(size_t height, const buffer_t &buf);

    term db_get_meta(common::term_env &dst, const meta_id &id, bool more);
    size_t db_get_meta_length(const meta_id &root_id, size_t lookahead_n);
    term db_get_meta_roots(common::term_env &dst, const meta_id &id, size_t spacing, size_t n);
    bool db_put_meta(common::term_env &src, common::term meta_term);
    term db_get_metas(common::term_env &dst, const meta_id &id, size_t n);
    term db_best_path(common::term_env &dst, const meta_id &id, size_t n);
    void db_put_metas(common::term_env &src, common::term meta_terms);
    bool db_parse_meta(common::term_env &src, common::term meta_term, meta_entry &out);

private:
    void custom_data_to_heap_block(const uint8_t *custom_data,
				   size_t custom_data_size,
				   common::heap_block &blk) {
	auto n = custom_data_size / sizeof(common::cell);
	common::cell *dst = blk.cells();
	const uint8_t *src = custom_data;
	for (size_t i = 0; i < n; i++, dst++, src += sizeof(uint64_t)) {
	    *dst = common::cell(db::read_uint64(src));
	}
	blk.trim(n);
    }

    void heap_block_to_custom_data(common::heap_block &blk,
				   uint8_t *custom_data,
				   size_t &custom_data_size) {
	auto n = blk.size();
	custom_data_size = sizeof(common::cell)*n;
	auto *dst = custom_data;
	const common::cell *src = blk.cells();
	for (size_t i = 0; i < n; i++, dst += sizeof(uint64_t), src++) {
	    db::write_uint64(dst, static_cast<uint64_t>(src->raw_value()));
	}
    }

    void init();

    std::string data_dir_;
    blockchain blockchain_;
    std::unique_ptr<global_interpreter> interp_;
    uint64_t commit_version_;
    uint32_t commit_height_;
    uint64_t commit_nonce_;
    common::utime commit_time_;
    buffer_t commit_goals_;
};

}}

#endif
