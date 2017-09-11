#pragma once

#ifndef _interp_wam_interpreter_hpp
#define _interp_wam_interpreter_hpp

#include <istream>
#include <vector>
#include "interpreter.hpp"

namespace prologcoin { namespace interp {

class test_wam_interpreter;

class wam_interpreter : public interpreter
{
public:
    wam_interpreter();

private:
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

    void print_prims(const std::vector<prim_unification> &prims) const;

    // Takes a nested term and unfolds it into a sequence of
    // primitive unifications.
    std::vector<prim_unification> flatten(const term t);
    void flatten_process(std::vector<prim_unification> &prims,
			 std::vector<term> &args);
    prim_unification new_unification(term t);

    friend class test_wam_interpreter;
};

}}

#endif
