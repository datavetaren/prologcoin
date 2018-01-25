#include "../common/term_env.hpp"
#include "interpreter_base.hpp"
#include "builtins_fileio.hpp"
#include "builtins_opt.hpp"
#include "wam_interpreter.hpp"
#include <boost/filesystem.hpp>
#include <boost/timer/timer.hpp>

#define PROFILER 0

namespace prologcoin { namespace interp {

using namespace prologcoin::common;

const common::term code_point::fail_term_ = common::ref_cell(0);

interpreter_base::interpreter_base() : register_pr_("", 0), comma_(",",2), empty_list_("[]", 0), implied_by_(":-", 2), arith_(*this)
{
    init();

    load_builtins();
    load_builtins_opt();
}

void interpreter_base::init()
{
    debug_ = false;
    file_id_count_ = 1;
    num_of_args_= 0;
    memset(register_ai_, 0, sizeof(register_ai_));
    stack_ = reinterpret_cast<word_t *>(new char[MAX_STACK_SIZE]);
    stack_ptr_ = 0;
    num_y_fn_ = &num_y;
    prepare_execution();
}

interpreter_base::~interpreter_base()
{
    arith_.unload();
    close_all_files();
    register_cp_.reset();
    register_qr_ = term();
    syntax_check_stack_.clear();
    builtins_.clear();
    builtins_opt_.clear();
    program_db_.clear();
    program_predicates_.clear();
}

std::string code_point::to_string(interpreter_base &interp) const
{
    if (has_wam_code()) {
	std::stringstream ss;
	wam_code()->print(ss, static_cast<wam_interpreter &>(interp));
	return ss.str();
    } else if (is_fail()) {
	return "fail";
    } else {
	return interp.to_string(term_code());
    }
}

void interpreter_base::close_all_files()
{
    for (auto f : open_files_) {
	delete f.second;
    }
    open_files_.clear();
}

void interpreter_base::reset_files()
{
    close_all_files();
    file_id_count_ = 1;
}

bool interpreter_base::is_file_id(size_t id) const
{
    return open_files_.find(id) != open_files_.end();
}

file_stream & interpreter_base::new_file_stream(const std::string &path)
{
    size_t new_id = file_id_count_;
    file_stream *fs = new file_stream(*this, file_id_count_, path);
    file_id_count_++;
    open_files_[new_id] = fs;
    return *fs;
}

void interpreter_base::close_file_stream(size_t id)
{
    if (open_files_.find(id) == open_files_.end()) {
        return;
    }
    file_stream *fs = open_files_[id];
    open_files_.erase(id);
    delete fs;
}

file_stream & interpreter_base::get_file_stream(size_t id)
{
    return *open_files_[id];
}

void interpreter_base::syntax_check()
{
    while (!syntax_check_stack_.empty()) {
	auto f = syntax_check_stack_.back();
	syntax_check_stack_.pop_back();
	f();
    }
}

void interpreter_base::load_clause(const term t)
{
    syntax_check_stack_.push_back(
		  std::bind(&interpreter_base::syntax_check_clause, this,
			    t));
    syntax_check();

    // This is a valid clause. Let's lookup the functor of its head.

    term head = clause_head(t);

    con_cell predicate = functor(head);
    
    auto found = program_db_.find(predicate);
    if (found == program_db_.end()) {
        program_db_[predicate] = std::vector<term>();
	program_predicates_.push_back(predicate);
    }
    program_db_[predicate].push_back(t);
}

void interpreter_base::load_builtin(con_cell f, builtin b)
{
    auto found = builtins_.find(f);
    if (found == builtins_.end()) {
        builtins_[f] = b;
    }
}

void interpreter_base::load_builtin_opt(con_cell f, builtin_opt b)
{
    auto found = builtins_opt_.find(f);
    if (found == builtins_opt_.end()) {
        builtins_opt_[f] = b;
    }    
}

void interpreter_base::load_builtins()
{
    // Profiling
    load_builtin(con_cell("profile", 0), &builtins::profile_0);

    // Simple
    load_builtin(con_cell("true",0), &builtins::true_0);

    // Control flow
    load_builtin(con_cell(",",2), &builtins::operator_comma);
    load_builtin(con_cell("!",0), &builtins::operator_cut);
    load_builtin(con_cell("_!",0), &builtins::operator_cut_if);
    load_builtin(con_cell("_#",0), &builtins::operator_deallocate_and_proceed);
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
    load_builtin(functor("compound",1), &builtins::compound_1);
    load_builtin(functor("callable",1), &builtins::callable_1);
    load_builtin(con_cell("ground", 1), &builtins::ground_1);

    // Arithmetics
    load_builtin(con_cell("is",2), &builtins::is_2);

    // Analyzing & constructing terms
    load_builtin(functor("functor",3), &builtins::functor_3);
    load_builtin(functor("copy_term",2), &builtins::copy_term_2);
    load_builtin(con_cell("=..", 2), &builtins::operator_deconstruct);

    // Meta
    load_builtin(con_cell("\\+", 1), &builtins::operator_disprove);
}

void interpreter_base::load_builtins_opt()
{
    // Profiling
    load_builtin_opt(con_cell("member", 2), &builtins_opt::member_2);
    load_builtin_opt(con_cell("sort", 2), &builtins_opt::sort_2);
}

void interpreter_base::enable_file_io()
{
    load_builtins_file_io();
}

void interpreter_base::set_current_directory(const std::string &dir)
{
    current_dir_ = dir;
}

std::string interpreter_base::get_full_path(const std::string &path) const
{
    boost::filesystem::path p(current_dir_);
    p /= path;
    return p.string();
}

void interpreter_base::load_builtins_file_io()
{
    load_builtin(con_cell("open", 3), &builtins_fileio::open_3);
    load_builtin(con_cell("close", 1), &builtins_fileio::close_1);
    load_builtin(con_cell("read", 2), &builtins_fileio::read_2);
    load_builtin(functor("at_end_of_stream", 1), &builtins_fileio::at_end_of_stream_1);
    load_builtin(con_cell("write", 1), &builtins_fileio::write_1);
    load_builtin(con_cell("nl", 0), &builtins_fileio::nl_0);
}

void interpreter_base::load_program(const term t)
{
    syntax_check_stack_.push_back(
		  std::bind(&interpreter_base::syntax_check_program, this,
			    t));
    syntax_check();

    for (auto clause : list_iterator(*this, t)) {
	load_clause(clause);
    }
}

void interpreter_base::load_program(const std::string &str)
{
    std::stringstream ss(str);
    load_program(ss);
}

void interpreter_base::load_program(std::istream &in)
{
    auto prog = parse(in);
    return load_program(prog);
}

void interpreter_base::syntax_check_program(const term t)
{
    if (!is_list(t)) {
	throw syntax_exception_program_not_a_list(t);
    }

    size_t sz = syntax_check_stack_.size();
    for (auto clause : list_iterator(*this, t)) {
	syntax_check_stack_.push_back(
		  std::bind(&interpreter_base::syntax_check_clause, this,
			    clause));
    }
    std::reverse(syntax_check_stack_.begin() + sz, syntax_check_stack_.end());
}

void interpreter_base::syntax_check_clause(const term t)
{
    static const con_cell def(":-", 2);

    auto f = functor(t);
    if (f == def) {
	auto head = arg(t, 0);
	auto body = arg(t, 1);
	syntax_check_stack_.push_back(
	      std::bind(&interpreter_base::syntax_check_head, this, head) );
	syntax_check_stack_.push_back(
	      std::bind(&interpreter_base::syntax_check_body, this, body) );
	return;
    }

    // This is a head only clause.

    syntax_check_stack_.push_back(
		  std::bind(&interpreter_base::syntax_check_head, this, t));
}

void interpreter_base::syntax_check_head(const term t)
{
    static con_cell def(":-", 2);
    static con_cell semi(";", 2);
    static con_cell comma(",", 2);
    static con_cell cannot_prove("\\+", 1);

    if (!is_functor(t)) {
	throw syntax_exception_clause_bad_head(t, "Head of clause is not a functor");
    }

    // Head cannot be functor ->, ; , or \+
    auto f = functor(t);

    if (f == def || f == semi || f == comma || f == cannot_prove) {
	throw syntax_exception_clause_bad_head(t, "Clause has an invalid head; cannot be '->', ';', ',' or '\\+'");
    }
}

void interpreter_base::syntax_check_body(const term t)
{
    static con_cell imply("->", 2);
    static con_cell semi(";", 2);
    static con_cell comma(",", 2);
    static con_cell cannot_prove("\\+", 1);

    if (is_functor(t)) {
	auto f = functor(t);
	if (f == imply || f == semi || f == comma || f == cannot_prove) {
	    auto num_args = f.arity();
	    for (size_t i = 0; i < num_args; i++) {
		auto a = arg(t, i);
		syntax_check_stack_.push_back(
		      std::bind(&interpreter_base::syntax_check_body, this, a));
	    }
	    return;
	}
    }

    syntax_check_stack_.push_back(
		  std::bind(&interpreter_base::syntax_check_goal, this, t));
}

void interpreter_base::syntax_check_goal(const term t)
{
    // Each goal must be a functor (e.g. a plain integer is not allowed)

    if (!is_functor(t)) {
	auto tg = t.tag();
	// We don't know what variables will be bound to, so we need
	// to conservatively skip the syntax check.
	if (tg == tag_t::REF) {
	    return;
	}
	throw syntax_exception_bad_goal(t, "Goal is not callable.");
    }
}

void interpreter_base::print_db() const
{
    print_db(std::cout);
}

void interpreter_base::print_db(std::ostream &out) const
{
    bool do_nl_p = false;
    for (const con_cell p : program_predicates_) {
	auto it = program_db_.find(p);
	if (it == program_db_.end()) {
	    continue;
	}
	const predicate &clauses = it->second;
	if (do_nl_p) {
	    out << "\n";
	}
	bool do_nl = false;
	for (auto clause : clauses) {
	    if (do_nl) out << "\n";
	    auto str = to_string(clause, term_emitter::STYLE_PROGRAM);
	    out << str;
	    do_nl = true;
	}
	do_nl_p = true;
    }
    out << "\n";
}

void interpreter_base::print_profile() const
{
    print_profile(std::cout);
}

void interpreter_base::print_profile(std::ostream &out) const
{
    struct entry {
	con_cell f;
	uint64_t t;

	bool operator < (const entry &e) const {
	    if (t < e.t) {
		return true;
	    } else if (t > e.t) {
		return false;
	    } else {
		return f.value() < e.f.value();
	    }
	}

	bool operator == (const entry &e) const {
	    return t == e.t && f == e.f;
	}
    };

    std::vector<entry> all;

    for (auto prof : profiling_) {
	auto f = prof.first;
	auto t = prof.second;
	all.push_back(entry{f,t});
    }
    std::sort(all.begin(), all.end());
    for (auto p : all) {
	auto f = p.f;
	auto t = p.t;
	std::cout << to_string(f) << ": " << t << "\n";
    }

}

void interpreter_base::abort(const interpreter_exception &ex)
{
    throw ex;
}

void interpreter_base::prepare_execution()
{
    stack_ptr_ = 0;
    num_of_args_= 0;
    memset(register_ai_, 0, sizeof(register_ai_));
    top_fail_ = false;
    register_b_ = nullptr;
    register_e_ = nullptr;
    register_e_is_wam_ = false;
    set_register_hb(heap_size());
    register_b0_ = nullptr;
    register_top_b_ = nullptr;
    register_top_e_ = nullptr;
    register_p_.reset();
}


void interpreter_base::tidy_trail()
{
    size_t from = (b() == nullptr) ? 0 : b()->tr;
    size_t to = trail_size();
    term_env::tidy_trail(from, to);
}

bool interpreter_base::definitely_inequal(const term a, const term b)
{
    using namespace common;
    if (a.tag() == tag_t::REF || b.tag() == common::tag_t::REF) {
	return false;
    }
    if (a.tag() != b.tag()) {
	return true;
    }
    switch (a.tag()) {
    case tag_t::REF: return false;
    case tag_t::CON: return a != b;
    case tag_t::STR: {
	con_cell fa = functor(a);
	con_cell fb = functor(b);
        return fa != fb;
    }
    case tag_t::INT: return a != b;
    case tag_t::BIG: return false;
    }

    return false;
}

common::cell interpreter_base::first_arg_index(const term t)
{
    switch (t.tag()) {
    case tag_t::REF: return t;
    case tag_t::CON: return t;
    case tag_t::STR: {
	con_cell f = functor(t);
	return f;
    }
    case tag_t::INT: return t;
    case tag_t::BIG: return t;
    }
    return t;
}

term interpreter_base::clause_head(const term clause)
{
    auto f = functor(clause);
    if (f == implied_by_) {
	return arg(clause, 0);
    } else {
	return clause;
    }
}

term interpreter_base::clause_body(const term clause)
{
    auto f = functor(clause);
    if (f == implied_by_) {
        return arg(clause, 1);
    } else {
        return empty_list();
    }
}

common::con_cell interpreter_base::clause_predicate(const term clause)
{
    return functor(clause_head(clause));
}

common::term interpreter_base::get_first_arg()
{
    if (num_of_args_ == 0) {
        return empty_list();
    }
    return a(0);
}


void interpreter_base::unwind_to_top_choice_point()
{
    if (top_b() == nullptr) {
        return;
    }
    reset_to_choice_point(top_b());
    set_b(top_b());
}

choice_point_t * interpreter_base::reset_to_choice_point(choice_point_t *b)
{
    auto ch = b;

    set_e(ch->ce);
    set_cp(ch->cp);
    unwind(ch->tr);
    trim_heap(ch->h);
    set_b0(ch->b0);
    set_register_hb(heap_size());
    
    register_qr_ = ch->qr;
    register_pr_ = ch->pr;

    return ch;
}

void interpreter_base::unwind(size_t from_tr)
{
    // Unbind variables
    unwind_trail(from_tr, trail_size());
    trim_trail(from_tr);
}


}}



