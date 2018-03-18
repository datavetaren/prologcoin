#pragma once

#ifndef _common_term_parser_hpp
#define _common_term_parser_hpp

#include <memory>
#include <vector>
#include "term.hpp"
#include "term_ops.hpp"
#include "term_tokenizer.hpp"

namespace prologcoin { namespace common {

class token_exception_unrecognized_operator : public token_exception
{
public:
    token_exception_unrecognized_operator(const token_position &pos, const std::string &msg) : token_exception(pos, msg) { }
};

class term_parse_exception : public ::std::runtime_error
{
public:
    term_parse_exception(const term_tokenizer::token &token,
			 const std::vector<std::string> &state_desc,
			 const std::vector<int> &expected_syms,
			 const std::string &msg)
	: ::std::runtime_error(msg), token_(token),
          state_description_(state_desc), expected_symbols_(expected_syms) { }

    const term_tokenizer::token &  token() const { return token_; }
    int line() const { return token().pos().line(); }
    int column() const { return token().pos().column(); }

    const std::vector<std::string> & state_description() const {
	return state_description_;
    }

    const std::vector<int> & expected_symbols() const {
	return expected_symbols_;
    }

private:
    term_tokenizer::token token_;
 
   std::vector<std::string> state_description_;
   std::vector<int> expected_symbols_;
};


//
// term_parser
//
// This class parses ASCII characters via a given tokenizer and builds
// a term on the provided heap (or errors.)
//
class term_parser_impl;
class term_env;

class term_parser {
public:
    term_parser(term_tokenizer &tokenizer, term_env &env);
    term_parser(term_tokenizer &tokenizer, heap &h, term_ops &ops);
    ~term_parser();

    void set_debug(bool dbg);

    term parse();

    term positions() const;
    int line(term position) const;
    int column(term position) const;
    term position_arg(term position, size_t arg) const;

    bool track_positions() const;
    void set_track_positions(bool b);

    term_tokenizer & tokenizer();
    const term_tokenizer::token & lookahead() const;
    const std::vector<term_tokenizer::token> & get_comments() const;
    std::string get_comments_string() const;

    std::vector<std::string> get_expected(const term_parse_exception &ex) const;

    bool is_eof();
    bool is_error();

    const std::string & get_variable_name( term t );

    void for_each_var_name(std::function<void (const term ref,
				       const std::string &name)> f) const;

    void clear_var_names();

private:
    term_parser_impl *impl_;
};

}}

#endif
