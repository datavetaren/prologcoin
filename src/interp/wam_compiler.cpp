#include <queue>
#include <algorithm>
#include "wam_compiler.hpp"
#include "wam_interpreter.hpp"

namespace prologcoin { namespace interp {

typedef wam_compiler::term term;

// --------------------------------------------------------------
//  wam_interim_code
// --------------------------------------------------------------

wam_interim_code::wam_interim_code(wam_interpreter &interp) : interp_(interp), size_(0)
{
}

wam_instruction_base * wam_interim_code::new_instruction(const wam_instruction_base &instr)
{
    wam_instruction_base *i = reinterpret_cast<wam_instruction_base *>(new char[instr.size_in_bytes()]);
    memcpy(i, &instr, instr.size_in_bytes());
    return i;
}

void wam_interim_code::push_back(const wam_instruction_base &instr)
{
    auto *i = new_instruction(instr);
    push_back(i);
}

void wam_interim_code::push_back(wam_instruction_base *instr)
{
    if (empty()) {
        push_front(instr);
	end_ = begin();
	size_++;
    } else {
        end_ = insert_after(end_, instr);
    }
}

void wam_interim_code::append(const wam_interim_code &other)
{
    for (auto instr : other) {
        push_back(instr);
    }
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

    term_map_.clear();
    argument_pos_.clear();

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

		if (is_found && !is_predicate) {
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
	    if (!is_predicate) {
		prims.push_back(p);
	    }
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

void wam_compiler::compile_query_str(wam_compiler::reg lhsreg, common::ref_cell lhsvar, common::term rhs, wam_interim_code &instrs)
{
    auto f = env_.functor(rhs);
    if (lhsreg.type == A_REG) {
        instrs.push_back(wam_instruction<PUT_STRUCTURE_A>(f,lhsreg.num));
	if (has_reg<X_REG>(lhsvar)) {
	    wam_compiler::reg xreg;
	    std::tie(xreg, std::ignore) = allocate_reg<X_REG>(lhsvar);
	    instrs.push_back(wam_instruction<GET_VALUE_X>(xreg.num, lhsreg.num));
	}
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
	    if (has_reg<A_REG>(ref)) {
		std::tie(r,std::ignore) = allocate_reg<A_REG>(ref);
	    } else {
		std::tie(r,isnew) = allocate_reg<X_REG>(ref);
	    }
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

void wam_compiler::compile_query(wam_compiler::reg lhsreg, common::ref_cell lhsvar, common::term rhs, wam_interim_code &instrs)
{
    switch (rhs.tag()) {
      case common::tag_t::INT:
      case common::tag_t::CON:
	// We have X = a or X = 4711
	if (lhsreg.type == A_REG) {
  	    instrs.push_back(wam_instruction<PUT_CONSTANT>(rhs, lhsreg.num));
	} else {
	    instrs.push_back(wam_instruction<SET_CONSTANT>(rhs));
	}
	break;
      case common::tag_t::REF:
	compile_query_ref(lhsreg, static_cast<common::ref_cell &>(rhs), instrs);
	break;
      case common::tag_t::STR:
	compile_query_str(lhsreg, lhsvar, rhs, instrs);
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

void wam_compiler::compile_program_str(wam_compiler::reg lhsreg, common::ref_cell lhsvar, common::term rhs, wam_interim_code &instrs)
{
    auto f = env_.functor(rhs);
    if (lhsreg.type == A_REG) {
        instrs.push_back(wam_instruction<GET_STRUCTURE_A>(f,lhsreg.num));
	if (has_reg<X_REG>(lhsvar)) {
	    wam_compiler::reg xreg;
	    std::tie(xreg, std::ignore) = allocate_reg<X_REG>(lhsvar);
	    instrs.push_back(wam_instruction<GET_VALUE_X>(xreg.num, lhsreg.num));
	}

    } else {
        instrs.push_back(wam_instruction<GET_STRUCTURE_X>(f,lhsreg.num));
    }
    size_t n = f.arity();
    for (size_t i = 0; i < n; i++) {
        auto arg = env_.arg(rhs, i);
	if (arg.tag() == common::tag_t::CON ||
	    arg.tag() == common::tag_t::INT) {
	    instrs.push_back(wam_instruction<UNIFY_CONSTANT>(arg));
	} else {
	    assert(arg.tag() == common::tag_t::REF);
	    auto ref = static_cast<common::ref_cell &>(arg);
	    reg r;
	    bool isnew = false;
	    if (has_reg<A_REG>(ref)) {
		std::tie(r,std::ignore) = allocate_reg<A_REG>(ref);
	    } else {
		std::tie(r,isnew) = allocate_reg<X_REG>(ref);
	    }
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
}

void wam_compiler::compile_program(wam_compiler::reg lhsreg, common::ref_cell lhsvar, common::term rhs, wam_interim_code &instrs)
{
    switch (rhs.tag()) {
      case common::tag_t::INT:
      case common::tag_t::CON:
      // We have X = a or X = 4711
        instrs.push_back(wam_instruction<GET_CONSTANT>(rhs, lhsreg.num));
	break;
      case common::tag_t::REF:
	compile_program_ref(lhsreg, static_cast<common::ref_cell &>(rhs), instrs);
	break;
      case common::tag_t::STR:
	compile_program_str(lhsreg, lhsvar, rhs, instrs);
	break;
    }
}

void wam_compiler::compile_query_or_program(wam_compiler::term t,
					    wam_compiler::compile_type c,
				      	    wam_interim_code &instrs)

{
    std::vector<prim_unification> prims = flatten(t, c, true);

    // std::cout << "COMPILE " << env_.to_string(t) << "\n";
    // print_prims(prims);

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
	    compile_query(lhsreg, lhsvar, rhs, instrs);
	} else {
	    compile_program(lhsreg, lhsvar, rhs, instrs);
	}
    }
}

common::int_cell wam_compiler::new_label()
{
    common::int_cell lab(label_count_);
    label_count_++;
    return lab;
}

std::function<size_t ()> wam_compiler::x_getter(wam_instruction_base *instr)
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
        case GET_LEVEL:
        case CUT:
	    return [=]{return reinterpret_cast<wam_instruction_unary_reg *>(instr)->reg();};
	default:
	    return nullptr;
    }
}

std::function<void (size_t)> wam_compiler::x_setter(wam_instruction_base *instr)
{
    switch (instr->type()) {
	case PUT_VARIABLE_X:
	case PUT_VALUE_X:
	case GET_VARIABLE_X:
	case GET_VALUE_X:
	    return [=](size_t xn){reinterpret_cast<wam_instruction_binary_reg *>(instr)->set_reg_1(xn);};
	case PUT_STRUCTURE_X:
	case GET_STRUCTURE_X:
	    return [=](size_t xn){reinterpret_cast<wam_instruction_con_reg *>(instr)->set_reg(xn);};
	case SET_VARIABLE_X:
	case SET_VALUE_X:
	case SET_LOCAL_VALUE_X:
	case UNIFY_VARIABLE_X:
	case UNIFY_VALUE_X:
	case UNIFY_LOCAL_VALUE_X:
        case GET_LEVEL:
        case CUT:
	    return [=](size_t xn){reinterpret_cast<wam_instruction_unary_reg *>(instr)->set_reg(xn);};
	default:
	    return nullptr;
    }
}

std::function<size_t ()> wam_compiler::y_getter(wam_instruction_base *instr)
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
        case GET_LEVEL:
        case CUT:
	    return [=]{return reinterpret_cast<wam_instruction_unary_reg *>(instr)->reg();};
	default:
	    return nullptr;
    }
}

std::function<void (size_t)> wam_compiler::y_setter(wam_instruction_base *instr)
{
    switch (instr->type()) {
	case PUT_VARIABLE_Y:
	case PUT_VALUE_Y:
	case GET_VARIABLE_Y:
	case GET_VALUE_Y:
	    return [=](size_t yn){reinterpret_cast<wam_instruction_binary_reg *>(instr)->set_reg_1(yn);};
	case PUT_STRUCTURE_Y:
	case GET_STRUCTURE_Y:
	    return [=](size_t yn){reinterpret_cast<wam_instruction_con_reg *>(instr)->set_reg(yn);};
	case SET_VARIABLE_Y:
	case SET_VALUE_Y:
	case SET_LOCAL_VALUE_Y:
	case UNIFY_VARIABLE_Y:
	case UNIFY_VALUE_Y:
	case UNIFY_LOCAL_VALUE_Y:
        case GET_LEVEL:
        case CUT:
	    return [=](size_t yn){reinterpret_cast<wam_instruction_unary_reg *>(instr)->set_reg(yn);};
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
        case GET_LEVEL:
        case CUT: break;
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
	    size_t xn = x_get();
	    if (instr->type() == GET_LEVEL) {
		x_to_y.push_back(xn);
	    } else {
		size_t id = (goal_number <= 1) ? 1 : goal_number;
		auto it = first_occur.find(xn);
		if (it == first_occur.end()) {
		    first_occur[xn] = id;
		} else if (it->second != id) {
		    x_to_y.push_back(xn);
		}
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

int wam_compiler::find_maximum_y_register(wam_interim_code &instrs)
{
    int biggest_y = -1;

    for (auto instr : instrs) {
        if (auto y_get = y_getter(instr)) {
	    auto y = y_get();
	    if (static_cast<int>(y) > biggest_y) {
		biggest_y = static_cast<int>(y);
	    }
	}	
    }

    return biggest_y;
}

void wam_compiler::update_calls_for_environment_trimming(wam_interim_code &instrs)
{
    auto instrs_rev = instrs.get_all_reversed();
    int biggest_y = -1;
    for (auto instr : instrs_rev) {
        if (auto y_get = y_getter(instr)) {
	    auto y = y_get();
	    if (static_cast<int>(y) > biggest_y) {
		biggest_y = static_cast<int>(y);
	    }
	}
	if (instr->type() == CALL) {
	    auto call_instr = reinterpret_cast<wam_instruction<CALL> *>(instr);
	    call_instr->set_num_y(static_cast<size_t>(biggest_y+1));
	}
	if (instr->type() == BUILTIN_R) {
	    auto bn_instr = reinterpret_cast<wam_instruction<BUILTIN_R> *>(instr);
	    bn_instr->set_num_y(static_cast<size_t>(biggest_y+1));
	}
    }
}

void wam_compiler::find_unsafe_y_registers(wam_interim_code &instrs,
					   std::unordered_set<size_t> &unsafe_y_regs)
{
    // We need at least one call instruction.
    size_t num_calls = 0;
    for (auto instr : instrs) {
        if (instr->type() == CALL || instr->type() == BUILTIN_R) {
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

void wam_compiler::eliminate_interim(wam_interim_code &instrs)
{
    auto it = instrs.begin();
    auto it_end = instrs.end();
    auto it_prev = instrs.before_begin();
    while (it != it_end) {
        if (is_interim_instruction(*it)) {
	    auto *instr = *it;
	    it = instrs.erase_after(it_prev);
	    delete instr;
        } else {
  	    it_prev = it;
	    ++it;
	}
    }
}

bool wam_compiler::has_cut(wam_interim_code &instrs)
{
    for (auto instr : instrs) {
	if (instr->type() == CUT) {
	    return true;
	}
    }
    return false;
}

wam_compiler::reg wam_compiler::allocate_cut(wam_interim_code &instrs)
{
    reg levreg;

    auto it = instrs.begin();
    auto it_end = instrs.end();

    while (it != it_end) {
        if ((*it)->type() == ALLOCATE) {
	    term t = env_.new_ref();
	    auto &v = reinterpret_cast<common::ref_cell &>(t);
	    std::tie(levreg, std::ignore) = allocate_reg<X_REG>(v);
	    const wam_instruction<GET_LEVEL>getlev(levreg.num);
	    it =instrs.insert_after(it,getlev);
        } else if ((*it)->type() == CUT) {
	    auto *cut_instr = reinterpret_cast<wam_instruction<CUT> *>(*it);
	    cut_instr->set_yn(levreg.num);
	    ++it;
	} else {
	    ++it;
	}
    }
    return levreg;
}

bool wam_compiler::clause_needs_environment(const term clause)
{
    auto body = clause_body(clause);
    (void)body;
    return true;
}

void wam_compiler::compile_builtin(common::con_cell f, wam_interim_code &seq)
{
    static const common::con_cell bn_true = common::con_cell("true",0);

    if (f != bn_true) {
	auto &bn = get_builtin(f);
	if (bn.is_recursive()) {
	    seq.push_back(wam_instruction<BUILTIN_R>(f, bn.fn(), 0));
	} else {
	    if (bn.fn() == builtins::operator_cut) {
		seq.push_back(wam_instruction<CUT>(0));
	    } else {
		seq.push_back(wam_instruction<BUILTIN>(f, bn.fn()));
	    }
	}
    }
}

void wam_compiler::compile_goal(const term goal, wam_interim_code &seq)
{
    static const common::con_cell bn_disj = common::con_cell(";",2);

    (void)bn_disj;

    auto f = env_.functor(goal);
    bool isbn = is_builtin(f);
    // if (isbn && f == bn_disj) {
	    // compile_disjunction(goal, seq);
	// }
    compile_query_or_program(goal, COMPILE_QUERY, seq);
    if (isbn) {
	compile_builtin(f, seq);
    } else {
	seq.push_back(wam_instruction<CALL>(f, 0));
    }
}

void wam_compiler::peephole_opt_execute(wam_interim_code &seq)
{
    auto it = seq.begin();

    // CALL + DEALLOCATE + PROCEED => EXECUTE

    while (it != seq.end()) {
	auto it_0 = it; ++it_0;
	auto it_1 = it_0; if (it_1 != seq.end()) ++it_1;
	auto it_2 = it_1; if (it_2 != seq.end()) ++it_2;

	// Find pattern CALL + DEALLOCATE + PROCEED
	if (seq.is_at_type(it_0,CALL) &&
	    seq.is_at_type(it_1,DEALLOCATE) &&
	    seq.is_at_type(it_2,PROCEED)) {
	    auto it_3 = it_2; ++it_3;

	    // Memorize CALL and extract predicate (saved as 'f')
	    wam_instruction<CALL> *call_instr
	       = reinterpret_cast<wam_instruction<CALL> *>(*it_0);
	    common::con_cell f = call_instr->pn();
	    delete *it_0;
	    delete *it_1;
	    delete *it_2;

	    // Remove all instructions
	    seq.erase_after(it, it_3);

	    // Add these new ones
	    it = seq.insert_after(it, wam_instruction<DEALLOCATE>());
	    it = seq.insert_after(it, wam_instruction<EXECUTE>(f));
	} else {
	    ++it;
	}
    }
}

void wam_compiler::compile_clause(const term clause0, wam_interim_code &seq)
{
    // We'll make a copy of the clause to be processed.
    // The reason is that the flattening process
    // (inside compile_query_or_program) touches the vars as it
    // unfolds the inner terms.

    term clause = interp_.copy(clause0);

    // First analyze how many calls we have.
    // We only need an environment if there are more than 1 call.

    bool needs_env = clause_needs_environment(clause);

    if (needs_env) {
	seq.push_back(wam_instruction<ALLOCATE>());
    }

    term head = clause_head(clause);
    seq.push_back(wam_interim_instruction<INTERIM_HEAD>());

    compile_query_or_program(head, COMPILE_PROGRAM, seq);

    term body = clause_body(clause);
    for (auto goal : for_all_goals(body)) {
	seq.push_back(wam_interim_instruction<INTERIM_GOAL>());
	compile_goal(goal, seq);
    }
    if (needs_env) {
	seq.push_back(wam_instruction<DEALLOCATE>());
    }
    seq.push_back(wam_instruction<PROCEED>());

    peephole_opt_execute(seq);

    seq.print(std::cout);
    
    if (has_cut(seq)) {
	allocate_cut(seq);
    }
    remap_x_registers(seq);
    allocate_y_registers(seq);
    remap_y_registers(seq);
    update_calls_for_environment_trimming(seq);
    remap_to_unsafe_y_registers(seq);
    eliminate_interim(seq);
}

void wam_compiler::emit_cp(std::vector<common::int_cell> &labels, size_t index, size_t n, wam_interim_code &instrs)
{
    instrs.push_back(wam_interim_instruction<INTERIM_LABEL>(labels[2*index]));
    if (index == 0) {
        instrs.push_back(wam_instruction<TRY_ME_ELSE>(labels[2*index+2]));
    } else if (index < n - 1) {
        instrs.push_back(wam_instruction<RETRY_ME_ELSE>(labels[2*index+2]));
    } else {
        instrs.push_back(wam_instruction<TRUST_ME>());
    }
    instrs.push_back(wam_interim_instruction<INTERIM_LABEL>(labels[2*index+1]));
}

std::vector<common::int_cell> wam_compiler::new_labels(size_t n)
{
    std::vector<common::int_cell> lbls;
    lbls.reserve(n);
    for (size_t i = 0; i < n; i++) {
        lbls.push_back(new_label());
    }
    return lbls;
}

std::vector<common::int_cell> wam_compiler::new_labels_dup(size_t n)
{
    std::vector<common::int_cell> lbls;
    lbls.reserve(n);
    for (size_t i = 0; i < n; i++) {
	auto lbl = new_label();
        lbls.push_back(lbl);
        lbls.push_back(lbl);
    }
    return lbls;
}

std::vector<size_t> wam_compiler::find_clauses_on_cat(const std::vector<term> &clauses, wam_compiler::first_arg_cat_t cat)
{
    std::vector<size_t> found;
    size_t index = 0;
    for (auto &clause : clauses) {
        if (first_arg_cat(clause) == cat) {
	    found.push_back(index);
        }
	index++;
    }
    return found;
}

void wam_compiler::emit_switch_on_term(std::vector<term> &subsection, std::vector<common::int_cell> &labels, wam_interim_code &instrs)
{
    auto on_var_cp = code_point(labels[0]);

    auto on_con = find_clauses_on_cat(subsection, FIRST_CON);
    auto on_con_cp = on_con.empty() ? code_point::fail() 
	           : (on_con.size() == 1) ? code_point(labels[2*on_con[0]+1])
	           : new_label();

    auto on_lst = find_clauses_on_cat(subsection, FIRST_LST);
    auto on_lst_cp = on_lst.empty() ? code_point::fail() 
	           : (on_lst.size() == 1) ? code_point(labels[2*on_lst[0]+1])
	           : new_label();

    auto on_str = find_clauses_on_cat(subsection, FIRST_STR);
    auto on_str_cp = on_str.empty() ? code_point::fail() 
	           : (on_str.size() == 1) ? code_point(labels[2*on_str[0]+1])
	           : new_label();

    instrs.push_back(wam_instruction<SWITCH_ON_TERM>(on_var_cp, on_con_cp, on_lst_cp, on_str_cp));

    emit_second_level_indexing(FIRST_CON,subsection,labels,on_con,on_con_cp,instrs);
    emit_second_level_indexing(FIRST_LST,subsection,labels,on_lst,on_lst_cp,instrs);
    emit_second_level_indexing(FIRST_STR,subsection,labels,on_str,on_str_cp,instrs);
}

void wam_compiler::emit_third_level_indexing(
	     std::vector<size_t> &clause_indices,
	     std::vector<common::int_cell> &labels,
	     wam_interim_code &instrs)
{
    size_t n = clause_indices.size();
    for (size_t i = 0; i < n; i++) {
	auto ci = clause_indices[i];
	if (i == 0) instrs.push_back(wam_instruction<TRY>(labels[2*ci+1]));
	else if (i < n - 1) instrs.push_back(wam_instruction<RETRY>(labels[2*ci+1]));
	else instrs.push_back(wam_instruction<TRUST>(labels[2*ci+1]));
    }
}

void wam_compiler::emit_second_level_indexing(
	      wam_compiler::first_arg_cat_t cat,
	      std::vector<term> &subsection,
	      std::vector<common::int_cell> &labels,
	      std::vector<size_t> &clause_indices,
	      code_point cp,
	      wam_interim_code &instrs)
{
    if (clause_indices.size() < 2) {
	return;
    }

    const common::int_cell &ic = static_cast<const common::int_cell &>(cp.term_code());
    instrs.push_back(wam_interim_instruction<INTERIM_LABEL>(ic));

    auto *map = interp_.new_hash_map();
    std::vector<term> for_third_arg;
    std::vector<std::vector<size_t> > for_third_indices;
    for (auto clause_index : clause_indices) {
	common::int_cell new_lbl(0);
	auto &clause = subsection[clause_index];

	auto arg0 = first_arg(clause);

	// Already managed?
	if (map->count(arg0)) {
	    continue;
	}

	// Get all clauses with the same arg
	std::vector<size_t> same_arg0;
	for (auto ci : clause_indices) {
	    auto &other_clause = subsection[ci];
	    auto other_arg0 = first_arg(other_clause);
	    if (arg0 == other_arg0) {
		same_arg0.push_back(ci);
	    }
	}
	if (same_arg0.size() == 1) {
	    // Unique? Then direct jump
	    map->insert(std::make_pair(arg0, code_point(labels[2*same_arg0[0]+1])));
	} else {
	    // Multiple, so create third level indexing
	    new_lbl = new_label();
	    map->insert(std::make_pair(arg0, code_point(new_lbl)));
	    for_third_arg.push_back(arg0);
	    for_third_indices.push_back(same_arg0);
	}
    }
    switch (cat) {
    case FIRST_CON: instrs.push_back(wam_instruction<SWITCH_ON_CONSTANT>(map)); break;
    case FIRST_STR: instrs.push_back(wam_instruction<SWITCH_ON_STRUCTURE>(map)); break;
    default: break;
    }
    size_t n = for_third_arg.size();
    for (size_t i = 0; i < n; i++) {
	auto arg = for_third_arg[i];
	auto &clause_indices = for_third_indices[i];
	auto &cp = (*map)[arg];
	const common::int_cell &lbl = static_cast<const common::int_cell &>(cp.term_code());
	instrs.push_back(wam_interim_instruction<INTERIM_LABEL>(lbl));
	emit_third_level_indexing(clause_indices, labels, instrs);
    }
}

void wam_compiler::compile_subsection(std::vector<term> &subsection, wam_interim_code &instrs)
{
    auto n = subsection.size();
    if (n > 1) {
        std::vector<common::int_cell> labels = new_labels(2*n);
	emit_switch_on_term(subsection, labels, instrs);
	for (size_t i = 0; i < n; i++) {
	    emit_cp(labels, i, n, instrs);
	    auto &clause = subsection[i];
	    wam_interim_code clause_instrs(interp_);
	    compile_clause(clause, clause_instrs);
	    instrs.append(clause_instrs);
	}
    } else {
        auto &clause = subsection[0];
	wam_interim_code clause_instrs(interp_);
	compile_clause(clause, clause_instrs);
	instrs.append(clause_instrs);
    }
}

void wam_compiler::compile_predicate(common::con_cell pred, wam_interim_code &instrs)
{
    auto &clauses = interp_.get_predicate(pred);
    auto sections = partition_clauses_nonvar(clauses);
    auto n = sections.size();
    if (n > 1) {
        std::vector<common::int_cell> labels = new_labels_dup(n);
	for (size_t i = 0; i < n; i++) {
	    emit_cp(labels, i, n, instrs);
	    compile_subsection(sections[i], instrs);
	}
    } else {
        compile_subsection(sections[0], instrs);
    }
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
    auto arg = env_.arg(clause_head(clause), 0);
    switch (arg.tag()) {
    case common::tag_t::REF: return arg;
    case common::tag_t::CON: return arg;
    case common::tag_t::INT: return arg;
    case common::tag_t::BIG: return arg;
    case common::tag_t::STR: return env_.functor(arg);
    default: return arg;
    }
}

wam_compiler::first_arg_cat_t wam_compiler::first_arg_cat(const term clause)
{
    term arg = first_arg(clause);

    if (interp_.is_dotted_pair(arg)) {
	return FIRST_LST;
    }

    switch (arg.tag()) {
    case common::tag_t::REF: return FIRST_VAR;
    case common::tag_t::CON: return interp_.is_atom(arg) ? FIRST_CON : FIRST_STR;
    case common::tag_t::INT: return FIRST_CON;
    case common::tag_t::BIG: return FIRST_CON;
    }
    assert(false);
    return FIRST_VAR;
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

bool wam_compiler::register_pool::contains(common::ref_cell ref)
{
    auto it = reg_map_.find(ref);
    return it != reg_map_.end();
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
