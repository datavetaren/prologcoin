#pragma once

#ifndef _common_token_chars_hpp
#define _common_token_chars_hpp

#include <vector>
#include <exception>
#include <string>
#include "term.hpp"
#include "term_ops.hpp"

namespace prologcoin { namespace common {

class token_chars
{
public:
    inline static bool is_layout_char(int ch)
    { return (ch >= 0 && ch <= 32) || (ch >= 127 && ch <= 159); }
    inline static bool is_small_letter(int ch)
    { return (ch >= 97 && ch <= 122) ||
	     ((ch >= 223 && ch <= 255) && (ch != 247)); }
    inline static bool is_capital_letter(int ch)
    { return (ch >= 65 && ch <= 90) ||
	     ((ch >= 192 && ch <= 222) && (ch != 215)); }
    inline static bool is_digit(int ch) {
      return ch >= 48 && ch <= 57;
    }
    inline static bool is_symbol_char(int ch) {
	return (ch >= 0 && ch <= 255) ? IS_SYMBOL[ch & 0xff] : false;
    }
    inline static bool is_solo_char(int ch) {
	return ch == 33 || ch == 59;
    }
    inline static bool is_punctuation_char(int ch) {
	return ch == 37 || ch == 40 || ch == 41 || ch == 44 ||
	       ch == 91 || ch == 93 || (ch >= 123 && ch <= 125);
    }
    inline static bool is_quote_char(int ch) {
	return ch == 34 || ch == 39;
    }
    inline static bool is_underline_char(int ch) {
	return ch == 95;
    }
    inline static bool is_alpha(int ch) {
	return is_capital_letter(ch) || is_small_letter(ch) ||
	       is_underline_char(ch) || is_digit(ch);
    }

    inline static bool should_be_escaped(int ch) {
	return is_layout_char(ch) || is_quote_char(ch);
    }

    static std::string escape(const std::string &str);

private:
    static const bool IS_SYMBOL [256];
};

}}

#endif
