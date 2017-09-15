#pragma once

#ifndef _interp_wam_compiler_hpp
#define _interp_wam_compiler_hpp

#include "../common/term_env.hpp"

namespace prologcoin { namespace interp {

class wam_compiler {
public:
    typedef common::term term;

    wam_compiler(common::term_env &env) : env_(env) { }

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

private:
    friend class test_wam_compiler;

    void print_prims(const std::vector<prim_unification> &prims) const;

    // Takes a nested term and unfolds it into a sequence of
    // primitive unifications.
    std::vector<prim_unification> flatten(const term t);
    void flatten_process(std::vector<prim_unification> &prims,
			 std::vector<term> &args);
    prim_unification new_unification(term t);



    common::term_env &env_;
};

}}

#endif


