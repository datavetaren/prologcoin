#pragma once

#ifndef _common_term_tokenizer_hpp
#define _common_term_tokenizer_hpp

#include <vector>
#include <exception>
#include <string>
#include "term.hpp"
#include "term_ops.hpp"
#include "token_chars.hpp"

namespace prologcoin { namespace common {

class token_exception : public ::std::runtime_error
{
public:
    token_exception(const std::string &msg) : ::std::runtime_error(msg) { }
};

class token_exception_control_char : public token_exception
{
public:
    token_exception_control_char(const std::string &msg)
       : token_exception(msg) { }
};

class token_exception_hex_code : public token_exception
{
public:
    token_exception_hex_code(const std::string &msg)
       : token_exception(msg) { }
};

class token_exception_no_char_code : public token_exception
{
public:
    token_exception_no_char_code(const std::string &msg)
        : token_exception(msg) { }
};


//
// term_tokenizer
//
// This class parses ASCII characters and builds a term (or errors)
//
class term_tokenizer : public token_chars {
public:
    term_tokenizer(std::istream &in, term_ops &ops);

    enum token_type {
	TOKEN_NAME,
	TOKEN_NATURAL_NUMBER,
	TOKEN_UNSIGNED_FLOAT,
	TOKEN_VARIABLE,
	TOKEN_STRING,
	TOKEN_PUNCTUATION_CHAR,
	TOKEN_LAYOUT_TEXT,
	TOKEN_FULL_STOP
    };

    struct token {
	token_type type_;
	std::string lexeme_;

	const std::string & lexeme() const { return lexeme_; }

	void reset() { lexeme_.clear(); }
    };

    bool has_more_tokens() const {
	if (peek_char() == -1) {
	    return false;
	}
	return !in_.eof();
    }

    const token & next_token();

private:
    inline int next_char()
    {
	return in_.get();
    }

    inline int next_char_la() const
    {
	return in_.get();
    }

    inline void unget_char() const
    {
	in_.unget();
    }

    inline int peek_char() const
    {
	return in_.peek();
    }

    bool is_eof() const
    {
	return in_.eof();
    }

    inline void set_token_type(token_type tt)
    {
	current_.type_ = tt;
    }

    inline void add_to_lexeme(int ch)
    {
	current_.lexeme_ += static_cast<char>(ch);
    }

    inline void consume_next_char()
    {
	add_to_lexeme(next_char());
    }

    bool is_comment_begin() const;
    bool is_full_stop() const;
    bool is_full_stop(int ch) const;
    void next_xs( const std::function<bool(int)> &predicate );
    void next_digits();
    void next_char_code();
    void next_number();
    void next_float();
    void next_decimal();
    void next_exponent();
    void next_alphas();
    void next_quoted_name();
    void next_escape_sequence();
    void next_word();
    void next_symbol();
    void next_variable();
    void next_layout_text();
    void next_solo_char();
    void next_punctuation_char();
    void next_string();
    void next_full_stop();
    void parse_block_comment();
    void parse_line_comment();
    int parse_hex_digit();
    int parse_oct_code();

    void next_number(std::string &s);

    std::istream &in_;
    term_ops &ops_;
    token current_;
    size_t column_;
    size_t line_;
};

}}

#endif
