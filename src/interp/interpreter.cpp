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
    register_b_ = 0;
    register_e_ = 0;
    register_tr_ = 0;
    register_h_ = 0;
    register_hb_ = 0;
    register_b0_ = 0;
}

interpreter::~interpreter()
{
    register_cp_ = term();
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
    term_env_->ensure_stack(at_index, environment_num_cells);
    environment_t *e = get_environment(at_index);
    e->ce.set_value(register_e_);
    e->cp = register_cp_;
    register_e_ = at_index;
}

void interpreter::deallocate_environment()
{
    // This does not manipulate the stack! It only updates E and CP registers
    // (current environment and continuation pointer), because a choice point
    // may still use the "deallocated" environment.

    environment_t *e = get_environment(register_e_);
    register_e_ = e->ce.value();
    register_cp_ = term_env_->to_term(e->cp);
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
    term_env_->ensure_stack(at_index, choice_point_num_cells);
    choice_point_t *cp = get_choice_point(at_index);
    cp->ce.set_value(register_e_);
    cp->cp = register_cp_;
    cp->b.set_value(register_b_);
    cp->bp.set_value((index_id << 8) | 0); // 8 bits shifted = index_id
                                           // lower 8 bits = clause no
    cp->tr.set_value(register_tr_);
    cp->h.set_value(register_h_);
    cp->b0.set_value(register_b0_);
    register_b_ = at_index;
    register_hb_ = register_h_;
}

void interpreter::abort(const interpreter_exception &ex)
{
    throw ex;
}

void interpreter::execute(const term &query)
{
    register_cp_ = query;

    do {
        while (!term_env_->is_empty_list(register_cp_)) {
	    execute_once();
        }
    } while (register_e_ != 0);
}

void interpreter::execute_once()
{
    con_cell f = term_env_->functor(register_cp_);
    term instruction = register_cp_;

    if (f == empty_list_) {
	deallocate_environment();
	return;
    }

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
    functor_index findex(functor, first_arg);
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

void interpreter::dispatch(const term &instruction)
{
    con_cell f = term_env_->functor(instruction);

    if (f == comma_) {
	allocate_environment();
	register_cp_ = instruction;
	return;
    }

    auto first_arg = get_first_arg(instruction);

    auto &indexed_clauses = matched_clauses(f, first_arg);
    auto &clauses = indexed_clauses.first;

    if (clauses.empty()) {
	std::stringstream msg;
	msg << "Undefined predicate " << term_env_->atom_name(f) << "/" << f.arity();
	abort(interpreter_exception_undefined_predicate(msg.str()));
    }

    // More than one clause that matches? We need a choice point.
    if (clauses.size() > 1) {
	allocate_choice_point(indexed_clauses.second);
    }

    // Let's go through clause by clause and see if we can find a match.
    
    
}

}}



