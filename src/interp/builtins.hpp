#pragma once

#ifndef _interp_builtins_hpp
#define _interp_builtins_hpp

#include "../common/term.hpp"

namespace prologcoin { namespace interp {

// This pair represents functor with first argument. If first argument
// is a STR tag, then we dereference it to a CON cell.
//
// FUNCTOR_INDEX = pair( QNAME, ARG_TYPE)
// QNAME = pair( MODULE, FUNCTOR )
//
typedef std::pair<common::con_cell, common::con_cell> qname;
    
    class wam_interpreter;
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
	static bool program_state_0(interpreter_base &interp, size_t arity, common::term args[]);
	static bool program_state_1(interpreter_base &interp, size_t arity, common::term args[]);	

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
	static bool cyclic_term_1(interpreter_base &interp, size_t arity, common::term args[]);

	// acyclic_term/1
	static bool acyclic_term_1(interpreter_base &interp, size_t arity, common::term args[]);

	// is_list/1
	static bool is_list_1(interpreter_base &interp, size_t arity, common::term args[]);


	//
	// Character properties
	//

	static bool upcase_atom_2(interpreter_base &interp, size_t arity, common::term args[]);

        // Not standard. Convert list of chars into a big integer.
        // Most significant byte is first.
        static bool bytes_number_2(interpreter_base &interp, size_t arity, common::term args[]);

	//
	// Arithmetics
	//

	static bool is_2(interpreter_base &interp, size_t arity, common::term args[]);
        static bool greater_than_equals_2(interpreter_base &interp, size_t arity, common::term args[]);
        static bool less_than_equals_2(interpreter_base &interp, size_t arity, common::term args[]);
        static bool greater_than_2(interpreter_base &interp, size_t arity, common::term args[]);
        static bool less_than_2(interpreter_base &interp, size_t arity, common::term args[]);

	//
	// Analyzing & constructing terms
	//
	static void store_p_on_heap_if_wam(interpreter_base &interp);
	static void restore_p_from_heap_if_wam(interpreter_base &interp);
	static bool arg_3_cp(interpreter_base &interp, size_t arity, common::term args[]);
        static bool arg_3(interpreter_base &interp, size_t arity, common::term args[]);
        static bool functor_3(interpreter_base &interp, size_t arity, common::term args[]);
	static bool copy_term_2(interpreter_base &interp, size_t arity, common::term args[]);
	static bool same_term_2(interpreter_base &interp, size_t arity, common::term args[]);
	static bool operator_deconstruct(interpreter_base &interp, size_t arity, common::term args[]);
        static bool sort_2(interpreter_base &interp, size_t arity, common::term args[]);
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

        static bool call_n(interpreter_base &interp, size_t arity, common::term args[]);
	static bool operator_disprove(interpreter_base &interp, size_t arity, common::term args[]);
	static bool operator_disprove_meta(interpreter_base &interp, const meta_reason_t &reason);
	static bool findall_3(interpreter_base &interp, size_t arity, common::term args[]);
	static bool findall_3_meta(interpreter_base &interp, const meta_reason_t &reason);
        static bool freeze_2(interpreter_base &interp, size_t arity, common::term args[]);

	//
	// System
	//
        static bool module_1(interpreter_base &interp, size_t arity, common::term args[]);
	static bool use_module_1(interpreter_base &interp, size_t arity, common::term args[]);

        //
        // Program database
        //
        static bool show_0(interpreter_base &interp, size_t arity, common::term args[]);
        static bool asserta_1(interpreter_base &interp, size_t arity, common::term args[]);
        static bool assertz_1(interpreter_base &interp, size_t arity, common::term args[]);
        static bool assert_1(interpreter_base &interp, size_t arity, common::term args[]);
        static bool retract_1(interpreter_base &interp, size_t arity, common::term args[]);
        static bool retractall_1(interpreter_base &interp, size_t arity, common::term args[]);
        static bool retract(interpreter_base &interp, const std::string &pame, common::term head, bool all);
        static qname check_predicate(interpreter_base &interp, const std::string &pname, common::term arg);
        static bool current_predicate_1(interpreter_base &interp, size_t arity, common::term args[]);
        static bool status_predicate_2(interpreter_base &interp, size_t arity, common::term args[]);

        //
        // Non-standard, Prologcoin specific
        //

        // frozen(+X, -Closure)
        static bool frozen_2(interpreter_base &interp, size_t arity, common::term args[] );
        static common::term get_frozen(interpreter_base &interp, common::term arg);

        // frozenk(+Start, +K, -Xs)
        static bool frozenk_3(interpreter_base &interp, size_t arity, common::term args[] );
        // defrost(+HeapAddress, -Closure, +Values)
        static bool defrost_3(interpreter_base &interp, size_t arity, common::term args[] );

        // password(+String, [+Options])
        // General password management. If password(String) used then
        // it temporarily sets the password for the next query (and then
        // immediately removes it from memory.) If password(String, persistent)
        // is used, then it is persistent until further notice. If
        // password(X) is used where X is a variable, then it retrieves the
        // current password if there's one set (or it fails.) This predicate
        // is convenient to do special things, e.g. the wallet uses the
        // password for encrypting the master private and by not having
        // the password constantly available it reduces the risk of possible
        // external hacks.
        static bool password_2(interpreter_base &interp, size_t arity, common::term args[] );

        // load
        static void load(interpreter_base &interp);
    };

}}

#endif


