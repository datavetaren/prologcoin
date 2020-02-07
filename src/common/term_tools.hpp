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
    void report();

    static void assert_equal(const std::string &s1, const std::string &s2,
			     const std::string &comment = "");
    static bool check(const std::string &s1, const std::string &s2);

    const std::string & line1() const { return line1_; }
    const std::string & line2() const { return line2_; }

    int line1_no() const { return line1_no_; }
    int line2_no() const { return line2_no_; }

private:
    void skip_whitespace(term_tokenizer &tokens, int &line_no, std::string &line);
    bool check_impl();

    std::string line1_;
    std::string line2_;

    int line1_no_;
    int line2_no_;

    term_tokenizer tokens1_;
    term_tokenizer tokens2_;

    term_tokenizer::token token1_;
    term_tokenizer::token token2_;

    bool done_;
    bool failed_;
};

}}

#endif

