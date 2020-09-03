#pragma once

#ifndef _global_global_interpreter_hpp
#define _global_global_interpreter_hpp

#include "../common/term_env.hpp"
#include "../common/term_serializer.hpp"
#include "../interp/interpreter.hpp"
#include "../db/util.hpp"
#include "../db/triedb.hpp"

namespace prologcoin { namespace global {

class global_interpreter;
    
class global_builtins {
public:
    using interpreter_base = interp::interpreter_base;
    using meta_context = interp::meta_context;
    using meta_reason_t = interp::meta_reason_t;

    using term = common::term;

    static global_interpreter & to_global(interpreter_base &interp)
    { return reinterpret_cast<global_interpreter &>(interp); }

    static bool operator_clause_2(interpreter_base &interp, size_t arity, term args[]);
};

class global_interpreter_exception : public interp::interpreter_exception {
public:
    global_interpreter_exception(const std::string &msg) :
	interpreter_exception(msg) { }
};

class global;

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

class global_interpreter : public interp::interpreter {
public:
    using interperter_base = interp::interpreter_base;
    using term = common::term;
    using term_serializer = common::term_serializer;
    using buffer_t = common::term_serializer::buffer_t;

    global_interpreter(global &g);

    void total_reset();

    global & get_global() { return global_; }

    void init_from_heap_db();
    void init_from_symbols_db();  
    void init_from_program_db();

    void commit_heap();
    void commit_symbols();
    void commit_program();
    void commit_closures();
  
    static void updated_predicate(interpreter_base &interp, const interp::qname &qn);
    virtual term get_frozen_closure(size_t addr) override;
    virtual void clear_frozen_closure(size_t addr) override;
    virtual void set_frozen_closure(size_t addr, term closure) override;
    virtual void get_frozen_closures(size_t from_addr, size_t to_addr,
				     size_t max_clousres,
	     std::vector<std::pair<size_t, term> > &closures) override;

    inline void updated_predicate(const interp::qname &qn) {
        updated_predicates_.push_back(qn);
    }
  
    inline void set_current_block(common::heap_block *b, size_t block_index) {
        current_block_ = b;
	current_block_index_ = block_index;
    }
    inline bool has_block_changed(size_t block_index) const {
        return block_index != current_block_index_;
    }
    inline common::heap_block & get_current_block() {
        return *current_block_;
    }
    inline size_t get_current_block_index() {
        return current_block_index_;
    }
  
    static void setup_consensus_lib(interpreter &interp);
  
    inline void set_naming(bool b) { naming_ = b; }
  
    bool execute_goal(term t, bool and_undo);
    bool execute_goal(buffer_t &serialized, bool and_undo);
    void execute_cut();
  
    inline bool is_empty_stack() const {
        bool r = !has_meta_context() &&
	       (e0() == nullptr) && (b() == nullptr);
	if (!r) {
	    std::cout << "HAS META: " << has_meta_context() << std::endl;
	    std::cout << "E0: " << e0() << std::endl;
	    std::cout << "B: " << b() << std::endl;
	    std::cout << "B0: " << b0() << std::endl;
	}
	return r;
    }
	
    inline bool is_empty_trail() const {
        return trail_size() == 0;
    }

    friend class global_builtins;
  
private:
    void init();

    static inline common::heap_block & call_get_heap_block(common::heap &h, void *context, size_t block_index)
    {
        auto *gi = reinterpret_cast<global_interpreter *>(context);
	return gi->get_heap_block(block_index);
    }

    static inline void call_modified_heap_block(common::heap_block &block, void *context)
    {
        auto *gi = reinterpret_cast<global_interpreter *>(context);
	gi->modified_heap_block(block);
    }

    static inline void call_new_atom(const common::heap &h, void *context, const std::string &atom_name, size_t atom_index)
    {
        auto *gi = reinterpret_cast<global_interpreter *>(context);
	gi->new_atom(atom_name, atom_index);
    }

    inline void new_atom(const std::string &atom_name, size_t atom_index)
    {
        new_atoms_.push_back(std::make_pair(atom_name, atom_index));
    }
  
    inline void modified_heap_block(common::heap_block &block)
    {
        // Won't delete block because of "changed" flag
        block_cache_.erase(block.index());

	// Reinsert this heap block in modified blocks
        modified_blocks_.insert(std::make_pair(block.index(), &block));
    }

    db::triedb_leaf * find_block_db(size_t block_index);

    inline common::heap_block & get_heap_block(size_t block_index)
    {
        if (current_block_index_ == block_index) {
	    return *current_block_;
        }
        if (block_index == common::heap::NEW_BLOCK) {
	    size_t new_block_index;
  	    if (get_head_block() == nullptr) {
	        new_block_index = 0;
	    } else {
	        new_block_index = num_blocks() + 1;
	    }
  	    auto *new_block = new common::heap_block(*this, new_block_index);
	    modified_blocks_.insert(std::make_pair(new_block_index, new_block));
	    set_head_block(new_block);
	    current_block_ = new_block;
	    current_block_index_ = new_block_index;
	    return *new_block;
        }
	current_block_index_ = block_index;
        auto it = modified_blocks_.find(block_index);
	if (it != modified_blocks_.end()) {
	    current_block_ = it->second;
	    return *current_block_;
	}
	auto *found = block_cache_.find(block_index);
	if (found != nullptr) {
  	    current_block_ = *found;
	    return *current_block_;
	}
        auto *leaf = find_block_db(block_index);
	common::heap_block *block = new common::heap_block(*this, block_index);
	heap_block_as_custom_data view(*block, leaf->custom_data(),
				       leaf->custom_data_size());
	view.read();
	block_cache_.insert(block_index, block);
	current_block_ = block;
	return *block;
    }

    void setup_builtins();

    global &global_;
    bool naming_;
    std::unordered_map<std::string, term> name_to_term_;

    size_t current_block_index_;
    common::heap_block *current_block_;

    struct block_flusher {
        void evicted(size_t, common::heap_block *block) {
	    if (!block->has_changed() && !block->is_head_block()) {
	        delete block;
	    }
        }
    };
    block_flusher block_flusher_;
    common::lru_cache<size_t, common::heap_block *, block_flusher> block_cache_;

    std::vector<std::pair<std::string, size_t> > new_atoms_;
  
    std::unordered_map<size_t, common::heap_block *> modified_blocks_;
 
    std::vector<interp::qname> updated_predicates_;

    // Use common::term() to indiciate removal of closure
    std::map<size_t, common::term> modified_closures_;
};

}}

#endif
