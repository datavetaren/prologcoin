#include "term_parser.hpp"

#include "term_parser_gen.hpp"

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

class term_parser_interim
{
protected:
  int current_state_;
  term_tokenizer &tokenizer_;
  heap &heap_;
  term_ops &ops_;

  struct sym {
      sym()
	: ordinal_(SYMBOL_UNKNOWN),
	  result_(),
	  token_() { }

      sym( const sym &other )
        : ordinal_(other.ordinal_),
	  result_(other.result_),
	  token_(other.token_) { }

      sym( const term_tokenizer::token &t, symbol_t ordinal )
        : ordinal_(ordinal),
  	  result_(),
  	  token_(t) { }

      sym( symbol_t ord, ext<cell> &result)
	: ordinal_(ord),
	  result_(result),
	  token_() { }

    inline symbol_t ordinal() const { return ordinal_; }
    const term_tokenizer::token & token() const { return token_; }
    ext<cell> result() { return result_; }

    private:
      symbol_t ordinal_;
      ext<cell> result_;
      term_tokenizer::token token_;
  };

  std::vector<sym> stack_;
  sym lookahead_;

  typedef std::vector<sym> args_t;
  args_t args_;
  
  friend class term_parser_gen<term_parser_interim, term_tokenizer, heap, term_ops>;

protected:
  inline int current_state() const { return current_state_; }

  inline const sym & lookahead() const { return lookahead_; }

  inline term_tokenizer & tokenizer() { return tokenizer_; }

  args_t & args(int numArgs) {
    args_.clear();
    return args_;
  }

  void push( symbol_t ordinal, ext<cell> &result ) { 
    stack_.push_back( sym(ordinal, result) );
  }

  void shift_and_goto_state(int new_state) {
    stack_.push_back(lookahead_);
    lookahead_ = next_token();
    current_state_ = new_state;
  }

  const sym next_token()
  {
    auto tok = tokenizer().next_token();
    auto lexeme = tok.lexeme();
    auto entry = ops_.prec(lexeme);
    symbol_t prec = SYMBOL_UNKNOWN;

    // Still working on this...

    if (!entry.is_none()) {
      if (entry.precedence <= 999) {
	prec = SYMBOL_OP_999;
      } else if (entry.precedence <= 1000) {
	prec = SYMBOL_OP_1000;
      } else {
	prec = SYMBOL_OP_1200;
      }
    } else {
      // No registered operator... check predefined

      /*
  SYMBOL_COMMA = 1000,
  SYMBOL_FULL_STOP = 1001,
  SYMBOL_INF = 1002,
  SYMBOL_LBRACE = 1003,
  SYMBOL_LBRACKET = 1004,
  SYMBOL_LPAREN = 1005,
  SYMBOL_MINUS = 1006,
  SYMBOL_NAME = 1007,
  SYMBOL_NAME_LPAREN = 1008,
  SYMBOL_NAN = 1009,
  SYMBOL_NATURAL_NUMBER = 1010,
  SYMBOL_OP_1000 = 1011,
  SYMBOL_OP_1200 = 1012,
  SYMBOL_OP_999 = 1013,
  SYMBOL_PLUS = 1014,
  SYMBOL_RBRACE = 1015,
  SYMBOL_RBRACKET = 1016,
  SYMBOL_RPAREN = 1017,
  SYMBOL_STRING = 1018,
  SYMBOL_UNSIGNED_FLOAT = 1019,
  SYMBOL_VARIABLE = 1020,
  SYMBOL_VBAR = 1021
      */

    }
    lookahead_ = sym(tok, prec);
  }

  // start :- ...

  ext<cell> reduce_start__subterm_1200_full_stop(args_t &args)
  {
    return args[0].result();
  }

  // subterm_1200 :- ...

  ext<cell> reduce_subterm_1200__term_1200(args_t &args)
  {
    return args[0].result();
  }

  ext<cell> reduce_subterm_1200__term_1000(args_t &args)
  {
    return args[0].result();
  }

  ext<cell> reduce_subterm_1200__term_999(args_t &args)
  {
    return args[0].result();
  }

  ext<cell> reduce_subterm_1200__term_0(args_t &args)
  {
    return args[0].result();
  }

  // subterm_1000 :- ...

  ext<cell> reduce_subterm_1000__term_1000(args_t &args)
  {
    return args[0].result();
  }

  ext<cell> reduce_subterm_1000__term_999(args_t &args)
  {
    return args[0].result();
  }

  ext<cell> reduce_subterm_1000__term_0(args_t &args)
  {
    return args[0].result();
  }

  // subterm_999 :- ...

  ext<cell> reduce_subterm_999__term_999(args_t &args)
  {
    return args[0].result();
  }

  ext<cell> reduce_subterm_999__term_0(args_t &args)
  {
    return args[0].result();
  }

  ext<cell> reduce_unary_op(const std::string &opname, ext<cell> operand)
  {   
    con_cell binop(opname, 1);
    auto newstr = heap_.new_str(binop);
    heap_.set_arg(newstr, 0, operand);
    return newstr;
  }

  ext<cell> reduce_binary_op(const std::string &opname,
			     ext<cell> operand0, ext<cell> operand1)
  {   
    con_cell binop(opname, 2);
    auto newstr = heap_.new_str(binop);
    heap_.set_arg(newstr, 0, operand0);
    heap_.set_arg(newstr, 1, operand1);
    return newstr;
  }

  ext<cell> reduce_unary_op_subterm(args_t &args)
  {
    return reduce_unary_op(args[0].token().lexeme(), args[1].result());
  }

  ext<cell> reduce_binary_op_subterm(args_t &args)
  {
    return reduce_binary_op(args[1].token().lexeme(),
			    args[0].result(),
			    args[2].result());
  }

  // term_1200 :- ...

  ext<cell> reduce_term_1200__op_1200_subterm_1200(args_t &args)
  {
    return reduce_unary_op_subterm(args);
  }

  ext<cell> reduce_term_1200__subterm_1200_op_1200_subterm_1200(args_t &args)
  {
    return reduce_binary_op_subterm(args);
  }

  // term_1000 :- ...

  ext<cell> reduce_term_1000__op_1000_subterm_1000(args_t &args)
  {
    return reduce_unary_op_subterm(args);
  }

  ext<cell> reduce_term_1000__subterm_1000_op_1000_subterm_1000(args_t &args)
  {
    return reduce_binary_op_subterm(args);
  }

  ext<cell> reduce_term_1000__subterm_999_comma_subterm_1000(args_t &args)
  {
    return reduce_binary_op(",", args[0].result(), args[2].result());
  }

  // term_999 :- ...

  ext<cell> reduce_term_999__op_999_subterm_999(args_t &args)
  {
    return reduce_unary_op_subterm(args);
  }

  ext<cell> reduce_term_999__subterm_999_op_999_subterm_999(args_t &args)
  {
    return reduce_binary_op_subterm(args);
  }

  // term_0 :- ...

  ext<cell> reduce_term_0__functor_lparen_arguments_rparen(args_t &args)
  {
    return args[1].result();
  }

  ext<cell> reduce_term_0__lparen_subterm_1200_rparen(args_t &args)
  {
    return args[1].result();
  }

  ext<cell> reduce_term_0__lbrace_subterm_1200_rbrace(args_t &args)
  {
    return reduce_unary_op("{}", args[1].result());
  }

  ext<cell> reduce_term_0__list(args_t &args)
  {
    return args[0].result();
  }

  ext<cell> reduce_term_0__string(args_t &args)
  {
    return args[0].result();
  }

  ext<cell> reduce_term_0__constant(args_t &args)
  {
    return args[0].result();
  }

  ext<cell> reduce_term_0__variable(args_t &args)
  {
    return args[0].result();
  }

  // arguments :- ...

  ext<cell> reduce_arguments__subterm_999(args_t &args)
  {
    return args[0].result();
  }

  ext<cell> reduce_arguments__subterm_999_comma_arguments(args_t &args)
  {
    return reduce_binary_op(",", args[0].result(), args[2].result());
  }

  // list :- ...

  ext<cell> reduce_list__lbracket_rbracket(args_t &args)
  {
    con_cell empty_list("[]", 0);
    return ext<cell>(heap_, empty_list);
  }

  ext<cell> reduce_list__lbracket_listexpr_rbracket(args_t &args)
  {
    return args[1].result();
  }

  // listexpr :- ...

  ext<cell> reduce_listexpr__subterm_999(args_t &args)
  {
    return args[0].result();
  }

  ext<cell> reduce_listexpr__subterm_999_comma_listexpr(args_t &args)
  {
    return reduce_binary_op(".", args[0].result(), args[2].result());
  }
  
  ext<cell> reduce_listexpr__subterm_999_vbar_subterm_999(args_t &args)
  {
    return reduce_binary_op(".", args[0].result(), args[2].result());
  }

  // constant :- ...

  ext<cell> reduce_constant__atom(args_t &args)
  {
    return args[0].result();
  }

  ext<cell> reduce_constant__number(args_t &args)
  {
    return args[0].result();
  }

  // number :- ...

  ext<cell> reduce_number__unsigned_number(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_number__sign_unsigned_number(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_number__sign_inf(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_number__sign_nan(args_t &args)
  {
    return ext<cell>();
  }

  // unsigned_number :- ...

  ext<cell> reduce_unsigned_number__natural_number(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_unsigned_number__unsigned_float(args_t &args)
  {
    return ext<cell>();
  }

  // atom :- ...

  ext<cell> reduce_atom__name(args_t &args)
  {
    con_cell name(args[0].token().lexeme(), 0);
    return ext<cell>(heap_, name);
  }

  // functor_lparen :- ...

  ext<cell> reduce_functor_lparen__name_lparen(args_t &args)
  {
    return ext<cell>();
  }

  // sign :- ...

  ext<cell> reduce_sign__plus(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_sign__minus(args_t &args)
  {
    return ext<cell>();
  }

public:
  term_parser_interim(term_tokenizer &tokenizer, heap &h, term_ops &ops)
    : tokenizer_(tokenizer), heap_(h), ops_(ops) {
    current_state_ = 0;
  }
};

class term_parser_impl : public term_parser_gen<term_parser_interim, term_tokenizer, heap, term_ops>
{
public:
  term_parser_impl(term_tokenizer &tokenizer, heap &h, term_ops &ops)
    : term_parser_gen<term_parser_interim, term_tokenizer, heap, term_ops>(tokenizer, h, ops) { }

  void process_next()
  {
    process_state();
  }
};

}}


