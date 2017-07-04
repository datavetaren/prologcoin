#pragma once

#ifndef _common_term_parser_hpp
#define _common_term_parser_hpp

#include <memory>
#include <vector>
#include "term.hpp"
#include "term_ops.hpp"
#include "term_tokenizer.hpp"

namespace prologcoin { namespace common {

class token_exception_unrecognized_operator : public ::std::runtime_error
{
public:
    token_exception_unrecognized_operator(const token_position &pos, const std::string &msg) : ::std::runtime_error(msg), pos_(pos) { }

    const token_position & pos() const { return pos_; }

private:
    token_position pos_;
};

//
// term_parser
//
// This class parses ASCII characters and builds a term (or errors)
//
// XXX: Parked this for now. I'll finish the (LA)LR state machine generator
//      in Prolog first.
//
class term_parser_impl;

class term_parser {
public:
    term_parser(term_tokenizer &tokenizer, heap &h, term_ops &ops);
    ~term_parser();

    void process_next();

private:
    term_parser_impl *impl_;
};

}}

#endif
