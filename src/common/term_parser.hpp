#pragma once

#ifndef _common_term_parser_hpp
#define _common_term_parser_hpp

#include <vector>
#include "term.hpp"
#include "term_ops.hpp"

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
    std::istream &in_;
    heap &heap_;
    term_ops &ops_;
    size_t column_;
    size_t line_;
};

}}

#endif
