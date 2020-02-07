#pragma once

#ifndef _common_term_tokenizer_hpp
#define _common_term_tokenizer_hpp

#include <vector>
#include <exception>
#include <string>
#include "term.hpp"
#include "token_chars.hpp"

namespace prologcoin { namespace common {

class token_position {
public:
    inline token_position()
      : line_(-1), column_(-1) { }
    inline token_position( const token_position &other )
      : line_(other.line_), column_(other.column_) { }
    inline token_position( int line, int column )
      : line_(line), column_(column) { }

    inline void operator = (const token_position &other)
    {
        line_ = other.line_;
        column_ = other.column_;
    }

    inline int line() const { return line_; }
    inline int column() const { return column_; }
        
    inline void next_column() { if (column_ != -1) column_++; }
    inline void prev_column() { if (column_ > 0) column_--; }
    inline void new_line() { if (column_ != -1) { column_ = 1; line_++; } }
    inline void next_tab()
    {
        if (column_ < 0) return;
        column_ = ((column_ / 8) + 1) * 8;
    }

    inline bool operator == (const token_position &other) const
    {
        return line_ == other.line_ && column_ == other.column_;
    }

    inline bool operator != (const token_position &other) const
    {
      return ! operator == (other);
    }
  
    const std::string str() const;

private:
    int line_;
    int column_;
};

class token_exception : public ::std::runtime_error
{
public:
    token_exception(const std::string &line,const token_position &pos, const std::string &msg) : ::std::runtime_error(msg), pos_(pos) { }

    const token_position & pos() const { return pos_; }
    int column() const { return pos().column(); }
    int line() const { return pos().line(); }
    const std::string & line_string() const { return line_string_; }

private:
    token_position pos_;
    std::string line_string_;
};

class token_exception_control_char : public token_exception
{
public:
    token_exception_control_char(const std::string &line, const token_position &pos, const std::string &msg = "")
	: token_exception(line, pos, msg) { }
};

class token_exception_hex_code : public token_exception
{
public:
    token_exception_hex_code(const std::string &line, const token_position &pos, const std::string &msg = "")
	: token_exception(line, pos, msg) { }
};

class token_exception_no_char_code : public token_exception
{
public:
    token_exception_no_char_code(const std::string &line, const token_position &pos, const std::string &msg = "")
	: token_exception(line, pos, msg) { }
};

class token_exception_missing_number_after_base : public token_exception
{
public:
    token_exception_missing_number_after_base(const std::string &line, const token_position &pos, const std::string &msg = "")
	: token_exception(line, pos, msg) { }
};

class token_exception_missing_decimal : public token_exception
{
public:
    token_exception_missing_decimal(const std::string &line, const token_position &pos, const std::string &msg = "")
	: token_exception(line, pos, msg) { }
};

class token_exception_missing_exponent : public token_exception
{
public:
    token_exception_missing_exponent(const std::string &line, const token_position &pos, const std::string &msg = "")
	: token_exception(line, pos, msg) { }
};

class token_exception_unterminated_string : public token_exception
{
public:
    token_exception_unterminated_string(const std::string &line, const token_position &pos, const std::string &msg = "")
	: token_exception(line, pos, msg) { }
};

class token_exception_unterminated_quoted_name : public token_exception
{
public:
    token_exception_unterminated_quoted_name(const std::string &line, const token_position &pos, const std::string &msg = "")
	: token_exception(line, pos, msg) { }
};

class token_exception_unterminated_escape : public token_exception
{
public:
    token_exception_unterminated_escape(const std::string &line, const token_position &pos, const std::string &msg = "")
	: token_exception(line, pos, msg) { }
};


//
// term_tokenizer
//
// This class parses ASCII characters and builds a term (or errors)
//
class term_tokenizer : public token_chars {
public:
    term_tokenizer(std::istream &in);

    enum token_type {
        TOKEN_UNKNOWN = 0,
	TOKEN_EOF = 1,
	TOKEN_NAME = 2,
	TOKEN_NATURAL_NUMBER = 3,
	TOKEN_UNSIGNED_FLOAT = 4,
	TOKEN_VARIABLE = 5,
	TOKEN_STRING = 6,
	TOKEN_PUNCTUATION_CHAR = 7,
	TOKEN_LAYOUT_TEXT = 8,
	TOKEN_FULL_STOP = 9
    };

    class token {
    public:
        inline token()
	  : type_(TOKEN_UNKNOWN),
	    lexeme_(),
	    quoted_(false),
	    position_() { }

        inline token(const token &other)
	  : type_(other.type_),
	    lexeme_(other.lexeme_),
	    quoted_(other.quoted_),
	    position_(other.position_) { }

	token_type type() const { return type_; }

	const std::string & lexeme() const { return lexeme_; }

	void set_lexeme(const std::string &str) { lexeme_ = str; }

	bool is_quoted() const { return quoted_; }

        bool is_whitespace() const {
	    return std::all_of(lexeme_.cbegin(), lexeme_.cend(), isspace);
        }

        // Pretty print token
        const std::string str() const;

        inline const token_position & pos() const
        { return position_; }

        // String cast
        inline operator const std::string () const
        { return str(); }

    private:
        friend class term_tokenizer;

	token_type type_;
	std::string lexeme_;
	bool quoted_;
        token_position position_;

	inline void reset() { lexeme_.resize(0); quoted_ = false; }
        inline void set_position(const token_position &pos) { position_ = pos; }
	inline void set_quoted(bool q) { quoted_ = q; }
    };

    bool has_more_tokens() const {
	if (peek_char() == -1) {
	    return false;
	}
	return !in_.eof();
    }

    const token & next_token();
    const token & peek_token();
    void consume_token();

    void clear_token();

    std::istream & in() { return in_; }

    const std::string & line_string() const
    { return line_string_; }

private:
    const token & next_token_helper();

    inline int next_char()
    {
	int ch = in_.get();
	update_position(ch);
	return ch;
    }

    inline void update_position(int ch)
    {
        if (ch == ' ' || !is_layout_char(ch)) {
	    position_.next_column();
        } else {
	    if (ch == '\n') {
	        position_.new_line();
	    } else if (ch == '\b' || ch == 127) {
	        position_.prev_column();
	    } else if (ch == '\t') {
	        position_.next_tab();
	    } else {
	        position_.next_column();
	    }
	}
    }

    // Lookahead version of next_char() (don't update position)
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
        (void) peek_char();
	return in_.eof();
    }

    inline void set_token_type(token_type tt)
    {
	current_.type_ = tt;
    }

    inline void add_to_lexeme(int ch)
    {
        if (ch != -1) {
    	    current_.lexeme_ += static_cast<char>(ch);
	}
    }

    inline void consume_next_char()
    {
	add_to_lexeme(next_char());
    }

    bool is_comment_begin() const;
    bool is_full_stop() const;
    bool is_full_stop(int ch) const;
    size_t next_xs( const std::function<bool(int)> &predicate );
    size_t next_integer_digits();
    size_t next_digits();
    size_t next_alphas();
    void next_char_code();
    void next_number();
    void next_float();
    void next_decimal();
    void next_exponent();
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

    inline const token_position & pos() const 
    {
        return current_.pos();
    }

    std::istream &in_;
    token current_;
    token_position position_;
    std::string line_string_;
};

}}

#endif
