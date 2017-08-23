#pragma once

#ifndef _interp_interpreter_hpp
#define _interp_interpreter_hpp

#include <istream>
#include <vector>
#include "../common/term_env.hpp"
#include "builtins.hpp"
#include "file_stream.hpp"
#include "arithmetics.hpp"

namespace prologcoin { namespace interp {
// This pair represents functor with first argument. If first argument
// is a STR tag, then we dereference it to a CON cell.
typedef std::pair<common::con_cell, common::cell> functor_index;
typedef std::vector<common::term> clauses;
typedef std::pair<clauses, builtin> executable;
typedef std::pair<executable, size_t> indexed_executable;
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

class interpreter {
    friend class builtins;
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

    void sync_with_heap() { env().sync_with_heap(); }

    void load_clause(const std::string &str);
    void load_clause(std::istream &is);
    void load_clause(const term &t);

    void load_program(const std::string &str);
    void load_program(std::istream &is);
    void load_program(const term &clauses);

    void print_db() const;
    void print_db(std::ostream &out) const;

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
	list_iterator(common::term_env &env, const common::term &t)
            : term_iterator(env, t) { }

	list_iterator begin() {
	    return *this;
	}

	list_iterator end() {
	    return list_iterator(env(), env().empty_list());
	}
    };

private:
    void load_builtin(con_cell f, builtin b);
    void load_builtins();
    void load_builtins_file_io();
    file_stream & new_file_stream(const std::string &path);
    void close_file_stream(size_t id);
    file_stream & get_file_stream(size_t id);

    void init();
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

    struct environment_t {
	int_cell ce; // Continuation environment
        int_cell b0; // Choice point when encountering a cut operation.
	cell cp;     // Continuation point
        cell qr;     // Current query (good for debugging/tracing)
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
    };

    static const int choice_point_num_cells = sizeof(choice_point_t) / sizeof(cell);

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

    void tidy_trail();

    environment_t * get_environment(size_t at_index);
    void allocate_environment();
    void deallocate_environment();

    inline choice_point_t * get_last_choice_point()
    {
	return get_choice_point(register_b_);
    }

    inline void move_cut_point_to_last_choice_point()
    {
	register_b0_ = register_b_;
    }

    choice_point_t * get_choice_point(size_t at_index);
    choice_point_t * allocate_choice_point(size_t index_id);
    void cut_last_choice_point();

    bool definitely_inequal(const term &a, const term &b);
    common::cell first_arg_index(const term &first_arg);
    term get_first_arg(const term &t);

    void compute_matched_executable(con_cell functor, const term &first_arg, executable &matched);
    size_t matched_executable_id(con_cell functor, const term &first_arg);

    inline executable & get_executable(size_t id)
    {
	return id_to_executable_[id];
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

    std::unordered_map<common::con_cell, executable> program_db_;
    std::vector<common::con_cell> program_predicates_;
    std::unordered_map<functor_index, size_t> executable_id_;
    std::vector<executable> id_to_executable_;

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

    con_cell comma_;
    con_cell empty_list_;
    con_cell implied_by_;

    std::string current_dir_; // Current directory

    std::unordered_map<size_t, file_stream *> open_files_;
    size_t file_id_count_;

    arithmetics arith_;
};

}}

#endif
