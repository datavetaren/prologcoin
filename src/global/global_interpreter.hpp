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
};

class global_interpreter_exception : public interp::interpreter_exception {
public:
    global_interpreter_exception(const std::string &msg) :
	interpreter_exception(msg) { }
};

class global_interpreter : public interp::interpreter {
public:
    using interperter_base = interp::interpreter_base;
    using term = common::term;
    using term_serializer = common::term_serializer;
    using buffer_t = common::term_serializer::buffer_t;

    global_interpreter();

    inline void set_naming(bool b) { naming_ = b; }
  
    bool execute_goal(term t);
    bool execute_goal(buffer_t &serialized);
    void execute_cut();
  
    inline bool is_empty_stack() const {
        return !has_meta_context() &&
	       (e0() == nullptr) && (b() == nullptr);
    }
	
    inline bool is_empty_trail() const {
        return trail_size() == 0;
    }
private:
    bool naming_;
    std::unordered_map<std::string, term> name_to_term_;
};

}}

#endif
