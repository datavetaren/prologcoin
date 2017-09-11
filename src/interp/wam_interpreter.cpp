#include "wam_interpreter.hpp"

namespace prologcoin { namespace interp {

wam_interpreter::wam_interpreter()
{
}

std::vector<wam_interpreter::prim_unification> wam_interpreter::flatten(const term t)
{
    std::vector<prim_unification> prims;

    auto it_end = end(t);

    size_t last_depth = 0;
    
    std::vector<std::vector<term> > terms;

    for (auto it = begin(t); it != it_end; ++it) {
	term t1 = *it;
	size_t depth = it.depth();

	if (depth > terms.size()) {
	    terms.resize(depth);
	}
	if (depth >= last_depth) {
	    terms[depth-1].push_back(t1);
	} else {
	    // We went up...
	    flatten_process(prims, terms[depth]);
	    auto f = functor(t1);
	    auto p = new_term(f, terms[depth]);
	    if (depth > 0) {
		terms[depth-1].push_back(p);
	    } else {
		prims.push_back(new_unification(p));
	    }
	    terms.resize(depth);
	}

	last_depth = depth;
    }
    
    return prims;
}

void wam_interpreter::flatten_process(std::vector<wam_interpreter::prim_unification> &prims, std::vector<term> &args)
{
    for (auto &a : args) {
	if (is_functor(a) && functor(a).arity() > 0) {
	    prim_unification prim = new_unification(a);
	    prims.push_back(prim);
	    a = prim.lhs();
	}
    }
}

wam_interpreter::prim_unification wam_interpreter::new_unification(term t)
{
    term namet = new_ref();
    common::ref_cell &name = static_cast<common::ref_cell &>(namet);
    return prim_unification(name, t);
}

void wam_interpreter::print_prims(const std::vector<wam_interpreter::prim_unification> &prims ) const
{
    for (auto &p : prims) {
	std::cout << "   " << to_string(p.lhs()) << " = " << to_string(p.rhs()) << std::endl;
    }
}

}}
