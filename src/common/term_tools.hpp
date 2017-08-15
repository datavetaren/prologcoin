#pragma once

#ifndef _common_term_tools_hpp
#define _common_term_tools_hpp

#include <istream>
#include "term.hpp"
#include "term_tokenizer.hpp"

namespace prologcoin { namespace common {

class term_token_diff {
public:
    term_token_diff(std::istream &in1, std::istream &in2);

    bool check();

    const std::string & line1() const { return line1_; }
    const std::string & line2() const { return line2_; }

    int line1_no() const { return line1_no_; }
    int line2_no() const { return line2_no_; }

private:
    void skip_whitespace(term_tokenizer &tokens, int &line_no, std::string &line);

    std::string line1_;
    std::string line2_;

    int line1_no_;
    int line2_no_;

    term_tokenizer tokens1_;
    term_tokenizer tokens2_;
};

}}

#endif

