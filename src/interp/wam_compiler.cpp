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
		auto arg = env_.arg(p.rhs(), i);
		prim_unification p1 = new_unification(arg);
		if (is_predicate) {
		    regs_.allocate(p1.lhs(), reg::A_REG, i);
		}
		env_.set_arg(p.rhs(), i, p1.lhs());
		worklist.push(p1);
		if (for_type == COMPILE_QUERY) {
		    prims.push_back(p1);
		}
	    }
	    prims.push_back(p);
	    break;
	}
	case common::tag_t::CON: 
	case common::tag_t::REF: 
	case common::tag_t::INT: {
	    break;
	}
	}
	is_predicate = false;
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

    print_prims(prims);

    size_t n = prims.size();

    for (size_t i = 0; i < n; i++) {
	auto &prim = prims[i];
	auto lhsvar = prim.lhs();

	// Won't allocate if there's already an allocation (e.g. if it is
	// an argument it has already been allocated)
	reg lhsreg;
	std::tie(lhsreg, std::ignore) = regs_.allocate(lhsvar, reg::X_REG);

	term rhs = prim.rhs();

	if (c == COMPILE_QUERY) {
	    switch (rhs.tag()) {
	    case common::tag_t::INT:
	    case common::tag_t::CON:
	      // We have X = a or X = 4711
	      instrs.add(wam_instruction<SET_CONSTANT>(rhs));
	      break;
	    case common::tag_t::REF:
	      { 
		auto rhsvar = static_cast<common::ref_cell &>(rhs);
		reg rhsreg;
		bool isnew;
		std::tie(rhsreg, isnew) = regs_.allocate(rhsvar, reg::X_REG);

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
              break;
	  case common::tag_t::STR:
	      {
		  auto f = env_.functor(rhs);
		  if (lhsreg.type == reg::A_REG) {
		      instrs.add(wam_instruction<PUT_STRUCTURE_A>(
						  f,lhsreg.num));
	 	  } else {
		      instrs.add(wam_instruction<PUT_STRUCTURE_X>(
						  f,lhsreg.num));
		  }
		  size_t n = f.arity();
	          for (size_t i = 0; i < n; i++) {
		      auto arg = env_.arg(rhs, i);
		      assert(arg.tag() == common::tag_t::REF);
		      auto ref = static_cast<common::ref_cell &>(arg);
		      reg r;
		      bool isnew = false;
		      std::tie(r,isnew) = regs_.allocate(ref, reg::X_REG);
		      if (r.type == reg::A_REG) {
			  if (isnew) {
			      instrs.add(wam_instruction<SET_VARIABLE_A>(
							 r.num));
			 } else {
			      instrs.add(wam_instruction<SET_VALUE_A>(
						         r.num));
		         }
		      } else {
			  if (isnew) {
			      instrs.add(wam_instruction<SET_VARIABLE_X>(
							 r.num));
			 } else {
			      instrs.add(wam_instruction<SET_VALUE_X>(
						         r.num));
		         }
		      }
		  }
	      }
	      break;
	  }
	}
    }
}

std::pair<wam_compiler::reg,bool> wam_compiler::register_pool::allocate(common::ref_cell var, wam_compiler::reg::reg_type regtype)
{
    return allocate(var, regtype, 0);
}

std::pair<wam_compiler::reg,bool> wam_compiler::register_pool::allocate(common::ref_cell var, wam_compiler::reg::reg_type regtype, size_t regno)
{
    auto it = reg_map_.find(var);
    if (it == reg_map_.end()) {
	size_t regcnt;
	switch (regtype) {
        case reg::A_REG: regcnt = regno; break;
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
