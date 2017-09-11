#pragma once

#ifndef _common_term_env_hpp
#define _common_term_env_hpp

#include <iterator>
#include "term.hpp"
#include "term_emitter.hpp"
#include "term_parser.hpp"
#include "term_tokenizer.hpp"

namespace prologcoin { namespace common {

//
// This is the environment that terms need when dealing with
// some basic operations.  It consists of a heap, a constant table
// (if constant names are bigger than 6 characters,) a stack and a trail.
//
// One of the core operations is unificiation. Variables may become
// bound during the unification process and thus need to be recorded so
// they can become unbound.
//
class term_exception_not_list : public std::runtime_error {
public:
    term_exception_not_list(term t)
       : std::runtime_error("term is not a list"), term_(t) { }
    ~term_exception_not_list() noexcept(true) { }

    term get_term() { return term_; }

private:
    term term_;
};

template<typename HT, typename ST, typename OT> class term_env_dock;

template<typename HT, typename ST, typename OT>
class term_iterator_templ : public std::iterator<std::forward_iterator_tag,
						 term,
						 term,
						 const term *,
						 const term &> {
public:
    typedef term_env_dock<HT,ST,OT> Env;
    inline term_iterator_templ(Env &env, const term t)
         : env_(env), current_(t) { elem_ = first_of(t); }

    inline bool operator == (const term_iterator_templ &other) const;
    inline bool operator != (const term_iterator_templ &other) const
        { return ! operator == (other); }

    inline term_iterator_templ & operator ++ ()
        { term_iterator_templ &it = *this; it.advance(); return it; }
    inline reference operator * () const { return elem_; }
    inline pointer operator -> () const { return &elem_; }

    inline Env & env() { return env_; }

private:
    inline term first_of(const term t);
    void advance();

    Env &env_;
    term current_;
    term elem_;
};

template<typename HT, typename ST, typename OT>
class term_dfs_iterator_templ : public std::iterator<std::forward_iterator_tag,
						     term,
						     term,
						     const term *,
						     const term &> {
public:
    typedef term_env_dock<HT,ST,OT> Env;

    inline term_dfs_iterator_templ(Env &env, const term &t)
           : env_(env) { elem_ = first_of(t); }

    inline term_dfs_iterator_templ(Env &env)
	   : env_(env) { }

    inline bool operator == (const term_dfs_iterator_templ &other) const;
    inline bool operator != (const term_dfs_iterator_templ &other) const
        { return ! operator == (other); }

    inline term_dfs_iterator_templ & operator ++ ()
        { term_dfs_iterator_templ &it = *this;
	  it.advance(); return it; }
    inline reference operator * () const { return elem_; }
    inline pointer operator -> () const { return &elem_; }

    inline Env & env() { return env_; }

private:
    inline term first_of(const term t);
    void advance();

    Env &env_;
    term elem_;

    struct dfs_pos {
	dfs_pos(const term &p, size_t i, size_t n) :
	    parent(p), index(i), arity(n) { }

	term parent;
	size_t index;
	size_t arity;
    };

    std::vector<dfs_pos> stack_;
};

template<typename T> class heap_dock : public T {
public:
    inline heap_dock() { }

    // Heap management
    inline void heap_set(size_t index, term t)
        { T::get_heap()[index] = t; }
    inline term heap_get(size_t index)
        { return T::get_heap()[index]; }

    // Term management
    inline term new_ref()
        { return T::get_heap().new_ref(); }
    inline term deref(const term t) const
        { return T::get_heap().deref(t); }
    inline con_cell functor(const term t) const
        { return T::get_heap().functor(t); }
    inline term arg(const term t, size_t index) const
        { return T::get_heap().arg(t, index); }
    inline void set_arg(term t, size_t index, const term arg)
        { return T::get_heap().set_arg(t, index, arg); }
    inline void trim_heap(size_t new_size)
        { return T::get_heap().trim(new_size); }
    inline size_t heap_size() const
        { return T::get_heap().size(); }

    // Term creation
    inline term empty_list()
        { return T::get_heap().empty_list(); }
    inline con_cell functor(const std::string &name, size_t arity)
        { return T::get_heap().functor(name, arity); }
    inline term new_term(con_cell functor)
        { return T::get_heap().new_str(functor); }
    inline term new_term(con_cell functor, std::initializer_list<term> args)
        { term t = new_term(functor);
          size_t i = 0;
	  for (auto arg : args) {
	      T::get_heap().set_arg(t, i, arg);
	      i++;
	  }
	  return t;
	}
    inline term new_dotted_pair(term a, term b)
        { return T::get_heap().new_dotted_pair(a,b); }
    inline con_cell to_atom(con_cell functor)
        { return T::get_heap().to_atom(functor); }
    inline con_cell to_functor(con_cell atom, size_t arity)
        { return T::get_heap().to_functor(atom, arity); }

    // Term functions
    inline size_t list_length(term lst)
        { return T::get_heap().list_length(lst); }
    inline std::string atom_name(con_cell f) const
        { return T::get_heap().atom_name(f); }

    // Term predicates
    inline bool is_dotted_pair(term t) const
       { return T::get_heap().is_dotted_pair(t); }
    inline bool is_empty_list(term t) const
       { return T::get_heap().is_empty_list(t); }
    inline bool is_comma(term t) const
       { return T::get_heap().is_comma(t); }
    inline bool is_list(term t) const
       { return T::get_heap().is_list(t); }
};

class heap_bridge
{
public:
    inline heap_bridge() : heap_(nullptr) { }

    inline heap & get_heap() { return *heap_; }
    inline const heap & get_heap() const { return *heap_; }
    inline void set_heap(heap &h) { heap_ = &h; }
private:
    heap * heap_;
};

class heap_proxy : public heap_dock<heap_bridge> 
{
public:
    inline heap_proxy(heap &h) { set_heap(h); }
};

class stacks {
public:
    inline stacks & get_stacks() { return *this; }
    inline const stacks & get_stacks() const { return *this; }
    inline std::vector<term> & get_stack() { return stack_; }
    inline const std::vector<term> & get_stack() const { return stack_; }
    inline std::vector<size_t> & get_trail() { return trail_; }
    inline const std::vector<size_t> & get_trail() const { return trail_; }
    inline std::vector<term> & get_temp() { return temp_; }
    inline const std::vector<term> & get_temp() const { return temp_; }
    inline size_t get_register_hb() const { return register_hb_; }
    inline void set_register_hb(size_t hb) { register_hb_ = hb; }

private:
    std::vector<term> stack_;
    std::vector<size_t> trail_;
    std::vector<term> temp_;
    size_t register_hb_;
};

template<typename T> class stacks_dock : public T {
public:
  inline stacks_dock() { }

  inline stacks & get_stacks()
      { return T::get_stacks(); }

  inline size_t allocate_stack(size_t num_cells)
      { size_t at_index = T::get_stack().size();
	T::get_stack().resize(at_index+num_cells);
	return at_index;
      }
  inline void ensure_stack(size_t at_index, size_t num_cells)
      { if (at_index + num_cells > T::get_stack().size()) {
	    allocate_stack(at_index+num_cells-T::get_stack().size());
	}
      }
  inline term * stack_ref(size_t at_index)
      { return &T::get_stack()[at_index]; }

  inline void push(const term t)
      { T::get_stack().push_back(t); }
  inline term pop()
      { term t = T::get_stack().back(); T::get_stack().pop_back(); return t; }
  inline size_t stack_size() const
      { return T::get_stack().size(); }
  inline void trim_stack(size_t new_size)
      { T::get_stack().resize(new_size); }

  inline void push_trail(size_t i)
      { T::get_trail().push_back(i); }
  inline size_t pop_trail()
      { auto p = T::get_trail().back(); T::get_trail().pop_back(); return p; }
  inline size_t trail_size() const
      { return T::get_trail().size(); }
  inline void trim_trail(size_t to)
      { T::get_trail().resize(to); }
  inline size_t trail_get(size_t i) const
      { return T::get_trail()[i]; }
  inline void trail_set(size_t i, size_t val)
      { T::get_trail()[i] = val; }
  inline void trail(size_t index)
      { if (index < T::get_register_hb()) {
	    push_trail(index);
	}
      }

  inline void temp_push(const term t)
      { T::get_temp().push_back(t); }
  inline term temp_pop()
      { auto t = T::get_temp().back(); T::get_temp().pop_back(); return t; }
};

class stacks_bridge
{
public:
    inline stacks_bridge() : stacks_(nullptr) { }

    inline stacks & get_stacks() { return *stacks_; }
    inline const stacks & get_stacks() const { return *stacks_; }
    inline void set_stacks(stacks &s) { stacks_ = &s; }

    inline std::vector<term> & get_stack() { return get_stacks().get_stack(); }
    inline const std::vector<term> & get_stack() const { return get_stacks().get_stack(); }
    inline std::vector<size_t> & get_trail() { return get_stacks().get_trail(); }
    inline const std::vector<size_t> & get_trail() const { return get_stacks().get_trail(); }

    inline std::vector<term> & get_temp() { return get_stacks().get_temp(); }
    inline const std::vector<term> & get_temp() const { return get_stacks().get_temp(); }

    inline size_t get_register_hb() { return get_stacks().get_register_hb(); }
    inline void set_register_hb(size_t index) { get_stacks().set_register_hb(index); }

private:
    stacks *stacks_;
};

class stacks_proxy : public stacks_dock<stacks_bridge> 
{
public:
    inline stacks_proxy(stacks &s) { set_stacks(s); }
};

class term_utils : public heap_proxy, stacks_proxy {
public:
    term_utils(heap &h, stacks &s) : heap_proxy(h), stacks_proxy(s) { }

    bool unify(term a, term b);
    term copy(const term t);
    bool equal(const term a, const term b);

    // Return -1, 0 or 1 when comparing standard order for 'a' and 'b'
    int standard_order(const term a, const term b);

private:
    bool unify_helper(term a, term b);
    int functor_standard_order(con_cell a, con_cell b);

    inline void bind(const ref_cell &a, term b)
    {
        size_t index = a.index();
        heap_set(index, b);
        trail(index);
    }

    inline void unwind_trail(size_t from, size_t to)
    {
        for (size_t i = from; i < to; i++) {
  	    size_t index = trail_get(i);
            heap_set(index, ref_cell(index));
        }
    }
};

template<typename T> class ops_dock : public T
{
public:
    ops_dock() { }
    ops_dock(T &t) : T(t) { }
};

class ops_bridge
{
public:
    inline ops_bridge(term_ops &ops) : ops_(ops) { }

    inline term_ops & get_ops() { return ops_; }
    inline const term_ops & get_ops() const { return ops_; }

    inline void set_ops(term_ops &ops) { ops_ = ops; }

private:
    term_ops ops_;
};

class ops_proxy : public ops_dock<ops_bridge> 
{
public:
    inline ops_proxy(term_ops &ops)
        : ops_dock<ops_bridge>(*this) { set_ops(ops); }
};


template<typename HT, typename ST, typename OT> class term_env_dock
  : public heap_dock<HT>, public stacks_dock<ST>, public ops_dock<OT>
{
public:
  inline term_env_dock() : register_hb_(0) { }

  inline bool is_atom(const term t) const
      { return is_functor(t) && heap_dock<HT>::functor(t).arity() == 0; }

  inline std::string atom_name(const term t) const
      { return heap_dock<HT>::atom_name(heap_dock<HT>::functor(t)); }

  inline bool is_functor(const term t) const
      { auto tt = heap_dock<HT>::deref(t).tag(); return tt == tag_t::CON || tt == tag_t::STR; }
	 
  inline bool is_functor(const term t, con_cell f)
     { return is_functor(t) && heap_dock<HT>::functor(t) == f; }

  inline bool is_ground(const term t) const
     {
        auto range = const_cast<term_env_dock<HT,ST,OT> &>(*this).iterate_over(t);
        for (auto t1 : range) {
   	    if (t1.tag() == tag_t::REF) {
	        return false;
	    }
        }
        return true;    
     }

  inline size_t get_register_hb() const
     { return register_hb_; }

  inline void set_register_hb(size_t index)
     { register_hb_ = index; }
  
  inline void clear_name(const term t)
     { var_naming_.erase(t); }
  inline bool has_name(const term t) const
     { return var_naming_.count(t) != 0; }
  inline void set_name(const term t, const std::string &name)
     { var_naming_[t] = name; }


  inline void unwind_trail(size_t from, size_t to)
  {
      for (size_t i = from; i < to; i++) {
	  size_t index = stacks_dock<ST>::trail_get(i);
          heap_dock<HT>::heap_set(index, ref_cell(index));
      }
  }

  inline void tidy_trail(size_t from, size_t to)
  {
      size_t i = from;
      while (i < to) {
   	  if (stacks_dock<ST>::trail_get(i) < register_hb_) {
   	      // This variable recording happened before the choice point.
	      // We can't touch it.
	      i++;
          } else {
	      // Remove this trail point, move one trail point we haven't
	      // visited to this location.
  	      stacks_dock<ST>::trail_set(i, stacks_dock<ST>::trail_get(to-1));
	      to--;
          }
      }

      // We're done. Trim the trail to the new end
      stacks_dock<ST>::trim_trail(to);
  }

  inline bool unify(term a, term b)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks());
      return utils.unify(a, b);
  }

  inline term copy(term t)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks());
      return utils.copy(t);
  }

  inline bool equal(const term a, const term b)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks());
      return utils.equal(a, b);
  }

  inline int standard_order(const term a, const term b)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks());
      return utils.standard_order(a, b);
  }

  class term_range {
  public:
      term_range(term_env_dock &env, const term t) : env_(env), term_(t) { }

      term_dfs_iterator_templ<HT,ST,OT> begin() { return env_.begin(term_); }
      term_dfs_iterator_templ<HT,ST,OT> end() { return env_.end(term_); }
  private:
      term_env_dock &env_;
      term term_;
  };

  term_range iterate_over(const term t)
      { return term_range(*this, t); }

  std::string status() const
  { 
    std::stringstream ss;
    ss << "term_env::status() { heap_size=" << heap_dock<HT>::heap_size()
       << ",stack_size=" << stacks_dock<ST>::stack_size() << ",trail_size="
       << stacks_dock<ST>::trail_size() <<"}";
    return ss.str();
  }

  term parse(const std::string &str)
  {
      std::stringstream ss(str);
      term_tokenizer tokenizer(ss);
      term_parser parser(tokenizer, heap_dock<HT>::get_heap(),
			 ops_dock<OT>::get_ops());
      term r = parser.parse();

      // Once parsing is done we'll copy over the var-name bindings
      // so we can pretty print the variable names.
      parser.for_each_var_name( [&](const term  &ref,
				    const std::string &name)
			     { var_naming_[ref] = name; } );
      return r;
  }

  std::string to_string(const term t,
			term_emitter::style style = term_emitter::STYLE_TERM) const
  {
      term t1 = heap_dock<HT>::deref(t);
      std::stringstream ss;
      term_emitter emitter(ss, heap_dock<HT>::get_heap(),
			   ops_dock<OT>::get_ops());
      emitter.set_style(style);
      emitter.set_var_naming(var_naming_);
      emitter.print(t1);
      return ss.str();
  }

  std::string safe_to_string(const term t, term_emitter::style style = term_emitter::STYLE_TERM) const
  {
      return to_string(t, style);
  }

  term_dfs_iterator_templ<HT,ST,OT> begin(const term t)
  {
      return term_dfs_iterator_templ<HT,ST,OT>(*this, t);
  }
  
  term_dfs_iterator_templ<HT,ST,OT> end(const term t)
  {
      return term_dfs_iterator_templ<HT,ST,OT>(*this);
  }


private:
  size_t register_hb_;
  std::unordered_map<term, std::string> var_naming_;
};

template<typename HT, typename ST, typename OT>
inline bool term_iterator_templ<HT,ST,OT>::operator == (const term_iterator_templ<HT,ST,OT> &other) const
{
    return env_.equal(current_, other.current_);
}

template<typename HT, typename ST, typename OT>
inline term term_iterator_templ<HT,ST,OT>::first_of(const term t)
{
    if (env_.is_empty_list(t)) {
	return t;
    } else {
	if (!env_.is_dotted_pair(t)) {
	    throw term_exception_not_list(t);
	}
	return env_.arg(t, 0);
    }
}

template<typename HT, typename ST, typename OT>
inline void term_iterator_templ<HT,ST,OT>::advance()
{
    term t = env_.deref(current_);
    if (t.tag() != common::tag_t::STR) {
	throw term_exception_not_list(current_);
    }
    current_ = env_.arg(current_, 1);
    elem_ = first_of(current_);
}

template<typename HT, typename ST, typename OT>
inline bool term_dfs_iterator_templ<HT,ST,OT>::operator == (const term_dfs_iterator_templ<HT,ST,OT> &other) const
{
    return elem_ == other.elem_ && stack_.size() == other.stack_.size();
}

template<typename HT, typename ST, typename OT>
inline term term_dfs_iterator_templ<HT,ST,OT>::first_of(const term t)
{
    term p = env_.deref(t);
    while (p.tag() == tag_t::STR) {
	con_cell f = env_.functor(p);
	size_t arity = f.arity();
	if (arity > 0) {
	    stack_.push_back(dfs_pos(p, 0, arity));
	    p = env_.arg(p, 0);
	} else {
	    return p;
	}
    }
    return p;
}

template<typename HT, typename ST, typename OT>
inline void term_dfs_iterator_templ<HT,ST,OT>::advance()
{
    if (stack_.empty()) {
	elem_ = term();
	return;
    }

    size_t new_index = ++stack_.back().index;
    if (new_index == stack_.back().arity) {
	elem_ = stack_.back().parent;
	stack_.pop_back();
	return;
    }
    elem_ = first_of(env_.arg(stack_.back().parent, new_index));
}

typedef term_iterator_templ<heap, stacks, term_ops> term_iterator;

class term_env : public term_env_dock<heap, stacks, term_ops>
{
public:
     term_env() : term_env_dock() { }
};

}}

#endif
