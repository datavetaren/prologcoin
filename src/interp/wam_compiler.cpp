#include <queue>
#include "wam_compiler.hpp"

namespace prologcoin { namespace interp {

std::vector<wam_compiler::prim_unification> wam_compiler::flatten(const term t)
{
    std::vector<prim_unification> prims;

    std::queue<prim_unification> queue;
    queue.push(new_unification(t));

    while (!queue.empty()) {
	prim_unification p = queue.front();
	queue.pop();
	switch (p.rhs().tag()) {
	case common::tag_t::STR: {
	    auto f = env_.functor(p.rhs());
	    size_t n = f.arity();
	    for (size_t i = 0; i < n; i++) {
		auto arg = env_.arg(p.rhs(), i);
		prim_unification p1 = new_unification(arg);
		env_.set_arg(p.rhs(), i, p1.lhs());
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
    return prims;
}

wam_compiler::prim_unification wam_compiler::new_unification(term t)
{
    term namet = env_.new_ref();
    common::ref_cell &name = static_cast<common::ref_cell &>(namet);
    return prim_unification(name, t);
}

void wam_compiler::compile_query_or_fact(term t, wam_compiler::compile_type c,
				  	 wam_instruction_sequence &instrs)

{
    std::vector<prim_unification> prims = flatten(t);

    size_t n = prims.size();

    register_pool regs;
    
    for (size_t i = 0; i < n; i++) {
	bool is_predicate = i == 0;
	auto &prim = prims[i];
	auto lhsvar = prim.lhs();

	if (!is_predicate) {
	    regs.allocate(lhsvar, reg::X_REG);
	}

	term rhs = prim.rhs();
	switch (rhs.tag()) {
	case common::tag_t::INT:
	case common::tag_t::CON:
	    // We have X = a or X = 4711
	    if (c == COMPILE_QUERY) {
		// instrs.add(wam_instruction<PUT_CONSTANT>(rhs));
	    }
	    break;
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
