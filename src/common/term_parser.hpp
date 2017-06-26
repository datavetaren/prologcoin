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
class term_parser {
public:
    term_parser(std::istream &in, heap &h, term_ops &ops);

private:
    term_tokenizer tokenizer_;
    heap &heap_;
    term_ops &ops_;
};

}}

#endif
