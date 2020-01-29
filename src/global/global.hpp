#pragma once

#ifndef _global_global_hpp
#define _global_global_hpp

#include "../common/term_env.hpp"
#include "../common/term_serializer.hpp"
#include "global_interpreter.hpp"

namespace prologcoin { namespace global {

//
// global. This class captures the global state that everybody shares
// in the network.
class global {
private:
    using term_env = prologcoin::common::term_env;
    using term = prologcoin::common::term;
    using buffer_t = prologcoin::common::term_serializer::buffer_t;
  
public:
    global();
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

    inline void increment_height() {
        current_height_++;
    }

    inline void set_height(size_t h) {
        current_height_ = h;
    }

    inline global_interpreter & interp() {
        return interp_;
    }
  
private:
    global_interpreter interp_;
    size_t current_height_;
};

}}

#endif
