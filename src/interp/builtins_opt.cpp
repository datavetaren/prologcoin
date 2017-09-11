#include "builtins_opt.hpp"
#include "interpreter.hpp"

namespace prologcoin { namespace interp {

    using namespace prologcoin::common;
    using namespace boost::logic;

    tribool builtins_opt::member_2(interpreter &interp, term &caller)
    {
	term arg = interp.env().arg(caller, 0);
	term lst = interp.env().arg(caller, 1);
	if (interp.env().is_ground(arg) && interp.env().is_ground(lst)) {
	    // Optimized version with no choice points
	    while (interp.env().is_dotted_pair(lst)) {
		term fst = interp.env().arg(lst, 0);
		if (interp.env().equal(fst, arg)) {
		    return tribool(true);
		}
		lst = interp.env().arg(lst, 1);
	    }
	    return tribool(false);
	} else {
	    return tribool(indeterminate);
	}
    }

    tribool builtins_opt::sort_2(interpreter &interp, term &caller)
    {
	term arg0 = interp.env().arg(caller, 0);
	term arg1 = interp.env().arg(caller, 1);

	if (arg0.tag() == tag_t::REF) {
            interp.abort(interpreter_exception_not_sufficiently_instantiated("sort/2: Arguments are not sufficiently instantiated"));
	}

	if (!interp.env().is_list(arg0)) {
            interp.abort(interpreter_exception_not_sufficiently_instantiated("sort/2: First argument is not a list; found " + interp.env().to_string(arg0)));
	}

	size_t n = interp.env().list_length(arg0);
	
	std::vector<term> vec(n);

	for (size_t i = 0; i < n; i++) {
	    term el = interp.env().arg(arg0, 0);
	    vec[i] = el;
	    arg0 = interp.env().arg(arg0, 1);
	}

	std::stable_sort(vec.begin(), vec.end(),
			 [&](const term &t1, const term &t2)
			 { return interp.env().standard_order(t1,t2) < 0; } );

	vec.erase( std::unique(vec.begin(), vec.end()), vec.end());
	n = vec.size();

	// Build list from vector

	term r = interp.env().empty_list();
	
	for (size_t i = 0; i < n; i++) {
	    term el = vec[n-i-1];
	    r = interp.new_dotted_pair(el, r);
	}

	bool ok = interp.unify(arg1, r);
	return ok;
    }
}}
