#pragma once

#ifndef _interp_interpreter_hpp
#define _interp_interpreter_hpp

#include <iostream>
#include "wam_interpreter.hpp"

namespace prologcoin { namespace interp {

class test_wam_compiler;
class test_wam_interpreter;

class interpreter;

struct no_processing {
    no_processing(interpreter &interp) { }
    void operator() (common::term clause) { }
};
    
template<typename Pre = no_processing, typename Post = no_processing>
struct standard_clause_processing {
    standard_clause_processing(interpreter &interp) : interp_(interp), pre_(interp), post_(interp) { }
    void operator () (common::term clause);
private:
    interpreter &interp_;
    Pre pre_;
    Post post_;
};
    
class interpreter : public wam_interpreter
{
public:
    interpreter();
    ~interpreter();

    void setup_standard_lib();

    template<typename Pre = no_processing, typename Post = no_processing> void load_program(const std::string &str) {
        standard_clause_processing<Pre,Post> process(*this);
	interpreter_base::load_program(str, process);
    }

    template<typename Pre = no_processing, typename Post = no_processing> void load_program(std::istream &is) {
        standard_clause_processing<Pre,Post> process(*this);
	interpreter_base::load_program(is, process);
    }

    template<typename Pre = no_processing, typename Post = no_processing> void load_program(term clauses) {
        standard_clause_processing<Pre,Post> process(*this);
	interpreter_base::load_program(clauses, process);
    }

    template<typename Pre = no_processing, typename Post = no_processing> void load_program(term clauses, con_cell &primary_module) {
        standard_clause_processing<Pre,Post> process(*this);
	interpreter_base::load_program(clauses, process, primary_module);
    }
  
    void new_instance();
    size_t num_instances() const { return num_instances_; }
    void delete_instance();

    bool execute(const term query);
    bool next();
    bool cont();
    void fail();

    inline common::term query() const { return qr(); }

    class binding {
    public:
	binding() { }
	binding(const std::string &name, const common::term value) 
	    : name_(name), value_(value) { }
	binding(const binding &other)
            : name_(other.name_), value_(other.value_) { }

	inline const std::string & name() const { return name_; }
	inline const common::term value() const { return value_; }

    private:
	std::string name_;
	common::term value_;
    };

    inline const std::vector<binding> & query_vars() const
    { return *query_vars_; }

    common::term query_var_list();

    inline bool has_more() const
    { if (top_b() != b()) return true;
      if (!has_meta_context()) {
	  return false;
      }
      return get_current_meta_context()->fn != interpreter::new_instance_meta;
    }

    inline void set_wam_enabled(bool enabled)
    { wam_enabled_ = enabled; }

    inline bool is_wam_enabled() const
    { return wam_enabled_; }

    std::string get_result(bool newlines = true) const;
    term get_result_term(const std::string &varname) const;
    term get_result_term() const;
    void print_result(std::ostream &out) const;

    void unwind(size_t trail_mark)
    {
	interpreter_base::unwind(trail_mark);
    }

    bool is_instance() const;

protected:
    inline void set_retain_state_between_queries(bool b) {
        retain_state_between_queries_ = b;
    }
  
private:
    static bool new_instance_meta(interpreter_base &interp, const meta_reason_t &reason);

    void dispatch();
    void dispatch_wam(wam_instruction_base *instruction);
    bool unify_args(term clause_head, const code_point &p);
    bool select_clause(const code_point &instruction,
		       size_t index_id,
		       managed_clauses &clauses,
		       size_t from_clause);

    inline const predicate & get_predicate(con_cell f)
    {
        return interpreter_base::get_predicate(f);
    }
  
    inline const predicate & get_predicate(con_cell module, con_cell f)
    {
        return interpreter_base::get_predicate(module, f);
    }

    inline predicate & get_predicate_by_id(size_t id)
    {
	return id_to_predicate_[id];
    }

    void compute_matched_predicate(con_cell module, con_cell functor,
				   const term first_arg,
				   predicate &matched);
    size_t matched_predicate_id(con_cell module,
				con_cell functor, const term first_arg);
    void clear_matched_predicate(con_cell module, con_cell functor);

    std::unordered_map<functor_index, size_t> predicate_id_;
    std::vector<predicate> id_to_predicate_;

    inline std::vector<binding> & query_vars()
        { return *query_vars_; }

    inline std::vector<binding> * query_vars_ptr()
        { return query_vars_; }

    inline void set_query_vars(std::vector<binding> *qv)
        { query_vars_ = qv; }

    bool wam_enabled_;
    std::vector<binding> *query_vars_;
    size_t num_instances_;
    bool retain_state_between_queries_;

    friend struct new_instance_context;
    friend class test_wam_interpreter;
    friend class test_wam_compiler;
};

template<typename Pre, typename Post> void standard_clause_processing<Pre,Post>::operator () (common::term clause)
{
    auto head = interp_.clause_head(clause);
    auto head_f = interp_.functor(head);

    if (head_f == interpreter_base::ACTION_BY) {
        // Make a copy of the term before executing it,
        // because we don't want any side-effects of bound variables
        // persist in the program database.
        auto clause_copy = interp_.copy(clause);
      
        pre_(clause_copy);
        auto body = interp_.arg(clause_copy, 0);
	interp_.execute(body);
	interp_.reset();
        post_(clause_copy);	
    }
}
    
}}

#endif

