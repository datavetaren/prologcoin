#include "term_parser.hpp"

namespace prologcoin { namespace common {

//
// This is a hand coded shift/reduce parser.
//

term_parser::term_parser(term_tokenizer &tokenizer, heap &h, term_ops &ops)
        : tokenizer_(tokenizer),
	  heap_(h),
	  ops_(ops),
	  debug_(false)
{
    (void)tokenizer_;
    (void)heap_;
    (void)ops_;
}

void term_parser::set_debug(bool d)
{
    debug_ = d;
}

term_parser::grammar_item::grammar_item()
    : type_(term_parser::grammar_item::UNKNOWN),
      terminal_(),
      non_terminal_(),
      precedence_(0)
{
}

term_parser::grammar_item::grammar_item(const token &token)
    : type_(term_parser::grammar_item::TERMINAL),
      terminal_(token),
      non_terminal_(),
      precedence_(0)
{
}

term_parser::grammar_item::grammar_item(term_parser::grammar_item::item_type ty, const ext<cell> &nt)
  : type_(ty),
    terminal_(),
    non_terminal_(nt),
    precedence_(0)
{
}

void term_parser::shift(const token &token)
{
    if (debug_) std::cout << "SHIFT: " << token.str() << "\n";

    stack_.push_back(token);
}

//
// Will be replaced with state machine generated code...
//

/*
void term_parser::process_next()
{
    typedef term_tokenizer::token_type TT;
    typedef grammar_item GI;

    // If the stack is empty, shift next token...
    if (stack_.empty()) {
	auto tok = tokenizer_.next_token();
	shift(tok);
	tokenizer_.clear_token();
	return;
    }

    // Is there something on the stack we can use for reduction?

    const grammar_item &item = stack_.back();
    const token &lookahead = tokenizer_.peek_token();

    std::cout << "LOOKAHEAD: " << lookahead.str() << "\n";

    auto item_is = [](const grammar_item &item, TT ty) -> bool {
	return item.type_ == GI::TERMINAL &&
	       item.terminal_.type() == ty;
    };

    auto lookahead_is = [&](TT ty, const std::string &l) -> bool {
	return lookahead.type() == ty && lookahead.lexeme() == l;
    };


    // Top item on stack is TOKEN_NAME
    if (item_is(item, TT::TOKEN_NAME)) {
	// If we see NAME (    then it's a functor
	if (lookahead_is(TT::TOKEN_PUNCTUATION_CHAR, "(")) {
	    tokenizer_.clear_token();
	    reduce_to_functor();
	    return;
	}

	// Otherwise it's a constant
	reduce_to_constant();
	return;
    }

    if (item_is(item, TT::TOKEN_NATURAL_NUMBER) ||
	item_is(item, TT::TOKEN_UNSIGNED_FLOAT)) {
        reduce_to_unsigned_number();
	return;
    }

    if (item_is(item, TT::TOKEN_STRING)) {
	reduce_to_string();
	return;
    }

    switch (item.type_) {
    case GI::NT_UNSIGNED_NUMBER:
	reduce_to_number();
	return;
    case GI::NT_LIST:
    case GI::NT_STRING:
    case GI::NT_CONSTANT:
    case GI::NT_VARIABLE:
	if (lookahead_is(TT::TOKEN_PUNCTUATION_CHAR, ",")) {
	    // Reduce to term
	    reduce_to_term0();
	    return;
	}
    default:
	break;
    }

    // Fall back on shift

    shift(lookahead);
    tokenizer_.clear_token();
}
*/


//
// Grammar for terms.
//
//	term-read-in      --> subterm(1200) full-stop
//
//	subterm(N)        --> term(M)
//	{ where M is less than or equal to N }
//
//	term(N)           --> op(N,fx) subterm(N-1)
//	{ except in the case of a number }
//      { if subterm starts with a (,
//        op must be followed by layout-text }
//	     |  op(N,fy) subterm(N)
//	      { if subterm starts with a (,
//	        op must be followed by layout-text }
//           |  subterm(N-1) op(N,xfx) subterm(N-1)
//   	     |  subterm(N-1) op(N,xfy) subterm(N)
//	     |  subterm(N) op(N,yfx) subterm(N-1)
//	     |  subterm(N-1) op(N,xf)
//           |  subterm(N) op(N,yf)
//     term(1000)        --> subterm(999) , subterm(1000)
//
//     term(0)           --> functor ( arguments )
//			   { provided there is no layout-text between
//			     the functor and the ( }
//		   |  ( subterm(1200) )
//	           |  { subterm(1200) }
//                 |  list
//                 |  string
//                 |  constant
//                 |  variable
//
//     op(N,T)           --> name
//			  { where name has been declared as an
//				  operator of type T and precedence N }
//
//     arguments         --> subterm(999)
//	           |  subterm(999) , arguments
//
//     list              --> []
//                 |  [ listexpr ]
//
//     listexpr          --> subterm(999)
//		   |  subterm(999) , listexpr
//		   |  subterm(999) | subterm(999)
//
//     constant          --> atom | number
//
//     number            --> unsigned-number
//                 |  sign unsigned-number
//                 |  sign inf
//                 |  sign nan
//
//     unsigned-number   --> natural-number | unsigned-float
//
//     atom              --> name
//
//     functor           --> name

}}


