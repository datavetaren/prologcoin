#include "../common/term_env.hpp"
#include "interpreter.hpp"

namespace prologcoin { namespace interp {

using namespace prologcoin::common;

interpreter::interpreter() : comma_(",",2), empty_list_("[]", 0), implied_by_(":-", 2)
{
    term_env_ = new term_env();
    owns_term_env_ = true;

    init();
}

interpreter::interpreter(term_env &env) : comma_(",",2), empty_list_("[]",0), implied_by_(":-", 2)
{
    term_env_ = &env;
    owns_term_env_ = false;

    init();
}


void interpreter::init()
{
    debug_ = false;
    top_fail_ = false;
    register_b_ = 0;
    register_e_ = 0;
    register_tr_ = term_env_->trail_size();
    register_h_ = term_env_->heap_size();
    register_hb_ = register_h_;
    register_b0_ = 0;
}

interpreter::~interpreter()
{
    register_cp_ = term();
    register_qr_ = term();
    query_vars_.clear();
    syntax_check_stack_.clear();
    program_db_.clear();
    program_predicates_.clear();
    predicate_clauses_.clear();
    indexed_clauses_.clear();
    id_indexed_clauses_.clear();
    if (owns_term_env_) {
	delete term_env_;
    }
}

void interpreter::syntax_check()
{
    while (!syntax_check_stack_.empty()) {
	auto f = syntax_check_stack_.back();
	syntax_check_stack_.pop_back();
	f();
    }
}

void interpreter::load_clause(const term &t)
{
    syntax_check_stack_.push_back(
			  std::bind(&interpreter::syntax_check_clause, this,
				    t));
    syntax_check();

    // This is a valid clause. Let's lookup the functor of its head.

    term head = clause_head(t);

    con_cell predicate = term_env_->functor(head);
    
    auto found = program_db_.find(predicate);
    if (found == program_db_.end()) {
	program_db_[predicate] = std::vector<term>();
	program_predicates_.push_back(predicate);
    }
    program_db_[predicate].push_back(t);
}

void interpreter::load_program(const term &t)
{
    syntax_check_stack_.push_back(
			  std::bind(&interpreter::syntax_check_program, this,
				    t));
    syntax_check();

    for (auto clause : list_iterator(*term_env_, t)) {
	load_clause(clause);
    }
}

void interpreter::syntax_check_program(const term &t)
{
    if (!term_env_->is_list(t)) {
	throw syntax_exception_program_not_a_list(t);
    }

    size_t sz = syntax_check_stack_.size();
    for (auto clause : list_iterator(*term_env_, t)) {
	syntax_check_stack_.push_back(
			  std::bind(&interpreter::syntax_check_clause, this,
				    clause));
    }
    std::reverse(syntax_check_stack_.begin() + sz, syntax_check_stack_.end());
}

void interpreter::syntax_check_clause(const term &t)
{
    static const con_cell def(":-", 2);

    auto f = term_env_->functor(t);
    if (f == def) {
	auto head = term_env_->arg(t, 0);
	auto body = term_env_->arg(t, 1);
	syntax_check_stack_.push_back(
	      std::bind(&interpreter::syntax_check_head, this, head) );
	syntax_check_stack_.push_back(
	      std::bind(&interpreter::syntax_check_body, this, body) );
	return;
    }

    // This is a head only clause.

    syntax_check_stack_.push_back(
		  std::bind(&interpreter::syntax_check_head, this, t));
}

void interpreter::syntax_check_head(const term &t)
{
    static con_cell def(":-", 2);
    static con_cell semi(";", 2);
    static con_cell comma(",", 2);
    static con_cell cannot_prove("\\+", 1);

    if (!term_env_->is_functor(t)) {
	throw syntax_exception_clause_bad_head(t, "Head of clause is not a functor");
    }

    // Head cannot be functor ->, ; , or \+
    auto f = term_env_->functor(t);

    if (f == def || f == semi || f == comma || f == cannot_prove) {
	throw syntax_exception_clause_bad_head(t, "Clause has an invalid head; cannot be '->', ';', ',' or '\\+'");
    }
}

void interpreter::syntax_check_body(const term &t)
{
    static con_cell imply("->", 2);
    static con_cell semi(";", 2);
    static con_cell comma(",", 2);
    static con_cell cannot_prove("\\+", 1);

    if (term_env_->is_functor(t)) {
	auto f = term_env_->functor(t);
	if (f == imply || f == semi || f == comma || f == cannot_prove) {
	    auto num_args = f.arity();
	    for (size_t i = 0; i < num_args; i++) {
		auto arg = term_env_->arg(t, i);
		syntax_check_stack_.push_back(
		      std::bind(&interpreter::syntax_check_body, this, arg));
	    }
	    return;
	}
    }

    syntax_check_stack_.push_back(
		  std::bind(&interpreter::syntax_check_goal, this, t));
}

void interpreter::syntax_check_goal(const term &t)
{
    // Each goal must be a functor (e.g. a plain integer is not allowed)

    if (!term_env_->is_functor(t)) {
	auto tg = t->tag();
	// We don't know what variables will be bound to, so we need
	// to conservatively skip the syntax check.
	if (tg == tag_t::REF || tg == tag_t::GBL) {
	    return;
	}
	throw syntax_exception_bad_goal(t, "Goal is not callable.");
    }
}

void interpreter::print_db() const
{
    print_db(std::cout);
}

void interpreter::print_db(std::ostream &out) const
{
    bool do_nl_p = false;
    for (auto p : program_predicates_) {
	const std::vector<term> &clauses = program_db_.find(p)->second;
	if (do_nl_p) {
	    out << "\n";
	}
	bool do_nl = false;
	for (auto clause : clauses) {
	    if (do_nl) out << "\n";
	    auto str = term_env_->to_string(clause, term_emitter::STYLE_PROGRAM);
	    out << str;
	    do_nl = true;
	}
	do_nl_p = true;
    }
    out << "\n";
}

inline interpreter::environment_t * interpreter::get_environment(size_t at_index)
{
    environment_t *e = reinterpret_cast<environment_t *>(term_env_->stack_ref(at_index));
    return e;
}

void interpreter::allocate_environment()
{
    // If the choice point protects the current environment, then
    // allocate after choice point.
    size_t at_index = (register_b_ > register_e_) ?
	register_b_ + choice_point_num_cells : register_e_ + environment_num_cells;
    if (at_index == 0) {
	at_index = 1;
    }
    term_env_->ensure_stack(at_index, environment_num_cells);
    environment_t *e = get_environment(at_index);
    e->ce.set_value(register_e_);
    e->b0.set_value(register_b0_);
    e->cp = register_cp_;
    e->qr = register_qr_;
    register_e_ = at_index;
}

void interpreter::deallocate_environment()
{
    // This does not manipulate the stack! It only updates E and CP registers
    // (current environment and continuation pointer), because a choice point
    // may still use the "deallocated" environment.

    environment_t *e = get_environment(register_e_);
    register_e_ = e->ce.value();
    register_b0_ = e->b0.value();
    register_cp_ = term_env_->to_term(e->cp);
    register_qr_ = term_env_->to_term(e->qr);
}

inline interpreter::choice_point_t * interpreter::get_choice_point(size_t at_index)
{
    choice_point_t *cp = reinterpret_cast<choice_point_t *>(term_env_->stack_ref(at_index));
    return cp;
}

void interpreter::allocate_choice_point(size_t index_id)
{
    size_t at_index = (register_e_ > register_b_) ?
	register_e_ + environment_num_cells : register_b_ + choice_point_num_cells;
    if (at_index == 0) {
	at_index = 1;
    }
    term_env_->ensure_stack(at_index, choice_point_num_cells);
    choice_point_t *ch = get_choice_point(at_index);
    ch->ce.set_value(register_e_);
    ch->cp = register_cp_;
    ch->b.set_value(register_b_);
    ch->bp.set_value((index_id << 8) | 0); // 8 bits shifted = index_id
                                           // lower 8 bits = clause no
    ch->tr.set_value(register_tr_);
    ch->h.set_value(register_h_);
    ch->b0.set_value(register_b0_);
    ch->qr = register_qr_;
    register_b_ = at_index;
    register_hb_ = register_h_;
    term_env_->set_last_choice_heap(register_hb_);
}

void interpreter::abort(const interpreter_exception &ex)
{
    throw ex;
}

bool interpreter::execute(const term &query)
{
    top_fail_ = false;

    term_env_->trim_trail(0);

    init();

    query_vars_.clear();

    // Record all vars for this query
    std::for_each( term_env_->begin(query),
		   term_env_->end(query),
		   [=](const term &t) {
		       if (t->tag() == tag_t::REF) {
			   const std::string name = term_env_->to_string(t);
			   query_vars_.push_back(binding(name,t));
		       }
		   } );

    register_cp_ = query;
    register_qr_ = query;

    return cont();
}

bool interpreter::cont()
{
    do {
      execute_once();
    } while (register_e_ != 0);

    register_h_ = term_env_->heap_size();

    return !top_fail_;
}

bool interpreter::next()
{
    fail();
    if (!top_fail_) {
	cont();
    }
    return !top_fail_;
}

std::string interpreter::get_result(bool newlines) const
{
    std::unordered_map<term, size_t> count_occurrences;
    std::for_each(term_env_->begin(register_qr_),
		  term_env_->end(register_qr_),
		  [&] (const term &t) {
		    if (t->tag() == tag_t::REF) {
		      ++count_occurrences[t];
		    }
		  }
		  );

    // Those vars with a singleton occurrence will be named
    // '_'.
    size_t named_var_count = 0;
    for (auto v : count_occurrences) {
        if (v.second == 1) {
            term_env_->set_name(v.first, "_");
	} else { // v.second > 1
  	    std::string name = "G_" + boost::lexical_cast<std::string>(
		       named_var_count);
	    named_var_count++;
	    term_env_->set_name(v.first, name);
	}
    }

    std::vector<std::string> result;

    bool first = true;

    std::stringstream ss;
    for (auto v : query_vars_) {
	auto &name = v.name();
	auto &value = v.value();
	auto value_str = term_env_->to_string(value);
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
        term_env_->clear_name(v.first);
    }

    if (newlines) {
	ss << std::endl;
    }

    return ss.str();
}

void interpreter::print_result(std::ostream &out) const
{
    out << get_result();
}

void interpreter::execute_once()
{
    con_cell f = term_env_->functor(register_cp_);

    term instruction = register_cp_;

    if (f == comma_) {
	instruction = term_env_->arg(register_cp_, 0);
	register_cp_ = term_env_->arg(register_cp_, 1);
    } else {
	register_cp_ = term_env_->empty_list();
    }

    dispatch(instruction);
}

bool interpreter::definitely_inequal(const term &a, const term &b)
{
    using namespace common;
    if (a->tag() == tag_t::REF || b->tag() == common::tag_t::REF) {
	return false;
    }
    if (a->tag() != b->tag()) {
	return true;
    }
    switch (a->tag()) {
    case tag_t::REF: return false;
    case tag_t::CON: return (*a) != (*b);
    case tag_t::STR: {
	con_cell fa = term_env_->functor(a);
	con_cell fb = term_env_->functor(b);
        return fa != fb;
    }
    case tag_t::INT: return (*a) != (*b);
    case tag_t::BIG: return false;
    }

    return false;
}

common::cell interpreter::first_arg_index(const term &t)
{
    switch (t->tag()) {
    case tag_t::REF: return t;
    case tag_t::GBL: return t;
    case tag_t::CON: return t;
    case tag_t::STR: {
	con_cell f = term_env_->functor(t);
	return f;
    }
    case tag_t::INT: return t;
    case tag_t::BIG: return t;
    }
    return t;
}

term interpreter::clause_head(const term &clause)
{
    auto f = term_env_->functor(clause);
    if (f == implied_by_) {
	return term_env_->arg(clause, 0);
    } else {
	return clause;
    }
}

term interpreter::clause_body(const term &clause)
{
    auto f = term_env_->functor(clause);
    if (f == implied_by_) {
        return term_env_->arg(clause, 1);
    } else {
        return term_env_->empty_list();
    }
}

void interpreter::compute_matched_clauses(con_cell functor,
					  const term &first_arg,
					  std::vector<common::term> &matched)
{
    auto &clauses = program_db_[functor];
    for (auto &clause : clauses) {
	// Extract head
	auto head = clause_head(clause);
	auto head_functor = term_env_->functor(head);
	if (head_functor.arity() > 0) {
	    auto head_first_arg = term_env_->arg(head, 0);
	    if (definitely_inequal(head_first_arg, first_arg)) {
		continue;
	    }
	}
	matched.push_back(clause);
    }
}

indexed_clauses & interpreter::matched_clauses(con_cell functor,
					       const term &first_arg)
{
    term index_arg = first_arg;
    switch (first_arg->tag()) {
    case tag_t::STR:
	index_arg = term_env_->to_term(term_env_->functor(first_arg)); break;
    case tag_t::CON:
    case tag_t::INT:
	break;
    case tag_t::REF:
	index_arg = term();
	break;
    }

    functor_index findex(functor, index_arg);
    auto it = indexed_clauses_.find(findex);
    if (it == indexed_clauses_.end()) {
	auto &indexed_clauses = indexed_clauses_[findex];
	compute_matched_clauses(functor, first_arg, indexed_clauses.first);
	it = indexed_clauses_.find(findex);
	size_t id = id_indexed_clauses_.size();
	id_indexed_clauses_.push_back(&indexed_clauses.first);
	it->second.second = id;
    }
    return it->second;
}

std::vector<common::term> & interpreter::matched_clauses(size_t id)
{
    return *id_indexed_clauses_[id];
}

common::term interpreter::get_first_arg(const term &t)
{
    if (t->tag() == tag_t::STR) {
	auto f = term_env_->functor(t);
	if (f.arity() == 0) {
	    return term_env_->empty_list();
	}
	return term_env_->arg(t, 0);
    } else {
	return term_env_->empty_list();
    }
}

void interpreter::dispatch(term &instruction)
{
    register_qr_ = instruction;
    register_h_ = term_env_->heap_size();

    con_cell f = term_env_->functor(instruction);

    if (f == comma_) {
	allocate_environment();
	register_cp_ = instruction;
	return;
    }

    if (f == empty_list_) {
        // Return
        if (is_debug()) {
   	    environment_t *e = get_environment(register_e_);
            std::cout << "interpreter::dispatch(): exit " << term_env_->to_string(term_env_->to_term(e->qr)) << "\n";
        }
        deallocate_environment();
	return;
    }

    auto first_arg = get_first_arg(instruction);

    auto &indexed_clauses = matched_clauses(f, first_arg);
    auto &clauses = indexed_clauses.first;

    if (clauses.empty()) {

	if (program_db_.find(f) == program_db_.end()) {
	    std::stringstream msg;
	    msg << "Undefined predicate " << term_env_->atom_name(f) << "/" << f.arity();
	    abort(interpreter_exception_undefined_predicate(msg.str()));
	    return;
	}

	fail();
	return;
    }

    if (is_debug()) {
        // Print call
      std::cout << "interpreter::dispatch(): call " << term_env_->to_string(instruction) << "\n";
    }

    size_t num_clauses = clauses.size();
    bool has_choices = num_clauses > 1;
    size_t index_id = indexed_clauses.second;

    // More than one clause that matches? We need a choice point.
    if (has_choices) {
	allocate_choice_point(index_id);
    }

    // Before making the actual call we'll remember the current choice
    // point. This is the one we backtrack to if we encounter a cut operation.
    register_b0_ = register_b_;

    if (!select_clause(instruction, index_id, clauses, 0)) {
	fail();
    }
}

bool interpreter::select_clause(term &instruction,
				size_t index_id,
				std::vector<term> &clauses,
				size_t from_clause)
{
    size_t num_clauses = clauses.size();
    bool has_choices = num_clauses > 1;

    // Let's go through clause by clause and see if we can find a match.
    for (size_t i = from_clause; i < num_clauses; i++) {
        auto &clause = clauses[i];

	size_t current_heap = term_env_->heap_size();
	auto copy_clause = term_env_->copy(clause); // Instantiate it
	register_h_ = term_env_->heap_size();

	term copy_head = clause_head(copy_clause);
	term copy_body = clause_body(copy_clause);

	if (term_env_->unify(copy_head, instruction)) { // Heads match?
	    register_h_ = term_env_->heap_size();
	    register_tr_ = term_env_->trail_size();
	    // Update choice point (where to continue on fail...)
	    if (has_choices) {
	        auto choice_point = get_choice_point(register_b_);
		if (i == num_clauses - 1) {
		    choice_point->bp.set_value(0);
		} else {
		    choice_point->bp.set_value((index_id << 8) + (i+1));
		}
	    }
	    allocate_environment();
	    register_cp_ = copy_body;
	    register_qr_ = copy_head;

	    return true;
	} else {
    	    // Discard garbage created on heap (due to copying clause)
	    term_env_->trim_heap(current_heap);
	    register_h_ = current_heap;
	}
    }

    // None found.
    return false;
}

void interpreter::fail()
{
    bool ok = false;

    size_t current_tr = register_tr_;
    bool unbound = false;

    do {
        if (register_b_ == 0) {
	    top_fail_ = true;
	    return;
        }
	auto ch = get_choice_point(register_b_);

	register_e_ = ch->ce.value();
	register_cp_ = term_env_->to_term(ch->cp);
	register_tr_ = ch->tr.value();
	register_h_ = ch->h.value();
	register_b0_ = ch->b0.value();
	register_hb_ = register_h_;
	register_qr_ = term_env_->to_term(ch->qr);
	term_env_->set_last_choice_heap(register_hb_);

	// Is there another clause to backtrack to?
	if (ch->bp.value() != 0) {
	    // Unbind variables
	    term_env_->unwind_trail(register_tr_, current_tr);
	    term_env_->trim_trail(register_tr_);
	    term_env_->trim_heap(register_h_);
	    unbound = true;
	    current_tr = register_tr_;

	    if (is_debug()) {
		std::cout << "interpreter::fail(): redo " << term_env_->to_string(register_qr_) << "\n";
	    }
      	    size_t index_id = ch->bp.value() >> 8;
	    auto &clauses = *id_indexed_clauses_[index_id];
	    size_t from_clause = ch->bp.value() & 0xff;

  	    ok = select_clause(register_qr_, index_id, clauses, from_clause);

	}
	if (!ok) {
	    unbound = false;
	    register_b_ = ch->b.value();
	}
    } while (!ok);

    if (!unbound) {
	term_env_->unwind_trail(register_tr_, current_tr);
	term_env_->trim_trail(register_tr_);
	term_env_->trim_heap(register_h_);
    }
}

}}



