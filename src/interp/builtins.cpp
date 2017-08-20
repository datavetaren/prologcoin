#include "builtins.hpp"
#include "interpreter.hpp"

namespace prologcoin { namespace interp {

    using namespace prologcoin::common;

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

}}
