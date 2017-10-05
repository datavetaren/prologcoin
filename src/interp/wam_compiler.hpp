#pragma once

#ifndef _interp_wam_compiler_hpp
#define _interp_wam_compiler_hpp

#include <forward_list>
#include <stack>
#include "../common/term_env.hpp"
#include "wam_interpreter.hpp"

namespace prologcoin { namespace interp {

enum wam_interim_instruction_type {
    INTERIM_FIRST = LAST + 1,
    INTERIM_LABEL,
    INTERIM_HEAD,
    INTERIM_GOAL
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

template<> class wam_interim_instruction<INTERIM_HEAD> : public wam_interim_instruction_base {
public:
    inline wam_interim_instruction() :
      wam_interim_instruction_base(&invoke, sizeof(*this), INTERIM_HEAD) {
        static bool init = [] {
	  register_printer(&invoke, &print); return true; } ();
        static_cast<void>(init);
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        assert("this instruction should never be executed." == nullptr);
    }

    static void print(std::ostream &out, wam_interpreter &interp,
		      wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_interim_instruction<INTERIM_HEAD> *>(self);
	(void)self1;
        out << "interim_head";
    }
};

template<> class wam_interim_instruction<INTERIM_GOAL> : public wam_interim_instruction_base {
public:
    inline wam_interim_instruction() :
      wam_interim_instruction_base(&invoke, sizeof(*this), INTERIM_GOAL) {
        static bool init = [] {
	  register_printer(&invoke, &print); return true; } ();
        static_cast<void>(init);
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        assert("this instruction should never be executed." == nullptr);
    }

    static void print(std::ostream &out, wam_interpreter &interp,
		      wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_interim_instruction<INTERIM_GOAL> *>(self);
	(void)self1;
        out << "interim_goal";
    }
};

class wam_interim_code : private std::forward_list<wam_instruction_base *> {
public:
    wam_interim_code(wam_interpreter &interp);

    void push_back(const wam_instruction_base &instr);

    void print(std::ostream &out) const;

    std::vector<wam_instruction_base *> get_all();
    std::vector<wam_instruction_base *> get_all_reversed();

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

    std::forward_list<wam_instruction_base *>::const_iterator end() const
    {
        return std::forward_list<wam_instruction_base *>::end();
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

    wam_compiler(common::term_env &env)
      : env_(env), regs_a_(A_REG), regs_x_(X_REG), regs_y_(Y_REG) { }

private:
    friend class test_wam_compiler;

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

	std::pair<reg,bool> allocate(common::ref_cell ref);
        std::pair<reg,bool> allocate(common::ref_cell ref, size_t index);
        void allocate(common::ref_cell ref, reg r);

        void deallocate(common::ref_cell ref);

    private:
        reg_type reg_type_;
	std::unordered_map<common::ref_cell, reg> reg_map_;
    };

    inline bool is_interim_instruction(wam_instruction_base *instr) const {
        return instr->type() >= INTERIM_FIRST;
    }

    void compile_query_ref(reg lhsreg, common::ref_cell rhsvar,
			   wam_interim_code &seq);
    void compile_query_str(reg lhsreg, term rhs,
			   wam_interim_code &seq);
    void compile_query(reg lhsreg, term rhs, wam_interim_code &seq);

    void compile_program_ref(reg lhsreg, common::ref_cell rhsvar,
			   wam_interim_code &seq);
    void compile_program_str(reg lhsreg, term rhs,
			   wam_interim_code &seq);
    void compile_program(reg lhsreg, term rhs, 
			 wam_interim_code &seq);


    void compile_query_or_program(term t, compile_type c,
			          wam_interim_code &seq);
    void remap_x_registers(wam_interim_code &seq);
    void remap_y_registers(wam_interim_code &seq);
    void find_x_to_y_registers(wam_interim_code &seq,
			       std::vector<size_t> &x_to_y);
    void allocate_y_registers(wam_interim_code &seq);
    void update_calls_for_environment_trimming(wam_interim_code &seq);
    void find_unsafe_y_registers(wam_interim_code &seq,
				 std::unordered_set<size_t> &unsafe_y_regs);
    void remap_to_unsafe_y_registers(wam_interim_code &instrs);

    bool clause_needs_environment(const term clause);
    void compile_clause(const term clause, wam_interim_code &seq);

    term clause_head(const term clause);
    term clause_body(const term clause);

    std::function<uint32_t ()> x_getter(wam_instruction_base *instr);
    std::function<void (uint32_t)> x_setter(wam_instruction_base *instr);

    std::function<uint32_t ()> y_getter(wam_instruction_base *instr);
    std::function<void (uint32_t)> y_setter(wam_instruction_base *instr);

    void change_x_to_y(wam_instruction_base *instr);

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
  
    term first_arg(const term clause);
    bool first_arg_is_var(const term clause);
    bool first_arg_is_con(const term clause);
    bool first_arg_is_str(const term clause);

    std::vector<std::vector<term> > partition_clauses(const std::vector<term> &clauses, std::function<bool (const term t1, const term t2)> pred);
    std::vector<std::vector<term> > partition_clauses_nonvar(const std::vector<term> &clauses);
    std::vector<std::vector<term> > partition_clauses_first_arg(const std::vector<term> &clauses);


    void print_partition(std::ostream &out, const std::vector<std::vector<term> > &partition);

    template<reg_type T> std::pair<reg,bool> allocate_reg(common::ref_cell ref);
    template<reg_type T> std::pair<reg,bool> allocate_reg(common::ref_cell ref, size_t index);
    template<reg_type T> void allocate_reg(common::ref_cell ref, reg r);

    bool is_argument(common::ref_cell ref)
    { return argument_pos_.count(ref) > 0; }

    size_t get_argument_index(common::ref_cell ref)
    { return argument_pos_[ref]; }

    common::term_env &env_;

    std::unordered_map<common::ref_cell, size_t> argument_pos_;
    std::unordered_map<common::eq_term, common::ref_cell> term_map_;

    register_pool regs_a_;
    register_pool regs_x_;
    register_pool regs_y_;
};

template<> inline std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg<wam_compiler::A_REG>(common::ref_cell ref)
{ return regs_a_.allocate(ref); }
template<> inline std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg<wam_compiler::A_REG>(common::ref_cell ref, size_t index)
    { return regs_a_.allocate(ref, index); }
template<> inline void wam_compiler::allocate_reg<wam_compiler::A_REG>(common::ref_cell ref, wam_compiler::reg r)
    { regs_a_.allocate(ref, r); }

template<> inline std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg<wam_compiler::X_REG>(common::ref_cell ref)
{ return regs_x_.allocate(ref); }
template<> inline std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg<wam_compiler::X_REG>(common::ref_cell ref, size_t index)
{ return regs_x_.allocate(ref, index); }
template<> inline void wam_compiler::allocate_reg<wam_compiler::X_REG>(common::ref_cell ref, wam_compiler::reg r)
{ regs_x_.allocate(ref, r); }

template<> inline std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg<wam_compiler::Y_REG>(common::ref_cell ref)
{ return regs_y_.allocate(ref); }
template<> inline std::pair<wam_compiler::reg,bool> wam_compiler::allocate_reg<wam_compiler::Y_REG>(common::ref_cell ref, size_t index)
{ return regs_y_.allocate(ref, index); }
template<> inline void wam_compiler::allocate_reg<wam_compiler::Y_REG>(common::ref_cell ref, wam_compiler::reg r)
{ regs_y_.allocate(ref, r); }



}}

#endif


