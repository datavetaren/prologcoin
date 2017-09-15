#include "wam_compiler.hpp"

namespace prologcoin { namespace interp {

std::vector<wam_compiler::prim_unification> wam_compiler::flatten(const term t)
{
    std::vector<prim_unification> prims;

    auto it_end = env_.end(t);

    size_t last_depth = 0;
    
    std::vector<std::vector<term> > terms;

    for (auto it = env_.begin(t); it != it_end; ++it) {
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
	    auto f = env_.functor(t1);
	    auto p = env_.new_term(f, terms[depth]);
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

void wam_compiler::flatten_process(std::vector<wam_compiler::prim_unification> &prims, std::vector<term> &args)
{
    for (auto &a : args) {
      prim_unification prim = new_unification(a);
      prims.push_back(prim);
      a = prim.lhs();
    }
}

wam_compiler::prim_unification wam_compiler::new_unification(term t)
{
    term namet = env_.new_ref();
    common::ref_cell &name = static_cast<common::ref_cell &>(namet);
    return prim_unification(name, t);
}

void wam_compiler::print_prims(const std::vector<wam_compiler::prim_unification> &prims ) const
{
    for (auto &p : prims) {
	std::cout << "   " << env_.to_string(p.lhs()) << " = " << env_.to_string(p.rhs()) << std::endl;
    }
}

}}
