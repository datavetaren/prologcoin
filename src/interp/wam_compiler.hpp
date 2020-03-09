#pragma once

#ifndef _interp_wam_compiler_hpp
#define _interp_wam_compiler_hpp

#include <forward_list>
#include <stack>
#include <bitset>
#include "../common/term_env.hpp"
#include "wam_interpreter.hpp"

namespace prologcoin { namespace interp {

enum wam_interim_instruction_type {
    INTERIM_FIRST = LAST + 1,
    INTERIM_LABEL,
    INTERIM_MERGE
};

class wam_exception : public std::runtime_error
{
public:
    wam_exception(const std::string &msg) : std::runtime_error(msg) { }
};

class wam_exception_too_many_vars_in_clause : public wam_exception
{
public:
    wam_exception_too_many_vars_in_clause(const std::string &msg)
	: wam_exception(msg) { }
};

class wam_interim_instruction_base : public wam_instruction_base {
public:
    inline wam_interim_instruction_base(fn_type fn, uint64_t sz_bytes,
					wam_interim_instruction_type t)
      : wam_instruction_base(fn, sz_bytes, static_cast<wam_instruction_type>(t))
    {
    }

    inline wam_interim_instruction_type type() const {
        return static_cast<wam_interim_instruction_type>(wam_instruction_base::type());
    }
};

template<wam_interim_instruction_type I> class wam_interim_instruction : public wam_interim_instruction_base
{
public:
    inline wam_interim_instruction(fn_type fn, uint64_t sz_bytes, wam_interim_instruction_type t)
      : wam_interim_instruction_base(fn, sz_bytes, I) { }
};

template<> class wam_interim_instruction<INTERIM_LABEL> : public wam_interim_instruction_base {
public:
     inline wam_interim_instruction(common::int_cell lab) :
       wam_interim_instruction_base(&invoke, sizeof(*this), INTERIM_LABEL),
       label_(lab) {
        static bool init = [] {
	  register_printer(&invoke, &print); return true; } ();
        static_cast<void>(init);
    }

    common::int_cell label() const
    {
	return label_;
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        assert("this instruction should never be executed." == nullptr);
    }

    static void print(std::ostream &out, wam_interpreter &interp,
		      wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_interim_instruction<INTERIM_LABEL> *>(self);
        out << "L:" << interp.to_string(self1->label_);
    }
private:
    common::int_cell label_;
};

class wam_compiler;

template<> class wam_interim_instruction<INTERIM_MERGE> : public wam_interim_instruction_base {
public:
     wam_interim_instruction(wam_compiler &compiler, const std::initializer_list<code_point> &cps);

    static inline void init() {
	static bool init = [] {
	    register_printer(&invoke, &print);
	    register_updater(&invoke, &updater);
	    return true; } ();
	static_cast<void>(init);
    }

    inline void update(code_t *old_base, code_t *new_base)
    {
	for (auto &p : *from_) {
	    update_ptr(p, old_base, new_base);
	}
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        assert("this instruction should never be executed." == nullptr);
    }

    static void print(std::ostream &out, wam_interpreter &interp,
		      wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_interim_instruction<INTERIM_MERGE> *>(self);
        out << "interim_merge [";
	bool first = true;
	for (code_point &cp : *self1->from_) {
	    if (!first) out << ",";
	    first = false;
	    out << interp.to_string(cp);
	}
	out << "]";
    }

    static void updater(wam_instruction_base *self, code_t *old_base, code_t *new_base)
    {
	auto self1 = reinterpret_cast<wam_interim_instruction<INTERIM_MERGE> *>(self);
	self1->update(old_base, new_base);
    }

    const std::vector<code_point> & sources() const {
	return *from_;
    }

private:
    std::vector<code_point> *from_;
};

class wam_interim_code : private std::forward_list<wam_instruction_base *> {
public:
    wam_interim_code(wam_interpreter &interp);

    wam_instruction_base * new_instruction(const wam_instruction_base &instr);
    wam_instruction_base * push_back(const wam_instruction_base &instr);
    void append(const wam_interim_code &instrs);

    void print(std::ostream &out) const;

    void get_all(std::vector<wam_instruction_base *> &all,
		 std::unordered_map<common::int_cell, size_t> &labels);

    void get_destinations(
	    std::vector<wam_instruction_base *> &instrs,
	    std::unordered_map<common::int_cell, size_t> &labels,
	    size_t index,
	    std::vector<size_t> &continuations);

    void get_topological_sort(
	    std::vector<wam_instruction_base *> &instrs,
	    std::unordered_map<common::int_cell, size_t> &labels,
	    std::vector<size_t> &sorted);

    void get_reversed_topological_sort(
	    std::vector<wam_instruction_base *> &instrs,
	    std::unordered_map<common::int_cell, size_t> &labels,
	    std::vector<size_t> &sorted);

    std::forward_list<wam_instruction_base *>::iterator begin()
    {
        return std::forward_list<wam_instruction_base *>::begin();
    }

    std::forward_list<wam_instruction_base *>::iterator end()
    {
        return std::forward_list<wam_instruction_base *>::end();
    }

    std::forward_list<wam_instruction_base *>::const_iterator begin() const
    {
        return std::forward_list<wam_instruction_base *>::begin();
    }

    std::forward_list<wam_instruction_base *>::const_iterator before_begin() const
    {
        return std::forward_list<wam_instruction_base *>::before_begin();
    }

    std::forward_list<wam_instruction_base *>::const_iterator end() const
    {
        return std::forward_list<wam_instruction_base *>::end();
    }

    bool is_at_type(std::forward_list<wam_instruction_base *>::iterator &it,
		    wam_instruction_type t) const {
	if (it == end()) {
	    return false;
	}
	return (*it)->type() == t;
    }

    size_t compute_size() const
    {
	size_t n = 0;
	for (auto instr : *this) {
	    (void)instr;
	    n++;
	}
	return n;
    }

    std::forward_list<wam_instruction_base *>::iterator erase_after(
	    std::forward_list<wam_instruction_base *>::iterator &it)
    {
	size_--;
	bool at_end = it == end_;
        auto it1 = std::forward_list<wam_instruction_base *>::erase_after(it);
	if (at_end) end_ = it1;
	return it1;
    }

    std::forward_list<wam_instruction_base *>::iterator erase_after(
	    std::forward_list<wam_instruction_base *>::iterator &it,
	    std::forward_list<wam_instruction_base *>::iterator &it_last)
    {
	auto n = std::distance(it, it_last) - 1;
	size_ -= n;
	bool at_end = it_last == end_;
        auto it1 = std::forward_list<wam_instruction_base *>::erase_after(it, it_last);
	if (at_end) end_ = it1;
	return it1;
    }

    std::forward_list<wam_instruction_base *>::iterator erase_after(
	    std::forward_list<wam_instruction_base *>::const_iterator &it)
    {
	size_--;
	bool at_end = it == end_;
        auto it1 = std::forward_list<wam_instruction_base *>::erase_after(it);
	if (at_end) end_ = it1;
	return it1;
    }

    std::forward_list<wam_instruction_base *>::iterator insert_after(
	    std::forward_list<wam_instruction_base *>::iterator &it,
	    const wam_instruction_base &e)
    {
	auto *i = new_instruction(e);
        return insert_after(it,i);
    }       								     

    std::forward_list<wam_instruction_base *>::iterator insert_after(
	    std::forward_list<wam_instruction_base *>::iterator &it,
	    wam_instruction_base *i)
    {
	size_++;
        return std::forward_list<wam_instruction_base *>::insert_after(it,i);
    }

    size_t size() const
    {
        return size_;
    }

private:
    void push_back(wam_instruction_base *instr);

    wam_interpreter &interp_;
    std::forward_list<wam_instruction_base *>::iterator end_;
    size_t size_;
};

class wam_goal_iterator : public std::iterator<std::forward_iterator_tag,
					       common::term,
					       common::term,
					       const common::term *,
					       const common::term &> {
public:
    typedef common::term term;

    wam_goal_iterator(common::term_env &env, term body) : env_(env) {
        stack_.push(body);
	first_of();
    }

    wam_goal_iterator(common::term_env &env) : env_(env) { }

    inline bool operator == (const wam_goal_iterator &other) const {
        return stack_ == other.stack_;
    }

    inline bool operator != (const wam_goal_iterator &other) const {
        return stack_ != other.stack_;
    }

    inline wam_goal_iterator & operator ++ () {
        advance(); return *this;
    }

    inline reference operator * () const { return stack_.top(); }
    inline pointer operator -> () const { return &stack_.top(); }

private:
    void first_of();
    void advance();

    common::term_env &env_;
    std::stack<term> stack_;
};

class wam_compiler {
public:
    typedef common::term term;

    wam_compiler(wam_interpreter &interp)
      : interp_(interp), env_(interp), regs_a_(A_REG), regs_x_(X_REG), regs_y_(Y_REG), label_count_(1), goal_count_(0), level_count_(0) { }

    ~wam_compiler();

    void clear();
  
    static inline bool is_interim_instruction(wam_instruction_base *instr) {
        return static_cast<wam_interim_instruction_type>(instr->type())
	                         >= INTERIM_FIRST;
    }

    static inline bool is_label_instruction(wam_instruction_base *instr) {
        return static_cast<wam_interim_instruction_type>(instr->type())
	                         == INTERIM_LABEL;
    }

    static inline bool is_merge_instruction(wam_instruction_base *instr) {
        return static_cast<wam_interim_instruction_type>(instr->type())
	                         == INTERIM_MERGE;
    }

    inline size_t get_environment_size_of(wam_interim_code &instrs)
    {
	return static_cast<size_t>(find_maximum_y_register(instrs) + 1);
    }

    inline size_t get_num_x_registers(wam_interim_code &instrs)
    {
        return static_cast<size_t>(find_maximum_x_register(instrs) + 1);
    }

    bool compile_predicate(const qname &qn, wam_interim_code &instrs);

    inline common::con_cell current_module()
    { return interp_.current_module(); }

private:
    friend class test_wam_compiler;

    friend class wam_interim_instruction<INTERIM_MERGE>;

    class prim_unification {
    public:
	inline prim_unification() : lhs_(common::ref_cell()), rhs_() { }
	inline prim_unification(const prim_unification &other)
	    : lhs_(other.lhs_), rhs_(other.rhs_) { }
	inline prim_unification(common::ref_cell lhs, term rhs)
	    : lhs_(lhs), rhs_(rhs) { }

	inline common::ref_cell lhs() const { return lhs_; }
	inline term rhs() const { return rhs_; }

    private:
	common::ref_cell lhs_;
	term rhs_;
    };

    enum compile_type { COMPILE_QUERY, COMPILE_PROGRAM };

    void print_prims(const std::vector<prim_unification> &prims) const;

    // Takes a nested term and unfolds it into a sequence of
    // primitive unifications.
    std::vector<wam_compiler::prim_unification> flatten(const term t, compile_type for_type, bool is_predicate_call);

    prim_unification new_unification(term t);
    prim_unification new_unification(common::ref_cell ref, term t);

    enum reg_type {
        A_REG,
	X_REG,
	Y_REG
    };

    struct reg {
	reg() : num(0), type(X_REG) { }
	reg(size_t n, reg_type t) : num(n), type(t) { }

	size_t num;
	reg_type type;
    };

    static const size_t MAX_VARS = 256;
    typedef std::bitset<MAX_VARS> varset_t;

    friend inline std::ostream & operator << (std::ostream &out, reg &r)
    {
        switch (r.type) {
	case A_REG: out << "a"; break;
	case X_REG: out << "x"; break;
	case Y_REG: out << "y"; break;
	}
	out << r.num;
	return out;
    }

    class register_pool {
    public:
        register_pool(reg_type type) : reg_type_(type) { }
        inline void clear() { reg_map_.clear(); }

	bool contains(common::ref_cell ref);
	std::pair<reg,bool> allocate(common::ref_cell ref);
        std::pair<reg,bool> allocate(common::ref_cell ref, size_t index);
        void allocate(common::ref_cell ref, reg r);
        void deallocate(common::ref_cell ref);

    private:
        reg_type reg_type_;
	std::unordered_map<common::ref_cell, reg> reg_map_;
    };

    inline bool is_builtin(const qname &qn) const
    {
        return interp_.is_builtin(qn);
    }

    inline bool is_builtin(common::con_cell module, common::con_cell f) const
    {
	return interp_.is_builtin(module, f);
    }

    inline builtin & get_builtin(const qname &qn) const
    {
        return interp_.get_builtin(qn);
    }

    inline builtin & get_builtin(common::con_cell module, common::con_cell f) const
    {
	return interp_.get_builtin(module, f);
    }

    void compile_query_ref(reg lhsreg, common::ref_cell rhsvar,
			   wam_interim_code &seq);
    void compile_query_str(reg lhsreg, common::ref_cell lhsvar, term rhs,
			   wam_interim_code &seq);
    void compile_query(reg lhsreg, common::ref_cell lhsvar, term rhs,
		       wam_interim_code &seq);

    void compile_program_ref(reg lhsreg, common::ref_cell rhsvar,
			   wam_interim_code &seq);
    void compile_program_str(reg lhsreg, common::ref_cell lhsvar, term rhs,
			   wam_interim_code &seq);
    void compile_program(reg lhsreg, common::ref_cell lhsvar, term rhs, 
			 wam_interim_code &seq);

    void compile_builtin(common::con_cell module,
			 common::con_cell f,
			 bool first_goal,
			 wam_interim_code &seq);


    void compile_query_or_program(term t, compile_type c,
			          wam_interim_code &seq);
    bool is_boundary_instruction(wam_instruction_base *i);
    void remap_x_registers(wam_interim_code &seq);
    void remap_y_registers(wam_interim_code &seq);

    void find_x_to_y_registers(wam_interim_code &seq,
			       std::vector<size_t> &x_to_y);
    void allocate_y_registers(wam_interim_code &seq);
    int find_maximum_x_register(wam_interim_code &seq);
    int find_maximum_y_register(wam_interim_code &seq);
    void update_calls_for_environment_trimming(wam_interim_code &seq);
    void find_unsafe_y_registers(wam_interim_code &seq,
				 std::unordered_set<size_t> &unsafe_y_regs);
    void remap_to_unsafe_y_registers(wam_interim_code &instrs);
    void fix_unsafe_set_unify(wam_interim_code &instr);
    void eliminate_interim_but_labels(wam_interim_code &instrs);

    bool has_cut(wam_interim_code &seq);
    void allocate_cut(wam_interim_code &seq);
    bool clause_needs_environment(const term clause);
    bool is_conjunction(const term goal);
    bool is_disjunction(const term goal);
    bool is_if_then_else(const term goal);
    void insert_phi_nodes(const term goal_a, const term goal_b,
			  wam_interim_code &code);
    void compile_conjunction(const term conj, wam_interim_code &code);
    void compile_if_then_else(const term disj, wam_interim_code &code);
    void compile_disjunction(const term disj, wam_interim_code &code);
    void compile_goal(const term goal, bool first_goal, wam_interim_code &seq);
    void peephole_opt_execute(wam_interim_code &seq);
    void peephole_opt_void(wam_interim_code &instr);
    void reset_clause_temps();
    bool is_relevant_varset_op(const term t);
    void compute_var_indices(const term t);
    void compute_varsets(const term t);
    void find_vars(const term t, varset_t &varset);
    size_t new_level();
  // void compile_clause(const term clause, wam_interim_code &seq);
    void compile_clause(const managed_clause &m_clause, wam_interim_code &seq);
    std::vector<common::int_cell> new_labels(size_t n);
    std::vector<common::int_cell> new_labels_dup(size_t n);
    void emit_cp(std::vector<common::int_cell> &labels, size_t index, size_t n,
		 wam_interim_code &instrs);
    void compile_subsection(const managed_clauses &subsection,
			    wam_interim_code &instrs);

    common::int_cell new_label();

    term clause_head(const term clause);
    term clause_body(const term clause);

    std::function<size_t ()> x_getter(wam_instruction_base *instr);
    std::function<void (size_t)> x_setter(wam_instruction_base *instr);

    std::function<size_t ()> y_getter(wam_instruction_base *instr);
    std::function<void (size_t)> y_setter(wam_instruction_base *instr);

    void change_x_to_y(wam_instruction_base *instr);

    std::pair<size_t, size_t> get_num_x_and_y(wam_interim_code &instrs);

    class goals_range {
    public:
       inline goals_range(common::term_env &env, term t)
	    : env_(env), from_(t) { }
       wam_goal_iterator begin() {
	 return wam_goal_iterator(env_, from_);
       }
       wam_goal_iterator end() {
	 return wam_goal_iterator(env_);
       }
    private:
       common::term_env &env_;
       term from_;
    };

    goals_range for_all_goals(const term t) {
       return goals_range(env_, t);
    }
  
    enum first_arg_cat_t {
        FIRST_VAR, FIRST_CON, FIRST_LST, FIRST_STR
    };
    first_arg_cat_t first_arg_cat(const term clause);

    term first_arg(const term clause);
    common::con_cell first_arg_functor(const term clause);
    bool first_arg_is_var(const term clause);
    bool first_arg_is_con(const term clause);
    bool first_arg_is_str(const term clause);

    std::vector<managed_clauses> partition_clauses(const managed_clauses &clauses, std::function<bool (const managed_clause &c1, const managed_clause &t2)> pred);
    std::vector<managed_clauses> partition_clauses_nonvar(const managed_clauses &clauses);
    std::vector<managed_clauses> partition_clauses_first_arg(const managed_clauses &clauses);
    std::vector<size_t> find_clauses_on_cat(const managed_clauses &clauses,
					    first_arg_cat_t cat);
    void emit_switch_on_term(const managed_clauses &subsection,
			     const std::vector<common::int_cell> &labels,
			     wam_interim_code &instrs);
    void emit_second_level_indexing(
	      wam_compiler::first_arg_cat_t cat,
	      const managed_clauses &subsection,
	      const std::vector<common::int_cell> &labels,
	      const std::vector<size_t> &clause_indices,
	      code_point cp,
	      wam_interim_code &instrs);
    void emit_third_level_indexing(
	     const std::vector<size_t> &clause_indices,
	     const std::vector<common::int_cell> &labels,
	     wam_interim_code &instrs);

    void print_partition(std::ostream &out,
			 const std::vector<managed_clauses> &partition);

    template<reg_type T> bool has_reg(common::ref_cell ref);

    template<reg_type T> std::pair<reg,bool> allocate_reg(common::ref_cell ref);
    template<reg_type T> std::pair<reg,bool> allocate_reg(common::ref_cell ref, size_t index);
    template<reg_type T> void allocate_reg(common::ref_cell ref, reg r);

    template<reg_type T1, reg_type T2> std::pair<reg,bool> allocate_reg2(common::ref_cell ref);

    bool is_argument(common::ref_cell ref)
    { return argument_pos_.count(ref) > 0; }

    size_t get_argument_index(common::ref_cell ref)
    { return argument_pos_[ref]; }

    wam_interpreter &interp_;
    common::term_env &env_;

    std::unordered_map<common::ref_cell, size_t> argument_pos_;
    std::unordered_map<common::eq_term, common::ref_cell> term_map_;

    register_pool regs_a_;
    register_pool regs_x_;
    register_pool regs_y_;

    size_t label_count_;
    size_t goal_count_;
    size_t level_count_;

    std::unordered_map<common::term, size_t> var_index_;
    std::vector<common::ref_cell> index_var_;
    varset_t seen_vars_;
    std::unordered_map<common::term, varset_t> varsets_;
    std::vector<common::term> stack_;

    std::string varset_to_string(varset_t &t);
    common::term find_var(size_t var_index);

    std::vector<code_point> * new_merge();
    std::vector<std::vector<code_point> *> merges_;
};

template<> inline bool wam_compiler::has_reg<wam_compiler::A_REG>(common::ref_cell ref) { return regs_a_.contains(ref); }
template<> inline std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg<wam_compiler::A_REG>(common::ref_cell ref)
{ return regs_a_.allocate(ref); }
template<> inline std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg<wam_compiler::A_REG>(common::ref_cell ref, size_t index)
    { return regs_a_.allocate(ref, index); }
template<> inline void wam_compiler::allocate_reg<wam_compiler::A_REG>(common::ref_cell ref, wam_compiler::reg r)
    { regs_a_.allocate(ref, r); }

template<> inline bool wam_compiler::has_reg<wam_compiler::X_REG>(common::ref_cell ref) { return regs_x_.contains(ref); }
template<> inline std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg<wam_compiler::X_REG>(common::ref_cell ref)
{ return regs_x_.allocate(ref); }
template<> inline std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg<wam_compiler::X_REG>(common::ref_cell ref, size_t index)
{ return regs_x_.allocate(ref, index); }
template<> inline void wam_compiler::allocate_reg<wam_compiler::X_REG>(common::ref_cell ref, wam_compiler::reg r)
{ regs_x_.allocate(ref, r); }

template<> inline bool wam_compiler::has_reg<wam_compiler::Y_REG>(common::ref_cell ref) { return regs_y_.contains(ref); }
template<> inline std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg<wam_compiler::Y_REG>(common::ref_cell ref)
{ return regs_y_.allocate(ref); }
template<> inline std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg<wam_compiler::Y_REG>(common::ref_cell ref, size_t index)
{ return regs_y_.allocate(ref, index); }
template<> inline void wam_compiler::allocate_reg<wam_compiler::Y_REG>(common::ref_cell ref, wam_compiler::reg r)
{ regs_y_.allocate(ref, r); }

template<> inline std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg2<wam_compiler::A_REG, wam_compiler::X_REG>(common::ref_cell ref)
{ if (regs_a_.contains(ref)) {
      return regs_a_.allocate(ref); 
   } else {
      return regs_x_.allocate(ref);
   }
}

inline wam_interim_instruction<INTERIM_MERGE>::wam_interim_instruction(wam_compiler &compiler, const std::initializer_list<code_point> &cps) :
    wam_interim_instruction_base(&invoke, sizeof(*this), INTERIM_MERGE) {
    init();
    from_ = compiler.new_merge();
    for (auto &cp : cps) {
	from_->push_back(cp);
    }
}


}}

#endif


