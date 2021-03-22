#include "interpreter.hpp"
#include "wam_compiler.hpp"
#include <boost/range/adaptor/reversed.hpp>
#include <boost/filesystem.hpp>

using namespace prologcoin::common;

namespace prologcoin { namespace interp {

interpreter::interpreter(const std::string &name) : wam_interpreter(name)
{
    wam_enabled_ = true;
    query_vars_ = nullptr;
    num_instances_ = 0;
    retain_state_between_queries_ = false;
}

void interpreter::total_reset()
{
    wam_interpreter::total_reset();

    wam_enabled_ = true;
    query_vars_ = nullptr;
    num_instances_ = 0;
    retain_state_between_queries_ = false;
}

interpreter::~interpreter()
{
    delete query_vars_;
}

void interpreter::setup_standard_lib()
{
    std::string lib = R"PROG(

%
% Some standard predicates
%

%
% member/2
%

member(X, Xs) :- '$member0'(Xs, X).
'$member0'([X|_], X).
'$member0'([_|Xs], X) :- '$member0'(Xs, X).

%
% reverse/2
%

reverse(Xs, Ys) :-
    '$reverse0'(Xs, [], Ys).

'$reverse0'([], Ys, Ys).
'$reverse0'([X|Xs], Acc, Ys) :-
    '$reverse0'(Xs, [X|Acc], Ys).

%
% append/3
%

append([], Zs, Zs).
append([X|Xs], Ys, [X|Zs]) :-
    append(Xs, Ys, Zs).

%
% forall/2
%

forall(Cond,Action) :-
    \+ ( Cond, \+ Action ).

%
% last/2
%

last([Last],Last).
last([X|Xs],Last) :- last(Xs,Last).

%
% length/2
%

length(Xs, N) :- '$length'(Xs,N,0).
'$length'([], N, N) :- !.
'$length'([_|Xs], N, I) :- I1 is I + 1, '$length'(Xs, N, I1).

)PROG";

    auto old_mod = current_module();
    set_current_module(con_cell("system",0));
    // Check if standard library is already available
    // (This enables the client of this class to load the library in a
    //  different way, e.g. the global interpreter will restore a global
    //  persistent state.)
    auto &member_2 = get_predicate(con_cell("system",0), con_cell("member",2));
    if (member_2.empty()) {
        // Nope, so load it
        load_program(lib);
	auto &member_2_verify = get_predicate(con_cell("system",0), con_cell("member",2));
	assert(!member_2_verify.empty());
    }
    load_builtin(con_cell("consult", 1), consult_1);
    load_builtin(con_cell("compile", 0), compile_0);
    load_builtin(con_cell("@",2), operator_at_2);
    load_builtin(con_cell("@-",2), operator_at_silent_2);
    load_builtin(con_cell("@=",2), operator_at_parallel_2);
    
    compile();
    set_current_module(con_cell("user",0));
    use_module(con_cell("system",0));
    set_current_module(old_mod);
}

// Save everything so interpreter state can be restored.
struct new_instance_context : public meta_context {
    new_instance_context(interpreter_base &i, meta_fn fn)
	: meta_context(i, fn),
	  old_accumulated_cost(reinterpret_cast<interpreter &>(i).accumulated_cost()),
	  old_num_of_args(reinterpret_cast<interpreter &>(i).num_of_args()),
	  old_top_fail(reinterpret_cast<interpreter &>(i).is_top_fail()),
	  old_complete(reinterpret_cast<interpreter &>(i).is_complete()),
	  old_top_hb(i.top_hb()),
	  old_top_tr(i.top_tr()),
	  old_query_vars(reinterpret_cast<interpreter &>(i).query_vars_ptr())
    {
	memcpy(&old_ai[0], i.args(), sizeof(common::term)*i.num_of_args());
	reinterpret_cast<interpreter &>(i).set_query_vars( nullptr );
    }

    uint64_t old_accumulated_cost;
    size_t old_num_of_args;
    bool old_top_fail;
    bool old_complete;
    size_t old_top_hb;
    size_t old_top_tr;
    std::vector<interpreter::binding> *old_query_vars;
    common::term old_ai[interpreter_base::MAX_ARGS];
};

void interpreter::new_instance()
{
    new_meta_context<new_instance_context>(interpreter::new_instance_meta);
    num_instances_++;
}

// new_instance_meta acts as a stop gap. It's like the interpeter is run
// for the first time. You won't get passed this marker. The only way would
// be to release the instance.
bool interpreter::new_instance_meta(interpreter_base &interp0, const meta_reason_t &reason)
{
    if (reason == meta_reason_t::META_RETURN) {
	return !interp0.is_top_fail();
    }

    if (reason == meta_reason_t::META_BACKTRACK) {
	return true;
    }

    if (reason == meta_reason_t::META_DELETE) {
	auto *context = reinterpret_cast<new_instance_context *>(interp0.get_current_meta_context());
	auto &interp = reinterpret_cast<interpreter &>(interp0);
	auto *qv = interp.query_vars_ptr();
        delete qv;
	interp.set_query_vars(context->old_query_vars);
	interp.reset_accumulated_cost(context->old_accumulated_cost);
	interp.set_num_of_args(context->old_num_of_args);
	interp.set_top_fail(context->old_top_fail);
	interp.set_complete(context->old_complete);
	interp.set_top_hb(context->old_top_hb);
	interp.set_top_tr(context->old_top_tr);
	memcpy(interp.args(), &context->old_ai[0], sizeof(common::term)*interp.num_of_args());
	interp.release_last_meta_context();
    }
    return false;
}

//
//
bool interpreter::debug_predicate_1(interpreter_base &interp, size_t arity, common::term args[]) {
    interpreter &i = static_cast<interpreter &>(interp);
    i.check_it();
    throw std::runtime_error("foo");
    return true;
}

	
bool interpreter::execute(const term query)
{
    using namespace prologcoin::common;

    bool do_new_instance = false;
    
    while (num_instances() > 0 && !has_more()) {
	delete_instance();
    }
    
    if (has_more()) {
	new_instance();
	do_new_instance = true;
    } else {
        // Overriding last instance
        if (!retain_state_between_queries_) {
	    clear_all_frozen_closures();
        }
    }
    if (query_vars_ == nullptr) {
	query_vars_ = new std::vector<binding>();
    }

    query_vars_->clear();
    reset_accumulated_cost();
    set_top_fail(false);
    prepare_execution();

    std::unordered_set<std::string> seen;
    bool query_has_vars = false;

    // Record all vars for this query
    std::for_each( begin(query),
		   end(query),
		   [&](term t) {

		     if (t.tag().is_ref()) {
		  	   query_has_vars = true;
		           t = reinterpret_cast<ref_cell &>(t).unwatch();
			   const std::string name = to_string(t);
			   if (!seen.count(name)) {
			       query_vars_->push_back(binding(name,t));
			       seen.insert(name);
			   }
		       }
		   } );

    set_p(code_point(interpreter_base::EMPTY_LIST));
    set_cp(code_point(interpreter_base::EMPTY_LIST));
    set_qr(query);

    set_p(code_point(query));

    bool b = cont();

    if (!is_persistent_password() && num_instances() == 0 && !has_more()) {
        clear_secret();
    }

    set_qr(query);

    if (!has_more() && !query_has_vars && do_new_instance) {
	delete_instance();
    }
    
    return b;
}

bool interpreter::cont()
{
    try {
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

	    if (is_complete() && has_meta_context()) {
	        meta_context *mc = get_current_meta_context();
		meta_fn fn = mc->fn;
		if (!fn(*this, meta_reason_t::META_RETURN)) {
		    set_complete(false);
		    fail();
		}
	    }
	}
	set_register_hb(b());
	tidy_trail();
    } catch (std::runtime_error &) {
        reset();
	throw;
    }

    bool r = !is_top_fail();

    if (!is_persistent_password() && num_instances() == 0 && !r) {
        clear_secret();
    }
    
    return r;
}

bool interpreter::is_instance() const
{
    if (has_more()) {
	return false;
    }
    return has_meta_context() &&
	   get_current_meta_context()->fn == interpreter::new_instance_meta;
}

void interpreter::delete_instance()
{
    num_instances_--;
    assert(!has_more());
    if (meta_context *mc = get_current_meta_context()) {
	meta_fn fn = mc->fn;
	if (fn == interpreter::new_instance_meta) {
	    fn(*this, meta_reason_t::META_DELETE);
	}
    }
}

bool interpreter::next()
{
    while (num_instances() > 0 && !has_more()) {
	delete_instance();
    }
    
    term old_qr = qr();

    reset_accumulated_cost();

    fail();
    if (!is_top_fail()) {
	cont();
    }

    set_qr(old_qr);

    bool r = !is_top_fail();

    return r;
}

common::term interpreter::query_var_list()
{
    using namespace prologcoin::common;

    static const con_cell eq_functor("=", 2);

    common::term varlist = interpreter_base::EMPTY_LIST;
    for (auto &binding : boost::adaptors::reverse(query_vars())) {
	auto binding_term =
	    new_term(eq_functor, {functor(binding.name(), 0),binding.value()});
	varlist = new_dotted_pair(binding_term, varlist);
    }
    return varlist;
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
	    // Invoke meta-function if we have one
	    if (m() != nullptr) {
		meta_fn fn = m()->fn;
		if (!fn(*this, meta_reason_t::META_BACKTRACK)) {
		    ok = false;
		}
	    }
        } else if (b_is_wam()) {
	    ok = backtrack_wam();
	} else {
	    auto ch = reset_to_choice_point(b());
	    auto bp = ch->bp;

	    if (bp.is_fail()) {
		// Do nothing
	    } else if (bp.is_builtin()) {
	        ok = bp.bn()(*this, ch->arity, args());	      
	    } else if (bp.term_code().tag() != common::tag_t::INT) {
	        // Direct query
	        static managed_clauses empty_clauses;
	        ok = select_clause(bp, 0, empty_clauses, 0);
	    } else {
		auto bpterm = bp.term_code();
		size_t bpval = reinterpret_cast<const int_cell &>(bpterm).value();
		// Is there another clause to backtrack to?
		if (bpval != 0) {
		    size_t pred_id = bpval >> 32;
		    
		    auto &pred = get_predicate(pred_id);
		    auto first_arg = get_first_arg(qr());
		    if (is_debug()) {
			std::string redo_str = to_string(qr());
			std::cout << "interpreter::fail(): redo " << redo_str << " (first_arg=" << to_string(first_arg) << ")" << std::endl;
		    }
		    auto &clauses = pred.get_clauses(*this, first_arg);
		    size_t from_clause = bpval & 0xffffffff;
		    ok = select_clause(code_point(qr()), pred_id, clauses, from_clause);
		}
	    }
	    if (!ok) {
		set_b(ch->b);
		set_register_hb(b());
	    }
	}
    }
}

bool interpreter::unify_args(term head, const code_point &p)
{
    static const common::con_cell colon(":",2);
    
    if (p.term_code().tag() == common::tag_t::STR) {
        term goal = p.term_code();
	if (functor(goal) == colon) {
	    goal = arg(goal, 1);
	}
	return unify(head, goal);
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
	if (head == colon) {
	    head = arg(head, 1);
	}
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
	term_env::tidy_trail(start_trail, trail_size());

	return !fail;
    }
}

bool interpreter::can_unify_args(term head, const code_point &p)
{
    static const common::con_cell colon(":",2);
    
    if (p.term_code().tag() == common::tag_t::STR) {
        term goal = p.term_code();
	if (functor(goal) == colon) {
	    goal = arg(goal, 1);
	}
	return can_unify(head, goal);
    } else {
	// Otherwise this is an already disected call with
	// args. So unify with arguments.
	size_t n = num_of_args();
	bool fail = false;
	if (head == colon) {
	    head = arg(head, 1);
	}
	for (size_t i = 0; i < n; i++) {
	    auto arg_i = arg(head, i);
	    if (!can_unify(arg_i, a(i))) {
		fail = true;
		break;
	    }
	}
	return !fail;
    }
}


bool interpreter::select_clause(const code_point &instruction,
				size_t predicate_id,
				const managed_clauses &clauses,
				size_t from_clause)
{
    if (is_debug()) {
	std::cout << "select clause predicate_id=" << predicate_id << " predicate=" << to_string(get_predicate(predicate_id).qualified_name()) << " from=" << from_clause << "\n";
      
    }
    if (predicate_id == 0) {
	set_b(b()->b);
        if (from_clause > 1) {
	    return false;
	}
        set_p(instruction);
	return true;
    }

    size_t num_clauses = clauses.size();
    bool has_choices = num_clauses > 1;

    if (is_debug()) {
        std::cout << "select clause: num_clauses=" << num_clauses << ": match with=" << to_string(instruction.term_code()) << "\n";
	for (size_t i = 0; i < num_clauses; i++) {
	    std::cout << "  [" << i << "]: " << to_string(clauses[i].clause()) << std::endl;
	}
    }
    
    // Let's go through clause by clause and see if we can find a match.
    for (size_t i = from_clause; i < num_clauses; i++) {
        auto &m_clause = clauses[i];

	// Avoid copying if it does not match
	if (!can_unify_args(clause_head(m_clause.clause()),
			    code_point(instruction.term_code()))) {
	    continue;
	}
	
	size_t current_heap = heap_size();
	// No need to copy the names when instantiating a new clause from
	// the program database, as it's the caller's names that matter.
	auto copy_clause = copy_without_names(m_clause.clause()); // Instantiate it

	term copy_head = clause_head(copy_clause);
	term copy_body = clause_body(copy_clause);

	if (unify_args(copy_head, code_point(instruction.term_code()))) { // Heads match?
	    // Update choice point (where to continue on fail...)
	    if (has_choices) {
	        auto choice_point = b();
		if (i == num_clauses - 1) {
		    // If we are at the last clause, then we can remove the choice point.
		    choice_point->bp = code_point::fail();
		    interpreter_base::cut();
		} else {
		    choice_point->bp = code_point(int_cell((predicate_id << 32) + (i+1)));
		}
	    }

	    allocate_environment<ENV_NAIVE>();
	    set_cp(code_point(interpreter_base::EMPTY_LIST));
	    set_p(code_point(copy_body));
	    set_qr(copy_head);

	    // We've found a clause to execute. At this point we'll
	    // add the cost of the clause. Note that unification above
	    // is also adding to the cost.
	    
	    add_accumulated_cost(m_clause.cost());

	    check_frozen(); // After unification of head we'll process
	                    // frozen closures (so these are run first.)
	    return true;
	} else {
    	    // Discard garbage created on heap (due to copying clause)
	    trim_heap_safe(current_heap);
	}
    }

    set_p(code_point(interpreter_base::EMPTY_LIST));
    // None found.
    return false;
}

void interpreter::dispatch()
{
    static const con_cell functor_colon(":",2);

    set_qr(p().term_code());

    con_cell f = functor(qr());

    if (f == interpreter_base::EMPTY_LIST) {
        // Return
	deallocate_and_proceed();
	if (is_debug()) {
	    std::cout << "interpreter::dispatch(): pop: e=" << e0() << std::endl;
	}
	if (e0() == top_e()) {
	    if (is_debug()) {
	        std::cout << "interpreter::dispatch: pop: e=top: p_is_wam=" << p().has_wam_code() << " num_of_args=" << num_of_args() << std::endl;

	    }
  	    if (!p().has_wam_code() && p().term_code() == interpreter_base::EMPTY_LIST) {
		set_complete(true);
	    }
	}
	return;
    }

    con_cell module = current_module_;

    // Refine module if we have one in the code point.
    if (p().module() != EMPTY_LIST) module = p().module();

    if (f == functor_colon) {
	// This is module referred
	module = functor(arg(qr(), 0));
	f = functor(arg(qr(), 1));
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
    // the term_code() is the call instruction encoded as a term.

    common::tag_t ptag = p().term_code().tag();
    switch (ptag) {
    case common::tag_t::STR: {
	static const common::con_cell colon(":",2);
	term goal = p().term_code();
	if (functor(goal) == colon) {
	    goal = arg(goal, 1);
	}
	for (size_t i = 0; i < arity; i++) {
	    a(i) = arg(goal, i);
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

    // Is instruction already a built-in (can happen for native backtracking)
    
    if (p().is_builtin()) {
	if (!(p().bn())(*this, arity, args())) {
	    fail();
	    return;
	}
	set_p(cp());
	return;
    }

    // Is this a built-in?
    qname qn(module, f);

    auto &code = get_code(qn);
    if (code.is_builtin()) {
	set_p(cp());
	if (!code.is_builtin_recursive()) {
	    // This enforces eventually a pop up the stack and
	    // P becomes CP.
	    set_cp(code_point(interpreter_base::EMPTY_LIST));
	}
	if (!(code.bn())(*this, arity, args())) {
	    fail();
	}
	check_frozen();
	return;
    }

    // Refine module and qualified name if this is an imported predicate
    if (!code.is_fail()) {
        module = code.module();
	qn = qname(module, f);
    }

    bool is_updated = has_updated_predicates() && is_updated_predicate(qn);

    if (is_wam_enabled()) {
	if (!code.has_wam_code() && is_auto_wam()) {
	    auto_compile(qn);
	} else if (is_updated) {
   	    recompile_if_needed(qn);
        }
	if (code.has_wam_code()) {
	    dispatch_wam(code.wam_code());
	    return;
	}
    }

    auto first_arg = get_first_arg();
    const predicate &pred = get_predicate(module, f);

    if (pred.empty()) {
        std::stringstream msg;
	msg << "Undefined predicate ";
	if (module != USER_MODULE) {
	    msg << atom_name(module) << ":";
	}

	msg << atom_name(f) << "/" << f.arity();
	throw interpreter_exception_undefined_predicate(msg.str());
	return;
    }

    set_pr(f);

    // Otherwise a vector of clauses
    auto &clauses = pred.get_clauses(*this, first_arg);

    set_b0(b());

    size_t num_clauses = clauses.size();
    bool has_choices = num_clauses > 1;
    size_t pred_id = pred.id();

    // More than one clause that matches? We need a choice point.
    if (has_choices) {
        int_cell index_id_int(static_cast<int64_t>(pred_id) << 32);
	code_point ch(index_id_int);
	allocate_choice_point(ch);
    }

    if (!select_clause(code_point(p().term_code()), pred_id, clauses, 0)) {
	fail();
    }
}

void interpreter::dispatch_wam(wam_instruction_base *instruction)
{
    allocate_environment<ENV_WAM>();
    set_p(instruction);
    set_cp(code_point(interpreter_base::EMPTY_LIST));
}

std::string interpreter::get_result(bool newlines) const
{
    using namespace prologcoin::common;

    interpreter &ii = const_cast<interpreter &>(*this);
    std::vector<ref_cell> touched;
    ii.prettify_var_names(qr(), touched);
    
    std::vector<std::string> result;

    bool first = true;

    std::stringstream ss;
    for (auto &v : *query_vars_) {
	auto &name = v.name();
	if (name == "_") continue;
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

    for (auto t : touched) {
        ii.clear_name(t);
    }

    if (first) {
	ss << "true";
    }

    if (newlines) {
	ss << std::endl;
    }

    return ss.str();
}

interpreter::term interpreter::get_result_term() const
{
    return deref(qr());
}

interpreter::term interpreter::get_result_term(const std::string &varname) const
{
    term t;
    for (auto &v : *query_vars_) {
	auto &name = v.name();
	if (name == varname) {
	    return deref(v.value());
	}
    }
    return t;
}

void interpreter::print_result(std::ostream &out) const
{
    out << get_result();
}

bool interpreter::compile_0(interpreter_base &interp0, size_t arity, common::term args[])
{
    auto &interp = reinterpret_cast<interpreter &>(interp0);
    interp.compile();
    return true;
}

void interpreter::load_file(const std::string &filename)
{
    std::ifstream infile(get_full_path(filename));
    if (!infile.good()) {
	throw interpreter_exception_file_not_found("Couldn't open file '" + filename + "'");
    }

    try {
        struct pre_action {
	    pre_action(interpreter &interp) : interp_(interp) { }

	    void operator () (term clause) {
		auto head = interp_.clause_head(clause);
		auto head_f = interp_.functor(head);

		if (head_f == interpreter_base::ACTION_BY) {
		    interp_.compile();
		}
	    }
	    interpreter &interp_;
	};
	load_program<pre_action>(infile);
	compile();
	infile.close();
    } catch (const syntax_exception &ex) {
	throw ex;
    } catch (const interpreter_exception &ex) {
	throw ex;
    } catch (const token_exception &ex) {
        throw ex;
    } catch (const term_parse_exception &ex) {
        throw ex;
    } catch (std::runtime_error &ex) {
	std::string msg("Unknown error: ");
	msg += ex.what();
	throw interpreter_exception_unknown(msg);
    } catch (...) {
	throw interpreter_exception_unknown("Unknown error");
    }
}

bool interpreter::consult_1(interpreter_base &interp0, size_t arity, common::term args[])
{
    using namespace prologcoin::common;

    auto &interp = reinterpret_cast<interpreter &>(interp0);

    std::string filename;
    if (interp.is_atom(args[0])) {
        filename = interp.atom_name(args[0]);
    } else if (interp.is_string(args[0])) {
        filename = interp.list_to_string(args[0]);        
    } else {
        throw interpreter_exception_wrong_arg_type("Atom or string expected; was " + interp.to_string(args[0]));
    }

    boost::filesystem::path path(filename);
    if (!path.has_extension()) {
	filename += ".pl";
    }

    interp.load_file(filename);

    return true;
}

bool interpreter::is_else(interpreter_base &interp, term t) {
    return t.tag() == tag_t::STR && interp.functor(t) == con_cell("else",2);
}

std::pair<term, term> interpreter::extract_else(interpreter_base &interp, term t) {
    return std::make_pair(interp.arg(t, 1), interp.arg(t, 0));
}

bool interpreter::is_timeout(interpreter_base &interp, term t) {
    return t.tag() == tag_t::STR && interp.functor(t) == con_cell("timeout",2); 
}

std::pair<size_t, term> interpreter::extract_timeout(interpreter_base &interp, term t) {
    size_t timeout = std::numeric_limits<size_t>::max();
    term timeout_term = interp.arg(t, 1);
    if (timeout_term.tag() == tag_t::INT) {
	auto v = reinterpret_cast<int_cell &>(timeout_term).value();
	if (v > 0) {
	    timeout = static_cast<size_t>(v);
	}
    }
    return std::make_pair(timeout, interp.arg(t, 0));
}

std::tuple<term, term, size_t> interpreter::deconstruct_where(interpreter_base &interp, term where) {
    size_t timeout = std::numeric_limits<size_t>::max();
    term else_do = EMPTY_LIST;
    if (is_else(interp, where)) {
	std::tie(else_do, where) = extract_else(interp, where);
	if (is_timeout(interp, where)) {
	    std::tie(timeout, where) = extract_timeout(interp, where);
	}
    } else if (is_timeout(interp, where)) {
	std::tie(timeout, where) = extract_timeout(interp, where);
	if (is_else(interp, where)) {
	    std::tie(else_do, where) = extract_else(interp, where);
	}
    }
    return std::make_tuple(where, else_do, timeout);
}

bool interpreter::operator_at_impl(interpreter_base &interp, size_t arity, term args[], const std::string &name, interp::remote_execute_mode mode) {

#define LL(interp) reinterpret_cast<interpreter &>(interp)
    term query = args[0];
    term where_term;
    term else_do;
    size_t timeout;

    std::tie(where_term, else_do, timeout) = deconstruct_where(interp, args[1]);
    auto where = interp.to_string(where_term);

    remote_execution_proxy proxy(interp,
	 [](interpreter_base &interp, term query, term else_do, const std::string &where, interp::remote_execute_mode mode, size_t timeout)
	     { return LL(interp).execute_at(query, else_do, interp, where, mode, timeout);},
	 [](interpreter_base &interp, term query, term else_do, const std::string &where, interp::remote_execute_mode mode, size_t timeout)
	 { return LL(interp).continue_at(query, else_do, interp, where, mode, timeout); },
	 [](interpreter_base &interp, const std::string &where)
	     { return LL(interp).delete_instance_at(interp, where); }
         );
    proxy.set_mode(mode);
    proxy.set_timeout(timeout);
    return proxy.start(query, else_do, where);
}

bool interpreter::operator_at_2(interpreter_base &interp0, size_t arity, term args[] )
{
    return operator_at_impl(interp0, arity, args, "@", MODE_NORMAL);
}

bool interpreter::operator_at_silent_2(interpreter_base &interp0, size_t arity, term args[] )
{
    return operator_at_impl(interp0, arity, args, "@-", MODE_SILENT);
}

bool interpreter::operator_at_parallel_2(interpreter_base &interp0, size_t arity, term args[] )
{
    return  operator_at_impl(interp0, arity, args, "@=", MODE_PARALLEL);
}

remote_return_t interpreter::execute_at(common::term query,
					common::term else_do,
					common::term_env &query_src,
					const std::string &where,
					remote_execute_mode mode,
					size_t timeout) {
    ensure_local_workers(4);
    ensure_at_local(where);
    
    auto *interp = at_local_[where];

    if (mode == MODE_PARALLEL) {
	auto *d = new delayed_t(*this, query, else_do);
	d->set_timeout_millis(timeout);
	add_delayed(d);
	local_service_.add(
	   [d, query, &query_src, interp, this](){
	       try {
		   term copy_query = interp->copy(query, query_src);
		   bool ok = interp->execute(copy_query);
		   if (ok) {
		       d->result = interp->get_result_term();
		       d->result_src = interp;
		   }
	       } catch (std::exception &) {
		   // Here we could store the exception...
	       }
	       delayed_ready(d);
	   }, where);
	return remote_return_t(query_src.EMPTY_LIST);
    } else {
	term copy_query = interp->copy(query, query_src);
	try {
	    bool ok = interp->execute(copy_query);
	    if (!ok) {
		return remote_return_t();
	    }
	} catch (std::exception &) {
	    return remote_return_t();
	}

	term result_term = interp->get_result_term();
	bool has_more = interp->has_more();
	bool at_end = !interp->has_more() && interp->is_instance();
	uint64_t cost = interp->accumulated_cost();
	uint64_t cost_tmp = 0;
	term result_copy = query_src.copy(result_term, *interp, cost_tmp);
	return remote_return_t(result_copy, has_more, at_end, cost);
    }
}

remote_return_t interpreter::continue_at(common::term query,
					 common::term else_do,
					 common::term_env &query_src,
					 const std::string &where,
					 remote_execute_mode mode,
					 size_t timeout) {
    auto *interp = at_local_[where];
    if (interp == nullptr) {
	return remote_return_t();
    }

    if (mode == MODE_PARALLEL) {
	auto *d = new delayed_t(*this, query, else_do);
	d->set_timeout_millis(timeout);
	add_delayed(d);
	local_service_.add(
	   [d, interp, this](){
	       try {
		   bool ok = interp->next();
		   if (ok) {
		       d->result = interp->get_result_term();
		       d->result_src = interp;
		   }
	       } catch (std::exception &ex) {
		   // Here we could store the exception...
	       }
	       delayed_ready(d);
	   });
	return remote_return_t(query_src.EMPTY_LIST);
    } else {
	auto ok = interp->next();
	if (!ok) {
	    return remote_return_t();
	}
	term result_term = interp->get_result_term();
	bool has_more = interp->has_more();
	bool at_end = !interp->has_more() && interp->is_instance();	
	uint64_t cost = interp->accumulated_cost();
	uint64_t cost_tmp = 0;
	term result_copy = query_src.copy(result_term, *interp, cost_tmp);
	return remote_return_t(result_copy, has_more, at_end, cost);	
    }
}

bool interpreter::delete_instance_at(term_env &query_src, const std::string &where) {
    auto *interp = at_local_[where];
    if (interp == nullptr) {
	return false;
    }
    interp->delete_instance();
    return true;
}
    
}}
