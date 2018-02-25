#include "interpreter.hpp"
#include "wam_compiler.hpp"

namespace prologcoin { namespace interp {

interpreter::interpreter() 
{
    compiler_ = new wam_compiler(*this);
    id_to_predicate_.push_back(predicate()); // Reserve index 0
    wam_enabled_ = true;

    set_debug_check_fn(
       [&] {
	   size_t n1 = to_stack_relative_addr((word_t *)e0());
	   size_t n2 = to_stack_relative_addr((word_t *)b());
	   size_t n = (n1 > n2) ? n1 : n2;
	   std::cout << "STACK: " << n << " " << ((n2 > n1) ? "B" : "E") << " HEAP: " << heap_size() << " TRAIL: " << trail_size() << " TIDY: " << tidy_size << "\n";
       });
}

interpreter::~interpreter()
{
    delete compiler_;
    query_vars_.clear();
    predicate_id_.clear();
    id_to_predicate_.clear();
}

bool interpreter::execute(const term query)
{
    using namespace prologcoin::common;

    set_top_fail(false);

    trim_trail(0);

    prepare_execution();

    query_vars_.clear();

    std::unordered_set<std::string> seen;

    // Record all vars for this query
    std::for_each( begin(query),
		   end(query),
		   [&](const term &t) {
		       if (t.tag() == tag_t::REF) {
			   const std::string name = to_string(t);
			   if (!seen.count(name)) {
			       query_vars_.push_back(binding(name,t));
			       seen.insert(name);
			   }
		       }
		   } );

    set_p(code_point(empty_list()));
    set_cp(code_point(empty_list()));
    set_qr(query);

    set_p(code_point(query));

    bool b = cont();

    set_qr(query);

    return b;
}

bool interpreter::cont()
{
    set_complete(false);
    while (!is_complete()) {
        while (!is_complete()) {
	    if (p().has_wam_code()) {
	        bool ok = cont_wam();
		if (!ok) {
	  	    fail();
		}
	    } else {
		dispatch();
	    }
	}

        while (is_complete() && has_meta_contexts()) {
  	    set_complete(false);
	    meta_context *mc = get_last_meta_context();
	    meta_fn fn = get_last_meta_function();
	    if (!fn(*this, mc)) {
	        fail();
	    }
        }
    }

    return !is_top_fail();
}

bool interpreter::next()
{
    term old_qr = qr();

    fail();
    if (!is_top_fail()) {
	cont();
    }

    set_qr(old_qr);
    return !is_top_fail();
}

void interpreter::fail()
{
    bool ok = false;

    while (!ok) {
	if (is_debug()) {
  	    std::cout << "interpreter::fail(): fail " << to_string(qr()) << "\n";
	}

        if (b() == top_b()) {
	    set_top_fail(true);
	    ok = true;
        } else if (b_is_wam()) {
	    ok = backtrack_wam();
	} else {
	    auto ch = reset_to_choice_point(b());
	    auto bp = ch->bp;

	    if (bp.is_fail()) {
		// Do nothing
	    } else if (bp.term_code().tag() != common::tag_t::INT) {
		// Direct query
		static managed_clauses empty_clauses;
		ok = select_clause(bp, 0, empty_clauses, 0);
	    } else {
		auto bpterm = bp.term_code();
		size_t bpval = static_cast<const int_cell &>(bpterm).value();
		// Is there another clause to backtrack to?
		if (bpval != 0) {
		    size_t index_id = bpval >> 8;
		    
		    if (is_debug()) {
			std::string redo_str = to_string(qr());
			std::cout << "interpreter::fail(): redo " << redo_str << std::endl;
		    }
		    auto &clauses = get_predicate(index_id);
		    size_t from_clause = bpval & 0xff;

		    ok = select_clause(qr(), index_id, clauses, from_clause);
		}
	    }
	    if (!ok) {
		set_b(ch->b);
		if (b() != nullptr) set_register_hb(b()->h);
	    }
	}
    }
}

bool interpreter::unify_args(term head, const code_point &p)
{
    if (p.term_code().tag() == common::tag_t::STR) {
	return unify(head, p.term_code());
    } else {
	// Otherwise this is an already disected call with
	// args. So unify with arguments.

	size_t start_trail = trail_size();
	size_t start_stack = stack_size();
	size_t old_register_hb = get_register_hb();

	// Record all bindings so we can undo them in case
	// unification fails.
	set_register_hb(heap_size());

	size_t n = num_of_args();
	bool fail = false;
	for (size_t i = 0; i < n; i++) {
	    auto arg_i = arg(head, i);
	    if (!unify(arg_i, a(i))) {
		fail = true;
		break;
	    }
	}
	
	if (fail) {
	    interpreter_base::unwind_trail(start_trail, trail_size());
	    trim_trail(start_trail);
	    trim_stack(start_stack);
	}
	
	set_register_hb(old_register_hb);
	return !fail;
    }
}

bool interpreter::select_clause(const code_point &instruction,
				size_t index_id,
				managed_clauses &clauses,
				size_t from_clause)
{
    if (index_id == 0) {
	set_b(b()->b);
        if (from_clause > 1) {
	    return false;
	}
        set_p(instruction);
	return true;
    }

    size_t num_clauses = clauses.size();
    bool has_choices = num_clauses > 1;

    // Let's go through clause by clause and see if we can find a match.
    for (size_t i = from_clause; i < num_clauses; i++) {
        auto &m_clause = clauses[i];

	size_t current_heap = heap_size();
	auto copy_clause = copy(m_clause.clause()); // Instantiate it

	term copy_head = clause_head(copy_clause);
	term copy_body = clause_body(copy_clause);

	if (unify_args(copy_head, instruction.term_code())) { // Heads match?
	    // Update choice point (where to continue on fail...)
	    if (has_choices) {
	        auto choice_point = b();
		if (i == num_clauses - 1) {
	  	    choice_point->bp = code_point::fail();
		} else {
		    choice_point->bp = code_point(int_cell((index_id << 8) + (i+1)));
		}
	    }

	    allocate_environment(false);
	    set_cp(empty_list());
	    set_p(copy_body);
	    set_qr(copy_head);

	    return true;
	} else {
    	    // Discard garbage created on heap (due to copying clause)
	    trim_heap(current_heap);
	}
    }

    set_p(empty_list());
    // None found.
    return false;
}

void interpreter::dispatch()
{
    /*
    if (p().is_fail()) {
	next();
	return;
	}
    */

    set_qr(p().term_code());

    con_cell f = functor(qr());

    if (f == empty_list()) {
        // Return
	if (is_debug()) {
	    std::cout << "interpreter::dispatch(): pop\n";
	}
	deallocate_and_proceed();
	if (e0() == top_e()) {
	    if (!p().has_wam_code() && p().term_code() == empty_list()) {
		set_complete(true);
	    }
	}
	return;
    }

    if (is_debug()) {
        // Print call
        std::cout << "interpreter::dispatch(): call " << to_string_cp(p()) << " cp=" << to_string_cp(cp()) << "\n";
    }

#if PROFILER
    struct scope_exit {
	scope_exit(interpreter_base &interp, con_cell f) : interp_(interp), f_(f) { }
	~scope_exit() {
	    auto dt = cpu_.elapsed().system;
	    interp_.profiling_[f_] += dt;
	}
	interpreter_base &interp_;
	con_cell f_;
	boost::timer::cpu_timer cpu_;
    } sc(*this, f);
#endif

    size_t arity = f.arity();

    // If this a call from WAM (fast code to slow code) then its call
    // instruction has already initialized the arguments and arity and
    // the term_code() is just a constant (not a compound STR.)


    common::tag_t ptag = p().term_code().tag();
    switch (ptag) {
    case common::tag_t::STR: {
	for (size_t i = 0; i < arity; i++) {
	    a(i) = arg(p().term_code(), i);
	}
	set_num_of_args(arity);
	break;
        } 
    case common::tag_t::CON: {
	for (size_t i = 0; i < arity; i++) {
	    a(i) = interpreter_base::deref(a(i));
	}
	set_num_of_args(arity);
	break;
        }
    default:
	break;
    }

    // Is this a built-in?
    auto bf = get_builtin(f);
    if (!bf.is_empty()) {
	set_p(cp());
	if (!bf.is_recursive()) {
	    // This enforces eventually a pop up the stack and
	    // P becomes CP.
	    set_cp(empty_list());
	}
	if (!(bf.fn())(*this, arity, args())) {
	    fail();
	}
	return;
    }

    // Is there a successful optimized built-in?
    auto obf = get_builtin_opt(f);
    if (obf != nullptr) {
        tribool r = obf(*this, arity, args());
	if (!indeterminate(r)) {
	    set_p(cp());
	    set_cp(empty_list());
	    if (!r) {
		fail();
	    }
	    return;
	}
    }

    if (is_wam_enabled()) {
	if (auto instr = resolve_predicate(f)) {
	    dispatch_wam(instr);
	    return;
	}
    }

    auto first_arg = get_first_arg();

    size_t predicate_id = matched_predicate_id(f, first_arg);
    predicate  &pred = get_predicate(predicate_id);

    set_pr(f);

    // Otherwise a vector of clauses
    auto &clauses = pred;

    if (clauses.empty()) {
        clauses = get_predicate(f);
	if (clauses.empty()) {
	    std::stringstream msg;
	    msg << "Undefined predicate " << atom_name(f) << "/" << f.arity();
	    abort(interpreter_exception_undefined_predicate(msg.str()));
	    return;
	}
	set_p(empty_list());
	fail();
	return;
    }

    set_b0(b());

    size_t num_clauses = clauses.size();
    bool has_choices = num_clauses > 1;
    size_t index_id = predicate_id;

    // More than one clause that matches? We need a choice point.
    if (has_choices) {
	int_cell index_id_int(index_id << 8);
	code_point ch(index_id_int);
	allocate_choice_point(ch);
    }

    if (!select_clause(p().term_code(), index_id, clauses, 0)) {
	fail();
    }
}

void interpreter::dispatch_wam(wam_instruction_base *instruction)
{
    set_p(instruction);
}

void interpreter::compute_matched_predicate(con_cell func,
					    const term first_arg,
					    predicate &matched)
{
    auto &m_clauses = get_predicate(func);
    for (auto &m_clause : m_clauses) {
	// Extract head
	auto head = clause_head(m_clause.clause());
	auto head_functor = functor(head);
	if (head_functor.arity() > 0) {
	    auto head_first_arg = arg(head, 0);
	    if (definitely_inequal(head_first_arg, first_arg)) {
		continue;
	    }
	}
	matched.push_back(m_clause);
    }
}

size_t interpreter::matched_predicate_id(con_cell func, const term first_arg)
{
    using namespace prologcoin::common;

    term index_arg = first_arg;
    switch (first_arg.tag()) {
    case tag_t::STR:
	index_arg = functor(first_arg); break;
    case tag_t::CON:
    case tag_t::INT:
	break;
    case tag_t::REF:
	index_arg = term();
	break;
    }

    functor_index findex(func, index_arg);
    auto it = predicate_id_.find(findex);
    size_t id;
    if (it == predicate_id_.end()) {
	id = id_to_predicate_.size();
	id_to_predicate_.push_back( predicate() );
	predicate_id_[findex] = id;
	auto &pred = id_to_predicate_[id];
	compute_matched_predicate(func, first_arg, pred);
    } else {
	id = it->second;
    }
    return id;
}

std::string interpreter::get_result(bool newlines) const
{
    using namespace prologcoin::common;

    std::unordered_map<term, size_t> count_occurrences;
    std::for_each(begin(qr()),
		  end(qr()),
		  [&] (const term t) {
		    if (t.tag() == tag_t::REF) {
			if (!has_name(t)) {
			    ++count_occurrences[t];
			}
		    }
		  }
		  );

    interpreter &ii = const_cast<interpreter &>(*this);

    // Those vars with a singleton occurrence will be named
    // '_'.
    size_t named_var_count = 0;
    for (auto v : count_occurrences) {
        if (v.second == 1) {
            ii.set_name(v.first, "_");
	} else { // v.second > 1
  	    std::string name = "G_" + boost::lexical_cast<std::string>(
		       named_var_count);
	    named_var_count++;
	    ii.set_name(v.first, name);
	}
    }

    std::vector<std::string> result;

    bool first = true;

    std::stringstream ss;
    for (auto v : query_vars_) {
	auto &name = v.name();
	auto &value = v.value();
	auto value_str = to_string(value);
	if (name != value_str) {
	    if (!first) {
		if (newlines) {
		    ss << "," << std::endl;
		} else {
		    ss << ", ";
		}
	    }
	    ss << name <<  " = " << value_str;
	    first = false;
	}
    }

    for (auto v : count_occurrences) {
        ii.clear_name(v.first);
    }

    if (first) {
	ss << "true";
    }

    if (newlines) {
	ss << std::endl;
    }

    return ss.str();
}

interpreter::term interpreter::get_result_term(const std::string &varname) const
{
    term t;
    for (auto v : query_vars_) {
	auto &name = v.name();
	if (name == varname) {
	    return v.value();
	}
    }
    return t;
}

void interpreter::print_result(std::ostream &out) const
{
    out << get_result();
}

void interpreter::compile(common::con_cell pred)
{
    wam_interim_code instrs(*this);
    compiler_->compile_predicate(pred, instrs);
    size_t yn_size = compiler_->get_environment_size_of(instrs);
    size_t first_offset = next_offset();
    load_code(instrs);
    auto *next_instr = to_code(first_offset);
    set_predicate(pred, next_instr, yn_size);
}

void interpreter::bind_code_point(std::unordered_map<size_t, size_t> &label_map, code_point &cp)
{
    if (cp.wam_code() == nullptr) {
	auto term = cp.term_code();
	if (term.tag() == common::tag_t::INT) {
	    auto lbl_int = static_cast<const int_cell &>(term);
	    auto lbl = static_cast<size_t>(lbl_int.value());
	    if (label_map.count(lbl)) {
		size_t offset = label_map[lbl];
		auto *instr = reinterpret_cast<wam_instruction_code_point *>(to_code(offset));
		cp.set_wam_code(instr);
	    }
	} else if (term.tag() == common::tag_t::CON) {
	    auto lbl_con = static_cast<const con_cell &>(term);
	    if (auto instr = resolve_predicate(lbl_con)) {
		cp.set_wam_code(instr);
	    }
	}
    }
}

void interpreter::load_code(wam_interim_code &instrs)
{
    std::unordered_map<size_t, size_t> label_map;
    size_t first_offset = next_offset();
    size_t offset = first_offset;
    // Collect labels
    for (auto *instr : instrs) {
	if (wam_compiler::is_label_instruction(instr)) {
	    auto *lbl_instr = static_cast<wam_interim_instruction<INTERIM_LABEL> *>(instr);
	    size_t lbl = static_cast<size_t>(lbl_instr->label().value());
	    label_map.insert(std::make_pair(lbl, offset));
	} else {
	    add(*instr);
	    size_t sz = instr->size();
	    offset += sz;
	}
    }

    // Update code points
    wam_instruction_base *instr = to_code(first_offset);
    for (size_t i = first_offset; i < offset;) {
	switch (instr->type()) {
	case TRY_ME_ELSE:
	case RETRY_ME_ELSE:
	case TRY:
	case RETRY:
	case TRUST:
	case CALL:
	case EXECUTE:
	case GOTO:
	    {
	    auto cp_instr = static_cast<wam_instruction_code_point *>(instr);
	    bind_code_point(label_map, cp_instr->cp());
	    break;
	    }
        case SWITCH_ON_TERM:
	    {
	    auto cp_instr = static_cast<wam_instruction<SWITCH_ON_TERM> *>(instr);
	    bind_code_point(label_map, cp_instr->pv());
	    bind_code_point(label_map, cp_instr->pc());
	    bind_code_point(label_map, cp_instr->pl());
	    bind_code_point(label_map, cp_instr->ps());
	    }
	    break;
        case SWITCH_ON_CONSTANT:
        case SWITCH_ON_STRUCTURE:
	    {
	    auto cp_instr = static_cast<wam_instruction_hash_map *>(instr);
	    for (auto &e : cp_instr->map()) {
		bind_code_point(label_map, e.second);
	    }
	    }
	    break;
	default:
	    break;
	}
	i += instr->size();
	instr = next_instruction(instr);
    }
}

}}


