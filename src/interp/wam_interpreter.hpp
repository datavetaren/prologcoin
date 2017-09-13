#pragma once

#ifndef _interp_wam_interpreter_hpp
#define _interp_wam_interpreter_hpp

#include <istream>
#include <vector>
#include <iomanip>
#include "interpreter.hpp"

namespace prologcoin { namespace interp {

class test_wam_interpreter;

struct static_ 
{
  template<typename T> static_ (T once) { once(); }
  ~static_ () {}
};

enum wam_instruction_type {
  PUT_VARIABLE_X,
  PUT_VARIABLE_Y,
  PUT_VALUE_X,
  PUT_VALUE_Y,
  PUT_UNSAFE_VALUE_Y,
  PUT_STRUCTURE,
  PUT_LIST,
  PUT_CONSTANT,
  
  GET_VARIABLE_X,
  GET_VARIABLE_Y,
  GET_STRUCTURE,
  GET_LIST,
  GET_CONSTANT,

  SET_VARIABLE_X,
  SET_VARIABLE_Y,
  SET_LOCAL_VALUE_X,
  SET_LOCAL_VALUE_Y,
  SET_CONSTANT,
  SET_VOID,

  UNIFY_VARIABLE_X,
  UNIFY_VARIABLE_Y,
  UNIFY_VALUE_X,
  UNIFY_VALUE_Y,
  UNIFY_LOCAL_VALUE_X,
  UNIFY_LOCAL_VALUE_Y,
  UNIFY_CONSTANT,
  UNIFY_VOID,

  ALLOCATE,
  DEALLOCATE,
  CALL,
  EXECUTE,
  PROCEED,
 
  TRY_ME_ELSE,
  RETRY_ME_ELSE,
  TRUST_ME,
  TRY,
  RETRY,
  TRUST,

  SWITCH_ON_TERM,
  SWITCH_ON_CONSTANT,
  SWITCH_ON_STRUCTURE,
 
  NECK_CUT,
  GET_LEVEL_Y,
  CUT_Y  
};

class wam_interpreter;

typedef uint64_t code_t;

class wam_instruction_base
{
protected:
    typedef void (*fn_type)(wam_interpreter &interp, wam_instruction_base *self);
    inline wam_instruction_base(fn_type fn, uint64_t sz_bytes)
      : fn_(fn), size_((sizeof(*this)+sz_bytes+sizeof(code_t)-1)/sizeof(code_t)) { }

public:
    inline void invoke(wam_interpreter &interp) {
        fn_(interp, this);
    }

    inline size_t size() const { return size_; }

private:
    fn_type fn_;
    uint64_t size_;

    typedef void (*print_fn_type)(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self);

public:
    static void register_printer(fn_type fn, print_fn_type print_fn)
    {
        print_fns_[fn] = print_fn;
    }

    void print(std::ostream &out, wam_interpreter &interp)
    {
        print_fn_type pfn = print_fns_[fn_];
	pfn(out, interp, this);
    }

private:
    static std::unordered_map<fn_type, print_fn_type> print_fns_;

    friend class wam_instruction_sequence;
};

template<wam_instruction_type I> class wam_instruction : protected wam_instruction_base
{
public:
    inline wam_instruction(fn_type fn, size_t sz_bytes)
      : wam_instruction_base(fn, sz_bytes) { }
};

class wam_instruction_sequence
{
public:
    wam_instruction_sequence(wam_interpreter &interp,
  			     size_t initial_capacity = 256)
      : interp_(interp), instrs_size_(0), instrs_capacity_(initial_capacity)
    { instrs_ = new code_t[instrs_capacity_]; }

    void add(const wam_instruction_base &i)
    {
        size_t sz = i.size();
        code_t *data = ensure_fit(sz);
	auto p = reinterpret_cast<wam_instruction_base *>(data);
	memcpy(p, &i, sz*sizeof(code_t));
    }

    void print(std::ostream &out);

private:
    code_t * ensure_fit(size_t sz)
    {
        ensure_capacity(instrs_size_ + sz);
	code_t *data = &instrs_[instrs_size_];
	instrs_size_ += sz;
	return data;
    }

    void ensure_capacity(size_t cap)
    {
        if (cap > instrs_capacity_) {
   	    size_t new_cap = 2*std::max(cap, instrs_size_);
	    code_t *new_instrs = new code_t[new_cap];
	    memcpy(new_instrs, instrs_, sizeof(code_t)*instrs_size_);
	    instrs_capacity_ = new_cap;
	    delete instrs_;
	    instrs_ = new_instrs;
        }
    }

    wam_interpreter &interp_;
    size_t instrs_size_;
    size_t instrs_capacity_;
    code_t *instrs_;
};

template<> class wam_instruction<CALL> : public wam_instruction_base {
public:
      inline wam_instruction(wam_instruction_base *p, uint32_t arity, uint32_t num_y) :
      wam_instruction_base(&invoke, sizeof(data)),
      p_(p), data((static_cast<uint64_t>(arity) << 32) | num_y) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
    }

    inline wam_instruction_base * p() const { return p_; }
    inline uint32_t arity() const { return data >> 32; }
    inline uint32_t num_y() const { return data & 0xffffffff; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self);
  /*
    {
        auto self1 = reinterpret_cast<wam_instruction<CALL> *>(self);
        interp.call(self1->p(), self1->arity(), self1->num_y());
    }
  */

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
      // auto self1 = reinterpret_cast<wam_instruction<PUT_UNSAFE_VALUE_Y> *>(self);
        // out << "call p " << self1->yn() << ", a" << self1->ai();
    }

    wam_instruction_base *p_;
    uint64_t data;
};

class wam_interpreter : public common::term_env
{
public:
    wam_interpreter();

    typedef common::term term;

private:
    template<wam_instruction_type I> friend class wam_instruction;

    struct environment_t {
        environment_t        *ce; // Continuation environment
        wam_instruction_base *cp; // Continuation point
        term                  yn[]; // Y variables
    };

    struct choice_point_t {
        environment_t        *ce;
        wam_instruction_base *cp;
        choice_point_t       *b;
        wam_instruction_base *bp;
        size_t                tr;
        size_t                h; 
        choice_point_t       *b0;
        size_t                arity;
        term                  ai[];
    };

    typedef union {
        environment_t *e;
        choice_point_t *cp;
        term term;
    } word_t;

    template<typename T> inline size_t words() const
    { return sizeof(T)/sizeof(word_t); }

    template<typename T> inline word_t * base(T *t) const
    { return reinterpret_cast<word_t *>(t); }

    inline size_t num_y(environment_t *e)
    {
        auto after_call = e->cp;
	auto at_call = reinterpret_cast<wam_instruction<CALL> *>(
		 reinterpret_cast<code_t *>(after_call) -
		 sizeof(wam_instruction<CALL>)/sizeof(code_t));
	return at_call->num_y();
    }

    inline term & x(size_t i)
    {
        return register_xn_[i];
    }

    inline term & y(size_t i)
    {
        return register_e_->yn[i];
    }

    inline term & a(size_t i)
    {
        return register_ai_[i];    
    }

    inline void backtrack()
    {
        if (register_b_ == register_top_b_) {
	    top_fail_ = true;
        } else {
	    register_b0_ = register_b_->b0;
	    register_p_ = register_b_->bp;
	}
    }

    bool top_fail_;
    enum mode_t { READ, WRITE } mode_;
    size_t num_of_args_;
    wam_instruction_base *register_p_;
    wam_instruction_base *register_cp_;
    environment_t *register_e_;
    choice_point_t *register_b_;
    choice_point_t *register_b0_;
    choice_point_t *register_top_b_;
    size_t register_s_;

    term register_xn_[1024];
    term register_ai_[256];

    // Stack is emulated at heap offset >= 2^59 (3 bits for tag, remember!)
    // (This conforms to the WAM standard where addr(stack) > addr(heap))
    const size_t STACK_BASE = 0x80000000000000;

    word_t    *stack_;
    size_t    stack_ptr_;

    inline term deref_stack(common::ref_cell ref0)
    {
        common::ref_cell ref;
        do {
	    ref = ref0;
  	    term t = to_stack(ref)->term;
	    if (t.tag() != common::tag_t::REF) {
	        return t;
	    }
	    ref = static_cast<common::ref_cell &>(t);
	    if (!is_stack(ref)) {
	        return term_env::deref(ref);
	    }
        } while (ref0 != ref);
	return ref;
    }

    inline term deref(term t)
    {
        if (t.tag() == common::tag_t::REF) {
  	    auto ref = static_cast<common::ref_cell &>(t);
	    if (is_stack(ref)) {
	        deref_stack(ref);
	    } else {
	        return term_env::deref(t);
	    }
        }
    }

    inline void bind(common::ref_cell &ref, term t)
    {
        if (is_stack(ref)) {
	    to_stack(ref)->term = t;
	    trail(ref.index());
        } else {
	    term_env::bind(ref, t);
	}
    }

    inline bool is_stack(common::ref_cell ref)
    {
        return ref.index() >= STACK_BASE;
    }

    inline word_t * to_stack(common::ref_cell ref)
    {
        return &stack_[ref.index() - STACK_BASE];
    }

    inline size_t to_stack_addr(word_t *p)
    {
        return static_cast<size_t>(p - stack_) + STACK_BASE;
    }

    inline void put_variable_x(uint32_t xn, uint32_t ai)
    {
        term ref = new_ref();
	x(xn) = ref;
	a(ai) = ref;
    }

    inline void put_variable_y(uint32_t yn, uint32_t ai)
    {
        term ref = new_ref();
	y(yn) = ref;
	a(ai) = ref;
    }

    inline void put_value_x(uint32_t xn, uint32_t ai)
    {
        a(ai) = x(xn);
    }

    inline void put_value_y(uint32_t yn, uint32_t ai)
    {
        a(ai) = y(yn);
    }

    inline void put_unsafe_value_y(uint32_t yn, uint32_t ai)
    {
        term t = deref(y(yn));
	if (t.tag() != common::tag_t::REF) {
	    a(ai) = t;
	    return;
	}
	auto ref = static_cast<common::ref_cell &>(t);
	if (!is_stack(ref) || to_stack(ref) < base(register_e_)) {
	    a(ai) = t;
        } else {
	    t = new_ref();
	    a(ai) = t;
	    bind(ref, t);
	}
    }

    inline void put_structure(common::con_cell f, uint32_t ai)
    {
        a(ai) = new_term_con(f);
    }

    inline void put_list(uint32_t ai)
    {
        a(ai) = new_dotted_pair();
    }

    inline void put_constant(common::con_cell c, uint32_t ai)
    {
        a(ai) = c;
    }

    inline void get_variable_x(uint32_t xn, uint32_t ai)
    {
        x(xn) = a(ai);
    }

    inline void get_variable_y(uint32_t yn, uint32_t ai)
    {
        y(yn) = a(ai);
    }

    inline bool get_value_x(uint32_t xn, uint32_t ai)
    {
        if (!unify(x(xn), a(ai))) {
	    backtrack();
	    return false;
        } else {
	    return true;
	}
    }

    inline bool get_value_y(uint32_t yn, uint32_t ai)
    {
        if (!unify(y(yn), a(ai))) {
 	    backtrack();
	    return false;
        } else {
	    return true;
	}
    }

    inline bool get_structure(common::con_cell f, uint32_t ai)
    {
        bool fail = false;
        term t = deref(a(ai));
	switch (t.tag()) {
	case common::tag_t::REF: {
	    term s = new_term_str(f);
	    auto ref = static_cast<common::ref_cell &>(t);
	    bind(ref, s);
	    mode_ = WRITE;
	    break;
	  }
	case common::tag_t::STR: {
  	    auto str = static_cast<common::str_cell &>(t);
	    common::con_cell c = functor(str);
	    if (c == f) {
	        register_s_ = str.index() + 1;
		mode_ = READ;
	    } else {
	        fail = true;
	    }
	    break;
	 }
	default:
	  fail = true;
	  break;
	}
	if (fail) {
	    backtrack();
	    return false;
	} else {
	    return true;
	}
    }

    inline bool get_list(common::con_cell c, uint32_t ai)
    {
        return get_structure(empty_list_con(), ai);
    }

    inline bool get_constant(common::con_cell c, uint32_t ai)
    {
        bool fail = false;
        term t = deref(a(ai));
	switch (t.tag()) {
	case common::tag_t::REF: {
	  auto ref = static_cast<common::ref_cell &>(t);
	  bind(ref, c);
	  break;
  	  }
	case common::tag_t::CON: {
	  auto c1 = static_cast<common::con_cell &>(t);
	  fail = c != c1;
	  break;
  	  }
	default:
	  fail = true;
	  break;
	}
	if (fail) {
	    backtrack();
	    return false;
	} else {
	    return true;
	}
    }

    inline void set_variable_x(uint32_t xn)
    {
        term t = new_ref();
	x(xn) = t;
    }

    inline void set_variable_y(uint32_t yn)
    {
        term t = new_ref();
	y(yn) = t;
    }

    inline void set_value_x(uint32_t xn)
    {
        new_term_copy_cell(x(xn));
    }

    inline void set_value_y(uint32_t yn)
    {
        new_term_copy_cell(y(yn));
    }

    inline void set_local_value_x(uint32_t xn)
    {
        term t = deref(x(xn));
	if (t.tag() == common::tag_t::REF) {
	    auto ref = static_cast<common::ref_cell &>(t);
	    if (is_stack(ref)) {
	        term h = new_ref();
		bind(ref, h);
	    } else {
  	        new_term_copy_cell(t);
	    }
	} else {
	    new_term_copy_cell(t);
	}
    }

    inline void set_local_value_y(uint32_t yn)
    {
        term t = deref(y(yn));
	if (t.tag() == common::tag_t::REF) {
	    auto ref = static_cast<common::ref_cell &>(t);
	    if (is_stack(ref)) {
	        term h = new_ref();
		bind(ref, h);
	    } else {
  	        new_term_copy_cell(t);
	    }
	} else {
	    new_term_copy_cell(t);
	}
    }

    inline void set_constant(common::con_cell c)
    {
        new_term_copy_cell(c);
    }

    inline void set_void(uint32_t n)
    {
        new_ref(n);
    }

    inline void unify_variable_x(uint32_t xn)
    {
        switch (mode_) {
	case READ: x(xn) = heap_get(register_s_); break;
	case WRITE: x(xn) = new_ref(); break;
        }
	register_s_++;
    }

    inline void unify_variable_y(uint32_t yn)
    {
        switch (mode_) {
	case READ: y(yn) = heap_get(register_s_); break;
	case WRITE: y(yn) = new_ref(); break;
        }
	register_s_++;
    }

    inline bool unify_value_x(uint32_t xn)
    {
        bool fail = false;
        switch (mode_) {
	case READ: fail = !unify(x(xn), heap_get(register_s_)); break;
	case WRITE: new_term_copy_cell(x(xn)); break;
        }
	register_s_++;
	if (fail) {
	    backtrack();
	    return false;
	} else {
  	    return true;
	}
    }

    inline bool unify_value_y(uint32_t yn)
    {
        bool fail = false;
        switch (mode_) {
	case READ: fail = !unify(y(yn), heap_get(register_s_)); break;
	case WRITE: new_term_copy_cell(y(yn)); break;
        }
	register_s_++;
	if (fail) {
	    backtrack();
	    return false;
	} else {
  	    return true;
	}
    }

    inline bool unify_local_value_x(uint32_t xn)
    {
        bool fail = false;
	switch (mode_) {
  	case READ: fail = !unify(x(xn), heap_get(register_s_)); break;
	case WRITE: {
	  term t = deref(x(xn));
	  if (t.tag() == common::tag_t::REF) {
	      auto ref = static_cast<common::ref_cell &>(t);
	      if (is_stack(ref)) {
		  auto h = new_ref();
		  bind(ref, h);
	      } else {
	  	  new_term_copy_cell(t);
	      }
	  } else {
  	      new_term_copy_cell(t);
	  }
	  break;
      	  }
	}
	register_s_++;
	if (fail) {
	    backtrack();
	    return false;
	} else {
	    return true;
	}
    }

    inline bool unify_local_value_y(uint32_t xn)
    {
        bool fail = false;
	switch (mode_) {
  	case READ: fail = !unify(x(xn), heap_get(register_s_)); break;
	case WRITE: {
	  term t = deref(x(xn));
	  if (t.tag() == common::tag_t::REF) {
	      auto ref = static_cast<common::ref_cell &>(t);
	      if (is_stack(ref)) {
		  auto h = new_ref();
		  bind(ref, h);
	      } else {
	  	  new_term_copy_cell(t);
	      }
	  } else {
  	      new_term_copy_cell(t);
	  }
	  break;
      	  }
	}
	register_s_++;
	if (fail) {
	    backtrack();
	    return false;
	} else {
	    return true;
	}
    }

    inline bool unify_constant(common::con_cell c)
    {
        bool fail = false;
        switch (mode_) {
	case READ: {
	    term t = deref(heap_get(register_s_));
	    switch (t.tag()) {
	    case common::tag_t::REF: {
	        auto ref = static_cast<common::ref_cell &>(t);
		bind(ref, c);
		break;
	      }
	    case common::tag_t::CON: {
	        auto c1 = static_cast<common::con_cell &>(t);
		fail = c != c1;
	      }
	    default:
	      fail = true;
	      break;
	    }
          }
	case WRITE: {
	    new_term_con(c);
	    break;
	  }
	}
	if (fail) {
	    backtrack();
	    return false;
	} else {
	    return true;
	}
    }

    inline void unify_void(uint32_t n)
    {
        switch (mode_) {
	case READ: register_s_ += n; break;
	case WRITE: new_ref(n); break;
        }
    }

    inline environment_t * allocate(size_t num_y)
    {
        word_t *new_e0;
	if (base(register_e_) > base(register_b_)) {
	    new_e0 = base(register_e_) + num_y + words<environment_t>();
	} else {
	    new_e0 = base(register_b_) + register_b_->arity +
	               words<choice_point_t>();
	}
	environment_t *new_e = reinterpret_cast<environment_t *>(new_e0);
	new_e->ce = register_e_;
	new_e->cp = register_cp_;
        register_e_ = new_e;
    }

    inline void deallocate()
    {
        register_cp_ = register_e_->cp;
        register_e_ = register_e_->ce;
    }

    inline void call(wam_instruction_base *p, size_t arity, uint32_t num_stack)
    {
        register_cp_ = reinterpret_cast<wam_instruction_base *>(reinterpret_cast<code_t *>(p) + p->size());
	num_of_args_ = arity;
	register_b0_ = register_b_;
	register_p_ = p;
    }

    inline void execute(wam_instruction_base *p, size_t arity)
    {
        num_of_args_ = arity;
	register_b0_ = register_b_;
	register_p_ = p;
    }

    inline void proceed()
    {
        register_p_ = register_cp_;
    }

    inline void allocate_choice_point(wam_instruction_base *p, wam_instruction_base *pelse)
    {
        word_t *new_b0;
	if (base(register_e_) > base(register_b_)) {
	     new_b0 = base(register_e_->cp) + num_y(register_e_) + words<environment_t>();
	} else {
	  new_b0 = base(register_b_->cp) + register_b_->arity + words<choice_point_t>();
	}
	auto *new_b = reinterpret_cast<choice_point_t *>(new_b0);
	new_b->arity = num_of_args_;
    }

    inline void try_me_else(wam_instruction_base *L)
    {
    }

    class prim_unification {
    public:
	prim_unification() : lhs_(common::ref_cell()), rhs_() { }
	prim_unification(const prim_unification &other)
	    : lhs_(other.lhs_), rhs_(other.rhs_) { }
	prim_unification(common::ref_cell lhs, term rhs)
	    : lhs_(lhs), rhs_(rhs) { }
	common::ref_cell lhs() const { return lhs_; }
	term rhs() const { return rhs_; }

    private:
	common::ref_cell lhs_;
	term rhs_;
    };

    void print_prims(const std::vector<prim_unification> &prims) const;

    // Takes a nested term and unfolds it into a sequence of
    // primitive unifications.
    std::vector<prim_unification> flatten(const term t);
    void flatten_process(std::vector<prim_unification> &prims,
			 std::vector<term> &args);
    prim_unification new_unification(term t);

    friend class test_wam_interpreter;
};

template<> class wam_instruction<PUT_VARIABLE_X> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t xn, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(data)),
      data((static_cast<uint64_t>(xn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
    }

    inline uint32_t xn() const { return data >> 32; }
    inline uint32_t ai() const { return data & 0xffffffff; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_VARIABLE_X> *>(self);
        interp.put_variable_x(self1->xn(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_VARIABLE_X> *>(self);
        out << "put_variable x" << self1->xn() << ", a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<PUT_VARIABLE_Y> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t yn, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(data)),
      data((static_cast<uint64_t>(yn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
    }

    inline uint32_t yn() const { return data >> 32; }
    inline uint32_t ai() const { return data & 0xffffffff; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_VARIABLE_Y> *>(self);
        interp.put_variable_y(self1->yn(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_VARIABLE_Y> *>(self);
        out << "put_variable y" << self1->yn() << ", a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<PUT_VALUE_X> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t xn, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(data)),
      data((static_cast<uint64_t>(xn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
    }

    inline uint32_t xn() const { return data >> 32; }
    inline uint32_t ai() const { return data & 0xffffffff; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_VALUE_X> *>(self);
        interp.put_value_x(self1->xn(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_VALUE_X> *>(self);
        out << "put_value x" << self1->xn() << ", a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<PUT_VALUE_Y> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t yn, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(data)),
      data((static_cast<uint64_t>(yn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
    }

    inline uint32_t yn() const { return data >> 32; }
    inline uint32_t ai() const { return data & 0xffffffff; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_VALUE_Y> *>(self);
        interp.put_value_y(self1->yn(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_VALUE_Y> *>(self);
        out << "put_value y" << self1->yn() << ", a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<PUT_UNSAFE_VALUE_Y> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t yn, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(data)),
      data((static_cast<uint64_t>(yn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
    }

    inline uint32_t yn() const { return data >> 32; }
    inline uint32_t ai() const { return data & 0xffffffff; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_UNSAFE_VALUE_Y> *>(self);
        interp.put_value_y(self1->yn(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_UNSAFE_VALUE_Y> *>(self);
        out << "put_unsafe_value y" << self1->yn() << ", a" << self1->ai();
    }

    uint64_t data;
};

}}

#endif
