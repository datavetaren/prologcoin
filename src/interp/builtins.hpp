#pragma once

#ifndef _interp_builtins_hpp
#define _interp_builtins_hpp

#include "../common/term.hpp"

namespace prologcoin { namespace interp {
    class interpreter;
    struct meta_context;

    typedef std::function<bool (interpreter &interp, common::term &caller)> builtin;

    class builtins {
    public:
	//
	// Simple
	//

	static bool true_0(interpreter &interp, common::term &caller);

        //
        // Control flow
        //

        static bool operator_comma(interpreter &interp, common::term &caller);
        static bool operator_cut(interpreter &interp, common::term &caller);
        static bool operator_cut_if(interpreter &interp, common::term &caller);
        static bool operator_disjunction(interpreter &interp, common::term &caller);
	static bool operator_arrow(interpreter &interp, common::term &caller);
        static bool operator_if_then(interpreter &interp, common::term &caller);
        static bool operator_if_then_else(interpreter &interp, common::term &caller);

	//
	// Standard order, equality and unification
	//

        // operator @<
        static bool operator_at_less_than(interpreter &interp, common::term &caller);
        // operator @=<
        static bool operator_at_equals_less_than(interpreter &interp, common::term &caller);
        // operator @>
        static bool operator_at_greater_than(interpreter &interp, common::term &caller);
        // operator @>=
        static bool operator_at_greater_than_equals(interpreter &interp, common::term &caller);
        // operator ==
        static bool operator_equals(interpreter &interp, common::term &caller);
        // operator \==
        static bool operator_not_equals(interpreter &interp, common::term &caller);

	// compare/3
	static bool compare_3(interpreter &interp, common::term &caller);

	// operator =
	static bool operator_unification(interpreter &interp, common::term &caller);
	// operator \=
	static bool operator_cannot_unify(interpreter &interp, common::term &caller);

	//
	// Type checks
	//

	// var/1
	static bool var_1(interpreter &interp, common::term &caller);

	// nonvar/1
	static bool nonvar_1(interpreter &interp, common::term &caller);

	// integer/1
	static bool integer_1(interpreter &interp, common::term &caller);

	// number/1
	static bool number_1(interpreter &interp, common::term &caller);

	// atom/1
	static bool atom_1(interpreter &interp, common::term &caller);
	
	// compound/1
	static bool compound_1(interpreter &interp, common::term &caller);
	
	// atomic/1
	static bool atomic_1(interpreter &interp, common::term &caller);

	// callable/1
	static bool callable_1(interpreter &interp, common::term &caller);

	// ground/1
	static bool ground_1(interpreter &interp, common::term &caller);

	// cyclic_term/1
	static bool cyclic_term(interpreter &interp, common::term &caller);

	// acyclic_term/1
	static bool acyclic_term(interpreter &interp, common::term &caller);

	//
	// Arithmetics
	//

	static bool is_2(interpreter &interp, common::term &caller);

	//
	// Analyzing & constructing terms
	//

	static bool copy_term_2(interpreter &interp, common::term &caller);
	static bool functor_3(interpreter &interp, common::term &caller);
	static bool operator_deconstruct(interpreter &interp, common::term &caller);
    private:
	static common::term deconstruct_write_list(interpreter &interp,
						   common::term &t,
						   size_t index);
        static bool deconstruct_read_list(interpreter &interp,
					  common::term lst,
					  common::term &t, size_t index);

    public:

	//
	// Meta
	//
	
	static bool operator_disprove(interpreter &interp, common::term &caller);
	static void operator_disprove_post(interpreter &interp, meta_context *context);
    };

}}

#endif


