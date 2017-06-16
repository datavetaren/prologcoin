#pragma once

#ifndef _common_term_emitter_hpp
#define _common_term_emitter_hpp

#include <stack>
#include "term.hpp"

namespace prologcoin { namespace common {

//
// term_emitter
//
// This class emits a term into a sequence of ASCII characters.
//
class term_emitter {
public:
    inline term_emitter(std::ostream &out, heap &h)
        : out_(out),
	  heap_(h),
          column_(0),
          line_(0),
          indent_(0) { }

    void print(cell c);

private:
    void print_from_stack();

    std::ostream &out_;
    heap & heap_;

    size_t column_;
    size_t line_;
    size_t indent_;

    std::stack<cell> stack_;
};

}}

#endif
