#pragma once

#ifndef _interp_wam_interpreter_hpp
#define _interp_wam_interpreter_hpp

#include <istream>
#include <vector>
#include <iomanip>
#include "interpreter.hpp"

namespace prologcoin { namespace interp {

class test_wam_interpreter;
class test_wam_compiler;

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
  PUT_STRUCTURE_A,
  PUT_STRUCTURE_X,
  PUT_STRUCTURE_Y,
  PUT_LIST,
  PUT_CONSTANT,
  
  GET_VARIABLE_X,
  GET_VARIABLE_Y,
  GET_VALUE_X,
  GET_VALUE_Y,
  GET_STRUCTURE_A,
  GET_STRUCTURE_X,
  GET_STRUCTURE_Y,
  GET_LIST,
  GET_CONSTANT,

  SET_VARIABLE_A,
  SET_VARIABLE_X,
  SET_VARIABLE_Y,
  SET_VALUE_A,
  SET_VALUE_X,
  SET_VALUE_Y,
  SET_LOCAL_VALUE_X,
  SET_LOCAL_VALUE_Y,
  SET_CONSTANT,
  SET_VOID,

  UNIFY_VARIABLE_A,
  UNIFY_VARIABLE_X,
  UNIFY_VARIABLE_Y,
  UNIFY_VALUE_A,
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
  GET_LEVEL,
  CUT
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

    inline fn_type fn() const { return fn_; }
    inline size_t size() const { return size_; }

protected:
    inline wam_instruction_base * update_ptr(wam_instruction_base *p, code_t *old_base, code_t *new_base)
    {
	if (p == nullptr) {
	    return p;
	}
	return reinterpret_cast<wam_instruction_base *>(reinterpret_cast<code_t *>(p) - old_base + new_base);
    }

    inline void update(code_t *old_base, code_t *new_base)
    {
	auto it = updater_fns_.find(fn());
	if (it != updater_fns_.end()) {
	    auto updater_fn = it->second;
	    updater_fn(this, old_base, new_base);
	}
    }

private:
    fn_type fn_;
    uint64_t size_;

    typedef void (*print_fn_type)(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self);

    typedef void (*updater_fn_type)(wam_instruction_base *self, code_t *old_base, code_t *new_base);

public:
    static void register_printer(fn_type fn, print_fn_type print_fn)
    {
        print_fns_[fn] = print_fn;
    }

    static void register_updater(fn_type fn, updater_fn_type updater_fn)
    {
	updater_fns_[fn] = updater_fn;
    }

    void print(std::ostream &out, wam_interpreter &interp)
    {
        print_fn_type pfn = print_fns_[fn_];
	pfn(out, interp, this);
    }

private:
    static std::unordered_map<fn_type, print_fn_type> print_fns_;
    static std::unordered_map<fn_type, updater_fn_type> updater_fns_;

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

    inline size_t to_code_addr(code_t *p) const
    {
	return static_cast<size_t>(p - instrs_);
    }

    inline size_t to_code_addr(wam_instruction_base *p) const
    {
	return static_cast<size_t>(reinterpret_cast<code_t *>(p) - instrs_);
    }

    inline wam_instruction_base * to_code(size_t addr) const
    {
	return reinterpret_cast<wam_instruction_base *>(&instrs_[addr]);
    }

    void add(const wam_instruction_base &i)
    {
        size_t sz = i.size();
	code_t *old_base = instrs_;
        code_t *data = ensure_fit(sz);
	code_t *new_base = instrs_;
	auto p = reinterpret_cast<wam_instruction_base *>(data);
	memcpy(p, &i, sz*sizeof(code_t));

	if (old_base != new_base) {
	    p->update(old_base, new_base);
	}
    }

    void print_code(std::ostream &out);

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

	    update(instrs_, new_instrs);

	    delete instrs_;
	    instrs_ = new_instrs;
        }
    }

    inline void update(code_t *old_base, code_t *new_base);

    wam_interpreter &interp_;
    size_t instrs_size_;
    size_t instrs_capacity_;
    code_t *instrs_;
};

template<> class wam_instruction<CALL> : public wam_instruction_base {
public:
      inline wam_instruction(common::con_cell pn, wam_instruction_base *p,
			     uint32_t num_y) :
      wam_instruction_base(&invoke, sizeof(*this)),
      pn_(pn), p_(p), data(num_y) { 
      static bool init = [] {
	    register_printer(&invoke, &print);
	    register_updater(&invoke, &updater);
	    return true; } ();
      static_cast<void>(init);
    }

    inline void update(code_t *old_base, code_t *new_base)
    {
	p_ = update_ptr(p_, old_base, new_base);
    }

    inline wam_instruction_base * p() const { return p_; }
    inline common::con_cell pn() const { return pn_; }
    inline size_t arity() const { return pn().arity(); }
    inline size_t num_y() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self);

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self);

    static void updater(wam_instruction_base *self, code_t *old_base, code_t *new_base);

    common::con_cell pn_;
    wam_instruction_base *p_;
    uint64_t data;
};

class wam_interpreter : public common::term_env, public wam_instruction_sequence
{
public:
    wam_interpreter();
    ~wam_interpreter();

    typedef common::term term;

    typedef std::unordered_map<term, wam_instruction_base *> hash_map;

    inline hash_map * new_hash_map()
    {
	auto *map = new hash_map();
	hash_maps_.push_back(map);
	return map;
    }

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

    inline void tidy_trail()
    {
	size_t i = register_b_->tr;
	size_t tr = trail_size();
	size_t b = to_stack_addr(base(register_b_));
	while (i < tr) {
	    if (trail_get(i) < get_register_hb() ||
		(heap_size() < trail_get(i) && trail_get(i) < b)) {
		i++;
	    } else {
		trail_set(i, trail_get(tr-1));
		tr--;
	    }
	}
    }

    inline void unwind_trail(size_t a1, size_t a2)
    {
	for (size_t i = a1; i < a2; i++) {
	    size_t index = trail_get(i);
	    if (is_stack(index)) {
		*reinterpret_cast<term *>(to_stack(index))
		    = common::ref_cell(index);
	    } else {
		heap_set(index, common::ref_cell(index));
	    }
	}
    }

    inline void trail(size_t a)
    {
	size_t b = to_stack_addr(base(register_b_));
	if (a < get_register_hb() || (is_stack(a) && a < b)) {
	    push_trail(a);
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

    std::vector<hash_map *> hash_maps_;

    term register_xn_[1024];
    term register_ai_[256];

    // Stack is emulated at heap offset >= 2^59 (3 bits for tag, remember!)
    // (This conforms to the WAM standard where addr(stack) > addr(heap))
    const size_t STACK_BASE = 0x80000000000000;

    word_t    *stack_;
    size_t    stack_ptr_;

  public:
    inline wam_instruction_base * next_instruction(wam_instruction_base *p)
    {
	return reinterpret_cast<wam_instruction_base *>(reinterpret_cast<code_t *>(p) + p->size());
    }

  private:

    inline void goto_next_instruction()
    {
	register_p_ = next_instruction(register_p_);
    }

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
	        return deref_stack(ref);
	    } else {
	        return term_env::deref(t);
	    }
        } else {
	    return t;
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

    inline void put_variable_x(uint32_t xn, uint32_t ai)
    {
        term ref = new_ref();
	x(xn) = ref;
	a(ai) = ref;
	goto_next_instruction();
    }

    inline void put_variable_y(uint32_t yn, uint32_t ai)
    {
        term ref = new_ref();
	y(yn) = ref;
	a(ai) = ref;
	goto_next_instruction();
    }

    inline void put_value_x(uint32_t xn, uint32_t ai)
    {
        a(ai) = x(xn);
	goto_next_instruction();
    }

    inline void put_value_y(uint32_t yn, uint32_t ai)
    {
        a(ai) = y(yn);
	goto_next_instruction();
    }

    inline void put_unsafe_value_y(uint32_t yn, uint32_t ai)
    {
        term t = deref(y(yn));
	if (t.tag() != common::tag_t::REF) {
	    a(ai) = t;
	    goto_next_instruction();
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
	goto_next_instruction();
    }

    inline void put_structure_a(common::con_cell f, uint32_t ai)
    {
        a(ai) = new_term_con(f);
	goto_next_instruction();
    }

    inline void put_structure_x(common::con_cell f, uint32_t xn)
    {
        x(xn) = new_term_con(f);
	goto_next_instruction();
    }

    inline void put_structure_y(common::con_cell f, uint32_t yn)
    {
        y(yn) = new_term_con(f);
	goto_next_instruction();
    }
  
    inline void put_list(uint32_t ai)
    {
        a(ai) = new_dotted_pair();
	goto_next_instruction();
    }

    inline void put_constant(term c, uint32_t ai)
    {
        a(ai) = c;
	goto_next_instruction();
    }

    inline void get_variable_x(uint32_t xn, uint32_t ai)
    {
        x(xn) = a(ai);
	goto_next_instruction();
    }

    inline void get_variable_y(uint32_t yn, uint32_t ai)
    {
        y(yn) = a(ai);
	goto_next_instruction();
    }

    inline void get_value_x(uint32_t xn, uint32_t ai)
    {
        if (!unify(x(xn), a(ai))) {
	    backtrack();
        } else {
	    goto_next_instruction();
	}
    }

    inline void get_value_y(uint32_t yn, uint32_t ai)
    {
        if (!unify(y(yn), a(ai))) {
 	    backtrack();
        } else {
	    goto_next_instruction();
	}
    }

    inline void get_structure_a(common::con_cell f, uint32_t ai)
    {
        term t = deref(a(ai));
	return get_structure(f, t);
    }

    inline void get_structure_x(common::con_cell f, uint32_t xn)
    {
        term t = deref(x(xn));
	return get_structure(f, t);
    }

    inline void get_structure_y(common::con_cell f, uint32_t yn)
    {
        term t = deref(y(yn));
	return get_structure(f, t);
    }

    inline void get_structure(common::con_cell f, common::term t)
    {
        bool fail = false;
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
	} else {
	    goto_next_instruction();
	}
    }

    inline void get_list(uint32_t ai)
    {
        get_structure_a(empty_list_con(), ai);
    }

    inline void get_constant(common::term c, uint32_t ai)
    {
        bool fail = false;
        term t = deref(a(ai));
	switch (t.tag()) {
	case common::tag_t::REF: {
	  auto ref = static_cast<common::ref_cell &>(t);
	  bind(ref, c);
	  break;
  	  }
	case common::tag_t::CON: case common::tag_t::INT: {
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
	} else {
	    goto_next_instruction();
	}
    }

    inline void set_variable_a(uint32_t ai)
    {
        term t = new_ref();
	a(ai) = t;
	goto_next_instruction();
    }

    inline void set_variable_x(uint32_t xn)
    {
        term t = new_ref();
	x(xn) = t;
	goto_next_instruction();
    }

    inline void set_variable_y(uint32_t yn)
    {
        term t = new_ref();
	y(yn) = t;
	goto_next_instruction();
    }

    inline void set_value_a(uint32_t ai)
    {
        new_term_copy_cell(a(ai));
	goto_next_instruction();
    }

    inline void set_value_x(uint32_t xn)
    {
        new_term_copy_cell(x(xn));
	goto_next_instruction();
    }

    inline void set_value_y(uint32_t yn)
    {
        new_term_copy_cell(y(yn));
	goto_next_instruction();
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
	goto_next_instruction();
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
	goto_next_instruction();
    }

    inline void set_constant(common::term c)
    {
        new_term_copy_cell(c);
	goto_next_instruction();
    }

    inline void set_void(uint32_t n)
    {
        new_ref(n);
	goto_next_instruction();
    }

    inline void unify_variable_a(uint32_t ai)
    {
        switch (mode_) {
	case READ: a(ai) = heap_get(register_s_); break;
	case WRITE: a(ai) = new_ref(); break;
        }
	register_s_++;
	goto_next_instruction();
    }

    inline void unify_variable_x(uint32_t xn)
    {
        switch (mode_) {
	case READ: x(xn) = heap_get(register_s_); break;
	case WRITE: x(xn) = new_ref(); break;
        }
	register_s_++;
	goto_next_instruction();
    }

    inline void unify_variable_y(uint32_t yn)
    {
        switch (mode_) {
	case READ: y(yn) = heap_get(register_s_); break;
	case WRITE: y(yn) = new_ref(); break;
        }
	register_s_++;
	goto_next_instruction();
    }

    inline void unify_value_a(uint32_t ai)
    {
        bool fail = false;
        switch (mode_) {
	case READ: fail = !unify(a(ai), heap_get(register_s_)); break;
	case WRITE: new_term_copy_cell(a(ai)); break;
        }
	register_s_++;
	if (fail) {
	    backtrack();
	} else {
	    goto_next_instruction();
	}
    }

    inline void unify_value_x(uint32_t xn)
    {
        bool fail = false;
        switch (mode_) {
	case READ: fail = !unify(x(xn), heap_get(register_s_)); break;
	case WRITE: new_term_copy_cell(x(xn)); break;
        }
	register_s_++;
	if (fail) {
	    backtrack();
	} else {
	    goto_next_instruction();
	}
    }

    inline void unify_value_y(uint32_t yn)
    {
        bool fail = false;
        switch (mode_) {
	case READ: fail = !unify(y(yn), heap_get(register_s_)); break;
	case WRITE: new_term_copy_cell(y(yn)); break;
        }
	register_s_++;
	if (fail) {
	    backtrack();
	} else {
	    goto_next_instruction();
	}
    }

    inline void unify_local_value_x(uint32_t xn)
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
	} else {
	    goto_next_instruction();
	}
    }

    inline void unify_local_value_y(uint32_t xn)
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
	} else {
	    goto_next_instruction();
	}
    }

    inline void unify_constant(common::term c)
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
	    case common::tag_t::CON: case common::tag_t::INT: {
	        auto c1 = static_cast<common::term &>(t);
		fail = c != c1;
	      }
	    default:
	      fail = true;
	      break;
	    }
          }
	case WRITE: {
	    new_term_copy_cell(c);
	    break;
	  }
	}
	if (fail) {
	    backtrack();
	} else {
	    goto_next_instruction();
	}
    }

    inline void unify_void(uint32_t n)
    {
        switch (mode_) {
	case READ: register_s_ += n; break;
	case WRITE: new_ref(n); break;
        }
	goto_next_instruction();
    }

    inline void allocate()
    {
        word_t *new_e0;
	if (base(register_e_) > base(register_b_)) {
	    new_e0 = base(register_e_) + num_y(register_e_) + words<environment_t>();
	} else {
	    new_e0 = base(register_b_) + register_b_->arity +
	               words<choice_point_t>();
	}
	environment_t *new_e = reinterpret_cast<environment_t *>(new_e0);
	new_e->ce = register_e_;
	new_e->cp = register_cp_;
        register_e_ = new_e;

	goto_next_instruction();
    }

    inline void deallocate()
    {
        register_cp_ = register_e_->cp;
        register_e_ = register_e_->ce;
	goto_next_instruction();
    }

    inline void call(wam_instruction_base *p, size_t arity, uint32_t num_stack)
    {
        register_cp_ = next_instruction(register_p_);
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

    inline void allocate_choice_point(wam_instruction_base *p_else)
    {
        word_t *new_b0;
	if (base(register_e_) > base(register_b_)) {
	     new_b0 = base(register_e_->cp) + num_y(register_e_) + words<environment_t>();
	} else {
	  new_b0 = base(register_b_->cp) + register_b_->arity + words<choice_point_t>();
	}
	auto *new_b = reinterpret_cast<choice_point_t *>(new_b0);
	new_b->arity = num_of_args_;
	for (size_t i = 0; i < num_of_args_; i++) {
	    new_b->ai[i] = a(i);
	}
	new_b->ce = register_e_;
	new_b->cp = register_cp_;
	new_b->b = register_b_;
	new_b->bp = p_else;
	new_b->tr = trail_size();
	new_b->h = heap_size();
	new_b->b0 = register_b0_;
	register_b_ = new_b;
	set_register_hb(heap_size());
    }

    void retry_choice_point(wam_instruction_base *p_else)
    {
	size_t n = register_b_->arity;
	for (size_t i = 0; i < n; i++) {
	    a(i) = register_b_->ai[i];
	}
	register_e_ = register_b_->ce;
	register_cp_ = register_b_->cp;
	register_b_->bp = p_else;
	unwind_trail(register_b_->tr, trail_size());
	trim_trail(register_b_->tr);
	trim_heap(register_b_->h);
	set_register_hb(heap_size());
    }

    void trust_choice_point()
    {
	size_t n = register_b_->arity;
	for (size_t i = 0; i < n; i++) {
	    a(i) = register_b_->ai[i];
	}
	register_e_ = register_b_->ce;
	register_cp_ = register_b_->cp;
	unwind_trail(register_b_->tr, trail_size());
	trim_trail(register_b_->tr);
	trim_heap(register_b_->h);
	register_b_ = register_b_->b;
	set_register_hb(register_b_->h);
    }


    inline void try_me_else(wam_instruction_base *L)
    {
	allocate_choice_point(L);
	goto_next_instruction();
    }

    inline void retry_me_else(wam_instruction_base *L)
    {
	retry_choice_point(L);
	goto_next_instruction();
    }

    inline void trust_me()
    {
	trust_choice_point();
	goto_next_instruction();
    }

    inline void try_(wam_instruction_base *L)
    {
	allocate_choice_point(next_instruction(register_p_));
	register_p_ = L;
    }

    inline void retry(wam_instruction_base *L)
    {
	retry_choice_point(next_instruction(register_p_));
	register_p_ = L;
    }

    inline void trust(wam_instruction_base *L)
    {
	trust_choice_point();
	register_p_ = L;
    }

    inline void switch_on_term(wam_instruction_base *pv,
			       wam_instruction_base *pc,
			       wam_instruction_base *pl,
			       wam_instruction_base *ps)
    {
	term t = deref(a(0));
	switch (t.tag()) {
	case common::tag_t::CON: case common::tag_t::INT:
	    if (pc == nullptr) {
		backtrack();
	    } else {
		register_p_ = pc;
	    }
	    break;
	case common::tag_t::STR: {
	    if (is_dotted_pair(t)) {
		if (pl == nullptr) {
		    backtrack();
		} else {
		    register_p_ = pl;
		}
	    } else {
		if (ps == nullptr) {
		    backtrack();
		} else {
		    register_p_ = ps;
		}
	    }
	}
	case common::tag_t::REF:
	default: {
	    if (pv == nullptr) {
		backtrack();
	    } else {
		register_p_ = pv;
	    }
	    break;
	}
	}
    }

    inline void switch_on_constant(hash_map &map)
    {
	term t = deref(a(0));
	auto it = map.find(t);
	if (it == map.end()) {
	    backtrack();
	} else {
	    register_p_ = it->second;
	}
    }

    inline void switch_on_structure(hash_map &map)
    {
	term t = deref(a(0));
	auto it = map.find(t);
	if (it == map.end()) {
	    backtrack();
	} else {
	    register_p_ = it->second;
	}
    }

    inline void neck_cut()
    {
	if (register_b_ > register_b0_) {
	    register_b_ = register_b0_;
	    tidy_trail();
	}
	goto_next_instruction();
    }

    inline void get_level(uint32_t yn)
    {
	y(yn) = common::int_cell(to_stack_addr(base(register_b0_)));
	goto_next_instruction();
    }

    inline void cut(uint32_t yn)
    {
	auto b0 = to_stack(static_cast<common::int_cell &>(y(yn)).value())->cp;
	if (register_b_ > b0) {
	    register_b_ = b0;
	}
    }

    friend class test_wam_interpreter;
};

inline void wam_instruction_sequence::update(code_t *old_base, code_t *new_base)
{
    wam_instruction_base *instr = reinterpret_cast<wam_instruction_base *>(new_base);
    for (size_t i = 0; i < instrs_size_;) {
	instr = interp_.next_instruction(instr);
	instr->update(old_base, new_base);
	i = static_cast<size_t>(reinterpret_cast<code_t *>(instr) - new_base);
    }
}

template<> class wam_instruction<PUT_VARIABLE_X> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t xn, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data((static_cast<uint64_t>(xn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
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
      wam_instruction_base(&invoke, sizeof(*this)),
      data((static_cast<uint64_t>(yn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
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
      wam_instruction_base(&invoke, sizeof(*this)),
      data((static_cast<uint64_t>(xn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
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
      wam_instruction_base(&invoke, sizeof(*this)),
      data((static_cast<uint64_t>(yn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
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
      wam_instruction_base(&invoke, sizeof(*this)),
      data((static_cast<uint64_t>(yn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t yn() const { return data >> 32; }
    inline uint32_t ai() const { return data & 0xffffffff; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_UNSAFE_VALUE_Y> *>(self);
        interp.put_unsafe_value_y(self1->yn(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_UNSAFE_VALUE_Y> *>(self);
        out << "put_unsafe_value y" << self1->yn() << ", a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<PUT_STRUCTURE_A> : public wam_instruction_base {
public:
    inline wam_instruction(common::con_cell f, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      f_(f),
      data(ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline common::con_cell f() const { return f_; }
    inline size_t ai() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_STRUCTURE_A> *>(self);
        interp.put_structure_a(self1->f(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_STRUCTURE_A> *>(self);
        out << "put_structure " << interp.to_string(self1->f()) << "/" << self1->f().arity() << ", a" << self1->ai();
    }

    common::con_cell f_;
    uint64_t data;
};

template<> class wam_instruction<PUT_STRUCTURE_X> : public wam_instruction_base {
public:
    inline wam_instruction(common::con_cell f, uint32_t xn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      f_(f),
      data(xn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline common::con_cell f() const { return f_; }
    inline size_t xn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_STRUCTURE_X> *>(self);
        interp.put_structure_x(self1->f(), self1->xn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_STRUCTURE_X> *>(self);
        out << "put_structure " << interp.to_string(self1->f()) << "/" << self1->f().arity() << ", x" << self1->xn();
    }

    common::con_cell f_;
    uint64_t data;
};

template<> class wam_instruction<PUT_STRUCTURE_Y> : public wam_instruction_base {
public:
    inline wam_instruction(common::con_cell f, uint32_t yn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      f_(f),
      data(yn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline common::con_cell f() const { return f_; }
    inline size_t yn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_STRUCTURE_Y> *>(self);
        interp.put_structure_y(self1->f(), self1->yn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_STRUCTURE_Y> *>(self);
        out << "put_structure " << interp.to_string(self1->f()) << "/" << self1->f().arity() << ", y" << self1->yn();
    }

    common::con_cell f_;
    uint64_t data;
};

template<> class wam_instruction<PUT_LIST> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t ai() const { return data & 0xffffffff; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_LIST> *>(self);
        interp.put_list(self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_LIST> *>(self);
        out << "put_list " << "a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<PUT_CONSTANT> : public wam_instruction_base {
public:
    inline wam_instruction(common::term c, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      c_(c),
      data(ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }
    
    inline common::term c() const { return c_; }
    inline size_t ai() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_CONSTANT> *>(self);
        interp.put_constant(self1->c(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<PUT_CONSTANT> *>(self);
        out << "put_constant " << interp.to_string(self1->c()) << ", a" << self1->ai();
    }

    common::term c_;
    uint64_t data;
};

template<> class wam_instruction<GET_VARIABLE_X> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t xn, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data((static_cast<uint64_t>(xn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t xn() const { return data >> 32; }
    inline uint32_t ai() const { return data & 0xffffffff; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_VARIABLE_X> *>(self);
        interp.get_variable_x(self1->xn(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_VARIABLE_X> *>(self);
        out << "get_variable x" << self1->xn() << ", a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<GET_VARIABLE_Y> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t yn, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data((static_cast<uint64_t>(yn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t yn() const { return data >> 32; }
    inline uint32_t ai() const { return data & 0xffffffff; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_VARIABLE_Y> *>(self);
        interp.get_variable_y(self1->yn(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_VARIABLE_Y> *>(self);
        out << "get_variable y" << self1->yn() << ", a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<GET_VALUE_X> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t xn, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data((static_cast<uint64_t>(xn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t xn() const { return data >> 32; }
    inline uint32_t ai() const { return data & 0xffffffff; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_VALUE_X> *>(self);
        interp.get_value_x(self1->xn(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_VALUE_X> *>(self);
        out << "get_value x" << self1->xn() << ", a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<GET_VALUE_Y> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t yn, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data((static_cast<uint64_t>(yn) << 32) | ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t yn() const { return data >> 32; }
    inline uint32_t ai() const { return data & 0xffffffff; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_VALUE_Y> *>(self);
        interp.get_value_y(self1->yn(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_VALUE_Y> *>(self);
        out << "get_value y" << self1->yn() << ", a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<GET_STRUCTURE_A> : public wam_instruction_base {
public:
    inline wam_instruction(common::con_cell f, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      f_(f),
      data(ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline common::con_cell f() const { return f_; }
    inline size_t ai() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_STRUCTURE_A> *>(self);
        interp.get_structure(self1->f(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
      auto self1 = reinterpret_cast<wam_instruction<GET_STRUCTURE_A> *>(self);
        out << "get_structure " << interp.to_string(self1->f()) << "/" << self1->f().arity() << ", a" << self1->ai();
    }

    common::con_cell f_;
    uint64_t data;
};

template<> class wam_instruction<GET_STRUCTURE_X> : public wam_instruction_base {
public:
    inline wam_instruction(common::con_cell f, uint32_t xn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      f_(f),
      data(xn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline common::con_cell f() const { return f_; }
    inline size_t xn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_STRUCTURE_X> *>(self);
        interp.get_structure(self1->f(), self1->xn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
      auto self1 = reinterpret_cast<wam_instruction<GET_STRUCTURE_X> *>(self);
        out << "get_structure " << interp.to_string(self1->f()) << "/" << self1->f().arity() << ", x" << self1->xn();
    }

    common::con_cell f_;
    uint64_t data;
};

template<> class wam_instruction<GET_STRUCTURE_Y> : public wam_instruction_base {
public:
    inline wam_instruction(common::con_cell f, uint32_t yn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      f_(f),
      data(yn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline common::con_cell f() const { return f_; }
    inline size_t yn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_STRUCTURE_Y> *>(self);
        interp.get_structure(self1->f(), self1->yn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
      auto self1 = reinterpret_cast<wam_instruction<GET_STRUCTURE_Y> *>(self);
        out << "get_structure " << interp.to_string(self1->f()) << "/" << self1->f().arity() << ", y" << self1->yn();
    }

    common::con_cell f_;
    uint64_t data;
};

template<> class wam_instruction<GET_LIST> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t ai() const { return data & 0xffffffff; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_LIST> *>(self);
        interp.get_list(self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_LIST> *>(self);
        out << "get_list " << "a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<GET_CONSTANT> : public wam_instruction_base {
public:
    inline wam_instruction(common::term c, uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      c_(c),
      data(ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }
    
    inline common::term c() const { return c_; }
    inline size_t ai() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_CONSTANT> *>(self);
        interp.get_constant(self1->c(), self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_CONSTANT> *>(self);
        out << "get_constant " << interp.to_string(self1->c()) << ", a" << self1->ai();
    }

    common::term c_;
    uint64_t data;
};

template<> class wam_instruction<SET_VARIABLE_A> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t ai() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VARIABLE_A> *>(self);
        interp.set_variable_a(self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VARIABLE_A> *>(self);
        out << "set_variable a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<SET_VARIABLE_X> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t xn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(xn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t xn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VARIABLE_X> *>(self);
        interp.set_variable_x(self1->xn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VARIABLE_X> *>(self);
        out << "set_variable x" << self1->xn();
    }

    uint64_t data;
};

template<> class wam_instruction<SET_VARIABLE_Y> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t yn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(yn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t yn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VARIABLE_Y> *>(self);
        interp.set_variable_y(self1->yn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VARIABLE_Y> *>(self);
        out << "set_variable y" << self1->yn();
    }

    uint64_t data;
};

template<> class wam_instruction<SET_VALUE_A> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t ai() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VALUE_A> *>(self);
        interp.set_value_a(self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VALUE_A> *>(self);
        out << "set_value a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<SET_VALUE_X> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t xn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(xn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t xn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VALUE_X> *>(self);
        interp.set_value_x(self1->xn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VALUE_X> *>(self);
        out << "set_value x" << self1->xn();
    }

    uint64_t data;
};

template<> class wam_instruction<SET_VALUE_Y> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t yn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(yn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t yn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VALUE_Y> *>(self);
        interp.set_value_y(self1->yn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VALUE_Y> *>(self);
        out << "set_value y" << self1->yn();
    }

    uint64_t data;
};

template<> class wam_instruction<SET_LOCAL_VALUE_X> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t xn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(xn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t xn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_LOCAL_VALUE_X> *>(self);
        interp.set_local_value_x(self1->xn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_LOCAL_VALUE_X> *>(self);
        out << "set_local_value x" << self1->xn();
    }

    uint64_t data;
};

template<> class wam_instruction<SET_LOCAL_VALUE_Y> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t yn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(yn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t yn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_LOCAL_VALUE_Y> *>(self);
        interp.set_local_value_y(self1->yn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_LOCAL_VALUE_Y> *>(self);
        out << "set_local_value y" << self1->yn();
    }

    uint64_t data;
};

template<> class wam_instruction<SET_CONSTANT> : public wam_instruction_base {
public:
    inline wam_instruction(common::term c) :
      wam_instruction_base(&invoke, sizeof(*this)),
      c_(c) {
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }
    
    inline common::term c() const { return c_; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_CONSTANT> *>(self);
        interp.set_constant(self1->c());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_CONSTANT> *>(self);
        out << "set_constant " << interp.to_string(self1->c());
    }

    common::term c_;
};

template<> class wam_instruction<SET_VOID> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t n) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(n) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t n() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VOID> *>(self);
        interp.set_void(self1->n());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<SET_VOID> *>(self);
        out << "set_void " << self1->n();
    }

    uint64_t data;
};

template<> class wam_instruction<UNIFY_VARIABLE_A> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t ai() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VARIABLE_A> *>(self);
        interp.unify_variable_a(self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VARIABLE_A> *>(self);
        out << "unify_variable a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<UNIFY_VARIABLE_X> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t xn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(xn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t xn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VARIABLE_X> *>(self);
        interp.unify_variable_x(self1->xn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VARIABLE_X> *>(self);
        out << "unify_variable x" << self1->xn();
    }

    uint64_t data;
};

template<> class wam_instruction<UNIFY_VARIABLE_Y> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t yn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(yn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t yn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VARIABLE_Y> *>(self);
        interp.unify_variable_y(self1->yn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VARIABLE_Y> *>(self);
        out << "unify_variable y" << self1->yn();
    }

    uint64_t data;
};

template<> class wam_instruction<UNIFY_VALUE_A> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t ai) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(ai) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t ai() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VALUE_A> *>(self);
        interp.unify_value_a(self1->ai());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VALUE_A> *>(self);
        out << "unify_value a" << self1->ai();
    }

    uint64_t data;
};

template<> class wam_instruction<UNIFY_VALUE_X> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t xn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(xn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t xn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VALUE_X> *>(self);
        interp.unify_value_x(self1->xn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VALUE_X> *>(self);
        out << "unify_value x" << self1->xn();
    }

    uint64_t data;
};

template<> class wam_instruction<UNIFY_VALUE_Y> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t yn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(yn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t yn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VALUE_Y> *>(self);
        interp.unify_value_y(self1->yn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VALUE_Y> *>(self);
        out << "unify_value y" << self1->yn();
    }

    uint64_t data;
};

template<> class wam_instruction<UNIFY_LOCAL_VALUE_X> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t xn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(xn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t xn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_LOCAL_VALUE_X> *>(self);
        interp.unify_local_value_x(self1->xn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_LOCAL_VALUE_X> *>(self);
        out << "unify_local_value x" << self1->xn();
    }

    uint64_t data;
};

template<> class wam_instruction<UNIFY_LOCAL_VALUE_Y> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t yn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(yn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t yn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_LOCAL_VALUE_Y> *>(self);
        interp.unify_local_value_y(self1->yn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_LOCAL_VALUE_Y> *>(self);
        out << "unify_local_value y" << self1->yn();
    }

    uint64_t data;
};

template<> class wam_instruction<UNIFY_CONSTANT> : public wam_instruction_base {
public:
    inline wam_instruction(common::term c) :
      wam_instruction_base(&invoke, sizeof(*this)),
      c_(c) {
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }
    
    inline common::term c() const { return c_; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_CONSTANT> *>(self);
        interp.unify_constant(self1->c());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_CONSTANT> *>(self);
        out << "unify_constant " << interp.to_string(self1->c());
    }

    common::term c_;
};

template<> class wam_instruction<UNIFY_VOID> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t n) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(n) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t n() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VOID> *>(self);
        interp.unify_void(self1->n());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<UNIFY_VOID> *>(self);
        out << "unify_void " << self1->n();
    }

    uint64_t data;
};

template<> class wam_instruction<ALLOCATE> : public wam_instruction_base {
public:
    inline wam_instruction() :
      wam_instruction_base(&invoke, sizeof(*this)) {
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	static_cast<void>(self);
        interp.allocate();
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	static_cast<void>(interp);
	static_cast<void>(self);
        out << "allocate";
    }
};

template<> class wam_instruction<DEALLOCATE> : public wam_instruction_base {
public:
    inline wam_instruction() :
      wam_instruction_base(&invoke, sizeof(*this)) {
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	static_cast<void>(self);
        interp.deallocate();
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	static_cast<void>(interp);
	static_cast<void>(self);
        out << "deallocate";
    }
};

inline void wam_instruction<CALL>::invoke(wam_interpreter &interp, wam_instruction_base *self)
{
    auto self1 = reinterpret_cast<wam_instruction<CALL> *>(self);
    interp.call(self1->p(), self1->arity(), self1->num_y());
}

inline void wam_instruction<CALL>::print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
{
    auto self1 = reinterpret_cast<wam_instruction<CALL> *>(self);
    out << "call " << interp.to_string(self1->pn()) << "/" << self1->arity() << ", " << self1->num_y();
}

inline void wam_instruction<CALL>::updater(wam_instruction_base *self, code_t *old_base, code_t *new_base)
{
    auto self1 = reinterpret_cast<wam_instruction<CALL> *>(self);
    self1->update(old_base, new_base);
}

template<> class wam_instruction<EXECUTE> : public wam_instruction_base {
public:
      inline wam_instruction(common::con_cell pn, wam_instruction_base *p) :
      wam_instruction_base(&invoke, sizeof(*this)),
      pn_(pn), p_(p) {
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline wam_instruction_base * p() const { return p_; }
    inline common::con_cell pn() const { return pn_; }
    inline size_t arity() const { return pn().arity(); }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<EXECUTE> *>(self);
	interp.execute(self1->p(), self1->arity());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<EXECUTE> *>(self);
	out << "execute " << interp.to_string(self1->pn()) << "/" << self1->arity();
    }

    common::con_cell pn_;
    wam_instruction_base *p_;
};

template<> class wam_instruction<PROCEED> : public wam_instruction_base {
public:
    inline wam_instruction() :
      wam_instruction_base(&invoke, sizeof(*this)) {
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	static_cast<void>(self);
        interp.proceed();
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	static_cast<void>(interp);
	static_cast<void>(self);
        out << "proceed";
    }
};

template<> class wam_instruction<TRY_ME_ELSE> : public wam_instruction_base {
public:
      inline wam_instruction(wam_instruction_base *p) :
      wam_instruction_base(&invoke, sizeof(*this)),
      p_(p) {
      static bool init = [] {
 	    register_printer(&invoke, &print);
 	    register_updater(&invoke, &updater);
	    return true; } ();
      static_cast<void>(init);
    }

    inline wam_instruction_base * p() const { return p_; }

    inline void update(code_t *old_base, code_t *new_base)
    {
	p_ = update_ptr(p_, old_base, new_base);
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<TRY_ME_ELSE> *>(self);
	interp.try_me_else(self1->p());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<TRY_ME_ELSE> *>(self);
	out << "try_me_else L:" << interp.to_code_addr(self1->p());
    }

    static void updater(wam_instruction_base *self, code_t *old_base, code_t *new_base)
    {
	auto self1 = reinterpret_cast<wam_instruction<TRY_ME_ELSE> *>(self);
	self1->update(old_base, new_base);
    }

    wam_instruction_base *p_;
};

template<> class wam_instruction<RETRY_ME_ELSE> : public wam_instruction_base {
public:
      inline wam_instruction(wam_instruction_base *p) :
      wam_instruction_base(&invoke, sizeof(*this)),
      p_(p) {
      static bool init = [] {
	    register_printer(&invoke, &print);
	    register_updater(&invoke, &updater);
	    return true; } ();
      static_cast<void>(init);
    }

    inline wam_instruction_base * p() const { return p_; }

    inline void update(code_t *old_base, code_t *new_base)
    {
	p_ = update_ptr(p_, old_base, new_base);
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<RETRY_ME_ELSE> *>(self);
	interp.retry_me_else(self1->p());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<RETRY_ME_ELSE> *>(self);
	out << "retry_me_else L:" << interp.to_code_addr(self1->p());
    }

    static void updater(wam_instruction_base *self, code_t *old_base, code_t *new_base)
    {
	auto self1 = reinterpret_cast<wam_instruction<RETRY_ME_ELSE> *>(self);
	self1->update(old_base, new_base);
    }


    wam_instruction_base *p_;
};

template<> class wam_instruction<TRUST_ME> : public wam_instruction_base {
public:
    inline wam_instruction() :
      wam_instruction_base(&invoke, sizeof(*this)) {
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	static_cast<void>(self);
        interp.trust_me();
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	static_cast<void>(interp);
	static_cast<void>(self);
        out << "trust_me";
    }
};

template<> class wam_instruction<TRY> : public wam_instruction_base {
public:
      inline wam_instruction(wam_instruction_base *p) :
      wam_instruction_base(&invoke, sizeof(*this)),
      p_(p) {
      static bool init = [] {
	    register_printer(&invoke, &print);
	    register_updater(&invoke, &updater);
	    return true; } ();
      static_cast<void>(init);
    }

    inline wam_instruction_base * p() const { return p_; }

    inline void update(code_t *old_base, code_t *new_base)
    {
	p_ = update_ptr(p_, old_base, new_base);
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<TRY> *>(self);
	interp.try_(self1->p());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<TRY> *>(self);
	out << "try L:" << interp.to_code_addr(self1->p());
    }

    static void updater(wam_instruction_base *self, code_t *old_base, code_t *new_base)
    {
	auto self1 = reinterpret_cast<wam_instruction<TRY> *>(self);
	self1->update(old_base, new_base);
    }

    wam_instruction_base *p_;
};

template<> class wam_instruction<RETRY> : public wam_instruction_base {
public:
      inline wam_instruction(wam_instruction_base *p) :
      wam_instruction_base(&invoke, sizeof(*this)),
      p_(p) {
      static bool init = [] {
	    register_printer(&invoke, &print);
	    register_updater(&invoke, &updater);
	    return true; } ();
      static_cast<void>(init);
    }

    inline wam_instruction_base * p() const { return p_; }

    inline void update(code_t *old_base, code_t *new_base)
    {
	p_ = update_ptr(p_, old_base, new_base);
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<RETRY> *>(self);
	interp.retry(self1->p());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<RETRY> *>(self);
	out << "retry L:" << interp.to_code_addr(self1->p());
    }

    static void updater(wam_instruction_base *self, code_t *old_base, code_t *new_base)
    {
	auto self1 = reinterpret_cast<wam_instruction<RETRY> *>(self);
	self1->update(old_base, new_base);
    }

    wam_instruction_base *p_;
};

template<> class wam_instruction<TRUST> : public wam_instruction_base {
public:
      inline wam_instruction(wam_instruction_base *p) :
      wam_instruction_base(&invoke, sizeof(*this)),
      p_(p) {
      static bool init = [] {
	    register_printer(&invoke, &print);
	    register_updater(&invoke, &updater);
	    return true; } ();
      static_cast<void>(init);
    }

    inline wam_instruction_base * p() const { return p_; }

    inline void update(code_t *old_base, code_t *new_base)
    {
	p_ = update_ptr(p_, old_base, new_base);
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<TRUST> *>(self);
	interp.trust(self1->p());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<TRUST> *>(self);
	out << "trust L:" << interp.to_code_addr(self1->p());
    }

    static void updater(wam_instruction_base *self, code_t *old_base, code_t *new_base)
    {
	auto self1 = reinterpret_cast<wam_instruction<TRUST> *>(self);
	self1->update(old_base, new_base);
    }

    wam_instruction_base *p_;
};

template<> class wam_instruction<SWITCH_ON_TERM> : public wam_instruction_base {
public:
      inline wam_instruction(wam_instruction_base *pv,
			     wam_instruction_base *pc,
			     wam_instruction_base *pl,
			     wam_instruction_base *ps) :
      wam_instruction_base(&invoke, sizeof(*this)),
      pv_(pv), pc_(pc), pl_(pl), ps_(ps) {
      static bool init = [] {
	    register_printer(&invoke, &print);
	    register_updater(&invoke, &updater);
	    return true; } ();
      static_cast<void>(init);
    }

    inline wam_instruction_base * pv() const { return pv_; }
    inline wam_instruction_base * pc() const { return pc_; }
    inline wam_instruction_base * pl() const { return pl_; }
    inline wam_instruction_base * ps() const { return ps_; }

    inline void update(code_t *old_base, code_t *new_base)
    {
	pv_ = update_ptr(pv_, old_base, new_base);
	pc_ = update_ptr(pc_, old_base, new_base);
	pl_ = update_ptr(pl_, old_base, new_base);
	ps_ = update_ptr(ps_, old_base, new_base);
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<SWITCH_ON_TERM> *>(self);
	interp.switch_on_term(self1->pv(), self1->pc(), self1->pl(), self1->ps());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<SWITCH_ON_TERM> *>(self);
	out << "switch_on_term ";
        if (self1->pv() == nullptr) {
	    out << "V->fail";
	} else {
	    out << "V->L:" << interp.to_code_addr(self1->pv());
	}
	out << ", ";
	if (self1->pc() == nullptr) {
	    out << "C->fail";
	} else {
	    out << "C->L:" << interp.to_code_addr(self1->pc());
	}
	out << ", ";
	if (self1->pl() == nullptr) {
	    out << "L->fail";
	} else {
	    out << "L->L:" << interp.to_code_addr(self1->pl());
	}
	out << ", ";
	if (self1->ps() == nullptr) {
	    out << "S->fail";
	} else {
	    out << "S->L:" << interp.to_code_addr(self1->ps());
	}
    }

    static void updater(wam_instruction_base *self, code_t *old_base, code_t *new_base)
    {
	auto self1 = reinterpret_cast<wam_instruction<SWITCH_ON_TERM> *>(self);
	self1->update(old_base, new_base);
    }

    wam_instruction_base *pv_;
    wam_instruction_base *pc_;
    wam_instruction_base *pl_;
    wam_instruction_base *ps_;
};

template<> class wam_instruction<SWITCH_ON_CONSTANT> : public wam_instruction_base {
public:
      inline wam_instruction(wam_interpreter::hash_map *map) :
      wam_instruction_base(&invoke, sizeof(*this)),
      map_(map) {
      static bool init = [] {
	    register_printer(&invoke, &print);
	    register_updater(&invoke, &updater);
	    return true; } ();
      static_cast<void>(init);
    }

    inline wam_interpreter::hash_map & map() const { return *map_; }

    inline void update(code_t *old_base, code_t *new_base)
    {
	for (auto &v : map()) {
	    v.second = update_ptr(v.second, old_base, new_base);
	}
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<SWITCH_ON_CONSTANT> *>(self);
	interp.switch_on_constant(self1->map());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<SWITCH_ON_CONSTANT> *>(self);
	out << "switch_on_constant ";
	bool first = true;
	for (auto &v : self1->map()) {
	    if (!first) out << ", ";
	    out << interp.to_string(v.first) << "->L:" << interp.to_code_addr(v.second);
	    first = false;
	}
    }

    static void updater(wam_instruction_base *self, code_t *old_base, code_t *new_base)
    {
	auto self1 = reinterpret_cast<wam_instruction<SWITCH_ON_CONSTANT> *>(self);
	self1->update(old_base, new_base);
    }


    wam_interpreter::hash_map *map_;
};

template<> class wam_instruction<SWITCH_ON_STRUCTURE> : public wam_instruction_base {
public:
      inline wam_instruction(wam_interpreter::hash_map *map) :
      wam_instruction_base(&invoke, sizeof(*this)),
      map_(map) {
      static bool init = [] {
	    register_printer(&invoke, &print);
	    register_updater(&invoke, &updater);
	    return true; } ();
      static_cast<void>(init);
    }

    inline wam_interpreter::hash_map & map() const { return *map_; }

    inline void update(code_t *old_base, code_t *new_base)
    {
	for (auto &v : map()) {
	    v.second = update_ptr(v.second, old_base, new_base);
	}
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<SWITCH_ON_STRUCTURE> *>(self);
	interp.switch_on_structure(self1->map());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	auto self1 = reinterpret_cast<wam_instruction<SWITCH_ON_STRUCTURE> *>(self);
	out << "switch_on_structure ";
	bool first = true;
	for (auto &v : self1->map()) {
	    if (!first) out << ", ";
	    auto str = static_cast<const common::str_cell &>(v.first);
	    auto f = interp.functor(str);
	    out << interp.to_string(v.first) << "/" << f.arity() << "->L:" << interp.to_code_addr(v.second);
	    first = false;
	}
    }

    static void updater(wam_instruction_base *self, code_t *old_base, code_t *new_base)
    {
	auto self1 = reinterpret_cast<wam_instruction<SWITCH_ON_STRUCTURE> *>(self);
	self1->update(old_base, new_base);
    }


    wam_interpreter::hash_map *map_;
};

template<> class wam_instruction<NECK_CUT> : public wam_instruction_base {
public:
    inline wam_instruction() :
      wam_instruction_base(&invoke, sizeof(*this)) {
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
	static_cast<void>(self);
        interp.neck_cut();
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
	static_cast<void>(interp);
	static_cast<void>(self);
        out << "neck_cut";
    }
};

template<> class wam_instruction<GET_LEVEL> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t yn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(yn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t yn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_LEVEL> *>(self);
        interp.get_level(self1->yn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<GET_LEVEL> *>(self);
        out << "get_level y" << self1->yn();
    }

    uint64_t data;
};

template<> class wam_instruction<CUT> : public wam_instruction_base {
public:
    inline wam_instruction(uint32_t yn) :
      wam_instruction_base(&invoke, sizeof(*this)),
      data(yn) { 
      static bool init = [] {
	    register_printer(&invoke, &print); return true; } ();
      static_cast<void>(init);
    }

    inline uint32_t yn() const { return data; }

    static void invoke(wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<CUT> *>(self);
        interp.cut(self1->yn());
    }

    static void print(std::ostream &out, wam_interpreter &interp, wam_instruction_base *self)
    {
        auto self1 = reinterpret_cast<wam_instruction<CUT> *>(self);
        out << "cut y" << self1->yn();
    }

    uint64_t data;
};


}}

#endif
