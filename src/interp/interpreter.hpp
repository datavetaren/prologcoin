#pragma once

#ifndef _interp_interpreter_hpp
#define _interp_interpreter_hpp

#include <istream>
#include <vector>
#include "../common/term_env.hpp"
#include "builtins.hpp"
#include "builtins_opt.hpp"
#include "file_stream.hpp"
#include "arithmetics.hpp"

namespace prologcoin { namespace interp {
// This pair represents functor with first argument. If first argument
// is a STR tag, then we dereference it to a CON cell.
typedef std::pair<common::con_cell, common::cell> functor_index;
typedef std::vector<common::term> clauses;
typedef clauses predicate;
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

struct meta_context {
    size_t old_top_b;
    size_t old_top_e;
};

typedef std::function<void (interpreter &, meta_context *)> meta_fn;
typedef std::pair<meta_context *, meta_fn> meta_entry;

class interpreter {
    friend class builtins;
    friend class builtins_opt;
    friend class builtins_fileio;
    friend class arithmetics;

public:
    typedef common::term term;
    typedef common::cell cell;
    typedef common::con_cell con_cell;
    typedef common::int_cell int_cell;

    interpreter();
    interpreter(common::term_env &env);
    ~interpreter();

    inline bool is_debug() const { return debug_; }
    inline void set_debug(bool dbg) { debug_ = dbg; arith_.set_debug(dbg); }

    void enable_file_io();
    void set_current_directory(const std::string &dir);
    std::string get_full_path(const std::string &path) const;
    void close_all_files();
    bool is_file_id(size_t id) const;

    inline common::term_env & env() { return *term_env_; }
    inline arithmetics & arith() { return arith_; }

    void load_clause(const std::string &str);
    void load_clause(std::istream &is);
    void load_clause(const term &t);

    void load_program(const std::string &str);
    void load_program(std::istream &is);
    void load_program(const term &clauses);

    void print_db() const;
    void print_db(std::ostream &out) const;
    void print_profile() const;
    void print_profile(std::ostream &out) const;

    bool execute(const term &query);

    bool next();

    class binding {
    public:
	binding() { }
	binding(const std::string &name, const term &value) 
	    : name_(name), value_(value) { }

	inline const std::string & name() const { return name_; }
	inline const term & value() const { return value_; }

    private:
	std::string name_;
	term value_;
    };

    inline const std::vector<binding> & query_vars() const
        { return query_vars_; }

    std::string get_result(bool newlines = true) const;
    void print_result(std::ostream &out) const;

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

private:
    void load_builtin(con_cell f, builtin b);
    void load_builtin_opt(con_cell f, builtin_opt b);
    void load_builtins();
    void load_builtins_file_io();
    void load_builtins_opt();
    file_stream & new_file_stream(const std::string &path);
    void close_file_stream(size_t id);
    file_stream & get_file_stream(size_t id);

    void init();
    bool unify(term &a, term &b);
    term copy(term &c);
    term new_dotted_pair(const term a, const term b);
    term new_term(con_cell functor);
    term new_term(con_cell functor, const std::initializer_list<term> &args);
    void prepare_execution();
    void abort(const interpreter_exception &ex);
    void fail();
    bool select_clause(term &instruction,
		       size_t index_id, std::vector<term> &clauses,
		       size_t from_clause);

    inline term & current_query()
    {
	return register_qr_;
    }

    inline void set_current_query(term &qr) 
    {
        register_qr_ = qr;
    }

    struct environment_t {
	int_cell ce; // Continuation environment
        int_cell b0; // Choice point when encountering a cut operation.
	cell cp;     // Continuation point
        cell qr;     // Current query (good for debugging/tracing)
	con_cell pr; // Current predicate
    };

    static const int environment_num_cells = sizeof(environment_t) / sizeof(cell);

    struct choice_point_t {
	int_cell ce; // Continuation environment
	cell cp;     // Continuation point (term that represents code)
	int_cell b;  // Previous choice point
	int_cell bp; // Encoded as a pair (clause index set id + clause no.)
	int_cell tr; // Trail pointer
	int_cell h;  // Heap pointer
	int_cell b0; // Cut pointer
        cell qr;     // Current query
	con_cell pr; // Current predicate
    };

    static const int choice_point_num_cells = sizeof(choice_point_t) / sizeof(cell);

    template<typename T> inline T * new_meta_context(meta_fn fn) {
        T *context = new T();
	context->old_top_b = register_top_b_;
	context->old_top_e = register_top_e_;
	meta_.push_back(std::make_pair(context, fn));
	return context;
    }

    void release_last_meta_context()
    {
        auto *context = meta_.back().first;
        register_top_b_ = context->old_top_b;
	register_top_e_ = context->old_top_e;
	delete context;
	meta_.pop_back();
    }

    inline void set_trail_pointer(size_t tr)
    {
        register_tr_ = tr;
    }

    inline size_t get_trail_pointer() const
    {
	return register_tr_;
    }

    inline void set_heap_pointer(size_t h)
    {
        register_h_ = h;
    }

    inline void sync_env()
    {
	register_h_ = env().heap_size();
    }

    inline void set_continuation_point(const term &cont)
    {
        register_cp_ = cont;
    }

    inline term & get_continuation_point()
    {
	return register_cp_;
    }

    inline bool has_late_choice_point() 
    {
        return register_b_ > register_b0_;
    }

    inline void reset_choice_point()
    {
        register_b_ = register_b0_;
    }

    void unwind_to_top_choice_point();

    inline void set_top_e()
    {
        register_top_e_ = register_e_;
    }

    inline size_t get_register_e()
    {
        return register_e_;
    }

    inline size_t get_top_e()
    {
        return register_top_e_;
    }

    inline size_t get_register_b()
    {
        return register_b_;
    }

    inline size_t get_top_b()
    {
        return register_top_b_;
    }

    inline void set_top_b()
    {
        register_top_b_ = register_b_;
    }

    inline void set_top_fail(bool b)
    {
        top_fail_ = b;
    }
  
    inline bool is_top_fail() const
    {
        return top_fail_;
    }

    void tidy_trail();

    environment_t * get_environment(size_t at_index);
    void allocate_environment();
    void deallocate_environment();

    choice_point_t * get_choice_point(size_t at_index);
    choice_point_t * allocate_choice_point(size_t index_id);
    void cut_last_choice_point();
    choice_point_t * reset_to_choice_point(size_t b);
    void unwind(size_t current_tr);

    inline choice_point_t * get_last_choice_point()
    {
	return get_choice_point(register_b_);
    }

    inline void move_cut_point_to_last_choice_point()
    {
	register_b0_ = register_b_;
    }

    bool definitely_inequal(const term &a, const term &b);
    common::cell first_arg_index(const term &first_arg);
    term get_first_arg(const term &t);

    void compute_matched_predicate(con_cell functor, const term &first_arg,
				   predicate &matched);
    size_t matched_predicate_id(con_cell functor, const term &first_arg);

    inline predicate & get_predicate(size_t id)
    {
	return id_to_predicate_[id];
    }

    void execute_once();
    bool cont();
    void dispatch(term &instruction);

    void syntax_check();

    void syntax_check_program(const term &term);
    void syntax_check_clause(const term &term);
    void syntax_check_head(const term &head);
    void syntax_check_body(const term &body);
    void syntax_check_goal(const term &goal);

    term clause_head(const term &clause);
    term clause_body(const term &clause);

    bool owns_term_env_;
    common::term_env *term_env_;
    bool debug_;

    std::vector<std::function<void ()> > syntax_check_stack_;

    std::unordered_map<common::con_cell, builtin> builtins_;
    std::unordered_map<common::con_cell, builtin_opt> builtins_opt_;
    std::unordered_map<common::con_cell, predicate> program_db_;
    std::vector<common::con_cell> program_predicates_;
    std::unordered_map<functor_index, size_t> predicate_id_;
    std::vector<predicate> id_to_predicate_;

    std::vector<binding> query_vars_;

    size_t stack_start_;

    bool top_fail_;

    term register_cp_;   // Continuation point
    size_t register_b_;  // Points to current choice point (0 means none)
    size_t register_e_;  // Points to current environment
    size_t register_tr_; // Trail pointer
    size_t register_h_;  // Heap pointer
    size_t register_hb_; // Heap pointer for current choice point
    size_t register_b0_; // Record choice point (for neck cuts)
    term register_qr_;   // Current query 
    con_cell register_pr_; // Current predicate (for profiling)

    con_cell comma_;
    con_cell empty_list_;
    con_cell implied_by_;

    std::string current_dir_; // Current directory

    std::unordered_map<size_t, file_stream *> open_files_;
    size_t file_id_count_;

    arithmetics arith_;

    size_t register_top_b_; // Fail if this register_b_ reaches this value.
    size_t register_top_e_; // Check meta if 'e' reaches this value.
    std::vector<meta_entry> meta_;

    std::unordered_map<common::con_cell, uint64_t> profiling_;
};

}}

#endif
