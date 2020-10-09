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

class global_interpreter : public interp::interpreter {
public:
    friend class global;

    using interperter_base = interp::interpreter_base;
    using term = common::term;
    using term_serializer = common::term_serializer;
    using buffer_t = common::term_serializer::buffer_t;

    global_interpreter(global &g);
    virtual ~global_interpreter() = default;

    void total_reset();

    global & get_global() { return global_; }

    void init_from_heap_db();
    void init_from_symbols_db();  
    void init_from_program_db();
    void new_heap();

    void commit_heap();
    void commit_symbols();
    void commit_program();
    void commit_closures();
  
    static void updated_predicate(interpreter_base &interp, const interp::qname &qn);
    static void load_predicate(interpreter_base &interp, const interp::qname &qn);
    static size_t unique_predicate_id(interpreter_base &interp, const common::con_cell module_name);
    virtual term get_frozen_closure(size_t addr) override;
    virtual void clear_frozen_closure(size_t addr) override;
    virtual void set_frozen_closure(size_t addr, term closure) override;
    virtual void get_frozen_closures(size_t from_addr, size_t to_addr,
				     size_t max_clousres,
	     std::vector<std::pair<size_t, term> > &closures) override;

    inline void updated_predicate(const interp::qname &qn) {
	auto p = internal_get_predicate(qn);
	if (p) {
	    old_predicates_.push_back(*p);
	}
        updated_predicates_.push_back(qn);
    }

    void load_predicate(const interp::qname &qn);

    size_t unique_predicate_id(const common::con_cell module);
  
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
  
    bool execute_goal(term t);
    bool execute_goal(buffer_t &serialized);
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

    static inline size_t call_new_atom(const common::heap &h, void *context, const std::string &atom_name)
    {
        auto *gi = reinterpret_cast<global_interpreter *>(context);
	return gi->new_atom(atom_name);
    }

    size_t new_atom(const std::string &atom_name);

    inline void modified_heap_block(common::heap_block &block)
    {
        // Won't delete block because of "changed" flag
        block_cache_.erase(block.index());

	// Reinsert this heap block in modified blocks
        modified_blocks_.insert(std::make_pair(block.index(), &block));
    }

    void discard_changes();

    common::heap_block * db_get_heap_block(size_t block_index);

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
	auto block = db_get_heap_block(block_index);
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

    std::unordered_map<size_t, common::heap_block *> modified_blocks_;
 
    std::vector<interp::qname> updated_predicates_;
    std::vector<interp::predicate> old_predicates_;

    // Use common::term() to indiciate removal of closure
    std::map<size_t, common::term> modified_closures_;

    size_t next_atom_id_;
    size_t start_next_atom_id_;
    std::vector<std::pair<size_t, std::string> > new_atoms_;

    size_t next_predicate_id_;
    size_t start_next_predicate_id_;
};

}}

#endif
