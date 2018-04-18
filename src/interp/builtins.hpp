#pragma once

#ifndef _interp_builtins_hpp
#define _interp_builtins_hpp

#include "../common/term.hpp"

namespace prologcoin { namespace interp {

    class interpreter_base;
    class meta_reason_t;
    struct meta_context;

    // We avoid std::function, because it is not as efficient as a
    // "raw" C function pointer.
    typedef bool (*builtin_fn)(interpreter_base &interp, size_t arity, common::term args[]);

    class builtin {
    public:
	builtin()
	    : fn_(nullptr), recursive_(false) { }
	builtin(const builtin &other)
	    : fn_(other.fn_), recursive_(other.recursive_) { }

	builtin(builtin_fn fn, bool is_rec = false)
            : fn_(fn), recursive_(is_rec) { }
	
	bool is_empty() const {
	    return fn_ == nullptr;
	}

	bool is_recursive() const {
	    return recursive_;
	}

	builtin_fn fn() const {
	    return fn_;
	}
	
    private:
	builtin_fn fn_;
	bool recursive_;
    };

    class builtins {
    public:
        static const size_t MAX_ARGS = 32;

	//
	// Profiling
	//

        static bool profile_0(interpreter_base &interp, size_t arity, common::term args []);

	static bool debug_on_0(interpreter_base &interp, size_t arity, common::term args []);
	static bool debug_check_0(interpreter_base &interp, size_t arity, common::term args[]);

	//
	// Simple
	//

	static bool true_0(interpreter_base &interp, size_t arity, common::term args[]);

	static bool fail_0(interpreter_base &interp, size_t arity, common::term args[]);

        //
        // Control flow
        //

        static bool operator_comma(interpreter_base &interp, size_t arity, common::term args[]);
        static bool operator_cut(interpreter_base &interp, size_t arity, common::term args[]);
        static bool operator_cut_if(interpreter_base &interp, size_t arity, common::term args[]);
        static bool operator_disjunction(interpreter_base &interp, size_t arity, common::term args[]);
	static bool operator_arrow(interpreter_base &interp, size_t arity, common::term args[]);
        static bool operator_if_then(interpreter_base &interp, size_t arity, common::term args[]);
        static bool operator_if_then_else(interpreter_base &interp, size_t arity, common::term args[]);

	//
	// Standard order, equality and unification
	//

        // operator @<
        static bool operator_at_less_than(interpreter_base &interp, size_t arity, common::term args[]);
        // operator @=<
        static bool operator_at_equals_less_than(interpreter_base &interp, size_t arity, common::term args[]);
        // operator @>
        static bool operator_at_greater_than(interpreter_base &interp, size_t arity,common::term args[]);
        // operator @>=
        static bool operator_at_greater_than_equals(interpreter_base &interp, size_t arity, common::term args[]);
        // operator ==
        static bool operator_equals(interpreter_base &interp, size_t arity, common::term args[]);
        // operator \==
        static bool operator_not_equals(interpreter_base &interp, size_t arity, common::term args[]);

	// compare/3
	static bool compare_3(interpreter_base &interp, size_t arity, common::term args[]);

	// operator =
	static bool operator_unification(interpreter_base &interp, size_t arity, common::term args[]);
	// operator \=
	static bool operator_cannot_unify(interpreter_base &interp, size_t arity, common::term args[]);

	//
	// Type checks
	//

	// var/1
        static bool var_1(interpreter_base &interp, size_t arity, common::term args[]);

	// nonvar/1
	static bool nonvar_1(interpreter_base &interp, size_t arity, common::term args[]);

	// integer/1
	static bool integer_1(interpreter_base &interp, size_t arity, common::term args[]);

	// number/1
	static bool number_1(interpreter_base &interp, size_t arity, common::term args[]);

	// atom/1
	static bool atom_1(interpreter_base &interp, size_t arity, common::term args[]);
	
	// compound/1
	static bool compound_1(interpreter_base &interp, size_t arity, common::term args[]);
	
	// atomic/1
	static bool atomic_1(interpreter_base &interp, size_t arity, common::term args[]);

	// callable/1
	static bool callable_1(interpreter_base &interp, size_t arity, common::term args[]);

	// ground/1
	static bool ground_1(interpreter_base &interp, size_t arity, common::term args[]);

	// cyclic_term/1
	static bool cyclic_term(interpreter_base &interp, size_t arity, common::term args[]);

	// acyclic_term/1
	static bool acyclic_term(interpreter_base &interp, size_t arity, common::term args[]);

	// is_list/1
	static bool is_list_1(interpreter_base &interp, size_t arity, common::term args[]);


	//
	// Character properties
	//

	static bool upcase_atom_2(interpreter_base &interp, size_t arity, common::term args[]);

	//
	// Arithmetics
	//

	static bool is_2(interpreter_base &interp, size_t arity, common::term args[]);

	//
	// Analyzing & constructing terms
	//

	static bool copy_term_2(interpreter_base &interp, size_t arity, common::term args[]);
	static bool functor_3(interpreter_base &interp, size_t arity, common::term args[]);
	static bool operator_deconstruct(interpreter_base &interp, size_t arity, common::term args[]);
    private:
	static common::term deconstruct_write_list(interpreter_base &interp,
						   common::term &t,
						   size_t index);
        static bool deconstruct_read_list(interpreter_base &interp,
					  common::term lst,
					  common::term &t, size_t index);

    public:

	//
	// Meta
	//
	
	static bool operator_disprove(interpreter_base &interp, size_t arity, common::term args[]);
	static bool operator_disprove_meta(interpreter_base &interp, const meta_reason_t &reason);
	static bool findall_3(interpreter_base &interp, size_t arity, common::term args[]);
	static bool findall_3_meta(interpreter_base &interp, const meta_reason_t &reason);
    };

}}

#endif


