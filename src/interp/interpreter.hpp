#pragma once

#ifndef _interp_interpreter_hpp
#define _interp_interpreter_hpp

#include "wam_interpreter.hpp"

namespace prologcoin { namespace interp {

class wam_interim_code;
class wam_compiler;

class interpreter : public wam_interpreter
{
public:
    interpreter();
    ~interpreter();

    void setup_standard_lib();

    void compile();
    void compile(const qname &pred);
    void compile(common::con_cell module, common::con_cell name);

    bool execute(const term query);
    bool next();
    bool cont();
    void fail();

    inline bool has_more() const
    { return b() != top_b() || has_meta_contexts(); }

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

private:
    void load_code(wam_interim_code &code);
    void bind_code_point(std::unordered_map<size_t, size_t> &label_map,
			 code_point &cp);
    void dispatch();
    void dispatch_wam(wam_instruction_base *instruction);
    bool unify_args(term clause_head, const code_point &p);
    bool select_clause(const code_point &instruction,
		       size_t index_id,
		       managed_clauses &clauses,
		       size_t from_clause);

    const predicate & get_predicate(con_cell module, con_cell f)
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


    std::unordered_map<functor_index, size_t> predicate_id_;
    std::vector<predicate> id_to_predicate_;


    class binding {
    public:
	binding() { }
	binding(const std::string &name, const term value) 
	    : name_(name), value_(value) { }

	inline const std::string & name() const { return name_; }
	inline const term value() const { return value_; }

    private:
	std::string name_;
	term value_;
    };

    inline const std::vector<binding> & query_vars() const
        { return query_vars_; }

    bool wam_enabled_;
    std::vector<binding> query_vars_;
    wam_compiler *compiler_;

    friend class test_wam_compiler;
};

}}

#endif

