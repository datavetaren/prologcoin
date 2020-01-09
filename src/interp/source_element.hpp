#pragma once

#ifndef _interp_source_element_hpp
#define _interp_source_element_hpp

#include "../common/term_env.hpp"

namespace prologcoin { namespace interp {

class source_element {
  using con_cell = prologcoin::common::con_cell;
  using term = prologcoin::common::term;
  using token = prologcoin::common::term_tokenizer::token;

public:
  enum element_type {
    SOURCE_NONE,
    SOURCE_PREDICATE,
    SOURCE_COMMENT,
    SOURCE_ACTION
  };

  inline source_element() : element_type_(SOURCE_NONE) { }
  inline source_element(const source_element &other) {
    element_type_ = SOURCE_NONE;
    operator = (other);
  }
  inline source_element(con_cell pred) : element_type_(SOURCE_PREDICATE), predicate_(pred) { }
  inline source_element(term t) : element_type_(SOURCE_ACTION), action_(t) { }
  inline source_element(const token &comment) : element_type_(SOURCE_COMMENT), comment_(new token(comment)) { }
  inline ~source_element() { if (element_type_ == SOURCE_COMMENT) delete comment_; }

  inline source_element & operator = (const source_element &other) {
      if (element_type_ == SOURCE_NONE && other.element_type_ == SOURCE_COMMENT) {
	  comment_ = new token();
      }
      element_type_ = other.element_type_;
      switch (element_type_) {
      case SOURCE_NONE: break;
      case SOURCE_PREDICATE: predicate_ = other.predicate_; break;
      case SOURCE_COMMENT: *comment_ = *other.comment_; break;
      case SOURCE_ACTION: action_ = other.action_; break;
      }
      return *this;
  }

  inline element_type type() const { return element_type_; }
  inline const token & comment() const { return *comment_; }
  inline con_cell predicate() const { return predicate_; }
  inline term action() const { return action_; }

private:
  element_type element_type_;
  union {
    con_cell predicate_;
    token *comment_;
    term action_;
  };
};

}}

#endif
