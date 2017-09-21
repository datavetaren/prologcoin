#include <queue>
#include "wam_compiler.hpp"

namespace prologcoin { namespace interp {

std::vector<wam_compiler::prim_unification> wam_compiler::flatten(const term t,
								  wam_compiler::compile_type for_type)
{
    std::vector<prim_unification> prims;

    std::queue<prim_unification> worklist;
    auto prim = new_unification(t);
    worklist.push(prim);

    bool is_predicate = true;

    while (!worklist.empty()) {
	prim_unification p = worklist.front();
	worklist.pop();
	switch (p.rhs().tag()) {
	case common::tag_t::STR: {
	    auto f = env_.functor(p.rhs());
	    size_t n = f.arity();
	    for (size_t i = 0; i < n; i++) {
	        auto pos = (for_type == COMPILE_QUERY) ? n - i - 1 : i;
		auto arg = env_.arg(p.rhs(), pos);
		auto found = term_map_.find(common::eq_term(env_,arg));
		common::ref_cell ref;
		bool is_found = found != term_map_.end();
		if (is_found) {
		    ref = found->second;
		} else {
		    auto ref1 = env_.new_ref();
  		    auto ref0 = static_cast<const common::ref_cell &>(ref1);
	  	    ref = ref0;
		    term_map_.insert(std::make_pair(common::eq_term(env_,arg), ref));
		}
		if (is_predicate) {
		    argument_pos_[ref] = pos;
		}

		prim_unification p1 = new_unification(ref, arg);
		env_.set_arg(p.rhs(), pos, ref);
		if (!is_found) {
		    worklist.push(p1);
		}
	    }
	    if (!is_predicate) {
	        prims.push_back(p);
	    }
	    break;
	}
	case common::tag_t::CON: 
	case common::tag_t::INT: {
	    prims.push_back(p);
	    break;
	}
	case common::tag_t::REF:
	    break;
	}
	is_predicate = false;
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

wam_compiler::prim_unification wam_compiler::new_unification(common::ref_cell ref, term t)
{
    return prim_unification(ref, t);
}

std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg(common::ref_cell ref)
{
    auto found = argument_pos_.find(ref);
    if (found != argument_pos_.end()) {
        return regs_.allocate(ref, reg::A_REG, found->second);
    } else {
        return regs_.allocate(ref, reg::X_REG);
    }
}

void wam_compiler::compile_query_ref(wam_compiler::reg lhsreg, common::ref_cell rhsvar, wam_instruction_sequence &instrs)
{
    reg rhsreg;
    bool isnew;
    std::tie(rhsreg, isnew) = allocate_reg(rhsvar);

    if (lhsreg.type == reg::A_REG) {
        // ai = xn
        assert(rhsreg.type == reg::X_REG);
	if (isnew) {
	    instrs.add(wam_instruction<PUT_VARIABLE_X>(
			       rhsreg.num, lhsreg.num ));
	} else {
	    instrs.add(wam_instruction<PUT_VALUE_X>(
  			       rhsreg.num, lhsreg.num ));
	}
    } else { // lhsreg.type == reg::X_REG
      // xn = V
      // No instruction needs to be emitted
    }
}

void wam_compiler::compile_query_str(wam_compiler::reg lhsreg, common::term rhs, wam_instruction_sequence &instrs)
{
    auto f = env_.functor(rhs);
    if (lhsreg.type == reg::A_REG) {
        instrs.add(wam_instruction<PUT_STRUCTURE_A>(f,lhsreg.num));
    } else {
        instrs.add(wam_instruction<PUT_STRUCTURE_X>(f,lhsreg.num));
    }
    size_t n = f.arity();
    for (size_t i = 0; i < n; i++) {
        auto arg = env_.arg(rhs, i);
	assert(arg.tag() == common::tag_t::REF);
	auto ref = static_cast<common::ref_cell &>(arg);
	reg r;
	bool isnew = false;
	std::tie(r,isnew) = allocate_reg(ref);
	if (r.type == reg::A_REG) {
	    if (isnew) {
	        instrs.add(wam_instruction<SET_VARIABLE_A>(r.num));
	    } else {
	        instrs.add(wam_instruction<SET_VALUE_A>(r.num));
	    }
	} else {
	    if (isnew) {
	        instrs.add(wam_instruction<SET_VARIABLE_X>(r.num));
	    } else {
	        instrs.add(wam_instruction<SET_VALUE_X>(r.num));
	    }
	}
    }
}

void wam_compiler::compile_query(wam_compiler::reg lhsreg, common::term rhs, wam_instruction_sequence &instrs)
{
    switch (rhs.tag()) {
      case common::tag_t::INT:
      case common::tag_t::CON:
      // We have X = a or X = 4711
	instrs.add(wam_instruction<SET_CONSTANT>(rhs));
	break;
      case common::tag_t::REF:
	compile_query_ref(lhsreg, static_cast<common::ref_cell &>(rhs), instrs);
	break;
      case common::tag_t::STR:
	compile_query_str(lhsreg, rhs, instrs);
	break;
    }
}

void wam_compiler::compile_program_ref(wam_compiler::reg lhsreg, common::ref_cell rhsvar, wam_instruction_sequence &instrs)
{
    reg rhsreg;
    bool isnew;
    std::tie(rhsreg, isnew) = allocate_reg(rhsvar);

    if (lhsreg.type == reg::A_REG) {
        // ai = xn
        assert(rhsreg.type == reg::X_REG);
	if (isnew) {
	    instrs.add(wam_instruction<GET_VARIABLE_X>(
			       rhsreg.num, lhsreg.num ));
	} else {
	    instrs.add(wam_instruction<GET_VALUE_X>(
  			       rhsreg.num, lhsreg.num ));
	}
    } else { // lhsreg.type == reg::X_REG
      // xn = V
      // No instruction needs to be emitted
    }
}

void wam_compiler::compile_program_str(wam_compiler::reg lhsreg, common::term rhs, wam_instruction_sequence &instrs)
{
    auto f = env_.functor(rhs);
    if (lhsreg.type == reg::A_REG) {
        instrs.add(wam_instruction<GET_STRUCTURE_A>(f,lhsreg.num));
    } else {
        instrs.add(wam_instruction<GET_STRUCTURE_X>(f,lhsreg.num));
    }
    size_t n = f.arity();
    for (size_t i = 0; i < n; i++) {
        auto arg = env_.arg(rhs, i);
	assert(arg.tag() == common::tag_t::REF);
	auto ref = static_cast<common::ref_cell &>(arg);
	reg r;
	bool isnew = false;
	std::tie(r,isnew) = allocate_reg(ref);
	if (r.type == reg::A_REG) {
	    if (isnew) {
	        instrs.add(wam_instruction<UNIFY_VARIABLE_A>(r.num));
	    } else {
	        instrs.add(wam_instruction<UNIFY_VALUE_A>(r.num));
	    }
	} else {
	    if (isnew) {
	        instrs.add(wam_instruction<UNIFY_VARIABLE_X>(r.num));
	    } else {
	        instrs.add(wam_instruction<UNIFY_VALUE_X>(r.num));
	    }
	}
    }
}

void wam_compiler::compile_program(wam_compiler::reg lhsreg, common::term rhs, wam_instruction_sequence &instrs)
{
    switch (rhs.tag()) {
      case common::tag_t::INT:
      case common::tag_t::CON:
      // We have X = a or X = 4711
	instrs.add(wam_instruction<UNIFY_CONSTANT>(rhs));
	break;
      case common::tag_t::REF:
	compile_program_ref(lhsreg, static_cast<common::ref_cell &>(rhs), instrs);
	break;
      case common::tag_t::STR:
	compile_program_str(lhsreg, rhs, instrs);
	break;
    }
}

void wam_compiler::compile_query_or_program(wam_compiler::term t,
					    wam_compiler::compile_type c,
				      	    wam_instruction_sequence &instrs)

{
    std::vector<prim_unification> prims = flatten(t, c);

    print_prims(prims);

    size_t n = prims.size();

    for (size_t i = 0; i < n; i++) {
	auto &prim = prims[i];
	auto lhsvar = prim.lhs();

	// Won't allocate if there's already an allocation (e.g. if it is
	// an argument it has already been allocated)
	reg lhsreg;
	std::tie(lhsreg, std::ignore) = allocate_reg(lhsvar);

	term rhs = prim.rhs();

	if (c == COMPILE_QUERY) {
	    compile_query(lhsreg, rhs, instrs);
	} else {
	    compile_program(lhsreg, rhs, instrs);
	}
    }
}

std::pair<wam_compiler::reg,bool> wam_compiler::register_pool::allocate(common::ref_cell ref, wam_compiler::reg::reg_type regtype)
{
    return allocate(ref, regtype, 0);
}

std::pair<wam_compiler::reg,bool> wam_compiler::register_pool::allocate(common::ref_cell ref, wam_compiler::reg::reg_type regtype, size_t regno)
{
    auto it = reg_map_.find(ref);
    if (it == reg_map_.end()) {
	size_t regcnt;
	switch (regtype) {
        case reg::A_REG: regcnt = regno; break;
	case reg::X_REG: regcnt = x_cnt_++; break;
	case reg::Y_REG: regcnt = y_cnt_++; break;
	}
	reg r(regcnt, regtype);
	reg_map_.insert(std::make_pair(ref, r));
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
