#pragma once

#ifndef _common_term_env_hpp
#define _common_term_env_hpp

#include <iterator>
#include "term.hpp"
#include "term_emitter.hpp"

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
class term_env_impl; // Forward

class term_exception_not_list : public std::runtime_error {
public:
    term_exception_not_list(term t)
       : std::runtime_error("term is not a list"), term_(t) { }
    ~term_exception_not_list() noexcept(true) { }

    term get_term() { return term_; }

private:
    term term_;
};

class term_env;

class term_iterator : public std::iterator<std::forward_iterator_tag,
					   term,
					   term,
					   const term *,
					   const term &> {
public:
    inline term_iterator(term_env &env, const term &t) : env_(env), current_(t) { elem_ = first_of(t); }

    inline bool operator == (const term_iterator &other) const;
    inline bool operator != (const term_iterator &other) const
        { return ! operator == (other); }

    inline term_iterator & operator ++ () { term_iterator &it = *this;
	                                    it.advance(); return it; }
    inline reference operator * () const { return elem_; }
    inline pointer operator -> () const { return &elem_; }

    inline term_env & env() { return env_; }

private:
    inline term first_of(const term &t);
    void advance();

    term_env &env_;
    term current_;
    term elem_;
};

class term_dfs_iterator : public std::iterator<std::forward_iterator_tag,
					       term,
					       term,
					       const term *,
					       const term &> {
public:
    inline term_dfs_iterator(term_env &env, const term &t)
           : env_(env) { elem_ = first_of(t); }

    inline term_dfs_iterator(term_env &env)
	   : env_(env) { }

    inline bool operator == (const term_dfs_iterator &other) const;
    inline bool operator != (const term_dfs_iterator &other) const
        { return ! operator == (other); }

    inline term_dfs_iterator & operator ++ () { term_dfs_iterator &it = *this;
	                                        it.advance(); return it; }
    inline reference operator * () const { return elem_; }
    inline pointer operator -> () const { return &elem_; }

    inline term_env & env() { return env_; }

private:
    inline term first_of(const term &t);
    void advance();

    term_env &env_;
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

class term_env {
public:
  // Create a complete new blank term environment
  term_env();

  // Destroy term environment (don't free the heap/ops if they
  // aren't owned.)
  ~term_env();

  heap & get_heap();
  term_ops & get_ops();

  void sync_with_heap();

  term parse(const std::string &str);

  std::string to_string(const term &t, term_emitter::style style = term_emitter::STYLE_TERM) const;

  term empty_list() const;

  bool is_atom(const term &t) const;
  std::string atom_name(con_cell functor) const;
  std::string atom_name(const term &t) const;

  con_cell functor(const std::string &name, size_t arity);
  con_cell functor(const term &t);
  bool is_functor(const term &t, con_cell f);
  bool is_functor(const term &t);
  bool is_dotted_pair(const term &t) const;
  bool is_empty_list(const term &t) const;
  bool is_list(const term &t) const;
  bool is_comma(const term &t) const;
  term arg(const term &t, size_t index);
  term new_term(con_cell functor, const std::initializer_list<term> &args);

  bool unify(term &a, term &b);
  term copy(const term &t);
  bool equal(const term &a, const term &b);
  void set_last_choice_heap(size_t at_index);

  // Return -1, 0 or 1 when comparing standard order for 'a' and 'b'
  int standard_order(const term &a, const term &b);

  size_t allocate_stack(size_t num_cells);
  void ensure_stack(size_t at_index, size_t num_cells);
  cell * stack_ref(size_t at_index);
  void push(const term &t);
  term pop();
  
  void clear_name(const term &ref);
  void set_name(const term &ref, const std::string &name);

  term to_term(cell cell) const;

  size_t stack_size() const;
  size_t heap_size() const;
  size_t trail_size() const;

  void unwind_trail(size_t from_addr, size_t to_addr);
  void tidy_trail(size_t from_addr, size_t to_addr);

  void trim_heap(size_t new_size);
  void trim_trail(size_t new_size);

  term_dfs_iterator begin(const term &t);
  term_dfs_iterator end(const term &t);

  class term_range {
  public:
      term_range(term_env &env, const term &t) : env_(env), term_(t) { }

      term_dfs_iterator begin() { return env_.begin(term_); }
      term_dfs_iterator end() { return env_.end(term_); }
  private:
      term_env &env_;
      term term_;
  };

  term_range iterate_over(const term &t)
      { return term_range(*this, t); }

  std::string status() const;

private:
  friend class term_env_impl;
  term_env_impl *impl_;
};

inline bool term_iterator::operator == (const term_iterator &other) const
{
    return env_.equal(current_, other.current_);
}

inline term term_iterator::first_of(const term &t)
{
    if (t.is_void() || env_.is_empty_list(t)) {
	return t;
    } else {
	if (!env_.is_dotted_pair(t)) {
	    throw term_exception_not_list(t);
	}
	return env_.arg(t, 0);
    }
}

inline void term_iterator::advance()
{
    if ((*current_).tag() != common::tag_t::STR) {
	throw term_exception_not_list(current_);
    }
    current_ = env_.arg(current_, 1);
    elem_ = first_of(current_);
}

inline bool term_dfs_iterator::operator == (const term_dfs_iterator &other) const
{
    if (elem_.is_void() && other.elem_.is_void()) {
	return true;
    }
    return elem_ == other.elem_;
}

inline term term_dfs_iterator::first_of(const term &t)
{
    term p = t;
    while (p->tag() == tag_t::STR) {
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

inline void term_dfs_iterator::advance()
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

}}

#endif
