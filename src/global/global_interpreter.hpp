#pragma once

#ifndef _global_global_interpreter_hpp
#define _global_global_interpreter_hpp

#include "../common/term_env.hpp"
#include "../common/term_serializer.hpp"
#include "../interp/interpreter.hpp"

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
    using interperter_base = interp::interpreter_base;
    using term = common::term;
    using term_serializer = common::term_serializer;
    using buffer_t = common::term_serializer::buffer_t;

    global_interpreter(global &g);

    global & get_global() { return global_; }

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
    void setup_builtins();

    global &global_;
    bool naming_;
    std::unordered_map<std::string, term> name_to_term_;

    common::heap_block *current_block_;
    size_t current_block_index_;
};

}}

#endif
