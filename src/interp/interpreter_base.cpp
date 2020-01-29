#include "../common/term_env.hpp"
#include "interpreter_base.hpp"
#include "builtins_fileio.hpp"
#include "wam_interpreter.hpp"
#include <boost/filesystem.hpp>
#include <boost/timer/timer.hpp>
#include <boost/algorithm/string.hpp>

#define PROFILER 0

namespace prologcoin { namespace interp {

using namespace prologcoin::common;

const common::term code_point::fail_term_ = common::ref_cell(0);
const common::con_cell interpreter_base::COMMA = common::con_cell(",",2);
const common::con_cell interpreter_base::EMPTY_LIST = common::con_cell("[]",0);
const common::con_cell interpreter_base::IMPLIED_BY = common::con_cell(":-",2);
const common::con_cell interpreter_base::ACTION_BY = common::con_cell(":-",1);
const common::con_cell interpreter_base::USER_MODULE = common::con_cell("user",0);

meta_context::meta_context(interpreter_base &i, meta_fn mfn)
{
    fn = mfn;
    old_m = i.m();
    old_top_b = i.top_b();
    old_b = i.b();
    old_b0 = i.b0();
    old_top_e = i.top_e();
    old_e = i.e();
    old_e_kind = i.e_kind();
    old_p = i.p();
    old_cp = i.cp();
    old_qr = i.qr();
    old_hb = i.get_register_hb();
}

interpreter_base::interpreter_base() : has_updated_predicates_(false), register_pr_("", 0), arith_(*this), locale_(*this), current_module_("system",0)
{
    init();

    // These will be loaded into the system module
    builtins::load(*this);

    tidy_size = 0;

    register_top_b_ = nullptr;
    register_top_e_ = nullptr;
    register_b_ = nullptr;
    register_b0_ = nullptr;
    register_e_ = nullptr;
    register_e_kind_ = ENV_NAIVE;
    register_p_.reset();
    register_cp_.reset();
    register_m_ = nullptr;
    num_of_args_ = 0;
    memset(&register_ai_[0], 0, sizeof(common::term)*MAX_ARGS);
    num_y_fn_ = nullptr;
    save_state_fn_ = nullptr;
    restore_state_fn_ = nullptr;
    maximum_cost_ = std::numeric_limits<uint64_t>::max();

    // This is only needed to be true for the global interpeter whichs
    // tracks the global state.
    frozen_closures.set_auto_rehash(false);
}

void interpreter_base::init()
{
    debug_ = false;
    track_cost_ = false;
    file_id_count_ = 3;
    num_of_args_= 0;
    memset(register_ai_, 0, sizeof(register_ai_));
    stack_ = reinterpret_cast<word_t *>(new char[MAX_STACK_SIZE]);
    num_y_fn_ = &num_y;
    save_state_fn_ = &save_state;
    restore_state_fn_ = &restore_state;
    standard_output_ = nullptr;
    prepare_execution();
}

interpreter_base::~interpreter_base()
{
    for (auto &p : managed_data_) {
        auto md = p.second;
	delete md;
    }
    managed_data_.clear();
    arith_.unload();
    close_all_files();
    register_cp_.reset();
    register_qr_ = term();
    syntax_check_stack_.clear();
    builtins_.clear();
    program_db_.clear();
    module_db_.clear();
    module_db_set_.clear();
    module_meta_db_.clear();
    program_predicates_.clear();
}

void interpreter_base::set_current_module(con_cell mod)
{
    static con_cell SYSTEM("system",0);
    bool is_new_module = !is_existing_module(mod) && mod != SYSTEM;
    get_module(mod);
    current_module_ = mod;
    if (is_new_module) {
        use_module(SYSTEM);
    }
}
    
void interpreter_base::reset()
{
    unwind_to_top_choice_point();
    while (has_meta_context()) {
	auto *m = get_current_meta_context();
	m->fn(*this, meta_reason_t::META_DELETE);
	unwind_to_top_choice_point();
    }
    bool cont = false;
    do {
        cont = false;
        while (b() != top_b()) {
	    reset_to_choice_point(b());
	    set_b(b()->b);
	    if (b() != nullptr) set_register_hb(b()->h);
	}
	if (top_b() != nullptr) {
	    set_top_b(top_b()->b);
	    cont = true;
	}
    } while (cont);
}

std::string code_point::to_string(const interpreter_base &interp) const
{
    if (has_wam_code()) {
	std::stringstream ss;
	wam_code()->print(ss, static_cast<wam_interpreter &>(const_cast<interpreter_base &>(interp)));
	return ss.str();
    } else if (is_fail()) {
	return "fail";
    } else {
        std::string s;
        if (module() != interpreter_base::USER_MODULE) {
	    s += interp.to_string(module());
	    s += ":";
        }
	s += interp.to_string(term_code());
	return s;
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
    file_id_count_ = 3;
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

file_stream & interpreter_base::standard_output()
{
    if (standard_output_ == nullptr) {
	standard_output_ = new file_stream(*this, 0, "<stdout>");
	standard_output_->open(std::cout);
    }
    return *standard_output_;
}

void interpreter_base::tell_standard_output(file_stream &fs)
{
    standard_output_stack_.push(standard_output_);
    standard_output_ = &fs;
}

void interpreter_base::told_standard_output()
{
    standard_output_ = standard_output_stack_.top();
    standard_output_stack_.pop();
}

bool interpreter_base::has_told_standard_outputs()
{
    return !standard_output_stack_.empty();
}

void interpreter_base::syntax_check()
{
    while (!syntax_check_stack_.empty()) {
	auto f = syntax_check_stack_.back();
	syntax_check_stack_.pop_back();
	f();
    }
}

void interpreter_base::preprocess_freeze(term t)
{
    term body = clause_body(t);
    preprocess_freeze_body(body);
}

void interpreter_base::preprocess_freeze_body(term goal)
{
    static const common::con_cell op_comma(",", 2);
    static const common::con_cell op_semi(";", 2);
    static const common::con_cell op_imply("->", 2);
    static const common::con_cell freeze("freeze", 2);

    if (goal.tag() != common::tag_t::STR) {
        return;
    }
    
    auto f = functor(goal);
    if (f == op_comma || f == op_semi || f == op_imply) {
        preprocess_freeze_body(arg(goal, 0));
	preprocess_freeze_body(arg(goal, 1));
	return;
    }
    if (f == freeze) {
        // Rewrite
        // freeze(Var, Body)
        // freeze(Var, '$freeze':<id>(<Var> <Closure Vars>))
        // where:
        // '$freeze':'<id>'(<Var> <Closure Vars> ...) :- Body.
        term freezeVar = arg(goal, 0);
	term freezeBody = arg(goal, 1);
	preprocess_freeze_body(freezeBody);
	
	set_arg(goal, 1, rewrite_freeze_body(freezeVar, freezeBody));
    }
}

term interpreter_base::rewrite_freeze_body(term freezeVar, term freezeBody)
{
    static const common::con_cell freeze_module("$freeze",0);
    static const common::con_cell op_clause(":-", 2);
    static const common::con_cell op_colon(":", 2);

    // Not a functor?
    if (freezeBody.tag() != common::tag_t::STR) {
        return freezeBody;
    }

    // Already rewritten? 
    if (functor(freezeBody) == op_colon) {
        if (arg(freezeBody, 0) == freeze_module) {
   	    return freezeBody;
	}
    }

    std::unordered_set<term> vars;
    std::vector<term> vars_list;
    std::unordered_map<term,term> varmap;

    vars_list.push_back(freezeVar);
    vars.insert(freezeVar);
    for (auto t : iterate_over(freezeBody)) {
      if (t.tag().is_ref()) {
	t = deref(t);
	if (vars.count(t) == 0) {
	  vars.insert(t);
	  vars_list.push_back(t);
	}
      }
    }
    
    auto qname = gen_predicate(freeze_module, vars_list.size());
	
    auto head = new_str(qname.second);
    for (size_t i = 0; i < vars_list.size(); i++) {
      varmap[vars_list[i]] = arg(head,i);
    }

    // Remap vars (closure vars to new vars)
    for (auto t : iterate_over(freezeBody)) {
      if (t.tag() == common::tag_t::STR) {
	size_t num_args = functor(t).arity();
	for (size_t i = 0; i < num_args; i++) {
	  term ai = arg(t, i);
	  if (ai.tag().is_ref()) {
	    set_arg(t, i, varmap[ai]);
	  }
	}
      }
    }

    term freeze_clause = new_str(op_clause);
    set_arg(freeze_clause, 0, head);
    set_arg(freeze_clause, 1, freezeBody);

    get_predicate(qname).push_back(managed_clause(freeze_clause,0));

    auto closure_head = new_str(qname.second);
    for (size_t i = 0; i < vars_list.size(); i++) {
        set_arg(closure_head, i, vars_list[i]);
    }
    
    auto closure_mod = new_str(op_colon);
    set_arg(closure_mod, 0, qname.first);
    set_arg(closure_mod, 1, closure_head);

    return closure_mod;
}
    
void interpreter_base::load_clause(term t, interpreter_base::clause_position pos)
{
    syntax_check_stack_.push_back(
		  std::bind(&interpreter_base::syntax_check_clause, this,
			    t));
    syntax_check();

    // We want to prerocess freeze/2 calls:
    //    foo(A,X,Y) :-
    //       bar(A,Y,Z),
    //       freeze(Z, (baz(X,Y,Z), baz2(Z,Z1))).
    // becomes:
    //    foo(A,X,Y) :-
    //       bar(A,Y,Z),
    //       '$closure'('$frz123',Z,X,Y).
    //
    //    '$frz123'(Z,X,Y) :-
    //       baz(X,Y,Z), baz2(Z,Z1).
    //
    // Where '$closure' will create a closure on the heap and
    // register its first variable as "frozen." It'll automatically
    // awake the closure and use the closure vars to create a call
    // to the generated clause '$frz123' (which is unique for each
    // closure construction.) This will minimize the amount of stuff
    // created on the heap; just the closure will be created and not
    // a duplicate of the clause body. We can also WAM compile this
    // generated predicate separately for accelaration.
    //
    // However, it won't be able to manage higher order constructions
    // of freeze; those will still be rather inefficient, but this covers
    // the most common case.
    preprocess_freeze(t);

    con_cell module = current_module_;

    // This is a valid clause. Let's lookup the functor of its head.

    term head = clause_head(t);

    if (head == ACTION_BY) {
	return;
    }

    con_cell predicate = functor(head);

    if (predicate == ACTION_BY) {
        return;
    }

    auto qn = std::make_pair(module, predicate);

    auto found = program_db_.find(qn);
    if (found == program_db_.end()) {
        program_db_[qn] = managed_clauses();
	program_predicates_.push_back(qn);
    } else {
        if (pos == LAST_CLAUSE_OVERRIDE_OLD) {
	    if (!is_updated_predicate(qn)) {
		// This is the first time. So we'll retract whatever
		// we currently have in our database.
		program_db_[qn].clear();
	    }
	}
    }
    add_updated_predicate(qn);

    auto &lst = program_db_[qn];
    if (pos == FIRST_CLAUSE) {
        lst.insert(lst.begin(), managed_clause(t, cost(t)));
    } else {
        lst.push_back(managed_clause(t, cost(t)));
    }

    if (module_db_set_[module].count(qn) == 0) {
        module_db_set_[module].insert(qn);
	module_db_[module].push_back(qn);
    }

    set_code(qn, code_point(module, predicate));
}

void interpreter_base::load_builtin(const qname &qn, builtin b)
{
    auto found = builtins_.find(qn);
    if (found == builtins_.end()) {
        builtins_[qn] = b;
	module_db_[qn.first].push_back(qn);
	set_code(qn, code_point(qn.second, b.fn(), b.is_recursive()));
    }
}

void interpreter_base::set_debug_enabled()
{
    load_builtin(functor("debug_on",0), &builtins::debug_on_0);
    load_builtin(functor("debug_check",0), &builtins::debug_check_0);
}

void interpreter_base::enable_file_io()
{
    load_builtins_file_io();
}

void interpreter_base::set_current_directory(const std::string &dir)
{
    current_dir_ = dir;
}

const std::string & interpreter_base::get_current_directory() const
{
    return current_dir_;
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
    load_builtin(con_cell("write", 2), &builtins_fileio::write_2);
    load_builtin(con_cell("nl", 0), &builtins_fileio::nl_0);
    load_builtin(con_cell("tell",1), &builtins_fileio::tell_1);
    load_builtin(con_cell("told",0), &builtins_fileio::told_0);
    load_builtin(con_cell("format",2), builtin(&builtins_fileio::format_2,true));
    load_builtin(con_cell("sformat",3), builtin(&builtins_fileio::sformat_3,true));
}

qname interpreter_base::gen_predicate(const common::con_cell module,
				      size_t arity)
{
    static const char ALPHABET[64] = {
      'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p',
      'q','r','s','t','u','v','w','x','y','z','A','B','C','D','E','F',
      'G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V',
      'W','X','Y','Z','0','1','2','3','4','5','6','7','8','9','-','+'};
    
    size_t num = program_predicates_.size() + 1;
    char name[7];
    size_t i;
    for (i = 0; i < 7 && num != 0; i++) {
        name[6-i] = ALPHABET[num & 0x3f];
	num >>= 6;
    }
    std::string atom_name(&name[6-(i-1)], i);

    common::con_cell p = functor(atom_name, arity);
    qname pn(module, p);
    program_predicates_.push_back(pn);
    updated_predicates_.insert(pn);
    has_updated_predicates_ = true;

    if (module_db_set_[module].count(pn) == 0) {
        module_db_set_[module].insert(pn);
	module_db_[module].push_back(pn);
    }

    return pn;
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
	if (tg.is_ref()) {
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
    for (const auto &qn : program_predicates_) {
	auto it = program_db_.find(qn);
	if (it == program_db_.end()) {
	    continue;
	}
	const predicate &m_clauses = it->second;
	if (do_nl_p) {
	    out << "\n";
	}
	emitter_options opt;
	opt.set(emitter_option::EMIT_PROGRAM);
	bool do_nl = false;
	for (auto &m_clause : m_clauses) {
	    if (do_nl) out << "\n";
	    std::string mod = "";
	    if (qn.first != USER_MODULE) {
	        mod = to_string(qn.first)+":";
	    }
	    auto str = mod+to_string(m_clause.clause(), opt);
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
    num_of_args_= 0;
    memset(register_ai_, 0, sizeof(register_ai_));
    top_fail_ = false;
    complete_ = false;
    register_top_b_ = register_b_;
    register_b0_ = register_b_;
    register_top_e_ = register_e_;
    set_register_hb(heap_size());
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
    if (a.tag().is_ref() || b.tag().is_ref()) {
        return false;
    }
    
    if (a.tag() != b.tag()) {
	return true;
    }
    switch (a.tag()) {
    case tag_t::REF: case tag_t::RFW: return false;
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
    case tag_t::REF: case tag_t::RFW: return t;
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
    if (f == IMPLIED_BY) {
	return arg(clause, 0);
    } else {
	return clause;
    }
}

term interpreter_base::clause_body(const term clause)
{
    auto f = functor(clause);
    if (f == IMPLIED_BY) {
        return arg(clause, 1);
    } else {
        return EMPTY_LIST;
    }
}

common::con_cell interpreter_base::clause_predicate(const term clause)
{
    return functor(clause_head(clause));
}

common::term interpreter_base::get_first_arg()
{
    if (num_of_args_ == 0) {
        return EMPTY_LIST;
    }
    return deref(a(0));
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

    set_m(ch->m);
    set_e(ch->ce);
    set_cp(ch->cp);
    unwind(ch->tr);
    trim_heap(ch->h);
    set_b0(ch->b0);
    set_register_hb(heap_size());
    
    register_qr_ = ch->qr;
    register_pr_ = ch->pr;

    size_t n = ch->arity;
    for (size_t i = 0; i < n; i++) {
	a(i) = ch->ai[i];
    }

    return ch;
}

void interpreter_base::unwind(size_t from_tr)
{
    size_t n = trail_size();
    unwind_frozen_closures(from_tr, n);
    unwind_trail(from_tr, n);
    trim_trail(from_tr);
}

void interpreter_base::use_module(con_cell module_name)
{
    auto &qnames = get_module(module_name);

    auto module = current_module();
    
    for (auto &qn : qnames) {
        auto &cp = get_code(qn);
        qname imported_qn(module, qn.second);
	set_code(imported_qn, cp);
    }
}
    
void interpreter_base::save_program(con_cell module, std::ostream &out)
{
    std::unordered_set<qname> seen_predicates;

    for (auto &se : module_meta_db_[module]) {
	switch (se.type()) {
	case source_element::SOURCE_NONE: break;
	case source_element::SOURCE_COMMENT: save_comment(se.comment(), out); break;
	case source_element::SOURCE_ACTION: save_clause(se.action(), out); break;
	case source_element::SOURCE_PREDICATE:
	    qname qn(module, se.predicate());
	    save_predicate(qn, out);
	    seen_predicates.insert(qn); break;
	}
    }

    for (auto &qn : module_db_[module]) {
        if (!seen_predicates.count(qn)) {
	    save_predicate(qn, out);
	}
    }
}

void interpreter_base::save_comment(const term_tokenizer::token &comment, std::ostream &out)
{
    std::string lexeme = comment.lexeme();
    boost::trim(lexeme);
    out << lexeme;
    out << std::endl;
}

void interpreter_base::save_clause(term t, std::ostream &out)
{
    term_emitter emit(out, *this);
    emit.set_var_naming(var_naming());
    emit.options().set(emitter_option::EMIT_PROGRAM);
    emit.print(t);
    emit.nl();
}

void interpreter_base::save_predicate(const qname &qn, std::ostream &out)
{
    term_emitter emit(out, *this);
    emit.set_var_naming(var_naming());    
    emit.options().set(emitter_option::EMIT_PROGRAM);
    bool empty = true;
    for (auto &managed : get_predicate(qn)) {
        empty = false;
	emit.print(managed.clause());
	emit.nl();
    }
    if (!empty) emit.nl();
}

}}
