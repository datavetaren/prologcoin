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

class wam_instruction_base
{
protected:
    typedef void (*fn_type)(wam_interpreter &interp, wam_instruction_base *self);
    inline wam_instruction_base(fn_type fn, uint64_t sz_bytes)
      : fn_(fn), size_((sizeof(*this)+sz_bytes+sizeof(uint64_t)-1)/sizeof(uint64_t)) { }

    inline void invoke(wam_interpreter &interp) {
        fn_(interp, this);
    }

    inline size_t size() const { return size_; }

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
    { instrs_ = new uint64_t[instrs_capacity_]; }

    void add(const wam_instruction_base &i)
    {
        size_t sz = i.size();
        uint64_t *data = ensure_fit(sz);
	auto p = reinterpret_cast<wam_instruction_base *>(data);
	memcpy(p, &i, sz*sizeof(uint64_t));
    }

    void print(std::ostream &out);

private:
    uint64_t * ensure_fit(size_t sz)
    {
        ensure_capacity(instrs_size_ + sz);
	uint64_t *data = &instrs_[instrs_size_];
	instrs_size_ += sz;
	return data;
    }

    void ensure_capacity(size_t cap)
    {
        if (cap > instrs_capacity_) {
   	    size_t new_cap = 2*std::max(cap, instrs_size_);
	    uint64_t *new_instrs = new uint64_t[new_cap];
	    memcpy(new_instrs, instrs_, sizeof(uint64_t)*instrs_size_);
	    instrs_capacity_ = new_cap;
	    delete instrs_;
	    instrs_ = new_instrs;
        }
    }

    wam_interpreter &interp_;
    size_t instrs_size_;
    size_t instrs_capacity_;
    uint64_t *instrs_;
};

class wam_interpreter : public interpreter
{
public:
    wam_interpreter();

private:
    template<wam_instruction_type I> friend class wam_instruction;

    term register_xn_[1024];
    term register_yn_[1024];
    term register_ai_[256];

    inline void put_variable_x(uint32_t xn, uint32_t ai)
    {
        term ref = new_ref();
	register_xn_[xn] = ref;
	register_ai_[ai] = ref;
    }

    inline void put_variable_y(uint32_t yn, uint32_t ai)
    {
        term ref = new_ref();
	register_yn_[yn] = ref;
	register_ai_[ai] = ref;
    }

    inline void put_value_x(uint32_t xn, uint32_t ai)
    {
        register_ai_[ai] = register_xn_[xn];
    }

    inline void put_value_y(uint32_t yn, uint32_t ai)
    {
        register_ai_[ai] = register_yn_[yn];
    }

    inline void put_unsafe_value_y(uint32_t yn, uint32_t ai)
    {
        // register_ai_[ai] = register_yn_[yn];
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
