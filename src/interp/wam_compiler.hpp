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
    std::vector<wam_compiler::prim_unification> flatten(const term t, compile_type for_type);

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
			   wam_instruction_sequence &seq);
    void compile_query_str(reg lhsreg, term rhs,
			   wam_instruction_sequence &seq);
    void compile_query(reg lhsreg, term rhs, wam_instruction_sequence &seq);

    void compile_program_ref(reg lhsreg, common::ref_cell rhsvar,
			   wam_instruction_sequence &seq);
    void compile_program_str(reg lhsreg, term rhs,
			   wam_instruction_sequence &seq);
    void compile_program(reg lhsreg, term rhs, wam_instruction_sequence &seq);

    void compile_query_or_program(term t, compile_type c,
			          wam_instruction_sequence &seq);

    std::pair<reg,bool> allocate_reg(common::ref_cell ref);

    std::unordered_map<common::ref_cell, size_t> argument_pos_;
    std::unordered_map<common::eq_term, common::ref_cell> term_map_;

    register_pool regs_;
    common::term_env &env_;
};

}}

#endif


