#include "builtins.hpp"
#include "interpreter.hpp"

namespace prologcoin { namespace interp {

    using namespace prologcoin::common;

    //
    // Simple
    //

    bool builtins::true_0(interpreter &interp, term &caller)
    {
	return true;
    }

    //
    // Control flow
    //

    bool builtins::operator_comma(interpreter &interp, term &caller)
    {
        term arg0 = interp.env().arg(caller, 0);
	term arg1 = interp.env().arg(caller, 1);
	interp.set_continuation_point(arg1);
        interp.allocate_environment();
	interp.set_continuation_point(arg0);
	return true;
    }

    bool builtins::operator_cut(interpreter &interp, term &caller)
    {
        if (interp.has_late_choice_point()) {
	    interp.reset_choice_point();
	    interp.tidy_trail();
	}
        return true;
    }

    bool builtins::operator_cut_if(interpreter &interp, term &caller)
    {
        if (interp.has_late_choice_point()) {
	    interp.reset_choice_point();
	    interp.tidy_trail();
	    auto *ch = interp.get_last_choice_point();
	    ch->bp = 2; // Don't back track to false clause
	}
        return true;
    }

    bool builtins::operator_disjunction(interpreter &interp, term &caller)
    {
	static con_cell arrow("->", 2);

	// Check if this is an if-then-else
	term arg0 = interp.env().arg(caller, 0);
	if (interp.env().functor(arg0) == arrow) {
	    return operator_if_then_else(interp, caller);
	}

        auto *ch = interp.allocate_choice_point(0);
        ch->bp = 1; // Index 0 and clause 1 (use query to get clause 1)
	interp.set_continuation_point(arg0);
	return true;
    }

    bool builtins::operator_if_then(interpreter &interp, term &caller)
    {
	static con_cell cut_op("!",0);

	auto *ch = interp.allocate_choice_point(0);
	ch->bp = 2;
	interp.move_cut_point_to_last_choice_point();
	term cut = interp.env().to_term(cut_op);
	term arg0 = interp.env().arg(caller, 0);
	term arg1 = interp.env().arg(caller, 1);
	interp.set_continuation_point(arg1);
	interp.allocate_environment();
	interp.set_continuation_point(cut);
	interp.allocate_environment();
	interp.set_continuation_point(arg0);
        return true;
    }

    bool builtins::operator_if_then_else(interpreter &interp, term &caller)
    {
	static con_cell cut_op_if("_!",0);

	term lhs = interp.env().arg(caller, 0);

	term cond = interp.env().arg(lhs, 0);
	term iftrue = interp.env().arg(lhs, 1);
	term cut_if = interp.env().to_term(cut_op_if);

	auto *ch = interp.allocate_choice_point(0);
	ch->bp = 1; // Go to 'C' the false clause if ((A->B) ; C) fails
	interp.move_cut_point_to_last_choice_point();
	interp.set_continuation_point(iftrue);
	interp.allocate_environment();
	interp.set_continuation_point(cut_if);
	interp.allocate_environment();
	interp.set_continuation_point(cond);
        return true;
    }

    //
    // Standard order, eqaulity and unification
    //

    bool builtins::operator_at_less_than(interpreter &interp, term &caller)
    {
        return interp.env().standard_order(interp.env().arg(caller, 0),
				           interp.env().arg(caller, 1)) < 0;
    }

    bool builtins::operator_at_equals_less_than(interpreter &interp, term &caller)
    {
        return interp.env().standard_order(interp.env().arg(caller, 0),
				           interp.env().arg(caller, 1)) <= 0;
    }

    bool builtins::operator_at_greater_than(interpreter &interp, term &caller)
    {
        return interp.env().standard_order(interp.env().arg(caller, 0),
				           interp.env().arg(caller, 1)) > 0;
    }

    bool builtins::operator_at_greater_than_equals(interpreter &interp, term &caller)
    {
        return interp.env().standard_order(interp.env().arg(caller, 0),
				           interp.env().arg(caller, 1)) >= 0;
    }

    bool builtins::operator_equals(interpreter &interp, term &caller)
    {
        return interp.env().standard_order(interp.env().arg(caller, 0),
					   interp.env().arg(caller, 1)) == 0;
    }

    bool builtins::operator_not_equals(interpreter &interp, term &caller)
    {
        return interp.env().standard_order(interp.env().arg(caller, 0),
					   interp.env().arg(caller, 1)) != 0;
    }

    bool builtins::compare_3(interpreter &interp, term &caller)
    {
        term order = interp.env().arg(caller, 0);

	int c = interp.env().standard_order(interp.env().arg(caller, 1),
					    interp.env().arg(caller, 2));
	if (c < 0) {
	    term lt = interp.env().to_term(con_cell("<",0));
	    return interp.unify(order, lt);
	} else if (c > 0) {
	    term gt = interp.env().to_term(con_cell(">",0));
	    return interp.unify(order, gt);
	} else {
	    term eq = interp.env().to_term(con_cell("=",0));
	    return interp.unify(order, eq);
	}
    }

    bool builtins::operator_unification(interpreter &interp, term &caller)
    {
	term arg0 = interp.env().arg(caller, 0);
	term arg1 = interp.env().arg(caller, 1);
	bool r = interp.unify(arg0, arg1);
	interp.set_heap_pointer(interp.env().heap_size());
	interp.set_trail_pointer(interp.env().trail_size());
	return r;
    }

    //
    // Type tests
    //
    
    bool builtins::var_1(interpreter &interp, term &caller)
    {
	return interp.env().arg(caller, 0)->tag() == tag_t::REF;
    }

    bool builtins::nonvar_1(interpreter &interp, term &caller)
    {
	return !var_1(interp, caller);
    }

    bool builtins::integer_1(interpreter &interp, term &caller)
    {
	return interp.env().arg(caller, 0)->tag() == tag_t::INT;
    }

    bool builtins::number_1(interpreter &interp, term &caller)
    {
	return integer_1(interp, caller);
    }

    bool builtins::atom_1(interpreter &interp, term &caller)
    {
	term arg = interp.env().arg(caller, 0);
	
	switch (arg->tag()) {
	case tag_t::CON: return true;
	case tag_t::STR: {
	    con_cell f = interp.env().functor(arg);
	    return f.arity() == 0;
	}
	default: return false;
	}
    }

    bool builtins::compound_1(interpreter &interp, term &caller)
    {
	term arg = interp.env().arg(caller, 0);
	if (arg->tag() != tag_t::STR) {
	    return false;
	}
	return interp.env().functor(arg).arity() > 0;
    }

    bool builtins::callable_1(interpreter &interp, term &caller)
    {
	return atom_1(interp, caller) || compound_1(interp, caller);
    }

    bool builtins::atomic_1(interpreter &interp, term &caller)
    {
	return atom_1(interp, caller);
    }

    bool builtins::ground_1(interpreter &interp, term &caller)
    {
	term arg = interp.env().arg(caller, 0);

	for (auto t : interp.env().iterate_over(arg)) {
	    if (t->tag() == tag_t::REF) {
		return false;
	    }
	}
	return true;
    }

    // TODO: cyclic_term/1 and acyclic_term/1

    //
    // Arithmetics
    //

    bool builtins::is_2(interpreter &interp, term &caller)
    {
	term lhs = interp.env().arg(caller, 0);
	term rhs = interp.env().arg(caller, 1);
	term result = interp.arith().eval(rhs, "is/2");
	return interp.unify(lhs, result);
    }	

    //
    // Meta
    //

    bool builtins::operator_disprove(interpreter &interp, term &caller)
    {
	term arg = interp.env().arg(caller, 0);
	auto *context = interp.new_meta_context<meta_context>(&operator_disprove_post);
	auto *ch = interp.allocate_choice_point(0);
	ch->b = 1;
	interp.set_top_b();
	interp.set_continuation_point(arg);
	return true;
    }

    void builtins::operator_disprove_post(interpreter &interp,
					  meta_context *context)
    {
        bool failed = interp.is_top_fail();
	if (!failed) {
	    interp.unwind_to_top_choice_point();
	}
	interp.release_last_meta_context();
	// Note that this is "disprove," so its success is the reverse of
	// the underlying expression to succeed.
	interp.set_top_fail(!failed);
    }

}}
