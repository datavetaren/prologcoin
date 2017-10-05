#include <queue>
#include "wam_compiler.hpp"

namespace prologcoin { namespace interp {

typedef wam_compiler::term term;

// --------------------------------------------------------------
//  wam_interim_code
// --------------------------------------------------------------

wam_interim_code::wam_interim_code(wam_interpreter &interp) : interp_(interp), size_(0)
{
}

void wam_interim_code::push_back(const wam_instruction_base &instr)
{
    wam_instruction_base *i = reinterpret_cast<wam_instruction_base *>(new char[instr.size_in_bytes()]);
    memcpy(i, &instr, instr.size_in_bytes());
    push_back(i);
}

void wam_interim_code::push_back(wam_instruction_base *instr)
{
    if (empty()) {
        push_front(instr);
	end_ = begin();
    } else {
        end_ = insert_after(end_, instr);
    }
    size_++;
}

std::vector<wam_instruction_base *> wam_interim_code::get_all_reversed()
{
    std::vector<wam_instruction_base *> instrs_rev(size_);
    size_t i = size_-1;

    for (auto instr : *this) {
        instrs_rev[i] = instr;
	i--;
    }
    return instrs_rev;
}

std::vector<wam_instruction_base *> wam_interim_code::get_all()
{
    std::vector<wam_instruction_base *> instrs(size_);
    size_t i = 0;

    for (auto instr : *this) {
        instrs[i] = instr;
	i++;
    }
    return instrs;
}

void wam_interim_code::print(std::ostream &out) const
{
    size_t cnt = 0;
    for (auto i : *this) {
	out << "[" << std::setw(5) << cnt << "]: ";
        i->print(out, interp_);
	cnt += i->size();
	out << std::endl;
    }
}

// --------------------------------------------------------------
//  wam_goal_iterator
// --------------------------------------------------------------

void wam_goal_iterator::first_of()
{
    term t;
    while (env_.is_comma((t = stack_.top()))) {
        stack_.pop();
	term arg1 = env_.arg(t, 1);
	term arg0 = env_.arg(t, 0);
	stack_.push(arg1);
	stack_.push(arg0);
    }
}

void wam_goal_iterator::advance()
{
    stack_.pop();
    if (!stack_.empty()) {
        first_of();
    }
}

// --------------------------------------------------------------
//  wam_compiler
// --------------------------------------------------------------

std::vector<wam_compiler::prim_unification> wam_compiler::flatten(
	  const term t,
	  wam_compiler::compile_type for_type,
	  bool is_predicate_call)
{
    std::vector<prim_unification> prims;

    std::queue<prim_unification> worklist;
    auto prim = new_unification(t);
    worklist.push(prim);

    bool is_predicate = is_predicate_call;

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

		// Only flatten constants if we're at the top-level predicate
		if (!is_predicate) {
		    if (arg.tag() == common::tag_t::REF ||
		        arg.tag() == common::tag_t::CON ||
		        arg.tag() == common::tag_t::INT) {
		        continue;
		    }
		}
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
		if (!is_found || is_predicate) {
		    worklist.push(p1);
		}
	    }
	    if (!is_predicate) {
	        prims.push_back(p);
	    }
	    break;
	}
	case common::tag_t::REF:
	case common::tag_t::CON: 
	case common::tag_t::INT: {
	    prims.push_back(p);
	    break;
	}
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

void wam_compiler::compile_query_ref(wam_compiler::reg lhsreg, common::ref_cell rhsvar, wam_interim_code &instrs)
{
    reg rhsreg;
    bool isnew;

    std::tie(rhsreg, isnew) = allocate_reg<X_REG>(rhsvar);

    if (lhsreg.type == A_REG) {
        // ai = xn
        assert(rhsreg.type == X_REG);
	if (isnew) {
	    instrs.push_back(wam_instruction<PUT_VARIABLE_X>(
			     rhsreg.num, lhsreg.num ));
	} else {
	    instrs.push_back(wam_instruction<PUT_VALUE_X>(
  			     rhsreg.num, lhsreg.num ));
	}
    } else { // lhsreg.type == X_REG
        assert(rhsreg.type == A_REG);
	// We shouldn't get Xi = Xj instructions.
	if (isnew) {
	    instrs.push_back(wam_instruction<PUT_VARIABLE_X>(
					     lhsreg.num, rhsreg.num ));
	} else {
	    instrs.push_back(wam_instruction<PUT_VALUE_X>(
					     lhsreg.num, rhsreg.num ));
        }
    }
}

void wam_compiler::compile_query_str(wam_compiler::reg lhsreg, common::term rhs, wam_interim_code &instrs)
{
    auto f = env_.functor(rhs);
    if (lhsreg.type == A_REG) {
        instrs.push_back(wam_instruction<PUT_STRUCTURE_A>(f,lhsreg.num));
    } else {
        instrs.push_back(wam_instruction<PUT_STRUCTURE_X>(f,lhsreg.num));
    }
    size_t n = f.arity();
    for (size_t i = 0; i < n; i++) {
        auto arg = env_.arg(rhs, i);
	if (arg.tag() == common::tag_t::CON ||
	    arg.tag() == common::tag_t::INT) {
	    instrs.push_back(wam_instruction<SET_CONSTANT>(arg));
	} else {
	    assert(arg.tag() == common::tag_t::REF);
	    auto ref = static_cast<common::ref_cell &>(arg);
	    reg r;
	    bool isnew = false;
	    std::tie(r,isnew) = allocate_reg<X_REG>(ref);
	    if (r.type == A_REG) {
	        if (isnew) {
		    instrs.push_back(wam_instruction<SET_VARIABLE_A>(r.num));
		} else {
		    instrs.push_back(wam_instruction<SET_VALUE_A>(r.num));
		}
	    } else {
	        if (isnew) {
		    instrs.push_back(wam_instruction<SET_VARIABLE_X>(r.num));
		} else {
		    instrs.push_back(wam_instruction<SET_VALUE_X>(r.num));
		}
	    }
	}
    }
}

void wam_compiler::compile_query(wam_compiler::reg lhsreg, common::term rhs, wam_interim_code &instrs)
{
    switch (rhs.tag()) {
      case common::tag_t::INT:
      case common::tag_t::CON:
      // We have X = a or X = 4711
	instrs.push_back(wam_instruction<SET_CONSTANT>(rhs));
	break;
      case common::tag_t::REF:
	compile_query_ref(lhsreg, static_cast<common::ref_cell &>(rhs), instrs);
	break;
      case common::tag_t::STR:
	compile_query_str(lhsreg, rhs, instrs);
	break;
    }
}

void wam_compiler::compile_program_ref(wam_compiler::reg lhsreg, common::ref_cell rhsvar, wam_interim_code &instrs)
{
    reg rhsreg;
    bool isnew;
    std::tie(rhsreg, isnew) = allocate_reg<X_REG>(rhsvar);

    if (lhsreg.type == A_REG) {
        // ai = xn
        assert(rhsreg.type == X_REG);
	if (isnew) {
	    instrs.push_back(wam_instruction<GET_VARIABLE_X>(
			       rhsreg.num, lhsreg.num ));
	} else {
	    instrs.push_back(wam_instruction<GET_VALUE_X>(
  			       rhsreg.num, lhsreg.num ));
	}
    } else { // lhsreg.type == reg::X_REG
        // xn = V
        allocate_reg<X_REG>(rhsvar, lhsreg);
    }
}

void wam_compiler::compile_program_str(wam_compiler::reg lhsreg, common::term rhs, wam_interim_code &instrs)
{
    auto f = env_.functor(rhs);
    if (lhsreg.type == A_REG) {
        instrs.push_back(wam_instruction<GET_STRUCTURE_A>(f,lhsreg.num));
    } else {
        instrs.push_back(wam_instruction<GET_STRUCTURE_X>(f,lhsreg.num));
    }
    size_t n = f.arity();
    for (size_t i = 0; i < n; i++) {
        auto arg = env_.arg(rhs, i);
	assert(arg.tag() == common::tag_t::REF);
	auto ref = static_cast<common::ref_cell &>(arg);
	reg r;
	bool isnew = false;
	std::tie(r,isnew) = allocate_reg<X_REG>(ref);
	if (r.type == A_REG) {
	    if (isnew) {
	        instrs.push_back(wam_instruction<UNIFY_VARIABLE_A>(r.num));
	    } else {
	        instrs.push_back(wam_instruction<UNIFY_VALUE_A>(r.num));
	    }
	} else {
	    if (isnew) {
	        instrs.push_back(wam_instruction<UNIFY_VARIABLE_X>(r.num));
	    } else {
	        instrs.push_back(wam_instruction<UNIFY_VALUE_X>(r.num));
	    }
	}
    }
}

void wam_compiler::compile_program(wam_compiler::reg lhsreg, common::term rhs, wam_interim_code &instrs)
{
    switch (rhs.tag()) {
      case common::tag_t::INT:
      case common::tag_t::CON:
      // We have X = a or X = 4711
	instrs.push_back(wam_instruction<UNIFY_CONSTANT>(rhs));
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
				      	    wam_interim_code &instrs)

{
    std::vector<prim_unification> prims = flatten(t, c, true);

    size_t n = prims.size();

    for (size_t i = 0; i < n; i++) {
	auto &prim = prims[i];
	auto lhsvar = prim.lhs();

	// Won't allocate if there's already an allocation (e.g. if it is
	// an argument it has already been allocated)
	reg lhsreg;
	if (is_argument(lhsvar)) {
  	    size_t pos = get_argument_index(lhsvar);
	    std::tie(lhsreg, std::ignore) = allocate_reg<A_REG>(lhsvar, pos);
	} else {
  	    std::tie(lhsreg, std::ignore) = allocate_reg<X_REG>(lhsvar);
	}

	term rhs = prim.rhs();

	if (c == COMPILE_QUERY) {
	    compile_query(lhsreg, rhs, instrs);
	} else {
	    compile_program(lhsreg, rhs, instrs);
	}
    }
}

std::function<uint32_t ()> wam_compiler::x_getter(wam_instruction_base *instr)
{
    switch (instr->type()) {
	case PUT_VARIABLE_X:
	case PUT_VALUE_X:
	case GET_VARIABLE_X:
	case GET_VALUE_X:
	    return [=]{return reinterpret_cast<wam_instruction_binary_reg *>(instr)->reg_1();};
	case PUT_STRUCTURE_X:
	case GET_STRUCTURE_X:
	    return [=]{return reinterpret_cast<wam_instruction_con_reg *>(instr)->reg();};
	case SET_VARIABLE_X:
	case SET_VALUE_X:
	case SET_LOCAL_VALUE_X:
	case UNIFY_VARIABLE_X:
	case UNIFY_VALUE_X:
	case UNIFY_LOCAL_VALUE_X:
	    return [=]{return reinterpret_cast<wam_instruction_unary_reg *>(instr)->reg();};
	default:
	    return nullptr;
    }
}

std::function<void (uint32_t)> wam_compiler::x_setter(wam_instruction_base *instr)
{
    switch (instr->type()) {
	case PUT_VARIABLE_X:
	case PUT_VALUE_X:
	case GET_VARIABLE_X:
	case GET_VALUE_X:
	    return [=](uint32_t xn){reinterpret_cast<wam_instruction_binary_reg *>(instr)->set_reg_1(xn);};
	case PUT_STRUCTURE_X:
	case GET_STRUCTURE_X:
	    return [=](uint32_t xn){reinterpret_cast<wam_instruction_con_reg *>(instr)->set_reg(xn);};
	case SET_VARIABLE_X:
	case SET_VALUE_X:
	case SET_LOCAL_VALUE_X:
	case UNIFY_VARIABLE_X:
	case UNIFY_VALUE_X:
	case UNIFY_LOCAL_VALUE_X:
	    return [=](uint32_t xn){reinterpret_cast<wam_instruction_unary_reg *>(instr)->set_reg(xn);};
	default:
	    return nullptr;
    }
}

std::function<uint32_t ()> wam_compiler::y_getter(wam_instruction_base *instr)
{
    switch (instr->type()) {
	case PUT_VARIABLE_Y:
	case PUT_VALUE_Y:
	case GET_VARIABLE_Y:
	case GET_VALUE_Y:
	    return [=]{return reinterpret_cast<wam_instruction_binary_reg *>(instr)->reg_1();};
	case PUT_STRUCTURE_Y:
	case GET_STRUCTURE_Y:
	    return [=]{return reinterpret_cast<wam_instruction_con_reg *>(instr)->reg();};
	case SET_VARIABLE_Y:
	case SET_VALUE_Y:
	case SET_LOCAL_VALUE_Y:
	case UNIFY_VARIABLE_Y:
	case UNIFY_VALUE_Y:
	case UNIFY_LOCAL_VALUE_Y:
	    return [=]{return reinterpret_cast<wam_instruction_unary_reg *>(instr)->reg();};
	default:
	    return nullptr;
    }
}

std::function<void (uint32_t)> wam_compiler::y_setter(wam_instruction_base *instr)
{
    switch (instr->type()) {
	case PUT_VARIABLE_Y:
	case PUT_VALUE_Y:
	case GET_VARIABLE_Y:
	case GET_VALUE_Y:
	    return [=](uint32_t yn){reinterpret_cast<wam_instruction_binary_reg *>(instr)->set_reg_1(yn);};
	case PUT_STRUCTURE_Y:
	case GET_STRUCTURE_Y:
	    return [=](uint32_t yn){reinterpret_cast<wam_instruction_con_reg *>(instr)->set_reg(yn);};
	case SET_VARIABLE_Y:
	case SET_VALUE_Y:
	case SET_LOCAL_VALUE_Y:
	case UNIFY_VARIABLE_Y:
	case UNIFY_VALUE_Y:
	case UNIFY_LOCAL_VALUE_Y:
	    return [=](uint32_t yn){reinterpret_cast<wam_instruction_unary_reg *>(instr)->set_reg(yn);};
	default:
	    return nullptr;
    }
}

void wam_compiler::change_x_to_y(wam_instruction_base *instr)
{
    switch (instr->type()) {
        case PUT_VARIABLE_X: instr->set_type<PUT_VARIABLE_Y>(); break;
        case PUT_VALUE_X: instr->set_type<PUT_VALUE_Y>(); break;
        case GET_VARIABLE_X: instr->set_type<GET_VARIABLE_Y>(); break;
        case GET_VALUE_X: instr->set_type<GET_VALUE_Y>(); break;
        case PUT_STRUCTURE_X: instr->set_type<PUT_STRUCTURE_Y>(); break;
        case GET_STRUCTURE_X: instr->set_type<GET_STRUCTURE_Y>(); break;
        case SET_VARIABLE_X: instr->set_type<SET_VARIABLE_Y>(); break;
        case SET_VALUE_X: instr->set_type<SET_VALUE_Y>(); break;
        case SET_LOCAL_VALUE_X: instr->set_type<SET_LOCAL_VALUE_Y>(); break;
        case UNIFY_VARIABLE_X: instr->set_type<UNIFY_VARIABLE_Y>(); break;
        case UNIFY_VALUE_X: instr->set_type<UNIFY_VALUE_Y>(); break;
        case UNIFY_LOCAL_VALUE_X: instr->set_type<UNIFY_LOCAL_VALUE_Y>(); break;
	default:
	    break;
    }
}

void wam_compiler::remap_x_registers(wam_interim_code &instrs)
{
    // Find live registers, and remap them.
    std::unordered_map<size_t, size_t> map;
    size_t cnt = 0;
    auto mapit = [&](size_t i) {
      if (map.find(i) == map.end()) {
	  map[i] = cnt;
  	  cnt++;
      }
    };

    for (auto instr : instrs) {
        if (auto x_get = x_getter(instr)) {
	    mapit(x_get());
	}
    }

    for (auto instr : instrs) {
	if (auto x_set = x_setter(instr)) {
	    auto x_get = x_getter(instr);
	    x_set(map[x_get()]);
	}
    }
}

void wam_compiler::find_x_to_y_registers(wam_interim_code &instrs,
					 std::vector<size_t> &x_to_y)
{
    std::unordered_map<size_t, size_t> first_occur;
    bool head_found = false;
    size_t goal_number = 0;
    for (auto instr : instrs) {
        if (is_interim_instruction(instr)) {
	    auto interim = reinterpret_cast<wam_interim_instruction_base *>(instr);
	    if (interim->type() == INTERIM_HEAD) {
	        head_found = true;
	    } else if (!head_found) {
	        continue;
	    }
	    if (interim->type() == INTERIM_GOAL) {
	        goal_number++;
	    }
	}
	// Map goal number 0 and 1 to the same ID 1 (as head and goal should be
	// unified into the same set)
	if (auto x_get = x_getter(instr)) {
	    size_t id = (goal_number <= 1) ? 1 : goal_number;
	    size_t xn = x_get();
	    auto it = first_occur.find(xn);
	    if (it == first_occur.end()) {
	        first_occur[xn] = id;
	    } else if (it->second != id) {
	        x_to_y.push_back(xn);
	    }
        }
    }
}


void wam_compiler::allocate_y_registers(wam_interim_code &instrs)
{
    std::vector<size_t> x_to_y;
    find_x_to_y_registers(instrs, x_to_y);

     size_t y_cnt = 0;
    std::unordered_map<size_t, size_t> map;

    // Create x => y map
    for (auto x : x_to_y) {
	map[x] = y_cnt;
	y_cnt++;
    }

    // Change X to Y instructions and set new register number
    for (auto instr : instrs) {
        if (auto x_get = x_getter(instr)) {
	    auto it = map.find(x_get());
	    if (it != map.end()) {
	        change_x_to_y(instr);
		auto y_set = y_setter(instr);
		y_set(it->second);
	    }
        }
    }
}

void wam_compiler::remap_y_registers(wam_interim_code &instrs)
{
    auto instrs_rev = instrs.get_all_reversed();

    std::unordered_map<size_t, size_t> map;
    size_t cnt = 0;
    auto mapit = [&](size_t i) {
      if (map.find(i) == map.end()) {
	  map[i] = cnt;
  	  cnt++;
      }
    };

    for (auto instr : instrs_rev) {
        if (auto y_get = y_getter(instr)) {
	    mapit(y_get());
	}
    }

    for (auto instr : instrs_rev) {
	if (auto y_set = y_setter(instr)) {
	    auto y_get = y_getter(instr);
	    y_set(map[y_get()]);
	}
    }
}

void wam_compiler::update_calls_for_environment_trimming(wam_interim_code &instrs)
{
    auto instrs_rev = instrs.get_all_reversed();
    size_t biggest_y = 0;
    for (auto instr : instrs_rev) {
        if (auto y_get = y_getter(instr)) {
	    biggest_y = std::max(biggest_y, y_get());
	}
	if (instr->type() == CALL) {
	    auto call_instr = reinterpret_cast<wam_instruction<CALL> *>(instr);
	    call_instr->set_num_y(biggest_y+1);
	}
    }
}

void wam_compiler::find_unsafe_y_registers(wam_interim_code &instrs,
					   std::unordered_set<size_t> &unsafe_y_regs)
{
    // We need at least one call instruction.
    size_t num_calls = 0;
    for (auto instr : instrs) {
        if (instr->type() == CALL) {
	    num_calls++;
	}
    }

    // There can't be unsafe variables if we dont' have call instructions
    if (num_calls == 0) {
        return;
    }

    auto instrs_rev = instrs.get_all_reversed();

    for (auto instr : instrs_rev) {
        // Don't search further when we've found the last call.
        // All Y variables after last call are in "unsafe" state.
        if (is_interim_instruction(instr)) {
	    auto interim_instr = reinterpret_cast<wam_interim_instruction_base *>(instr);
	    if (interim_instr->type() == INTERIM_GOAL) {
	        // No point in going beyond the last call
	        break;
	    }
        }
	if (instr->type() == PUT_VALUE_Y) {
	    auto y_get = y_getter(instr);
	    size_t yn = y_get();
	    unsafe_y_regs.insert(yn);
        }
    }
}

void wam_compiler::remap_to_unsafe_y_registers(wam_interim_code &instrs)
{
    std::unordered_set<size_t> unsafe_y_regs;
    find_unsafe_y_registers(instrs, unsafe_y_regs);
    if (unsafe_y_regs.empty()) {
        return;
    }
    auto all = instrs.get_all();
    auto n = all.size();
    size_t last_goal_index = 0;
    for (size_t i = 0; i < n; i++) {
        size_t j = n - i - 1;
        auto instr = all[j];
	if (is_interim_instruction(instr)) {
	    auto interim_instr = reinterpret_cast<wam_interim_instruction_base *>(instr);
	    if (interim_instr->type() == INTERIM_GOAL) {
	        last_goal_index = j;
	        break;
	    }
	}
    }

    for (size_t i = last_goal_index; i < n; i++) {
        auto instr = all[i];
	if (instr->type() == PUT_VALUE_Y) {
	    size_t yn = y_getter(instr)();
	    if (unsafe_y_regs.find(yn) != unsafe_y_regs.end()) {
	        instr->set_type<PUT_UNSAFE_VALUE_Y>();
		unsafe_y_regs.erase(yn);
	    }
	}
    }
}

bool wam_compiler::clause_needs_environment(const term clause)
{
    auto body = clause_body(clause);
    (void)body;
    return true;
}

void wam_compiler::compile_clause(const term clause, wam_interim_code &seq)
{
    common::con_cell unification_op("=", 2);

    // First analyze how many calls we have.
    // We only need an environment if there are more than 1 call.

    bool needs_env = clause_needs_environment(clause);

    if (needs_env) seq.push_back(wam_instruction<ALLOCATE>());

    term head = clause_head(clause);
    seq.push_back(wam_interim_instruction<INTERIM_HEAD>());
    compile_query_or_program(head, COMPILE_PROGRAM, seq);

    term body = clause_body(clause);
    term last_goal;
    for (auto goal : for_all_goals(body)) {
        if (last_goal) {
	    auto f = env_.functor(last_goal);
	    bool is_prim = f == unification_op;
	    if (!is_prim) {
	        seq.push_back(wam_instruction<CALL>(f, 0));
	    }
        }
	auto f = env_.functor(goal);
	bool is_prim = f == unification_op;
	if (!is_prim) {
	    seq.push_back(wam_interim_instruction<INTERIM_GOAL>());
	    compile_query_or_program(goal, COMPILE_QUERY, seq);
	}
	last_goal = goal;
    }
    if (needs_env) seq.push_back(wam_instruction<DEALLOCATE>());
    if (last_goal) {
        auto f = env_.functor(last_goal);
	bool is_prim = f == unification_op;
	if (!is_prim) {
	    seq.push_back(wam_instruction<EXECUTE>(f));
	}
    }

    remap_x_registers(seq);
    allocate_y_registers(seq);
    remap_y_registers(seq);
    update_calls_for_environment_trimming(seq);
    remap_to_unsafe_y_registers(seq);
}

term wam_compiler::clause_head(const term clause)
{
    common::con_cell implication(":-", 2);
    if (env_.functor(clause) == implication) {
        return env_.arg(clause, 0);
    } else {
        return clause; // It's a fact
    }
}

term wam_compiler::clause_body(const term clause)
{
    common::con_cell implication(":-", 2);
    if (env_.functor(clause) == implication) {
        return env_.arg(clause, 1);
    } else {
        common::con_cell ctrue("true", 0);
        return ctrue;
    }
}

std::vector<std::vector<term> > wam_compiler::partition_clauses(const std::vector<term> &clauses, std::function<bool (const term t1, const term t2)> pred)
{
    std::vector<std::vector<term> > partitioned;

    partitioned.push_back(std::vector<term>());
    auto *v = &partitioned.back();
    bool has_last_clause = false;
    term last_clause;
    for (auto &clause : clauses) {
        auto head = clause_head(clause);
	auto f = env_.functor(head);
	if (f.arity() < 1) {
	    // There's no argument, so no partition can be made. All clauses
	    // becomes a single partition.
	    *v = clauses;
	    return partitioned;
	}

	bool is_diff = false;

	// If there's a preceding clause, use the predicate to see
	// if these two are disjoint or not.
	if (has_last_clause) {
	    is_diff = pred(last_clause, clause);
	}
	if (is_diff) {
	    // It's a var, if v is non-empty, push it and create a new one
	    if (!v->empty()) {
	        partitioned.push_back(std::vector<term>());
		v = &partitioned.back();
	    }
	}
	v->push_back(clause);
	last_clause = clause;
	has_last_clause = true;
    }

    return partitioned;
}

term wam_compiler::first_arg(const term clause)
{
    return env_.arg(clause_head(clause), 0);
}

bool wam_compiler::first_arg_is_var(const term clause)
{
    term arg = first_arg(clause);
    return arg.tag() == common::tag_t::REF;
}

bool wam_compiler::first_arg_is_con(const term clause)
{
    term arg = first_arg(clause);
    return arg.tag() == common::tag_t::CON;
}

bool wam_compiler::first_arg_is_str(const term clause)
{
    term arg = first_arg(clause);
    return arg.tag() == common::tag_t::STR;
}

std::vector<std::vector<term> > wam_compiler::partition_clauses_nonvar(const std::vector<term> &clauses)
{
    return partition_clauses(clauses,
       [&] (const term c1, const term c2)
	     { return first_arg_is_var(c1) || first_arg_is_var(c2); } );
}

std::vector<std::vector<term> > wam_compiler::partition_clauses_first_arg(const std::vector<term> &clauses)
{
    std::unordered_map<term, std::vector<term> > map;
    std::vector<term> refs;
    std::vector<term> order;

    for (auto &clause : clauses) {
        auto head = clause_head(clause);
	auto arg = first_arg(head);
	switch (arg.tag()) {
	case common::tag_t::REF:
	    refs.push_back(clause);
	    break;
	case common::tag_t::CON:
	case common::tag_t::STR: {
	    auto f = env_.functor(arg);
	    bool is_new = map.count(f) == 0;
	    auto &v = map[f];
	    v.push_back(clause);
	    if (is_new) order.push_back(f);
	    break;
	    }
	case common::tag_t::INT: {
	    bool is_new = map.count(arg) == 0;
	    auto &v = map[arg];
	    v.push_back(clause);
	    if (is_new) order.push_back(arg);
	    break;
    	    }
	}
    }

    std::vector<std::vector<term> > result;
    if (!refs.empty()) {
        result.push_back(refs);
    }

    for (auto o : order) {
        result.push_back(map[o]);
    }

    return result;
}

void wam_compiler::print_partition(std::ostream &out, const std::vector<std::vector<term> > &p)
{
    size_t i = 0;
    for (auto &cs : p) {
        out << "Section " << i << ": " << std::endl;
	for (auto &c : cs) {
	    out << "   " << env_.to_string(c) << std::endl;
	}
	i++;
    }
}

std::pair<wam_compiler::reg,bool> wam_compiler::register_pool::allocate(common::ref_cell ref)
{
    auto it = reg_map_.find(ref);
    if (it == reg_map_.end()) {
        size_t regcnt = reg_map_.size();
	reg r(regcnt, reg_type_);
	allocate(ref, r);
	return std::make_pair(r, true);
    } else {
        return std::make_pair(it->second, false);
    }
}

std::pair<wam_compiler::reg,bool> wam_compiler::register_pool::allocate(common::ref_cell ref, size_t index)
{
    reg r(index, reg_type_);
    allocate(ref, r);
    return std::make_pair(r, true);
}

void wam_compiler::register_pool::allocate(common::ref_cell ref, wam_compiler::reg r)
{
    reg_map_[ref] = r;
}


void wam_compiler::register_pool::deallocate(common::ref_cell ref)
{
    reg_map_.erase(ref);
}

void wam_compiler::print_prims(const std::vector<wam_compiler::prim_unification> &prims ) const
{
    for (auto &p : prims) {
	std::cout << "   " << env_.to_string(p.lhs()) << " = " << env_.to_string(p.rhs()) << std::endl;
    }
}

}}
