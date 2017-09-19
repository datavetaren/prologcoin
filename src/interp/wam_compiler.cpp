#include <queue>
#include "wam_compiler.hpp"

namespace prologcoin { namespace interp {

std::vector<wam_compiler::prim_unification> wam_compiler::flatten(const term t, wam_compiler::compile_type for_type)
{
    std::vector<prim_unification> prims;

    std::queue<prim_unification> queue;
    auto prim = new_unification(t);
    prim.set_predicate(true);
    queue.push(prim);

    while (!queue.empty()) {
	prim_unification p = queue.front();
	queue.pop();
	switch (p.rhs().tag()) {
	case common::tag_t::STR: {
	    auto f = env_.functor(p.rhs());
	    size_t n = f.arity();
	    for (size_t i = 0; i < n; i++) {
	        size_t pos = (for_type == COMPILE_QUERY) ? n - i - 1 : i;
		auto arg = env_.arg(p.rhs(), pos);
		prim_unification p1 = new_unification(arg);
		if (p.is_predicate()) {
		    p1.set_lhs_arg_index(pos);
		}
		env_.set_arg(p.rhs(), pos, p1.lhs());
		queue.push(p1);
	    }
	    prims.push_back(p);
	    break;
	}
	case common::tag_t::CON: 
	case common::tag_t::REF: 
	case common::tag_t::INT: {
	    prims.push_back(p);
	    break;
	}
	}
    }
    
    if (for_type == COMPILE_QUERY) {
        std::reverse(prims.begin(), prims.end());
    }

    return prims;
}

wam_compiler::prim_unification wam_compiler::new_unification(term t)
{
    term namet = env_.new_ref();
    common::ref_cell &name = static_cast<common::ref_cell &>(namet);
    return prim_unification(name, t);
}

void wam_compiler::compile_query_or_program(wam_compiler::term t,
					    wam_compiler::compile_type c,
				      	    wam_instruction_sequence &instrs)

{
    std::vector<prim_unification> prims = flatten(t, c);

    size_t n = prims.size();
    
    register_pool regs;

    for (size_t i = 0; i < n; i++) {
	auto &prim = prims[i];
        bool is_predicate = prim.is_predicate();
	auto lhsvar = prim.lhs();

	if (!is_predicate) {
	    regs.allocate(lhsvar, reg::X_REG);
	}
	term rhs = prim.rhs();

	if (c == COMPILE_QUERY) {
	    switch (rhs.tag()) {
	    case common::tag_t::REF:
	      /*
	      { auto ref = static_cast<common::ref_cell &>(rhs);
		reg r;
		std::tie(r, isnew) = regs.allocate(ref, X_REG);
		if (isnew) {
		  instrs.add(wam_instruction<PUT__X>(r);
	        }
	      }
	      */
	      break;

	    case common::tag_t::INT:
	    case common::tag_t::CON:
	      // We have X = a or X = 4711
	      instrs.add(wam_instruction<SET_CONSTANT>(rhs));
	      break;
	    }
	}
    }
}

std::pair<wam_compiler::reg,bool> wam_compiler::register_pool::allocate(common::ref_cell var, wam_compiler::reg::reg_type regtype)
{
    auto it = reg_map_.find(var);
    if (it == reg_map_.end()) {
	size_t regcnt;
	switch (regtype) {
	case reg::A_REG: regcnt = a_cnt_++; break;
	case reg::X_REG: regcnt = x_cnt_++; break;
	case reg::Y_REG: regcnt = y_cnt_++; break;
	}
	reg r(regcnt, regtype);
	reg_map_.insert(std::make_pair(var, r));
	return std::make_pair(r, true);
    } else {
	return std::make_pair(it->second, false);
    }
}

void wam_compiler::print_prims(const std::vector<wam_compiler::prim_unification> &prims ) const
{
    for (auto &p : prims) {
	std::cout << "   " << env_.to_string(p.lhs()) << " = " << env_.to_string(p.rhs()) << std::endl;
    }
}

}}
