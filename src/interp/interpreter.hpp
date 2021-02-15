#pragma once

#ifndef _interp_interpreter_hpp
#define _interp_interpreter_hpp

#include <iostream>
#include "wam_interpreter.hpp"
#include "../common/local_service.hpp"

namespace prologcoin { namespace interp {

class test_wam_compiler;
class test_wam_interpreter;

class interpreter;

struct no_processing {
    no_processing(interpreter &interp) { }
    void operator() (common::term clause) { }
};
    
template<typename Pre = no_processing, typename Post = no_processing>
struct standard_clause_processing {
    standard_clause_processing(interpreter &interp) : interp_(interp), pre_(interp), post_(interp) { }
    void operator () (common::term clause);
private:
    interpreter &interp_;
    Pre pre_;
    Post post_;
};

class remote_return_t {
public:
    remote_return_t() : result_(), has_more_(false), at_end_(false) { }
    remote_return_t(common::term r) : result_(r), has_more_(false), at_end_(false) { }
    remote_return_t(common::term r, bool has_more, bool at_end, uint64_t cost) : result_(r), has_more_(has_more), at_end_(at_end), cost_(cost) { }
    remote_return_t(const remote_return_t &other) = default;

    common::term result() const { return result_; }
    bool failed() const { return result_ == common::term(); }
    bool has_more() const { return has_more_; }
    bool at_end() const { return at_end_; }
    uint64_t get_cost() const { return cost_; }

private:
    common::term result_;
    bool has_more_;
    bool at_end_;
    uint64_t cost_;
};

//
// This is a generic meta context for remote execution.
// It basically captures the query and the destination where it should
// be executed (represented as a string). This string can represent anything
// (which is up to the actual builtin to implement. For example, the
//  string could be a name of a connection, or an IP address. As we'll have
// multiple sub-interpreters implementing remote execution is a slightly
// different way (e.g. the node vs the wallet) we'll at least provide the
// common framework for it here.
//
enum remote_execute_mode { MODE_NORMAL, MODE_SILENT, MODE_PARALLEL };
typedef std::function<remote_return_t (interpreter_base &, common::term, common::term, const std::string &, remote_execute_mode, size_t)> remote_execute_fn_t;
typedef std::function<remote_return_t (interpreter_base &, common::term, common::term, const std::string &, remote_execute_mode, size_t)> remote_continue_fn_t;
typedef std::function<bool (interpreter_base &, const std::string &)> remote_delete_fn_t;
  
class meta_context_remote : public meta_context {
public:
    inline meta_context_remote(interpreter_base &interp, meta_fn fn,
			       common::term query, common::term else_do,
			       const std::string &where,
			       remote_continue_fn_t remote_cont,
			       remote_delete_fn_t remote_del)
	: meta_context(interp, fn), interp_(interp), mode_(MODE_NORMAL),
	  timeout_(std::numeric_limits<size_t>::max()),
	  query_(query), else_do_(else_do), where_(where),
	  remote_continue_(remote_cont), remote_delete_(remote_del) { }

    const std::string & where() const {
        return where_;
    }
    common::term query() const {
        return query_;
    }
    remote_return_t do_remote_continue() {
        return remote_continue_(interp_, query_, else_do_, where_, mode_, timeout_);
    }
    bool do_remote_delete() {
        return remote_delete_(interp_, where_);
    }

    remote_execute_mode mode() const {
	return mode_;
    }
    void set_mode(remote_execute_mode m) {
	mode_ = m;
    }

    size_t timeout() const {
	return timeout_;
    }

    void set_timeout(size_t t) {
	timeout_ = t;
    }
    
private:
    interpreter_base &interp_;
    remote_execute_mode mode_;
    size_t timeout_;
    common::term query_;
    common::term else_do_;
    std::string where_;
    remote_continue_fn_t remote_continue_;
    remote_delete_fn_t remote_delete_;
};

class interpreter_exception_unknown : public interpreter_exception {
public:
    interpreter_exception_unknown(const std::string &msg) :
	interpreter_exception(msg) { }
};

class interpreter_exception_remote : public interpreter_exception {
public:
    interpreter_exception_remote(const std::string &msg)
      : interpreter_exception(msg) { }
};

class remote_execution_proxy {
public:
    remote_execution_proxy(interpreter_base &interp,
			   remote_execute_fn_t remote_execute_fn,
			   remote_continue_fn_t remote_continue_fn,
			   remote_delete_fn_t remote_delete_fn)
      : interp_(interp),
	mode_(MODE_NORMAL),
	timeout_( std::numeric_limits<size_t>::max() ),
	remote_execute_(remote_execute_fn),
        remote_continue_(remote_continue_fn),
        remote_delete_(remote_delete_fn) { }

    static bool callback(interpreter_base& interp,
			 const interp::meta_reason_t &reason) {

        auto *mc = interp.get_current_meta_context<meta_context_remote>();

	interp.set_p(code_point(interp.EMPTY_LIST));
	interp.set_cp(code_point(interp.EMPTY_LIST));

	if (reason == interp::meta_reason_t::META_DELETE) {
	    interp.release_last_meta_context();
	    return true;
	}

	bool failed = interp.is_top_fail();
	if (!failed) {
  	    // If we did not fail, then unwind the environment and
	    // the current meta context. (The current meta context will be
	    // restored if we backtrack to a choice point.)
	    interp.set_top_fail(false);
	    interp.set_complete(false);
	    interp.set_top_e(mc->old_top_e);
	    interp.set_m(mc->old_m);
	    return true;
	}

	interp.set_top_fail(false);
	interp.set_complete(false);

	// interp.unwind_to_top_choice_point();
	// Query remote machine for next solution (this should be implemented
	// by the builtin)
	auto r = mc->do_remote_continue();
	if (r.failed()) {
	    interp.release_last_meta_context();
	    if (r.at_end()) {
	        bool ok = mc->do_remote_delete();
		if (!ok) {
		    throw interpreter_exception_remote(
			       "Failed to delete remote instance at "
			       + mc->where() + " for query "
			       + interp.to_string(mc->query()));
		}
	    }
	    return false;
	}
	common::term qr = mc->query();

	if (!r.has_more()) {
	    interp.release_last_meta_context();
	    if (r.at_end()) {
	        bool ok = mc->do_remote_delete();
		if (!ok) {
		    throw interpreter_exception_remote(
			       "Failed to delete remote instance at "
			       + mc->where() + " for query "
			       + interp.to_string(mc->query()));
		}
	    }
	    return false;
	} else {
	    interp.allocate_choice_point(code_point::fail());
	    interp.set_p(code_point(interp.EMPTY_LIST));
	}

	auto context = interp.get_current_meta_context<meta_context_remote>();
	if (context->mode() != MODE_NORMAL) {
	    return true;
	} else {
	    
	    return interp.unify(qr, r.result());
	}
    }
			 
    bool start(common::term query, common::term else_do, const std::string where) {
        auto result = remote_execute_(interp_, query, else_do, where, mode_, timeout_);

	if (result.failed()) {
	    if (else_do != interp_.EMPTY_LIST) {
		interp_.allocate_environment<ENV_FROZEN, environment_frozen_t *>();
		interp_.allocate_environment<ENV_NAIVE, environment_naive_t *>();
		interp_.set_cp(code_point(interp_.EMPTY_LIST));
		interp_.set_p(code_point(else_do));
		interp_.set_qr(else_do);
		return true;
	    } else {
		return false;
	    }
	}

	if (result.has_more()) {
	    auto context = interp_.new_meta_context<meta_context_remote>(&callback, query, else_do, where, remote_continue_, remote_delete_);
	    context->set_mode(mode_);
	    interp_.set_top_b(interp_.b());
	    interp_.allocate_choice_point(code_point::fail());
	}
	if (mode_ != MODE_NORMAL) {
	    return true;
	} else {
	    return interp_.unify(result.result(), query);
	}
    }

    bool mode() const {
	return mode_;
    }
    
    void set_mode(remote_execute_mode m) {
	mode_ = m;
    }

    size_t timeout() const {
	return timeout_;
    }

    void set_timeout(size_t t) {
	timeout_ = t;
    }
    
private:
    interpreter_base &interp_;
    remote_execute_mode mode_;
    size_t timeout_;
    remote_execute_fn_t remote_execute_;
    remote_continue_fn_t remote_continue_;
    remote_delete_fn_t remote_delete_;
};
    
class interpreter : public wam_interpreter
{
public:
    interpreter(const std::string &name);
    ~interpreter();

    void total_reset();

    void setup_standard_lib();

    template<typename Pre = no_processing, typename Post = no_processing> void load_program(const std::string &str) {
        standard_clause_processing<Pre,Post> process(*this);
	interpreter_base::load_program(str, process);
    }

    template<typename Pre = no_processing, typename Post = no_processing> void load_program(std::istream &is) {
        standard_clause_processing<Pre,Post> process(*this);
	interpreter_base::load_program(is, process);
    }

    template<typename Pre = no_processing, typename Post = no_processing> void load_program(term clauses) {
        standard_clause_processing<Pre,Post> process(*this);
	interpreter_base::load_program(clauses, process);
    }

    template<typename Pre = no_processing, typename Post = no_processing> void load_program(term clauses, con_cell &primary_module) {
        standard_clause_processing<Pre,Post> process(*this);
	interpreter_base::load_program(clauses, process, primary_module);
    }
  
    void new_instance();
    size_t num_instances() const { return num_instances_; }
    void delete_instance();

    bool execute(const term query);
    bool next();
    bool cont();
    void fail();

    inline common::term query() const { return qr(); }

    class binding {
    public:
	binding() { }
	binding(const std::string &name, const common::term value) 
	    : name_(name), value_(value) { }
	binding(const binding &other)
            : name_(other.name_), value_(other.value_) { }

	inline const std::string & name() const { return name_; }
	inline const common::term value() const { return value_; }

    private:
	std::string name_;
	common::term value_;
    };

    inline const std::vector<binding> & query_vars() const
    { return *query_vars_; }

    common::term query_var_list();

    inline bool has_more() const
    { if (top_b() != b()) return true;
      if (!has_meta_context()) {
	  return false;
      }
      return get_current_meta_context()->fn != interpreter::new_instance_meta;
    }

    inline void set_wam_enabled(bool enabled)
    { wam_enabled_ = enabled; }

    inline bool is_wam_enabled() const
    { return wam_enabled_; }

    std::string get_result(bool newlines = true) const;
    term get_result_term(const std::string &varname) const;
    term get_result_term() const;
    void print_result(std::ostream &out) const;

    void unwind(size_t trail_mark)
    {
	interpreter_base::unwind(trail_mark);
    }

    bool is_instance() const;

    void add_local_workers(size_t cnt) {
	local_service_.add_workers(cnt);
    }

    void ensure_local_workers(size_t cnt) {
	local_service_.ensure_workers(cnt);
    }

    // Can be overridden if desired
    virtual void ensure_at_local(const std::string &name) {
	if (at_local_.find(name) == at_local_.end()) {
	    auto *interp = new interpreter(name);
	    interp->enable_file_io();
	    interp->setup_standard_lib();
	    add_at_local(name, interp);
	}
    }

    inline bool is_retain_state_between_queries() const {
	return retain_state_between_queries_;
    }
    
    inline void set_retain_state_between_queries(bool b) {
        retain_state_between_queries_ = b;
    }
  
protected:
    void add_at_local(const std::string &name, interpreter *interp) {
	at_local_[name] = interp;
    }

    void remove_at_local(const std::string &name) {
	if (auto *i = at_local_[name]) {
	    at_local_.erase(name);
	    delete i;
	}
    }

    bool delete_instance_at(term_env &query_src, const std::string &where);

    interp::remote_return_t execute_at(term query, term else_do,
				       term_env &query_src,
				       const std::string &where,
				       interp::remote_execute_mode mode,
				       size_t timeout);

    interp::remote_return_t continue_at(term query, term else_do,
					term_env &query_src,
					const std::string &where,
					interp::remote_execute_mode mode,
					size_t timeout);

private:
    static bool compile_0(interpreter_base &interp, size_t arity, common::term args[]);    
    static bool consult_1(interpreter_base &interp, size_t arity, common::term args[]);

public:
    static std::tuple<common::term, common::term, size_t> deconstruct_where(interpreter_base &interp, common::term where);
    static bool operator_at_impl(interpreter_base &interp, size_t arity, common::term args[], const std::string &name, remote_execute_mode mode);
    static bool operator_at_2(interpreter_base &interp, size_t arity, common::term args[] );
    static bool operator_at_silent_2(interpreter_base &interp, size_t arity, common::term args[] );
    static bool operator_at_parallel_2(interpreter_base &interp, size_t arity, common::term args[] );
private:
    static bool is_else(interpreter_base &interp, common::term t);
    static std::pair<common::term, common::term> extract_else(interpreter_base &interp, common::term t);
    static bool is_timeout(interpreter_base &interp, common::term t);
    static std::pair<size_t, common::term> extract_timeout(interpreter_base &interp, common::term t);    
  
    static bool new_instance_meta(interpreter_base &interp, const meta_reason_t &reason);

    void dispatch();
    void dispatch_wam(wam_instruction_base *instruction);
    bool unify_args(term clause_head, const code_point &p);
    bool can_unify_args(term clause_head, const code_point &p);  
    bool select_clause(const code_point &instruction,
		       size_t index_id,
		       const managed_clauses &clauses,
		       size_t from_clause);
  
    inline std::vector<binding> & query_vars()
        { return *query_vars_; }

    inline std::vector<binding> * query_vars_ptr()
        { return query_vars_; }

    inline void set_query_vars(std::vector<binding> *qv)
        { query_vars_ = qv; }

    bool wam_enabled_;
    std::vector<binding> *query_vars_;
    size_t num_instances_;
    bool retain_state_between_queries_;
    std::vector<size_t> last_hb_;

    friend struct new_instance_context;
    friend class test_wam_interpreter;
    friend class test_wam_compiler;

    common::local_service local_service_;
    std::unordered_map<std::string, interpreter *> at_local_;
};

template<typename Pre, typename Post> void standard_clause_processing<Pre,Post>::operator () (common::term clause)
{
    auto head = interp_.clause_head(clause);
    auto head_f = interp_.functor(head);

    if (head_f == interpreter_base::ACTION_BY) {
        // Make a copy of the term before executing it,
        // because we don't want any side-effects of bound variables
        // persist in the program database.
        auto clause_copy = interp_.copy(clause);
      
        pre_(clause_copy);
        auto body = interp_.arg(clause_copy, 0);
	interp_.execute(body);
	interp_.reset();
        post_(clause_copy);	
    }
}
    
}}

#endif

