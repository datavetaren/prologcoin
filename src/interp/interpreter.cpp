#include "../common/term_env.hpp"
#include "interpreter.hpp"
#include "builtins_fileio.hpp"
#include <boost/filesystem.hpp>

namespace prologcoin { namespace interp {

using namespace prologcoin::common;

interpreter::interpreter() : comma_(",",2), empty_list_("[]", 0), implied_by_(":-", 2), arith_(*this)
{
    term_env_ = new term_env();
    owns_term_env_ = true;

    init();

    load_builtins();
}

interpreter::interpreter(term_env &env) : comma_(",",2), empty_list_("[]",0), implied_by_(":-", 2), arith_(*this)
{
    term_env_ = &env;
    owns_term_env_ = false;

    init();
}

void interpreter::init()
{
    debug_ = false;
    file_id_count_ = 1;
    id_to_executable_.push_back(executable()); // Reserve executable index 0
    prepare_execution();
}

bool interpreter::unify(term &a, term &b)
{
    bool r = term_env_->unify(a, b);
    if (r) {
        register_tr_ = term_env_->trail_size();
    }
    return r;
}

interpreter::~interpreter()
{
    arith_.unload();
    close_all_files();
    register_cp_ = term();
    register_qr_ = term();
    query_vars_.clear();
    syntax_check_stack_.clear();
    program_db_.clear();
    program_predicates_.clear();
    executable_id_.clear();
    id_to_executable_.clear();
    if (owns_term_env_) {
	delete term_env_;
    }
}

void interpreter::close_all_files()
{
    for (auto f : open_files_) {
	delete f.second;
    }
    open_files_.clear();
}

bool interpreter::is_file_id(size_t id) const
{
    return open_files_.find(id) != open_files_.end();
}

file_stream & interpreter::new_file_stream(const std::string &path)
{
    size_t new_id = file_id_count_;
    file_stream *fs = new file_stream(env(), file_id_count_, path);
    file_id_count_++;
    open_files_[new_id] = fs;
    return *fs;
}

void interpreter::close_file_stream(size_t id)
{
    if (open_files_.find(id) == open_files_.end()) {
        return;
    }
    file_stream *fs = open_files_[id];
    open_files_.erase(id);
    delete fs;
}

file_stream & interpreter::get_file_stream(size_t id)
{
    return *open_files_[id];
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
        program_db_[predicate] = std::make_pair(std::vector<term>(), nullptr);
	program_predicates_.push_back(predicate);
    }
    program_db_[predicate].first.push_back(t);
    program_db_[predicate].second = nullptr; // Builtin function set to null
}

void interpreter::load_builtin(con_cell f, builtin b)
{
    auto found = program_db_.find(f);
    if (found == program_db_.end()) {
        program_db_[f] = std::make_pair(std::vector<term>(), nullptr);
	program_predicates_.push_back(f);
    }
    program_db_[f].first.clear();
    program_db_[f].second = b;
}

void interpreter::load_builtins()
{
    // Simple
    load_builtin(con_cell("true",0), &builtins::true_0);

    // Control flow
    load_builtin(con_cell(",",2), &builtins::operator_comma);
    load_builtin(con_cell("!",0), &builtins::operator_cut);
    load_builtin(con_cell("_!",0), &builtins::operator_cut_if);
    load_builtin(con_cell(";",2), &builtins::operator_disjunction);
    load_builtin(con_cell("->",2), &builtins::operator_if_then);

    // Standard order, equality and unification

    load_builtin(con_cell("@<",2), &builtins::operator_at_less_than);
    load_builtin(con_cell("@=<",2), &builtins::operator_at_equals_less_than);
    load_builtin(con_cell("@>",2), &builtins::operator_at_greater_than);
    load_builtin(con_cell("@>=",2), &builtins::operator_at_greater_than_equals);
    load_builtin(con_cell("==",2), &builtins::operator_equals);
    load_builtin(con_cell("\\==",2), &builtins::operator_not_equals);
    load_builtin(con_cell("compare",3), &builtins::compare_3);
    load_builtin(con_cell("=",2), &builtins::operator_unification);
    load_builtin(con_cell("\\=",2), &builtins::operator_cannot_unify);

    // Type tests
    load_builtin(con_cell("var",1), &builtins::var_1);
    load_builtin(con_cell("nonvar",1), &builtins::nonvar_1);
    load_builtin(con_cell("integer",1), &builtins::integer_1);
    load_builtin(con_cell("number",1), &builtins::number_1);
    load_builtin(con_cell("atom",1), &builtins::atom_1);
    load_builtin(con_cell("atomic",1), &builtins::atomic_1);
    load_builtin(env().functor("compound",1), &builtins::compound_1);
    load_builtin(env().functor("callable",1), &builtins::callable_1);
    load_builtin(con_cell("ground", 1), &builtins::ground_1);

    // Arithmetics
    load_builtin(con_cell("is",2), &builtins::is_2);

    // Analyzing & constructing terms
    load_builtin(env().functor("functor",3), &builtins::functor_3);
    load_builtin(env().functor("copy_term",2), &builtins::copy_term_2);

    // Meta
    load_builtin(con_cell("\\+", 1), &builtins::operator_disprove);
}

void interpreter::enable_file_io()
{
    load_builtins_file_io();
}

void interpreter::set_current_directory(const std::string &dir)
{
    current_dir_ = dir;
}

std::string interpreter::get_full_path(const std::string &path) const
{
    boost::filesystem::path p(current_dir_);
    p /= path;
    return p.string();
}

void interpreter::load_builtins_file_io()
{
    load_builtin(con_cell("open", 3), &builtins_fileio::open_3);
    load_builtin(con_cell("close", 1), &builtins_fileio::close_1);
    load_builtin(con_cell("read", 2), &builtins_fileio::read_2);
    load_builtin(term_env_->functor("at_end_of_stream", 1), &builtins_fileio::at_end_of_stream_1);
    load_builtin(con_cell("write", 1), &builtins_fileio::write_1);
    load_builtin(con_cell("nl", 0), &builtins_fileio::nl_0);
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
	if (tg == tag_t::REF) {
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
        auto &def = program_db_.find(p)->second;
	if (def.second != nullptr) {
	    continue; // Builtin
	}
	const std::vector<term> &clauses = def.first;
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

interpreter::choice_point_t * interpreter::allocate_choice_point(size_t index_id)
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

    return ch;
}

void interpreter::abort(const interpreter_exception &ex)
{
    throw ex;
}

void interpreter::prepare_execution()
{
    top_fail_ = false;
    register_b_ = 0;
    register_e_ = 0;
    register_tr_ = term_env_->trail_size();
    register_h_ = term_env_->heap_size();
    register_hb_ = register_h_;
    register_b0_ = 0;
    register_top_b_ = 0;
}

bool interpreter::execute(const term &query)
{
    top_fail_ = false;

    term_env_->trim_trail(0);

    prepare_execution();

    query_vars_.clear();

    std::unordered_set<std::string> seen;

    // Record all vars for this query
    std::for_each( term_env_->begin(query),
		   term_env_->end(query),
		   [&](const term &t) {
		       if (t->tag() == tag_t::REF) {
			   const std::string name = term_env_->to_string(t);
			   if (!seen.count(name)) {
			       query_vars_.push_back(binding(name,t));
			       seen.insert(name);
			   }
		       }
		   } );

    register_cp_ = query;
    register_qr_ = query;

    return cont();
}

bool interpreter::cont()
{
    do {
	do {
	    execute_once();
	} while (register_e_ != 0 && !top_fail_);
	
	register_h_ = term_env_->heap_size();

	if (!meta_.empty()) {
	    meta_entry &e = meta_.back();
	    meta_context *mc = e.first;
	    meta_fn fn = e.second;
	    fn(*this, mc);
	}
    } while (register_e_ != 0 && !top_fail_);

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
        if (term_env_->has_name(v.first)) {
    	    continue;
	}
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

    if (first) {
	ss << "true";
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
    term instruction = register_cp_;
    register_cp_ = term_env_->empty_list();
    dispatch(instruction);
}

void interpreter::tidy_trail()
{
    size_t from = get_choice_point(register_b_)->tr;
    size_t to = register_tr_;
    term_env_->tidy_trail(from, to);
    register_tr_ = term_env_->trail_size();
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

void interpreter::compute_matched_executable(con_cell functor,
					     const term &first_arg,
					     executable &matched)
{
    auto &executable = program_db_[functor];
    if (executable.second != nullptr) {
	matched = executable;
	return;
    }
    auto &clauses = executable.first;
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
	matched.first.push_back(clause);
    }
}

size_t interpreter::matched_executable_id(con_cell functor, const term &first_arg)
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
    auto it = executable_id_.find(findex);
    size_t id;
    if (it == executable_id_.end()) {
	id = id_to_executable_.size();
	id_to_executable_.push_back( executable() );
	executable_id_[findex] = id;
	auto &executable = id_to_executable_[id];
	compute_matched_executable(functor, first_arg, executable);
    } else {
	id = it->second;
    }
    return id;
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

    if (f == empty_list_) {
        // Return
        if (is_debug()) {
   	    environment_t *e = get_environment(register_e_);
            std::cout << "interpreter::dispatch(): exit " << term_env_->to_string(term_env_->to_term(e->qr)) << "\n";
        }
        deallocate_environment();
	return;
    }

    if (is_debug()) {
        // Print call
      std::cout << "interpreter::dispatch(): call " << term_env_->to_string(instruction) << "\n";
    }

    auto first_arg = get_first_arg(instruction);

    size_t executable_id = matched_executable_id(f, first_arg);
    executable &e = get_executable(executable_id);

    // Is this a built-in?
    auto bf = e.second;
    if (bf != nullptr) {
	if (!bf(*this, instruction)) {
	    fail();
	}
	register_h_ = term_env_->heap_size();
	return;
    }

    // Otherwise a vector of clauses
    auto &clauses = e.first;

    if (clauses.empty()) {
        e = program_db_[f];
	clauses = e.first;
	if (clauses.empty()) {
	    std::stringstream msg;
	    msg << "Undefined predicate " << term_env_->atom_name(f) << "/" << f.arity();
	    abort(interpreter_exception_undefined_predicate(msg.str()));
	    return;
	}

	fail();
	return;
    }

    size_t num_clauses = clauses.size();
    bool has_choices = num_clauses > 1;
    size_t index_id = executable_id;

    // More than one clause that matches? We need a choice point.
    if (has_choices) {
        // Before making the actual call we'll remember the current choice
        // point. This is the one we backtrack to if we encounter a cut operation.
        register_b0_ = register_b_;

	allocate_choice_point(index_id);
    }

    if (!select_clause(instruction, index_id, clauses, 0)) {
	fail();
    }
}

bool interpreter::select_clause(term &instruction,
				size_t index_id,
				std::vector<term> &clauses,
				size_t from_clause)
{
    if (index_id == 0) {
        if (from_clause > 1) {
	    return false;
	}
        register_cp_ = term_env_->arg(register_qr_, from_clause);
	auto choice_point = get_choice_point(register_b_);
	choice_point->bp.set_value(from_clause+1);
	return true;
    }

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

	if (unify(copy_head, instruction)) { // Heads match?
	    register_h_ = term_env_->heap_size();
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

interpreter::choice_point_t * interpreter::reset_to_choice_point(size_t b)
{
    auto ch = get_choice_point(register_b_);

    register_e_ = ch->ce.value();
    register_cp_ = term_env_->to_term(ch->cp);
    register_tr_ = ch->tr.value();
    register_h_ = ch->h.value();
    register_b0_ = ch->b0.value();
    register_hb_ = register_h_;
    register_qr_ = term_env_->to_term(ch->qr);
    term_env_->set_last_choice_heap(register_hb_);

    return ch;
}

void interpreter::unwind(size_t current_tr)
{
    // Unbind variables
    term_env_->unwind_trail(register_tr_, current_tr);
    term_env_->trim_trail(register_tr_);
    term_env_->trim_heap(register_h_);
}

void interpreter::fail()
{
    bool ok = false;

    size_t current_tr = register_tr_;
    bool unbound = false;

    do {
	if (is_debug()) {
	    std::cout << "interpreter::fail(): fail " << term_env_->to_string(register_qr_) << "\n";
	}

        if (register_b_ == register_top_b_) {
	    top_fail_ = true;
	    return;
        }

	auto ch = reset_to_choice_point(register_b_);

	size_t bpval = ch->bp.value();

	// Is there another clause to backtrack to?
	if (bpval != 0) {
	    size_t index_id = bpval >> 8;

	    // Unbind variables
	    unwind(current_tr);
	    current_tr = register_tr_;

	    unbound = true;

	    if (is_debug()) {
	        std::string redo_str = term_env_->to_string(register_qr_);
		std::cout << "interpreter::fail(): redo " << redo_str << std::endl;
	    }
	    auto &clauses = id_to_executable_[index_id].first;
	    size_t from_clause = bpval & 0xff;

  	    ok = select_clause(register_qr_, index_id, clauses, from_clause);

	}
	if (!ok) {
	    unbound = false;
	    register_b_ = ch->b.value();
	}
    } while (!ok);

    if (!unbound) {
	unwind(current_tr);
    }
}

}}



