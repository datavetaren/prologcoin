#pragma once

#ifndef _interp_wam_compiler_hpp
#define _interp_wam_compiler_hpp

#include <forward_list>
#include <stack>
#include "../common/term_env.hpp"
#include "wam_interpreter.hpp"

namespace prologcoin { namespace interp {

class wam_interim_code : private std::forward_list<wam_instruction_base *> {
public:
    wam_interim_code(wam_interpreter &interp);

    void push_back(const wam_instruction_base &instr);

    void print(std::ostream &out) const;

private:
    void push_back(wam_instruction_base *instr);

    wam_interpreter &interp_;
    std::forward_list<wam_instruction_base *>::iterator end_;
};

class wam_goal_iterator : public std::iterator<std::forward_iterator_tag,
					       common::term,
					       common::term,
					       const common::term *,
					       const common::term &> {
public:
    typedef common::term term;

    wam_goal_iterator(common::term_env &env, term body) : env_(env) {
        stack_.push(body);
	first_of();
    }

    wam_goal_iterator(common::term_env &env) : env_(env) { }

    inline bool operator == (const wam_goal_iterator &other) const {
        return stack_ == other.stack_;
    }

    inline bool operator != (const wam_goal_iterator &other) const {
        return stack_ != other.stack_;
    }

    inline wam_goal_iterator & operator ++ () {
        advance(); return *this;
    }

    inline reference operator * () const { return stack_.top(); }
    inline pointer operator -> () const { return &stack_.top(); }

private:
    void first_of();
    void advance();

    common::term_env &env_;
    std::stack<term> stack_;
};

class wam_compiler {
public:
    typedef common::term term;

    wam_compiler(common::term_env &env) : env_(env) { }

private:
    friend class test_wam_compiler;

    class prim_unification {
    public:
	inline prim_unification() : lhs_(common::ref_cell()), rhs_() { }
	inline prim_unification(const prim_unification &other)
	    : lhs_(other.lhs_), rhs_(other.rhs_) { }
	inline prim_unification(common::ref_cell lhs, term rhs)
	    : lhs_(lhs), rhs_(rhs) { }

	inline common::ref_cell lhs() const { return lhs_; }
	inline term rhs() const { return rhs_; }

    private:
	common::ref_cell lhs_;
	term rhs_;
    };

    enum compile_type { COMPILE_QUERY, COMPILE_PROGRAM };

    void print_prims(const std::vector<prim_unification> &prims) const;

    // Takes a nested term and unfolds it into a sequence of
    // primitive unifications.
    std::vector<wam_compiler::prim_unification> flatten(const term t, compile_type for_type, bool is_predicate_call);

    prim_unification new_unification(term t);
    prim_unification new_unification(common::ref_cell ref, term t);

    struct reg {

	enum reg_type {
	    A_REG,
	    X_REG,
	    Y_REG
	};

	reg() : num(0), type(X_REG) { }
	reg(size_t n, reg_type t) : num(n), type(t) { }

	size_t num;
	reg_type type;
    };

    class register_pool {
    public:
	register_pool() : x_cnt_(0), y_cnt_(0) { }

	std::pair<reg,bool> allocate(common::ref_cell ref, reg::reg_type regtype);
	std::pair<reg,bool> allocate(common::ref_cell ref, reg::reg_type regtype, size_t no);

    private:
	std::unordered_map<common::ref_cell, reg> reg_map_;
	size_t x_cnt_;
	size_t y_cnt_;
    };

    void compile_query_ref(reg lhsreg, common::ref_cell rhsvar,
			   wam_interim_code &seq);
    void compile_query_str(reg lhsreg, term rhs,
			   wam_interim_code &seq);
    void compile_query(reg lhsreg, term rhs, wam_interim_code &seq);

    void compile_program_ref(reg lhsreg, common::ref_cell rhsvar,
			   wam_interim_code &seq);
    void compile_program_str(reg lhsreg, term rhs,
			   wam_interim_code &seq);
    void compile_program(reg lhsreg, term rhs, 
			 wam_interim_code &seq);

    void compile_query_or_program(term t, compile_type c,
				  bool is_predicate,
			          wam_interim_code &seq);

    bool clause_needs_environment(const term clause);
    void compile_clause(const term clause, wam_interim_code &seq);

    term clause_head(const term clause);
    term clause_body(const term clause);

    class goals_range {
    public:
       inline goals_range(common::term_env &env, term t)
	    : env_(env), from_(t) { }
       wam_goal_iterator begin() {
	 return wam_goal_iterator(env_, from_);
       }
       wam_goal_iterator end() {
	 return wam_goal_iterator(env_);
       }
    private:
       common::term_env &env_;
       term from_;
    };

    goals_range for_all_goals(const term t) {
       return goals_range(env_, t);
    }
  
    term first_arg(const term clause);
    bool first_arg_is_var(const term clause);
    bool first_arg_is_con(const term clause);
    bool first_arg_is_str(const term clause);

    std::vector<std::vector<term> > partition_clauses(const std::vector<term> &clauses, std::function<bool (const term t1, const term t2)> pred);
    std::vector<std::vector<term> > partition_clauses_nonvar(const std::vector<term> &clauses);
    std::vector<std::vector<term> > partition_clauses_first_arg(const std::vector<term> &clauses);


    void print_partition(std::ostream &out, const std::vector<std::vector<term> > &partition);

    std::pair<reg,bool> allocate_reg(common::ref_cell ref);

    std::unordered_map<common::ref_cell, size_t> argument_pos_;
    std::unordered_map<common::eq_term, common::ref_cell> term_map_;

    register_pool regs_;
    common::term_env &env_;
};

}}

#endif


