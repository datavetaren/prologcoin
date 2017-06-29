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

private:
    typedef term_tokenizer::token token;

    void shift(const token &t);
    void reduce_to_functor();
    void reduce_to_constant();
    void reduce_to_string();
    void reduce_to_variable();
    void reduce_to_unsigned_number();
    void reduce_to_number();
    void reduce_to_term0();

    term_tokenizer &tokenizer_;
    heap &heap_;
    term_ops &ops_;
    bool debug_;

    struct grammar_item {

	enum item_type { UNKNOWN,
			 TERMINAL,
			 NT_ATOM,
			 NT_FUNCTOR,
			 NT_UNSIGNED_NUMBER,
			 NT_NUMBER,
			 NT_CONSTANT,
			 NT_LISTEXPR,
			 NT_LIST,
			 NT_STRING,
			 NT_VARIABLE,
			 NT_ARGUMENTS,
			 NT_OP,
			 NT_TERM,
			 NT_SUBTERM
	               } type_;

	grammar_item();
	grammar_item(const token &t);
	grammar_item(item_type ty, const ext<cell> &nt);
	token terminal_;
	ext<cell> non_terminal_;
	int precedence_;
    };

    size_t state_;
    std::vector<grammar_item> stack_;
};

}}

#endif
