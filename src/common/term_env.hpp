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

class term_env {
public:
  // Create a complete new blank term environment
  term_env();

  // Destroy term environment (don't free the heap/ops if they
  // aren't owned.)
  ~term_env();

  term parse(const std::string &str);
  std::string to_string(const term &t, term_emitter::style style = term_emitter::STYLE_TERM) const;

  term empty_list() const;

  std::string atom_name(con_cell functor) const;

  con_cell functor(const term &t);
  bool is_functor(const term &t, con_cell f);
  bool is_functor(const term &t);
  bool is_dotted_pair(const term &t) const;
  bool is_empty_list(const term &t) const;
  bool is_list(const term &t) const;
  bool is_comma(const term &t) const;
  term arg(const term &t, size_t index);

  bool unify(term &a, term &b);
  term copy(const term &t);
  bool equal(const term &a, const term &b);

  size_t allocate_stack(size_t num_cells);
  void ensure_stack(size_t at_index, size_t num_cells);
  cell * stack_ref(size_t at_index);
  void push(const term &t);
  term pop();

  term to_term(cell cell) const;

  size_t stack_size() const;
  size_t heap_size() const;
  size_t trail_size() const;

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


}}

#endif
