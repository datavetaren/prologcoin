#include "builtins.hpp"
#include "interpreter.hpp"

namespace prologcoin { namespace interp {

    using namespace prologcoin::common;


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
	    return interp.env().unify(order, lt);
	} else if (c > 0) {
	    term gt = interp.env().to_term(con_cell(">",0));
	    return interp.env().unify(order, gt);
	} else {
	    term eq = interp.env().to_term(con_cell("=",0));
	    return interp.env().unify(order, eq);
	}
    }

    bool builtins::operator_unification(interpreter &interp, term &caller)
    {
	term arg0 = interp.env().arg(caller, 0);
	term arg1 = interp.env().arg(caller, 1);
	return interp.env().unify(arg0, arg1);
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

}}
