#pragma once

#ifndef _common_term_env_hpp
#define _common_term_env_hpp

#include "term.hpp"

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
class term_env {
public:
  // Create a complete new blank term environment
  term_env();

  // Destroy term environment (don't free the heap/ops if they
  // aren't owned.)
  ~term_env();

  ext<cell> parse(const std::string &str);
  std::string to_string(ext<cell> &cell) const;

  bool unify(ext<cell> &a, ext<cell> &b);

  size_t stack_size() const;
  size_t heap_size() const;
  size_t trail_size() const;

  std::string status() const;

private:
  friend class term_env_impl;
  term_env_impl *impl_;
};

}}

#endif
