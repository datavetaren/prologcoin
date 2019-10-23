#include "builtins_opt.hpp"
#include "interpreter_base.hpp"

namespace prologcoin { namespace interp {

    using namespace prologcoin::common;
    using namespace boost::logic;

    tribool builtins_opt::sort_2(interpreter_base &interp, size_t arity, term args[])
    {
        term arg0 = args[0];
	term arg1 = args[1];

	if (arg0.tag() == tag_t::REF) {
            interp.abort(interpreter_exception_not_sufficiently_instantiated("sort/2: Arguments are not sufficiently instantiated"));
	}

	if (!interp.is_list(arg0)) {
            interp.abort(interpreter_exception_not_sufficiently_instantiated("sort/2: First argument is not a list; found " + interp.to_string(arg0)));
	}

	interp.add_accumulated_cost(interp.cost(arg0));

	size_t n = interp.list_length(arg0);

	std::vector<term> vec(n);

	for (size_t i = 0; i < n; i++) {
	    term el = interp.arg(arg0, 0);
	    vec[i] = el;
	    arg0 = interp.arg(arg0, 1);
	}

	std::stable_sort(vec.begin(), vec.end(),
			 [&](const term &t1, const term &t2)
			 { return interp.standard_order(t1,t2) < 0; } );

	vec.erase( std::unique(vec.begin(), vec.end()), vec.end());
	n = vec.size();

	// Build list from vector

	term r = interpreter_base::EMPTY_LIST;
	
	for (size_t i = 0; i < n; i++) {
	    term el = vec[n-i-1];
	    r = interp.new_dotted_pair(el, r);
	}

	bool ok = interp.unify(arg1, r);
	return ok;
    }
}}
