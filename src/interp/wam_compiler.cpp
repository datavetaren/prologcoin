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

wam_instruction_base * wam_interim_code::push_back(const wam_instruction_base &instr)
{
    auto *i = new_instruction(instr);
    push_back(i);
    return i;
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

void wam_interim_code::get_all(std::vector<wam_instruction_base *> &instrs_array,
			       std::unordered_map<common::int_cell, size_t> &labels)
{
    instrs_array.resize(size_);
    size_t i = 0;

    for (auto instr : *this) {
        instrs_array[i] = instr;
	if (wam_compiler::is_label_instruction(instr)) {
	    auto *lbl = reinterpret_cast<wam_interim_instruction<INTERIM_LABEL> *>(instr);
	    labels[lbl->label()] = i;
	}
	i++;
    }
}

void wam_interim_code::get_destinations(
	    std::vector<wam_instruction_base *> &instrs,
	    std::unordered_map<common::int_cell, size_t> &labels,
	    size_t index,
	    std::vector<size_t> &continuations)
{
    continuations.clear();
    auto *instr = instrs[index];
    size_t n = instrs.size();

    switch (instr->type()) {
    case GOTO: {
	auto *goto_instr = reinterpret_cast<wam_instruction<GOTO> *>(instr);
	continuations.push_back(labels[goto_instr->p().label()]);
	break;
        }
    case TRY:
    case RETRY:
    case TRY_ME_ELSE:
    case RETRY_ME_ELSE: {
	auto *cp_instr = reinterpret_cast<wam_instruction_code_point *>(instr);
	continuations.push_back(labels[cp_instr->cp().label()]);
	if (index+1 < n) continuations.push_back(index+1);
	break;
	}
    default: {
	if (index+1 < n) continuations.push_back(index+1);
	break;
        }
    }
}

void wam_interim_code::get_topological_sort(
	    std::vector<wam_instruction_base *> &instrs,
	    std::unordered_map<common::int_cell, size_t> &labels,
	    std::vector<size_t> &sorted)
{
    std::vector<size_t> worklist;

    auto pop_back = [&] { auto r = worklist.back();
			  worklist.pop_back();
			  return r; };

    sorted.clear();

    size_t n = instrs.size();
    std::vector<bool> expanded(n);

    worklist.push_back(0); // First instruction

    std::vector<size_t> cont;

    while (!worklist.empty()) {
	auto index = pop_back();
	if (!expanded[index]) {
	    get_destinations(instrs, labels, index, cont);
	    worklist.push_back(index); // Revisited when expanded
	    for (auto c : cont) {
		if (!expanded[c]) {
		    worklist.push_back(c);
		}
	    }
	    expanded[index] = true;
	} else {
	    // We've processed all successors and we're back.
	    sorted.push_back(index);
	}
    }
}

void wam_interim_code::get_reversed_topological_sort(
	    std::vector<wam_instruction_base *> &instrs,
	    std::unordered_map<common::int_cell, size_t> &labels,
	    std::vector<size_t> &sorted)
{
    get_topological_sort(instrs, labels, sorted);
    std::reverse(sorted.begin(), sorted.end());
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

std::vector<code_point> * wam_compiler::new_merge()
{
    auto *merge = new std::vector<code_point>();
    merges_.push_back(merge);
    return merge;
}

wam_compiler::~wam_compiler()
{
    for (auto *merge : merges_) {
	delete merge;
    }
}

std::vector<wam_compiler::prim_unification> wam_compiler::flatten(
	  const term t,
	  wam_compiler::compile_type for_type,
	  bool is_predicate_call)
{
    static const common::con_cell colon(":", 2);

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
	    auto rhs = p.rhs();
	    auto f = env_.functor(rhs);
	    if (f == colon && is_predicate) {
		// This is a module predicate call, so look under colon
		rhs = env_.arg(rhs, 1);
		f = env_.functor(rhs);
	    }
	    size_t n = f.arity();
	    for (size_t i = 0; i < n; i++) {
	        auto pos = (for_type == COMPILE_QUERY) ? n - i - 1 : i;
		auto arg = env_.arg(rhs, pos);

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
		env_.set_arg(rhs, pos, ref);
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
	if (f == interp_.dotted_pair()) {
	    instrs.push_back(wam_instruction<PUT_LIST_X>(lhsreg.num));
	} else {
	    instrs.push_back(wam_instruction<PUT_STRUCTURE_X>(f,lhsreg.num));
	}
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
	if (f == interp_.dotted_pair()) {
	    instrs.push_back(wam_instruction<GET_LIST_A>(lhsreg.num));
	} else {
	    instrs.push_back(wam_instruction<GET_STRUCTURE_A>(f,lhsreg.num));
	}
	if (has_reg<X_REG>(lhsvar)) {
	    wam_compiler::reg xreg;
	    std::tie(xreg, std::ignore) = allocate_reg<X_REG>(lhsvar);
	    instrs.push_back(wam_instruction<GET_VALUE_X>(xreg.num, lhsreg.num));
	}

    } else {
	if (f == interp_.dotted_pair()) {
	    instrs.push_back(wam_instruction<GET_LIST_X>(lhsreg.num));
	} else {
	    instrs.push_back(wam_instruction<GET_STRUCTURE_X>(f,lhsreg.num));
	}
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
        case GET_LIST_X:
        case PUT_LIST_X:
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
        case GET_LIST_X:
        case PUT_LIST_X:
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
        case GET_LIST_Y:
        case PUT_LIST_Y:
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
        case GET_LIST_Y:
        case PUT_LIST_Y:
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
        case GET_LIST_X: instr->set_type<GET_LIST_Y>(); break;
        case PUT_LIST_X: instr->set_type<PUT_LIST_Y>(); break;
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

std::pair<size_t, size_t> wam_compiler::get_num_x_and_y(wam_interim_code &instrs)
{
    int max_x = -1, max_y = -1;
    for (auto *instr : instrs) {
	if (auto x_get = x_getter(instr)) {
	    max_x = std::max(static_cast<int>(x_get()),max_x);
	}
	if (auto y_get = y_getter(instr)) {
	    max_y = std::max(static_cast<int>(y_get()),max_y);
	}
    }
    return std::make_pair(static_cast<size_t>(max_x+1),
			  static_cast<size_t>(max_y+1));
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

bool wam_compiler::is_boundary_instruction(wam_instruction_base *instr)
{
    return instr->type() == CALL || instr->type() == BUILTIN_R ||
	   instr->type() == TRUST_ME;
}

void wam_compiler::find_x_to_y_registers(wam_interim_code &instrs,
					 std::vector<size_t> &x_to_y)
{
    //
    // Get instructions as array
    //
    std::vector<wam_instruction_base *> instrs1;
    std::unordered_map<common::int_cell, size_t> labels;
    instrs.get_all(instrs1, labels);
    std::vector<size_t> boundary_count(instrs1.size());

    // 
    // Because of the presence of GOTO and internal TRY_ME_ELSE instructions
    // we don't have straight control flow. This means every instruction has
    // its own "boundary count," i.e. how many boundary instructions
    // (CALL or BUILTIN_R) are crossed in its control flow path.
    // This also means we have to be conservative. If there are two paths
    // to this instruction, then we'll take the maximum boundary count of
    // each path. Also, because of this, we need to process the instructions
    // in reversed topological order. This means that predecessors are
    // always processed before getting to the merge.
    //

    std::vector<size_t> sorted;
    instrs.get_reversed_topological_sort(instrs1, labels, sorted);

    std::vector<size_t> dest;

    for (auto index : sorted) {
	auto *instr = instrs1[index];

	if (is_boundary_instruction(instr)) {
	    boundary_count[index]++;
	}
	instrs.get_destinations(instrs1, labels, index, dest);
	size_t max = boundary_count[index];
	for (auto d : dest) {
	    if (max > boundary_count[d]) {
		boundary_count[d] = max;
	    }
	}
    }

    //
    // At this point every instruction has a proper boundary count.
    // Now we need to sweep through to see if a register lives across
    // boundaries.
    //

    std::unordered_map<size_t, size_t> some_occur;

    size_t n = instrs1.size();
    for (size_t i = 0; i < n; i++) {
	auto *instr = instrs1[i];
	if (auto x_get = x_getter(instr)) {
	    size_t xn = x_get();
	    // GET_LEVEL should always use Y registers!
	    if (instr->type() == GET_LEVEL) {
		x_to_y.push_back(xn);
	    } else {
		size_t count = boundary_count[i];
		auto it = some_occur.find(xn);
		if (it == some_occur.end()) {
		    some_occur[xn] = count;
		} else if (it->second != count) {
		    // We've found another register access on an instruction
		    // with a different boundary count, so it needs to be
		    // a Y register.
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
    std::vector<wam_instruction_base *> instrs1;
    std::unordered_map<common::int_cell, size_t> labels;
    instrs.get_all(instrs1, labels);

    std::unordered_map<size_t, size_t> map;
    size_t cnt = 0;
    auto mapit = [&](size_t i) {
      if (map.find(i) == map.end()) {
	  map[i] = cnt;
  	  cnt++;
      }
    };

    //
    // We need to walk in topological order to get the most
    // efficient renaming of Y registers.
    //

    std::vector<size_t> sorted;
    instrs.get_topological_sort(instrs1, labels, sorted);

    for (auto index : sorted) {
	auto *instr = instrs1[index];

        if (auto y_get = y_getter(instr)) {
	    mapit(y_get());
	}
    }

    // Sweep through all instructions and update to renamed Y registers
    for (auto *instr : instrs) {
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
    //
    // Walk backwards through control flow graph in topological
    // order (= reversed control flow.) Keep track of maximum Y
    // for each instruction.
    //

    std::vector<wam_instruction_base *> instrs1;
    std::unordered_map<common::int_cell, size_t> labels;
    instrs.get_all(instrs1, labels);

    std::vector<size_t> sorted;
    instrs.get_topological_sort(instrs1, labels, sorted);

    std::vector<int> y_max(instrs1.size(), -1);

    std::vector<size_t> dest;
    
    for (auto index : sorted) {
	auto *instr = instrs1[index];

	int y = -1;
        if (auto y_get = y_getter(instr)) {
	    y = static_cast<int>(y_get());
	}
	instrs.get_destinations(instrs1, labels, index, dest);
	for (auto d : dest) {
	    y = std::max(y_max[d], y);
	}
	y_max[index] = y;

    }

    //
    // At this point every instruction has the proper y_max count.
    // Now we need to update all call instructions.
    //

    size_t n = instrs1.size();
    for (size_t i = 0; i < n; i++) {
	auto *instr = instrs1[i];
	switch (instr->type()) {
	case CALL:
	case BUILTIN_R:
	case RESET_LEVEL: {
	    auto ii= reinterpret_cast<wam_instruction_code_point_reg *>(instr);
	    ii->set_reg(y_max[i]+1);
	    break;
	    }
	default:
	    break;
	}
    }
}

void wam_compiler::find_unsafe_y_registers(wam_interim_code &instrs,
					   std::unordered_set<size_t> &unsafe_y_regs)
{
    // We need at least one call instruction.
    size_t boundary_count = 0;
    for (auto instr : instrs) {
        if (is_boundary_instruction(instr)) {
	    boundary_count++;
	}
    }

    // There can't be unsafe variables if we dont' have call instructions
    if (boundary_count == 0) {
        return;
    }

    //
    // We don't have straight control flow, so we walk backwards
    // in bread first order. We stop if we encounter a CALL instruction
    // as there's no point going further.
    //

    std::vector<wam_instruction_base *> instrs1;
    std::unordered_map<common::int_cell, size_t> labels;
    instrs.get_all(instrs1, labels);
    std::queue<size_t> worklist;
    std::vector<bool> visited(instrs1.size());
    worklist.push(instrs1.size()-1); // Push last instruction

    while (!worklist.empty()) {
	size_t index = worklist.front();
	worklist.pop();
	if (visited[index]) {
	    continue;
	}
	visited[index] = true;

	auto *instr = instrs1[index];
        // Don't search further when we've found the last call.
	if (is_boundary_instruction(instr)) {
	    continue;
	}

	if (instr->type() == PUT_VALUE_Y) {
	    auto y_get = y_getter(instr);
	    size_t yn = y_get();
	    unsafe_y_regs.insert(yn);
        }

	// Push predecessors (only merge nodes have multiple
	// predecessors.)
	if (is_merge_instruction(instr)) {
	    auto *merge = reinterpret_cast<
		wam_interim_instruction<INTERIM_MERGE> *>(instr);
	    for (auto source : merge->sources()) {
		worklist.push(labels[source.label()]);
	    }
	} else {
	    if (index > 0 && instr->type() != GOTO) {
		worklist.push(index-1);
	    }
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

    //
    // Find the first PUT_VALUE_Y instruction if Y is unsafe.
    // As we don't have linear control flow, we need visit nodes
    // in reversed sorted topological order (= in control flow order.)
    //

    std::vector<wam_instruction_base *> instrs1;
    std::unordered_map<common::int_cell, size_t> labels;
    instrs.get_all(instrs1, labels);

    std::vector<size_t> sorted;
    // Reversed topological sorted order starts with the first instruction
    instrs.get_reversed_topological_sort(instrs1, labels, sorted);

    for (auto index : sorted) {
	auto *instr = instrs1[index];
	if (instr->type() == PUT_VALUE_Y) {
	    size_t yn = y_getter(instr)();
	    if (unsafe_y_regs.find(yn) != unsafe_y_regs.end()) {
	        instr->set_type<PUT_UNSAFE_VALUE_Y>();
		unsafe_y_regs.erase(yn);
	    }
	}
    }
}

void wam_compiler::fix_unsafe_set_unify(wam_interim_code &instrs)
{
    // set_value and unify_value on a register that is not initialized
    // with a set_variable or unify_variable are unsafe and must be
    // translated to set_local_value and unify_local_value respectively.

    std::vector<wam_instruction_base *> instrs1;
    std::unordered_map<common::int_cell, size_t> labels;
    instrs.get_all(instrs1, labels);

    std::vector<size_t> sorted;
    instrs.get_reversed_topological_sort(instrs1, labels, sorted);

    size_t num_x = 0, num_y = 0;
    std::tie(num_x, num_y) = get_num_x_and_y(instrs);

    std::vector<bool> seen_x_regs(num_x);
    std::vector<bool> seen_y_regs(num_y);
    std::vector<bool> unsafe_x_regs(num_x);
    std::vector<bool> unsafe_y_regs(num_y);

    for (auto index : sorted) {

	auto *instr = instrs1[index];
	switch (instr->type()) {
	case PUT_STRUCTURE_X:
	case GET_STRUCTURE_X:
	    {
		auto *instr1 =
		    reinterpret_cast<wam_instruction_con_reg *>(instr);
		auto xn = instr1->reg();
		seen_x_regs[xn] = true;
		break;
	    }
	case GET_LIST_X:
	case PUT_LIST_X:
	    {
		auto *instr1 =
		    reinterpret_cast<wam_instruction_unary_reg *>(instr);
		auto xn = instr1->reg();
		seen_x_regs[xn] = true;
		break;
	    }
	case PUT_STRUCTURE_Y:
	case GET_STRUCTURE_Y:
	case GET_LIST_Y:
	case PUT_LIST_Y:
	    {
		auto *instr1 =
		    reinterpret_cast<wam_instruction_unary_reg *>(instr);
		auto yn = instr1->reg();
		seen_y_regs[yn] = true;
		break;
	    }

	case PUT_VARIABLE_X:
	case GET_VARIABLE_X:
	    {
		auto *instr1 =
		    reinterpret_cast<wam_instruction_unary_reg *>(instr);
		auto xn = instr1->reg();
		if (!seen_x_regs[xn]) {
		    unsafe_x_regs[xn] = true;
		}
		seen_x_regs[xn] = true;
		break;
	    }
	case PUT_VARIABLE_Y:
	case GET_VARIABLE_Y:
	    {
		auto *instr1 =
		    reinterpret_cast<wam_instruction_unary_reg *>(instr);
		auto yn = instr1->reg();
		if (!seen_y_regs[yn]) {
		    unsafe_y_regs[yn] = true;
		}
		seen_y_regs[yn] = true;
		break;
	    }
	case SET_VALUE_X:
	    {
		auto *instr1 =
		    reinterpret_cast<wam_instruction_unary_reg *>(instr);
		if (unsafe_x_regs[instr1->reg()]) {
		    instr->set_type<SET_LOCAL_VALUE_X>();
		}
		break;
	    }
	case SET_VALUE_Y:
	    {
		auto *instr1 =
		    reinterpret_cast<wam_instruction_unary_reg *>(instr);
		if (unsafe_y_regs[instr1->reg()]) {
		    instr->set_type<SET_LOCAL_VALUE_Y>();
		}
		break;
	    }
	case UNIFY_VALUE_X:
	    {
		auto *instr1 =
		    reinterpret_cast<wam_instruction_unary_reg *>(instr);
		if (unsafe_x_regs[instr1->reg()]) {
		    instr->set_type<UNIFY_LOCAL_VALUE_X>();
		}
		break;
	    }
	case UNIFY_VALUE_Y:
	    {
		auto *instr1 =
		    reinterpret_cast<wam_instruction_unary_reg *>(instr);
		if (unsafe_y_regs[instr1->reg()]) {
		    instr->set_type<UNIFY_LOCAL_VALUE_Y>();
		}
		break;
	    }

	default:
	    break;
	}
    }
}

void wam_compiler::eliminate_interim_but_labels(wam_interim_code &instrs)
{
    auto it = instrs.begin();
    auto it_end = instrs.end();
    auto it_prev = instrs.before_begin();
    while (it != it_end) {
        if (is_interim_instruction(*it) && !is_label_instruction(*it)) {
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
	    auto *cut_instr = reinterpret_cast<wam_instruction<CUT> *>(instr);
	    return cut_instr->yn() == 0;
	}
    }
    return false;
}

void wam_compiler::allocate_cut(wam_interim_code &instrs)
{
    size_t lvl;

    auto it = instrs.begin();
    auto it_end = instrs.end();

    while (it != it_end) {
        if ((*it)->type() == ALLOCATE) {
	    lvl = new_level();
	    const wam_instruction<GET_LEVEL>getlev(lvl);
	    it = instrs.insert_after(it,getlev);
        } else if ((*it)->type() == CUT) {
	    auto *cut_instr = reinterpret_cast<wam_instruction<CUT> *>(*it);
	    // Only touch user cuts (not other internal cuts by if-then-else)
	    if (cut_instr->yn() == 0) {
		cut_instr->set_yn(lvl);
	    }
	    ++it;
	} else {
	    ++it;
	}
    }
}

bool wam_compiler::clause_needs_environment(const term clause)
{
    static const common::con_cell colon(":", 2);
    static const common::con_cell cut_op("!", 0);
    static const term none = common::int_cell(0);

    auto body = clause_body(clause);
    bool has_calls = false;
    term last_call = none;
    term last_goal = none;
    for (auto goal : for_all_goals(body)) {
	auto module = current_module();
	auto f = env_.functor(goal);
	if (f == colon) {
	    module = env_.functor(env_.arg(goal, 0));
	    goal = env_.arg(goal, 1);
	    f = env_.functor(goal);
	}
	bool isbn = is_builtin(module, f);
	if (!isbn) {
	    if (has_calls) {
		// Multiple calls! We need an environment.
		return true;
	    }
	    has_calls = true;
	    last_call = goal;
	} else if (module == env_.empty_list() && f == cut_op) {
	    return true;
	} else {
	    auto &bn = get_builtin(module, f);
	    if (bn.is_recursive()) {
		return true;
	    }
	}
	last_goal = goal;
    }

    if (!has_calls) {
	return false;
    }

    // If the last goal is the last call, then it will be mapped
    // to "execute", so no need for an environment
    if (last_call == last_goal) {
	return false;
    }

    return true;
}

void wam_compiler::compile_builtin(common::con_cell module,
				   common::con_cell f, bool first_goal,
				   wam_interim_code &seq)
{
    static const common::con_cell bn_true = common::con_cell("true",0);

    if (module != env_.empty_list() || f != bn_true) {
	auto &bn = get_builtin(module, f);
	if (bn.is_recursive()) {
	    seq.push_back(wam_instruction<BUILTIN_R>(module, f, bn.fn(), 0));
	} else {
	    if (bn.fn() == builtins::operator_cut) {
		if (first_goal) {
		    seq.push_back(wam_instruction<NECK_CUT>());
		} else {
		    seq.push_back(wam_instruction<CUT>(0));
		}
	    } else {
		seq.push_back(wam_instruction<BUILTIN>(module, f, bn.fn()));
	    }
	}
    }
}

bool wam_compiler::is_if_then_else(const term goal)
{
    static const common::con_cell bn_impl = common::con_cell("->",2);

    if (!is_disjunction(goal)) {
	return false;
    }
    auto f = env_.functor(env_.arg(goal, 0));
    return f == bn_impl;
}

bool wam_compiler::is_conjunction(const term goal)
{
    static const common::con_cell bn_conj = common::con_cell(",",2);

    auto f = env_.functor(goal);
    return f == bn_conj;
}

bool wam_compiler::is_disjunction(const term goal)
{
    static const common::con_cell bn_disj = common::con_cell(";",2);

    auto f = env_.functor(goal);
    return f == bn_disj;
}

void wam_compiler::insert_phi_nodes(const term goal_a, const term goal_b,
				    wam_interim_code &seq)
{
    //
    // Find vars that we haven't seen, but occurs either in A or B?
    // In that case we insert X = X (PHI node) to indicate that
    // the variable needs to be alive before entering the disjunction.
    //

    varset_t new_vars = (varsets_[goal_a] | varsets_[goal_b]) & (~seen_vars_);

    for (size_t i = 0; i < new_vars.size(); i++) {
	if (new_vars[i]) {
	    auto new_var = index_var_[i];
	    reg new_reg;
	    std::tie(new_reg, std::ignore) = allocate_reg<X_REG>(new_var);
	    seq.push_back(wam_instruction<SET_VARIABLE_X>(new_reg.num));
	}
    }
}

void wam_compiler::compile_if_then_else(const term goal, wam_interim_code &seq)
{
    // Compile (A -> B ; C)
    // into
    //
    //     try_me_else L1
    //        [A]
    //     cut
    //        [B]
    // LX:
    //     goto L2
    // L1: trust_me
    //        [C]
    // L2: merge LX
    //

    const term goal_a = env_.arg(env_.arg(goal, 0),0);
    const term goal_b = env_.arg(env_.arg(goal, 0),1);
    const term goal_c = env_.arg(goal,1);
    common::int_cell l1 = new_label();
    common::int_cell l2 = new_label();
    common::int_cell to_merge_0 = new_label();
    common::int_cell to_merge_1 = new_label();

    insert_phi_nodes(env_.arg(goal, 0), goal_c, seq);

    varset_t old_seen = seen_vars_;

    size_t lvl = new_level();
    seq.push_back(wam_instruction<RESET_LEVEL>());

    seq.push_back(wam_instruction<TRY_ME_ELSE>(l1));
    seq.push_back(wam_interim_instruction<INTERIM_LABEL>(to_merge_0));
    seq.push_back(wam_instruction<GET_LEVEL>(static_cast<uint32_t>(lvl)));
    compile_goal(goal_a, false, seq);

    seq.push_back(wam_instruction<CUT>(static_cast<uint32_t>(lvl)));
    compile_goal(goal_b, false, seq);
    seq.push_back(wam_instruction<GOTO>(l2));
    seq.push_back(wam_interim_instruction<INTERIM_LABEL>(l1));
    seq.push_back(wam_interim_instruction<INTERIM_LABEL>(to_merge_1));
    seq.push_back(wam_instruction<TRUST_ME>());

    varset_t seen_vars_0 = seen_vars_;
    seen_vars_ = old_seen;

    compile_goal(goal_c, false, seq);
    seq.push_back(wam_interim_instruction<INTERIM_LABEL>(l2));
    seq.push_back(wam_interim_instruction<INTERIM_MERGE>(*this, {l2, to_merge_0, to_merge_1}));

    seen_vars_ |= seen_vars_0;
} 

void wam_compiler::compile_conjunction(const term conj, wam_interim_code &seq)
{
    // Compile (A , B)
    // into
    //     [A]
    //     [B]

    const term goal_a = env_.arg(conj, 0);
    const term goal_b = env_.arg(conj, 1);

    compile_goal(goal_a, false, seq);
    seen_vars_ |= varsets_[goal_a];
    compile_goal(goal_b, false, seq);
    seen_vars_ |= varsets_[goal_b];
}

void wam_compiler::compile_disjunction(const term disj, wam_interim_code &seq)
{
    // Compile (A ; B)
    // into
    //
    //     try_me_else L1
    //        [A]
    // LX:
    //     goto L2
    // L1: trust_me
    //        [B]
    // L2: merge LX
    
    const term goal_a = env_.arg(disj, 0);
    const term goal_b = env_.arg(disj, 1);
    common::int_cell l1 = new_label();
    common::int_cell l2 = new_label();
    common::int_cell to_merge = new_label();

    insert_phi_nodes(goal_a, goal_b, seq);

    varset_t old_seen = seen_vars_;

    // Even though there are no internal cuts, we need the RESET_LEVEL
    // instruction to ensure that the correct stack space is reserved
    // by following TRY_ME_ELSE
    seq.push_back(wam_instruction<RESET_LEVEL>());

    seq.push_back(wam_instruction<TRY_ME_ELSE>(l1));
    compile_goal(goal_a, false, seq);

    seq.push_back(wam_interim_instruction<INTERIM_LABEL>(to_merge));
    seq.push_back(wam_instruction<GOTO>(l2));
    seq.push_back(wam_interim_instruction<INTERIM_LABEL>(l1));
    seq.push_back(wam_instruction<TRUST_ME>());

    varset_t seen_vars_a = seen_vars_;
    seen_vars_ = old_seen;
    compile_goal(goal_b, false, seq);
    seq.push_back(wam_interim_instruction<INTERIM_LABEL>(l2));
    seq.push_back(wam_interim_instruction<INTERIM_MERGE>(*this, {to_merge,l2}));

    seen_vars_ |= seen_vars_a;
}

size_t wam_compiler::new_level()
{
    reg levreg;
    term t = env_.new_ref();
    auto &v = reinterpret_cast<common::ref_cell &>(t);
    std::tie(levreg, std::ignore) = allocate_reg<X_REG>(v);
    return levreg.num;
}

void wam_compiler::compile_goal(const term goal, bool first_goal,
				wam_interim_code &seq)
{
    static const common::con_cell colon(":", 2);

    if (is_if_then_else(goal)) {
	compile_if_then_else(goal, seq);
	return;
    } else if (is_disjunction(goal)) {
	compile_disjunction(goal, seq);
	return;
    } else if (is_conjunction(goal)) {
	compile_conjunction(goal, seq);
	return;
    }

    common::con_cell module = current_module();
    auto f = env_.functor(goal);
    if (f == colon) {
	module = env_.functor(env_.arg(goal, 0));
	f = env_.functor(env_.arg(goal, 1));
    }
    bool isbn = is_builtin(module, f);
    compile_query_or_program(goal, COMPILE_QUERY, seq);
    if (isbn) {
	compile_builtin(module, f, first_goal, seq);
    } else {
	seq.push_back(wam_instruction<CALL>(module, f, 0));
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
	} else if (seq.is_at_type(it_0,CALL) &&
    	           seq.is_at_type(it_1,PROCEED)) {
	    // Memorize CALL and extract predicate (saved as 'f')
	    wam_instruction<CALL> *call_instr
	       = reinterpret_cast<wam_instruction<CALL> *>(*it_0);
	    common::con_cell f = call_instr->pn();
	    delete *it_0;
	    delete *it_1;

	    // Remove all instructions
	    seq.erase_after(it, it_2);

	    // Add these new ones
	    it = seq.insert_after(it, wam_instruction<EXECUTE>(f));
	} else {
	    ++it;
	}
    }
}

void wam_compiler::peephole_opt_void(wam_interim_code &instrs)
{
    // Find singleton occurrences
    std::unordered_map<size_t, size_t> reg_count;
    for (auto *instr : instrs) {
	if (auto x_get = x_getter(instr)) {
	    auto xn = x_get();
	    reg_count[xn]++;
	}
    }

    bool contains_void = false;
    for (auto *instr : instrs) {
	if (auto x_get = x_getter(instr)) {
	    auto xn = x_get();
	    if (reg_count[xn] == 1) {
		if (instr->type() == SET_VARIABLE_X) {
		    instr->set_type<SET_VOID>();
		    reinterpret_cast<wam_instruction<SET_VOID> *>(instr)->set_reg(1);
		    contains_void = true;
		}
		if (instr->type() == UNIFY_VARIABLE_X) {
		    instr->set_type<UNIFY_VOID>();
		    reinterpret_cast<wam_instruction<UNIFY_VOID> *>(instr)->set_reg(1);
		    contains_void = true;
		}
	    }
	}
    }
    if (!contains_void) {
	return;
    }

    //
    // Coalesce consecutive set_voids and unify_voids
    //

    auto it = instrs.begin();
    auto it_end = instrs.end();
    auto it_prev = instrs.before_begin();
    wam_instruction_unary_reg *found = nullptr;

    while (it != it_end) {
	auto *instr = *it;
	if (instr->type() == SET_VOID || instr->type() == UNIFY_VOID) {
	    if (found && instr->type() == found->type()) {
		found->set_reg(found->reg()+1);
		it = instrs.erase_after(it_prev);
		delete instr;
	    } else {
		found = static_cast<wam_instruction_unary_reg *>(instr);
		it_prev = it;
		++it;
	    }
	} else {
	    found = nullptr;
	    it_prev = it;
	    ++it;
	}
    }
}

void wam_compiler::reset_clause_temps()
{
    goal_count_ = 0;
    level_count_ = 0;
    seen_vars_.reset();
    varsets_.clear();
    index_var_.clear();
    var_index_.clear();
}

void wam_compiler::compute_var_indices(const term clause)
{
    size_t index = 0;
    for (auto t : env_.iterate_over(clause)) {
	if (t.tag() == common::tag_t::REF) {
	    if (index == 256) {
		std::string msg = "Number of vars in clause exceeded the maximum of " + boost::lexical_cast<std::string>(seen_vars_.size());
		throw wam_exception_too_many_vars_in_clause(msg);
	    }
	    auto &v = reinterpret_cast<common::ref_cell &>(t);
	    index_var_.push_back(v);
	    var_index_[t] = index++;
	}
    }
}

void wam_compiler::find_vars(const term t0, varset_t &varset)
{
    varset.reset();
    for (auto t : env_.iterate_over(t0)) {
	if (t.tag() == common::tag_t::REF) {
	    varset.set(var_index_[t]);
	}
    }
}

bool wam_compiler::is_relevant_varset_op(const term t)
{
    static const common::con_cell bn_left = common::con_cell(":-",2);
    static const common::con_cell bn_disj = common::con_cell(";",2);
    static const common::con_cell bn_conj = common::con_cell(",",2);
    static const common::con_cell bn_impl = common::con_cell("->",2);


    if (t.tag() != common::tag_t::STR) {
	return false;
    }
    auto f = env_.functor(t);
    return f == bn_left || f == bn_disj || f == bn_conj || f == bn_impl;
}

common::term wam_compiler::find_var(size_t var_index)
{
    for (auto p : var_index_) {
	if (p.second == var_index) {
	    return p.first;
	}
    }
    return common::term();
}

std::string wam_compiler::varset_to_string(wam_compiler::varset_t &varset)
{
    std::string str = "[";
    bool first = true;
    for (size_t index = 0; index < MAX_VARS; index++) {
	if (varset[index]) {
	    if (!first) str += ",";
	    first = false;
	    str += env_.to_string(find_var(index));
	}
    }
    return str + "]";
}

void wam_compiler::compute_varsets(const term t0)
{
    compute_var_indices(t0);
    varsets_.clear();
    stack_.clear();
    stack_.push_back(t0);
    while (!stack_.empty()) {
	auto t = stack_.back();
	stack_.pop_back();

	switch (t.tag()) {
	case common::tag_t::INT: {
	    // Post processing on a term: Take union of the child varsets
	    auto parent = stack_.back();
	    stack_.pop_back();
	    auto child_0 = env_.arg(parent, 0);
	    auto child_1 = env_.arg(parent, 1);
	    varsets_[parent] = varsets_[child_0] | varsets_[child_1];
	    break;
	    }
	case common::tag_t::STR: {
	    if (is_relevant_varset_op(t)) {
		stack_.push_back(t);
		stack_.push_back(common::int_cell(0));
		for (size_t i = 0; i < 2; i++) {
		    auto arg = env_.arg(t, i);
		    if (is_relevant_varset_op(arg)) {
			stack_.push_back(arg);
		    } else {
			find_vars(arg, varsets_[arg]);
		    }
		}
	    }
	    break;
	    }
	default: // Do nothing on all other
	    break;
	}
    }
}

void wam_compiler::compile_clause(const term clause,
				  wam_interim_code &seq)
{
    compile_clause(managed_clause(clause, env_.cost(clause)), seq);
}

void wam_compiler::compile_clause(const managed_clause &m_clause,
				  wam_interim_code &seq)
{
    const term clause0 = m_clause.clause();

    // We'll make a copy of the clause to be processed.
    // The reason is that the flattening process
    // (inside compile_query_or_program) touches the vars as it
    // unfolds the inner terms.

    term clause = interp_.copy(clause0);

    seq.push_back(wam_instruction<COST>(m_clause.cost()));

    reset_clause_temps();
    compute_varsets(clause);

    // First analyze how many calls we have.
    // We only need an environment if there are more than 1 call.

    bool needs_env = clause_needs_environment(clause);

    if (needs_env) {
	seq.push_back(wam_instruction<ALLOCATE>());
    }

    term head = clause_head(clause);

    compile_query_or_program(head, COMPILE_PROGRAM, seq);
    seen_vars_ |= varsets_[head];

    term body = clause_body(clause);
    bool first_goal = true;
    for (auto goal : for_all_goals(body)) {
	compile_goal(goal, first_goal, seq);
	first_goal = false;
	seen_vars_ |= varsets_[goal];
    }
    if (needs_env) {
	seq.push_back(wam_instruction<DEALLOCATE>());
    }
    seq.push_back(wam_instruction<PROCEED>());

    peephole_opt_execute(seq);

    if (has_cut(seq)) {
	allocate_cut(seq);
    }

    peephole_opt_void(seq);

    remap_x_registers(seq);
    allocate_y_registers(seq);
    remap_y_registers(seq);
    update_calls_for_environment_trimming(seq);
    remap_to_unsafe_y_registers(seq);
    fix_unsafe_set_unify(seq);
    eliminate_interim_but_labels(seq);
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

std::vector<size_t> wam_compiler::find_clauses_on_cat(
      const managed_clauses &m_clauses, wam_compiler::first_arg_cat_t cat)
{
    std::vector<size_t> found;
    size_t index = 0;
    for (auto &m_clause : m_clauses) {
        if (first_arg_cat(m_clause.clause()) == cat) {
	    found.push_back(index);
        }
	index++;
    }
    return found;
}

void wam_compiler::emit_switch_on_term(const managed_clauses &subsection,
	       const std::vector<common::int_cell> &labels,
	       wam_interim_code &instrs)
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
	     const std::vector<size_t> &clause_indices,
	     const std::vector<common::int_cell> &labels,
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
	      const managed_clauses &subsection,
	      const std::vector<common::int_cell> &labels,
	      const std::vector<size_t> &clause_indices,
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
	auto &m_clause = subsection[clause_index];

	auto arg0 = first_arg(m_clause.clause());

	// Already managed?
	if (map->count(arg0)) {
	    continue;
	}

	// Get all clauses with the same arg
	std::vector<size_t> same_arg0;
	for (auto ci : clause_indices) {
	    auto &other_m_clause = subsection[ci];
	    auto other_arg0 = first_arg(other_m_clause.clause());
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

void wam_compiler::compile_subsection(const managed_clauses &subsection,
				      wam_interim_code &instrs)
{
    auto n = subsection.size();
    if (n > 1) {
        std::vector<common::int_cell> labels = new_labels(2*n);
	emit_switch_on_term(subsection, labels, instrs);
	for (size_t i = 0; i < n; i++) {
	    emit_cp(labels, i, n, instrs);
	    auto &m_clause = subsection[i];
	    wam_interim_code clause_instrs(interp_);
	    compile_clause(m_clause, clause_instrs);
	    instrs.append(clause_instrs);
	}
    } else {
        auto &m_clause = subsection[0];
	wam_interim_code clause_instrs(interp_);
	compile_clause(m_clause, clause_instrs);
	instrs.append(clause_instrs);
    }
}

void wam_compiler::compile_predicate(const qname &qn, wam_interim_code &instrs)
{
    auto &clauses = interp_.get_predicate(qn);

    if (clauses.empty()) {
	return;
    }

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

std::vector<managed_clauses> wam_compiler::partition_clauses(
	    const managed_clauses &clauses,
	    std::function<bool (const managed_clause &c1,
				const managed_clause &c2)> pred)
{
    std::vector<managed_clauses> partitioned;

    partitioned.push_back(managed_clauses());
    auto *v = &partitioned.back();
    bool has_last_clause = false;
    managed_clause last_clause;
    for (auto &m_clause : clauses) {
        auto head = clause_head(m_clause.clause());
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
	    is_diff = pred(last_clause, m_clause);
	}
	if (is_diff) {
	    // It's a var, if v is non-empty, push it and create a new one
	    if (!v->empty()) {
	        partitioned.push_back(managed_clauses());
		v = &partitioned.back();
	    }
	}
	v->push_back(m_clause);
	last_clause = m_clause;
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

wam_compiler::first_arg_cat_t wam_compiler::first_arg_cat(const term cl)
{
    term arg = first_arg(cl);

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

std::vector<managed_clauses> wam_compiler::partition_clauses_nonvar(const managed_clauses &clauses)
{
    return partition_clauses(clauses,
       [&] (const managed_clause &c1, const managed_clause &c2)
	     { 
		 term c1_term = c1.clause();
		 term c2_term = c2.clause();
	       return first_arg_is_var(c1_term) || first_arg_is_var(c2_term);
	     });
}

std::vector<managed_clauses> wam_compiler::partition_clauses_first_arg(const managed_clauses &clauses)
{
    std::unordered_map<term, managed_clauses> map;
    managed_clauses refs;
    std::vector<term> order;

    for (auto &m_clause : clauses) {
        auto head = clause_head(m_clause.clause());
	auto arg = first_arg(head);
	switch (arg.tag()) {
	case common::tag_t::REF:
	    refs.push_back(m_clause);
	    break;
	case common::tag_t::CON:
	case common::tag_t::STR: {
	    auto f = env_.functor(arg);
	    bool is_new = map.count(f) == 0;
	    auto &v = map[f];
	    v.push_back(m_clause);
	    if (is_new) order.push_back(f);
	    break;
	    }
	case common::tag_t::INT: {
	    bool is_new = map.count(arg) == 0;
	    auto &v = map[arg];
	    v.push_back(m_clause);
	    if (is_new) order.push_back(arg);
	    break;
    	    }
	}
    }

    std::vector<managed_clauses> result;
    if (!refs.empty()) {
        result.push_back(refs);
    }

    for (auto o : order) {
        result.push_back(map[o]);
    }

    return result;
}

void wam_compiler::print_partition(std::ostream &out,
				   const std::vector<managed_clauses> &p)
{
    size_t i = 0;
    for (auto &m_clauses : p) {
        out << "Section " << i << ": " << std::endl;
	for (auto &m_clause : m_clauses) {
	    out << "   " << env_.to_string(m_clause.clause()) << std::endl;
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
