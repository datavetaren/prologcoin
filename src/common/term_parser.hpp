#pragma once

#ifndef _common_term_parser_hpp
#define _common_term_parser_hpp

#include <vector>
#include "term.hpp"
#include "term_ops.hpp"
#include "term_tokenizer.hpp"

namespace prologcoin { namespace common {

//
// term_parser
//
// This class parses ASCII characters and builds a term (or errors)
//
// XXX: Parked this for now. I'll finish the (LA)LR state machine generator
//      in Prolog first.
//
class term_parser {
public:
    term_parser(term_tokenizer &tokenizer, heap &h, term_ops &ops);

    void set_debug(bool d);

    void process_next();

    inline term_tokenizer & tokenizer() { return tokenizer_; }

private:
    typedef term_tokenizer::token token;

    void shift(const token &t);

    term_tokenizer &tokenizer_;
    heap &heap_;
    term_ops &ops_;
    bool debug_;

    size_t state_;
};

}}

#endif
