#pragma once

#ifndef _interp_wam_compiler_hpp
#define _interp_wam_compiler_hpp

#include "../common/term_env.hpp"
#include "wam_interpreter.hpp"

namespace prologcoin { namespace interp {

class wam_compiler {
public:
    typedef common::term term;

    wam_compiler(common::term_env &env) : env_(env) { }

private:
    friend class test_wam_compiler;

    class prim_unification {
    public:
	prim_unification() : lhs_(common::ref_cell()), rhs_() { }
	prim_unification(const prim_unification &other)
	    : lhs_(other.lhs_), rhs_(other.rhs_) { }
	prim_unification(common::ref_cell lhs, term rhs)
	    : lhs_(lhs), rhs_(rhs) { }
	common::ref_cell lhs() const { return lhs_; }
	term rhs() const { return rhs_; }

    private:
	common::ref_cell lhs_;
	term rhs_;
    };

    enum compile_type { COMPILE_QUERY, COMPILE_FACT };

    void compile_query_or_fact(term t, compile_type c,
			       wam_instruction_sequence &seq);

    void print_prims(const std::vector<prim_unification> &prims) const;

    // Takes a nested term and unfolds it into a sequence of
    // primitive unifications.
    std::vector<prim_unification> flatten(const term t);
    void flatten_process(std::vector<prim_unification> &prims,
			 std::vector<term> &args);
    prim_unification new_unification(term t);

    struct reg {

	enum reg_type {
	    A_REG,
	    X_REG,
	    Y_REG
	};

	reg(size_t n, reg_type t) : num(n), type(t) { }

	size_t num;
	reg_type type;
    };

    class register_pool {
    public:
	register_pool() : a_cnt_(0), x_cnt_(0), y_cnt_(0) { }

	std::pair<reg,bool> allocate(common::ref_cell var, reg::reg_type regtype);

    private:
	std::unordered_map<common::ref_cell, reg> reg_map_;
	size_t a_cnt_;
	size_t x_cnt_;
	size_t y_cnt_;
    };

    common::term_env &env_;
};

}}

#endif


