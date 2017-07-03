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

      sym( const term_tokenizer::token &t )
        : ordinal_(compute_ordinal(t)),
  	  result_(),
  	  token_(t) { }

      sym( symbol_t ord, ext<cell> &result)
	: ordinal_(ord),
	  result_(result),
	  token_() { }

    inline symbol_t ordinal() const { return ordinal_; }

    private:
      symbol_t compute_ordinal(const term_tokenizer::token &t);

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
    auto &tok = tokenizer().next_token();
    lookahead_ = sym(tok);
  }

  // start :- ...

  ext<cell> reduce_start__subterm_1200_full_stop(args_t &args)
  {
    return ext<cell>();
  }

  // subterm_1200 :- ...

  ext<cell> reduce_subterm_1200__term_1200(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_subterm_1200__term_1000(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_subterm_1200__term_999(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_subterm_1200__term_0(args_t &args)
  {
    return ext<cell>();
  }

  // subterm_1000 :- ...

  ext<cell> reduce_subterm_1000__term_1000(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_subterm_1000__term_999(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_subterm_1000__term_0(args_t &args)
  {
    return ext<cell>();
  }

  // subterm_999 :- ...

  ext<cell> reduce_subterm_999__term_999(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_subterm_999__term_0(args_t &args)
  {
    return ext<cell>();
  }

  // term_1200 :- ...

  ext<cell> reduce_term_1200__unary_op_1200_subterm_1200(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_term_1200__subterm_1200_bin_op_1200_subterm_1200(args_t &args)
  {
    return ext<cell>();
  }

  // term_1000 :- ...

  ext<cell> reduce_term_1000__unary_op_1000_subterm_1000(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_term_1000__subterm_1000_bin_op_1000_subterm_1000(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_term_1000__subterm_999_comma_subterm_1000(args_t &args)
  {
    return ext<cell>();
  }

  // term_999 :- ...

  ext<cell> reduce_term_999__unary_op_999_subterm_999(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_term_999__subterm_999_bin_op_999_subterm_999(args_t &args)
  {
    return ext<cell>();
  }

  // term_0 :- ...

  ext<cell> reduce_term_0__functor_lparen_arguments_rparen(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_term_0__lparen_subterm_1200_rparen(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_term_0__lbrace_subterm_1200_rbrace(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_term_0__list(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_term_0__string(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_term_0__constant(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_term_0__variable(args_t &args)
  {
    return ext<cell>();
  }

  // arguments :- ...

  ext<cell> reduce_arguments__subterm_999(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_arguments__subterm_999_comma_arguments(args_t &args)
  {
    return ext<cell>();
  }

  // list :- ...

  ext<cell> reduce_list__lbracket_rbracket(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_list__lbracket_listexpr_rbracket(args_t &args)
  {
    return ext<cell>();
  }

  // listexpr :- ...

  ext<cell> reduce_listexpr__subterm_999(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_listexpr__subterm_999_comma_listexpr(args_t &args)
  {
    return ext<cell>();
  }
  
  ext<cell> reduce_listexpr__subterm_999_vbar_subterm_999(args_t &args)
  {
    return ext<cell>();
  }

  // constant :- ...

  ext<cell> reduce_constant__atom(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_constant__number(args_t &args)
  {
    return ext<cell>();
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
    return ext<cell>();
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


