#pragma once

#ifndef _interp_interpreter_base_hpp
#define _interp_interpreter_base_hpp

#include <istream>
#include <vector>
#include <stack>
#include "../common/term_env.hpp"
#include "builtins.hpp"
#include "builtins_opt.hpp"
#include "file_stream.hpp"
#include "arithmetics.hpp"

namespace prologcoin { namespace interp {
// This pair represents functor with first argument. If first argument
// is a STR tag, then we dereference it to a CON cell.
typedef std::pair<common::con_cell, common::cell> functor_index;

class managed_clause {
public:
    inline managed_clause()
	: clause_(), cost_(0) { }
    inline managed_clause(common::term cl, uint64_t cost)
        : clause_(cl), cost_(cost) { }

    inline managed_clause(const managed_clause &other)
	: clause_(other.clause_), cost_(other.cost_) { }

    inline common::term clause() const {
	return clause_;
    }

    inline uint64_t cost() const {
	return cost_;
    }

private:
    common::term clause_;
    size_t cost_;
};

typedef std::vector<managed_clause> managed_clauses;
typedef managed_clauses predicate;
typedef std::pair<predicate, size_t> indexed_predicate;
}}

namespace std {
    template<> struct hash<prologcoin::interp::functor_index> {
	size_t operator()(const prologcoin::interp::functor_index &k) const {
	    return hash<prologcoin::interp::functor_index::first_type>()(k.first) +
	 	   hash<prologcoin::interp::functor_index::second_type>()(k.second);
	}
    };
}

namespace prologcoin { namespace interp {

// Syntax exceptions...

class syntax_exception : public std::runtime_error
{
public:
    syntax_exception(const common::term &t, const std::string &msg)
	: std::runtime_error(msg), term_(t) { }
    ~syntax_exception() noexcept(true) { }

    const common::term & get_term() const { return term_; }

private:
    common::term term_;
};

class syntax_exception_program_not_a_list : public syntax_exception
{
public:
    syntax_exception_program_not_a_list(const common::term &t)
	: syntax_exception(t, "Program is not a list") { }
};

class syntax_exception_not_a_clause : public syntax_exception
{
public:
    syntax_exception_not_a_clause(const common::term &t)
	: syntax_exception(t, "Not a clause") { }
};

class syntax_exception_clause_bad_head : public syntax_exception
{
public:
    syntax_exception_clause_bad_head(const common::term &t, const std::string &msg)
	: syntax_exception(t, msg) { }

};

class syntax_exception_bad_goal : public syntax_exception
{
public:
    syntax_exception_bad_goal(const common::term &t, const std::string &msg)
	: syntax_exception(t, msg) { }
};

// Interpreter exceptions...

class interpreter_exception : public std::runtime_error
{
public:
    interpreter_exception(const std::string &msg)
	: std::runtime_error(msg) { }
};

class interpreter_exception_stack_overflow : public interpreter_exception
{
public:
    interpreter_exception_stack_overflow(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_undefined_predicate : public interpreter_exception
{
public:
    interpreter_exception_undefined_predicate(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_wrong_arg_type : public interpreter_exception
{
public:
      interpreter_exception_wrong_arg_type(const std::string &msg)
	  : interpreter_exception(msg) { }
};

class interpreter_exception_file_not_found : public interpreter_exception
{
public:
      interpreter_exception_file_not_found(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_nothing_told : public interpreter_exception
{
public:
    interpreter_exception_nothing_told(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_argument_not_number : public interpreter_exception
{
public:
    interpreter_exception_argument_not_number(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_not_sufficiently_instantiated : public interpreter_exception
{
public:
    interpreter_exception_not_sufficiently_instantiated(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_not_list : public interpreter_exception
{
public:
    interpreter_exception_not_list(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_unsupported : public interpreter_exception
{
public:
    interpreter_exception_unsupported(const std::string &msg)
	: interpreter_exception(msg) { }
};

class interpreter_exception_undefined_function : public interpreter_exception
{
public:
    interpreter_exception_undefined_function(const std::string &msg)
	: interpreter_exception(msg) { }
};

class wam_instruction_base;

// Contemplated this be a union, but it's better to ensure
// portability. And it might be good to have access to both
// the instruction pointer as well as the label.
class code_point {
public:
    inline code_point() : wam_code_(nullptr), term_code_(common::ref_cell(0)){}
    inline code_point(const common::term t) : wam_code_(nullptr),term_code_(t){}
    inline code_point(const common::con_cell l) : wam_code_(nullptr), term_code_(l){}
    inline code_point(const common::int_cell i) : wam_code_(nullptr), term_code_(i){}
    inline code_point(const code_point &other)
        : wam_code_(other.wam_code_), term_code_(other.term_code_) { }
    inline code_point(wam_instruction_base *i)
        : wam_code_(i), term_code_(common::ref_cell(0)) { }
    inline code_point(const common::term t, builtin_fn f)
        : bn_(f), term_code_(t) { }

    inline static code_point fail() {
        return code_point();
    }
    inline void reset() { wam_code_ = nullptr; term_code_ = fail_term_; }

    inline bool is_fail() const { return term_code_ == fail_term_; }

    inline bool has_wam_code() const { return wam_code_ != nullptr; }

    inline wam_instruction_base * wam_code() const { return wam_code_; }
    inline builtin_fn bn() { return bn_; }
    inline const common::cell & term_code() const { return term_code_; }
    inline const common::int_cell & label() const { return static_cast<const common::int_cell &>(term_code_); }

    inline void set_wam_code(wam_instruction_base *p) { wam_code_ = p; }
    inline void set_term_code(const common::term t) { term_code_ = t; }

    std::string to_string(interpreter_base &interp) const;

private:
    static const common::cell fail_term_;

    union {
        wam_instruction_base *wam_code_;
        builtin_fn bn_;
    };
    common::cell term_code_;
};

// Saved wrapped environment
struct environment_base_t;
struct environment_saved_t {
    uint64_t saved;

    environment_saved_t(environment_base_t *e, bool is_wam)
    {
        saved = reinterpret_cast<uint64_t>(e) + (is_wam ? 1 : 0);
    }

    environment_base_t * ce0()
    {
        return reinterpret_cast<environment_base_t *>(saved & ~0x1);
    }

    bool is_wam()
    {
        return saved & 0x1;
    }

    std::pair<environment_base_t *, bool> ce()
    {
        return std::make_pair(ce0(), is_wam());
    }
};

// Base data type for stack frames
struct environment_base_t {
    environment_saved_t   ce; // Continuation environment
    code_point            cp; // Continuation point
};

// Data type for stack frames with Y variables (for WAM)
struct environment_t : public environment_base_t {
    common::term          yn[]; // Y variables
};

struct choice_point_t {
    environment_saved_t   ce;
    code_point            cp;
    choice_point_t       *b;
    code_point            bp;
    size_t                tr;
    size_t                h; 
    choice_point_t       *b0;
    common::term          qr; // Only used for naive interpreter (for now)
    common::con_cell      pr; // Only used for naive interpreter (for now)
    size_t                arity;
    common::term          ai[];
};

// Standard environment (for naive interpreter)
struct environment_ext_t : public environment_base_t {
    choice_point_t       *b0;
    common::term          qr;
    common::con_cell      pr;
};

typedef union {
    environment_t *e;
    environment_ext_t *ee;
    choice_point_t *cp;
    common::term term;
} word_t;

//
// The purpose of the meta context is to store environments &
// choice points upon recursive invocation of the interpreter.
//
struct meta_context {
    choice_point_t *old_top_b;
    choice_point_t *old_b;
    environment_base_t *old_top_e;
    environment_base_t *old_e;
    code_point old_p;
    code_point old_cp;
    common::term old_qr;
    size_t old_hb;
};

typedef std::function<bool (interpreter_base &, meta_context *)> meta_fn;
typedef std::pair<meta_context *, meta_fn> meta_entry;

class interpreter_base : public common::term_env {
    friend class builtins;
    friend class builtins_opt;
    friend class builtins_fileio;
    friend class arithmetics;

public:
    typedef common::term term;
    typedef common::cell cell;
    typedef common::con_cell con_cell;
    typedef common::int_cell int_cell;

    interpreter_base();
    ~interpreter_base();

    inline bool is_debug() const { return debug_; }
    inline void set_debug(bool dbg) { debug_ = dbg; arith_.set_debug(dbg); }
    inline void debug_check() { debug_check_fn_(); }
    inline void set_debug_check_fn(std::function<void ()> fn)
        { debug_check_fn_ = fn; }
    void set_debug_enabled();

    bool is_track_cost() const { return track_cost_; }
    void set_track_cost(bool b) { track_cost_ = b; }

    void enable_file_io();
    const std::string & get_current_directory() const;
    void set_current_directory(const std::string &dir);
    std::string get_full_path(const std::string &path) const;
    void close_all_files();
    bool is_file_id(size_t id) const;
    void reset_files();

    inline arithmetics & arith() { return arith_; }

    void load_clause(const std::string &str);
    void load_clause(std::istream &is);
    void load_clause(const term t);

    term clause_head(const term clause);
    term clause_body(const term clause);
    common::con_cell clause_predicate(const term clause);

    void load_program(const std::string &str);
    void load_program(std::istream &is);
    void load_program(const term clauses);

    inline const predicate & get_predicate(const common::con_cell pn)
        { return program_db_[pn]; }

    inline const std::vector<common::con_cell> get_predicates() const
        { return program_predicates_; }

    std::string to_string_cp(const code_point &cp)
        { return cp.to_string(*this); }

    void print_db() const;
    void print_db(std::ostream &out) const;
    void print_profile() const;
    void print_profile(std::ostream &out) const;

    class list_iterator : public common::term_iterator {
    public:
	list_iterator(Env &env, const common::term t)
            : common::term_iterator(env, t) { }

	list_iterator begin() {
	    return *this;
	}

	list_iterator end() {
	    return list_iterator(env(), env().empty_list());
	}
    };

    inline builtin & get_builtin(con_cell f)
    {
	static builtin empty_bn_;

        auto it = builtins_.find(f);
        if (it == builtins_.end()) {
	    return empty_bn_;
        } else {
	    return it->second;
	}
    }

    inline builtin_opt get_builtin_opt(con_cell f)
    {
        auto it = builtins_opt_.find(f);
        if (it == builtins_opt_.end()) {
	    return nullptr;
        } else {
	    return it->second;
	}
    }

    inline bool is_builtin(con_cell f) const
        { return builtins_.find(f) != builtins_.end(); }

    inline uint64_t accumulated_cost() const
        { return accumulated_cost_; }

    inline bool unify(term a, term b)
       { uint64_t cost = 0;
	 bool ok = common::term_env::unify(a, b, cost);
	 add_accumulated_cost(cost);
	 return ok;
       }

    inline int standard_order(const term a, const term b)
       { uint64_t cost = 0;
	int cmp = common::term_env::standard_order(a, b, cost);
	add_accumulated_cost(cost);
	return cmp;
       }

    inline term copy(term t, term_env &src)
       { uint64_t cost = 0;
         term c = common::term_env::copy(t, src, cost);
	 add_accumulated_cost(cost);
	 return c;
       }

    inline term copy(term t)
       { uint64_t cost = 0;
         term c = common::term_env::copy(t, cost);
	 add_accumulated_cost(cost);
	 return c;
       }

protected:
    inline con_cell empty_list() const
    {
        return empty_list_;
    }

    template<typename T> inline size_t words() const
    { return sizeof(T)/sizeof(word_t); }

    template<typename T> inline word_t * base(T *t) const
    { return reinterpret_cast<word_t *>(t); }

    inline bool is_stack(common::ref_cell ref)
    {
        return ref.index() >= STACK_BASE;
    }

    inline bool is_stack(size_t ref)
    {
	return ref >= STACK_BASE;
    }

    inline word_t * to_stack(common::ref_cell ref)
    {
        return &stack_[ref.index() - STACK_BASE];
    }

    inline word_t * to_stack(size_t ref)
    {
	return &stack_[ref - STACK_BASE];
    }

    inline size_t to_stack_addr(word_t *p)
    {
        return static_cast<size_t>(p - stack_) + STACK_BASE;
    }
    inline size_t to_stack_relative_addr(word_t *p)
    {
        return static_cast<size_t>(p - stack_);
    }

    typedef size_t (*num_y_fn_t)(interpreter_base *interp, environment_base_t *);

    inline num_y_fn_t num_y_fn()
    {
        return num_y_fn_;
    }

    inline void set_num_y_fn( num_y_fn_t num_y_fn)
    {
        num_y_fn_ = num_y_fn;
    }

    inline term & a(size_t i)
    {
        return register_ai_[i];    
    }

    inline term & y(size_t i)
    {
        return e()->yn[i];
    }

    inline term * args()
    {
        return &register_ai_[0];
    }

    inline size_t num_of_args()
    {
        return num_of_args_;
    }

    inline void set_num_of_args(size_t n)
    {
        num_of_args_ = n;
    }

    static inline size_t num_y(interpreter_base *, environment_base_t *)
    {
        return (sizeof(environment_ext_t)-sizeof(environment_base_t))/sizeof(term);
    }

    inline code_point & p()
    {
        return register_p_;
    }

    inline void set_p(const code_point &p1)
    {
        register_p_ = p1;
    }

    inline void set_cp(const code_point &cp)
    {
        register_cp_ = cp;
    }

    inline code_point & cp()
    {
	return register_cp_;
    }

    inline bool has_late_choice_point() 
    {
        return b() > b0();
    }

    inline void cut_direct()
    {
	set_b(b0());
	if (b() != nullptr) set_register_hb(b()->h);
	tidy_trail();
    }
    inline void cut()
    {
	if (b() > b0()) {
	    cut_direct();
	}
    }

    void unwind_to_top_choice_point();

    inline void set_top_e()
    {
        register_top_e_ = register_e_;
    }

    inline void set_top_e(environment_base_t *e)
    {
        register_top_e_ = e;
    }

    inline environment_base_t * e0()
    {
        return register_e_;
    }

    inline environment_t * e()
    {
        return reinterpret_cast<environment_t *>(e0());
    }

    inline environment_ext_t * ee()
    {
        return reinterpret_cast<environment_ext_t *>(e0());
    }

    inline bool e_is_wam()
    {
        return register_e_is_wam_;
    }

    inline environment_saved_t save_e()
    {
        return environment_saved_t(e0(), e_is_wam());
    }

    inline void set_e(environment_saved_t saved)
    {
        std::tie(register_e_, register_e_is_wam_) = saved.ce();
    }

    inline void set_e(environment_base_t *e)
    {
        register_e_ = e;
	register_e_is_wam_ = true;
    }

    inline void set_ee(environment_ext_t *ee)
    {
        register_e_ = ee;
	register_e_is_wam_ = false;
    }

    inline environment_base_t * top_e() const
    {
        return register_top_e_;
    }

    inline choice_point_t * b() const
    {
        return register_b_;
    }

    // Choice point is WAM based if continuation environment is WAM based.
    inline bool b_is_wam() const
    {
	if (b() == nullptr) {
	    return false;
	} else {
	    return b()->bp.has_wam_code();
	}
    }

    inline void set_b(choice_point_t *b)
    {
        register_b_ = b;
    }

    inline choice_point_t * b0() const
    {
        return register_b0_;
    }

    inline void set_b0(choice_point_t *b)
    {
        register_b0_ = b;
    }

    inline choice_point_t * top_b() const
    {
        return register_top_b_;
    }

    inline void set_top_b(choice_point_t *b)
    {
        register_top_b_ = b;
    }

    inline void set_top_fail(bool b)
    {
        top_fail_ = b;
	set_complete(true);
    }
  
    inline bool is_top_fail() const
    {
        return top_fail_;
    }

    inline void set_complete(bool b)
    {
        complete_ = b;
    }

    inline bool is_complete() const
    {
        return complete_;
    }

    inline environment_base_t * allocate_environment(bool for_wam)
    {
        word_t *new_e0;
	if (base(e0()) > base(b())) {
	    auto n = words<term>()*(num_y_fn()(this, e0())) + words<environment_base_t>();
	    new_e0 = base(e0()) + n;
	} else {
	    if (b() == nullptr) {
	        new_e0 = stack_;
  	    } else {
	        new_e0 = base(b()) + words<term>()*b()->arity + words<choice_point_t>();
	    }
	}

	if (to_stack_relative_addr(new_e0) + MAX_STACK_FRAME_WORDS
	    >= MAX_STACK_SIZE_WORDS) {
	    throw interpreter_exception_stack_overflow("Exceeded maximum stack size (" + boost::lexical_cast<std::string>(MAX_STACK_SIZE) + " bytes.)");
	}

	if (for_wam) {
	    environment_t *new_e = reinterpret_cast<environment_t *>(new_e0);
	    new_e->ce = save_e();
	    new_e->cp = cp();
	    set_e(new_e);
	} else {
	    environment_ext_t *new_ee = reinterpret_cast<environment_ext_t *>(new_e0);
	    new_ee->ce = save_e();
	    new_ee->cp = cp();
	    new_ee->b0 = b0();
	    new_ee->qr = register_qr_;
	    new_ee->pr = register_pr_;

	    set_ee(new_ee);
	}
	// std::cout << "allocate_environment: e=" << new_e0 << " cp=" << to_string_cp(cp()) << "\n";
	// std::cout << "allocate_environment: qr=" << to_string(qr()) << " " << qr().tag() << " " << qr().raw_value() << "\n";

	return e0();
    }

    inline void deallocate_environment()
    {
        // std::cout << "[before] deallocate_environment: e=" << e() << " p=" << to_string_cp(p()) << " cp=" << to_string_cp(cp()) << "\n";
	if (!e_is_wam()) {
	    environment_ext_t *ee1 = ee();
	    set_b0(ee1->b0);
	    set_qr(ee1->qr);
	    set_pr(ee1->pr);
	}
        set_cp(e0()->cp);
        set_e(e0()->ce);
	// std::cout << "[after]  deallocate_environment: e=" << e() << " p=" << to_string_cp(p()) << " cp=" << to_string_cp(cp()) << "\n";
    }

    inline void allocate_choice_point(const code_point &cont)
    {
        word_t *new_b0;
	if (base(e0()) > base(b())) {
	    new_b0 = base(e0()) + num_y_fn()(this, e0()) + words<environment_t>();
	} else {
  	    if (b() == nullptr) {
	        new_b0 = stack_;
	    } else {
  	        new_b0 = base(b()) + b()->arity + words<choice_point_t>();
	    }
	}

	if (to_stack_relative_addr(new_b0) + MAX_STACK_FRAME_WORDS
	            >= MAX_STACK_SIZE_WORDS) {
	    throw interpreter_exception_stack_overflow("Exceeded maximum stack size (" + boost::lexical_cast<std::string>(MAX_STACK_SIZE) + " bytes.)");
	}

	auto *new_b = reinterpret_cast<choice_point_t *>(new_b0);
	new_b->arity = num_of_args_;
	for (size_t i = 0; i < num_of_args_; i++) {
	    new_b->ai[i] = a(i);
	}
	new_b->ce = save_e();
	new_b->cp = register_cp_;
	new_b->b = register_b_;
	new_b->bp = cont;
	new_b->tr = trail_size();
	new_b->h = heap_size();
	new_b->b0 = register_b0_;
	new_b->qr = register_qr_;
	new_b->pr = register_pr_;
	register_b_ = new_b;
	set_register_hb(heap_size());

	// std::cout << cnt << ": allocate_choice_point(): b=" << new_b << " trail_size=" << trail_size() << "\n";

        // std::cout << "allocate_choice_point b=" << new_b << " (prev=" << new_b->b << ")\n";
	// std::cout << "allocate_choice_point saved qr=" << to_string(register_qr_) << "\n";
    }

    inline void deallocate_and_proceed()
    {
	if (e0() != top_e()) {
	    deallocate_environment();
	} else {
	    set_cp(empty_list());
	}
	set_p(cp());
	set_cp(empty_list());
    }
    
    inline void proceed_and_deallocate()
    {
        set_p(cp());
	if (e0() != top_e()) {
	    deallocate_environment();
	    if (is_debug()) {
		std::cout << "interpreter_base::proceed_and_deallocate(): p="
			  << to_string_cp(p()) << " cp="
			  << to_string_cp(cp()) << " e=" << e0() << "\n";
	    }
	} else {
	    set_cp(empty_list());
	    if (is_debug()) {
		std::cout << "interpreter_base::proceed_and_deallocate(): p="
			  << to_string_cp(p()) << " cp="
			  << to_string_cp(cp()) << " e=" << e0() << "\n";
	    }
	}
    }

    void prepare_execution();

    inline term qr() const
        { return register_qr_; }

    inline void set_qr(term qr)
        { register_qr_ = qr; }

    inline void set_pr(common::con_cell pr)
        { register_pr_ = pr; }

    choice_point_t * reset_to_choice_point(choice_point_t *b);

    void unwind(size_t current_tr);

    term get_first_arg();

    void abort(const interpreter_exception &ex);
    bool definitely_inequal(const term a, const term b);

    template<typename T> inline T * new_meta_context(meta_fn fn) {
        T *context = new T();
	context->old_top_b = register_top_b_;
	context->old_b = register_b_;
	context->old_top_e = register_top_e_;
	context->old_e = register_e_;
	context->old_p = register_p_;
	context->old_cp = register_cp_;
	context->old_qr = register_qr_;
	context->old_hb = get_register_hb();
	meta_.push_back(std::make_pair(context, fn));

	if (is_debug()) {
	    std::cout << "interpreter_base::new_meta_context(): ---- e=" << context->old_e << " ----\n";
	}

	return context;
    }

    inline void release_last_meta_context()
    {
        auto *context = meta_.back().first;
        set_top_b(context->old_top_b);
	set_b(context->old_b);
	set_top_e(context->old_top_e);
	set_e(context->old_e);
	set_p(context->old_p);
	set_cp(context->old_cp);
	set_qr(context->old_qr);
	set_register_hb(context->old_hb);
	delete context;
	meta_.pop_back();

	if (is_debug()) {
	    std::cout << "interpreter_base::release_meta_context(): ---- e=" << e() << " ----\n";
	}
    }

    inline meta_context * get_last_meta_context()
        { return meta_.back().first; }

    template<typename T> inline T * get_last_meta_context()
        { return reinterpret_cast<T *>(get_last_meta_context()); }

    inline  meta_fn get_last_meta_function()
        { return meta_.back().second; }

    inline bool has_meta_contexts() const
        { return !meta_.empty(); }

    inline term_env & secondary_env()
        { return secondary_env_; }

    inline void reset_accumulated_cost()
        { accumulated_cost_ = 0; }

    inline void add_accumulated_cost(uint64_t cost)
        { accumulated_cost_ += cost; }

private:
    void load_builtin(con_cell f, builtin b);
    void load_builtin_opt(con_cell f, builtin_opt b);
    void load_builtins();
    void load_builtins_file_io();
    void load_builtins_opt();
    file_stream & new_file_stream(const std::string &path);
    void close_file_stream(size_t id);
    file_stream & get_file_stream(size_t id);
    file_stream & standard_output();
    void tell_standard_output(file_stream &fs);
    void told_standard_output();
    bool has_told_standard_outputs();

    void init();
    void tidy_trail();

    inline choice_point_t * get_last_choice_point()
    {
        return b();
    }

    inline void move_cut_point_to_last_choice_point()
    {
        set_b0(b());
    }

    common::cell first_arg_index(const term first_arg);

    void syntax_check();

    void syntax_check_program(const term term);
    void syntax_check_clause(const term term);
    void syntax_check_head(const term head);
    void syntax_check_body(const term body);
    void syntax_check_goal(const term goal);

    // Useful for meta predicates as scratch area to temporarily
    // copy terms.
    term_env secondary_env_;

    bool debug_;
    bool track_cost_;
    std::vector<std::function<void ()> > syntax_check_stack_;

    std::unordered_map<common::con_cell, builtin> builtins_;
    std::unordered_map<common::con_cell, builtin_opt> builtins_opt_;
    std::unordered_map<common::con_cell, predicate> program_db_;
    std::vector<common::con_cell> program_predicates_;

    // Stack is emulated at heap offset >= 2^59 (3 bits for tag, remember!)
    // (This conforms to the WAM standard where addr(stack) > addr(heap))
    const size_t STACK_BASE = 0x80000000000000;
    const size_t MAX_STACK_SIZE = 1024*1024*1024;
    const size_t MAX_STACK_SIZE_WORDS = MAX_STACK_SIZE / sizeof(word_t);
    const size_t MAX_STACK_FRAME_WORDS = 4096 / sizeof(word_t);

    word_t    *stack_;

    bool top_fail_;
    bool complete_;

    code_point register_p_;
    code_point register_cp_;

    bool register_e_is_wam_; // If the register_e is a WAM (compressed) env.
    environment_base_t *register_e_;  // Points to current environment
    environment_base_t *register_top_e_;

    choice_point_t *register_b_;
    choice_point_t *register_b0_;
    choice_point_t *register_top_b_;

    term register_ai_[256];

    size_t num_of_args_;

    num_y_fn_t num_y_fn_;

    term register_qr_;     // Current query 
    con_cell register_pr_; // Current predicate (for profiling)

    std::vector<meta_entry> meta_;

    con_cell comma_;
    con_cell empty_list_;
    con_cell implied_by_;

    std::string current_dir_; // Current directory

    std::unordered_map<size_t, file_stream *> open_files_;
    size_t file_id_count_;
    file_stream * standard_output_;
    std::stack<file_stream *> standard_output_stack_;

    arithmetics arith_;

    std::unordered_map<common::con_cell, uint64_t> profiling_;

    std::function<void ()> debug_check_fn_;

    // Keep track of accumulated cost while interpreter is executing
    uint64_t accumulated_cost_;

protected:
    size_t tidy_size;
};

}}

#endif
