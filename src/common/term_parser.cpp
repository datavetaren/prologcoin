#include "term_parser.hpp"

#include "term_parser_gen.hpp"

namespace prologcoin { namespace common {

//
// This is a hand coded shift/reduce parser.
//

class term_parser_interim
{
protected:
  int current_state_;
  term_tokenizer &tokenizer_;
  heap &heap_;
  term_ops &ops_;
  bool is_debug_;
  std::unordered_map<std::string, symbol_t> predefined_symbols_;
  cell empty_list_;
  cell dotted_pair_;

  struct sym {
      sym()
	: ordinal_(SYMBOL_UNKNOWN),
	  result_(),
	  token_(),
          old_state_(-1) { }

      sym( int old_state, const sym &other )
        : ordinal_(other.ordinal_),
	  result_(other.result_),
	  token_(other.token_),
          old_state_(old_state) { }

      sym( int old_state, const term_tokenizer::token &t, symbol_t ordinal )
        : ordinal_(ordinal),
  	  result_(),
  	  token_(t),
          old_state_(old_state) { }

      sym( int old_state, symbol_t ord, const ext<cell> &result)
	: ordinal_(ord),
	  result_(result),
	  token_(),
	  old_state_(old_state) { }

    inline void clear() { ordinal_ = SYMBOL_UNKNOWN; }

    inline int old_state() const { return old_state_; }
    inline symbol_t ordinal() const { return ordinal_; }
    const term_tokenizer::token & token() const { return token_; }
    ext<cell> result() { return result_; }
    inline void set_old_state(int no) { old_state_ = no; }

    private:
      symbol_t ordinal_;
      ext<cell> result_;
      term_tokenizer::token token_;
      int old_state_;
  };

  std::vector<sym> stack_;
  sym lookahead_;
  sym old_lookahead_;

  ext<cell> result_;
  bool accept_;
  bool error_;

  typedef std::vector<sym> args_t;
  args_t args_;

  typedef std::unordered_map<ext<cell>, std::string> var_name_map_type;
  typedef std::unordered_map<std::string, ext<cell> > name_var_map_type;

  var_name_map_type var_name_map_;
  name_var_map_type name_var_map_;

  friend class term_parser_gen<term_parser_interim, term_tokenizer, heap, term_ops>;

protected:
  inline int current_state() const { return current_state_; }

  inline const sym & lookahead() const { return lookahead_; }

  inline term_tokenizer & tokenizer() { return tokenizer_; }

  args_t & args(int numArgs) {
    args_.clear();
    args_.resize(numArgs);
    while (numArgs > 0) {
      current_state_ = stack_.back().old_state();
      args_[numArgs-1] = stack_.back();
      stack_.pop_back();
      numArgs--;
    }
    return args_;
  }

  void reduce( symbol_t ordinal, const ext<cell> &result ) { 
    old_lookahead_ = lookahead_;
    lookahead_ = sym(current_state_, ordinal, result);
    if (is_debug_) {
      std::cout << "reduce(): " << lookahead_.ordinal() << "\n";
    }
    stack_.push_back(lookahead_);
  }

  void shift_and_goto_state(int new_state) {
    if (is_debug_) {
      std::cout << "shift_and_goto_state(): " << new_state << "\n";
    }
    stack_.push_back(lookahead_);
    lookahead_.clear();
    current_state_ = new_state;
  }

  void goto_state(int new_state) {
    if (is_debug_) {
      std::cout << "goto_state(): " << new_state << "\n";
    }
    current_state_ = new_state;
    lookahead_ = old_lookahead_;
    old_lookahead_.clear();
  }

  void parse_error() {
    if (is_debug_) {
      std::cout << "parse_error(): at state " << current_state_ << "\n";
    }
    error_ = true;
  }

  void skip_whitespace()
  {
    while (tokenizer().peek_token().type()
	   == term_tokenizer::TOKEN_LAYOUT_TEXT) {
      tokenizer().consume_token();
    }
  }

  bool is_last_char_alpha(const term_tokenizer::token &token) const
  {
    return token_chars::is_alpha(token.lexeme()[token.lexeme().length()-1]);
  }

  const sym next_symbol()
  {
    skip_whitespace();

    auto tok = tokenizer().peek_token();
    auto lexeme = tok.lexeme();

    //    std::cout << "Next_symbol(): " << lexeme << " " << tok.type() << "\n";

    bool consumed_name = false;

    symbol_t prec = SYMBOL_UNKNOWN;

    if (!tok.is_quoted()) {
	prec = predefined_symbols_[lexeme];
    }

    if (prec != SYMBOL_UNKNOWN) {
        lookahead_ = sym(current_state_, tok, prec);
        tokenizer().consume_token();
        return lookahead_;
    }

    switch (tok.type()) {
    case term_tokenizer::TOKEN_UNKNOWN:
    case term_tokenizer::TOKEN_PUNCTUATION_CHAR: // Should already be handled
    case term_tokenizer::TOKEN_LAYOUT_TEXT:      // --- " " ---
    case term_tokenizer::TOKEN_FULL_STOP:        // --- " " ---
      lookahead_ = sym(current_state_, tok, SYMBOL_UNKNOWN);
      return lookahead_;
    case term_tokenizer::TOKEN_EOF:
      lookahead_ = sym(current_state_, tok, SYMBOL_EOF);
      return lookahead_;
    case term_tokenizer::TOKEN_NAME:
      // Prolog has this funny grammar rule that if an '(' (LPAREN)
      // immediately follows a name (with no white space in between)
      // then it is a functor.
      lookahead_ = sym(current_state_, tok, SYMBOL_NAME);
      tokenizer().consume_token();
      consumed_name = true;
      if ((is_last_char_alpha(tok) || tok.is_quoted()) &&
	  tokenizer().peek_token().type()
	  == term_tokenizer::TOKEN_PUNCTUATION_CHAR) {
	if (tokenizer().peek_token().lexeme() == "(") {
	  lookahead_ = sym(current_state_, lookahead_.token(),
			   SYMBOL_FUNCTOR_LPAREN);
	  tokenizer().consume_token();
	}
        return lookahead_;
      }
      if (tok.is_quoted()) {
	return lookahead_;
      }
      break; // Requires operator check...
    case term_tokenizer::TOKEN_NATURAL_NUMBER:
      lookahead_ = sym(current_state_, tok, SYMBOL_NATURAL_NUMBER);
      tokenizer().consume_token();
      return lookahead_;
    case term_tokenizer::TOKEN_UNSIGNED_FLOAT:
      lookahead_ = sym(current_state_, tok, SYMBOL_UNSIGNED_FLOAT);
      tokenizer().consume_token();
      return lookahead_;
    case term_tokenizer::TOKEN_VARIABLE:
      lookahead_ = sym(current_state_, tok, SYMBOL_VARIABLE);
      tokenizer().consume_token();
      return lookahead_;
    case term_tokenizer::TOKEN_STRING:
      lookahead_ = sym(current_state_, tok, SYMBOL_STRING);
      tokenizer().consume_token();
      return lookahead_;	
    }

    auto entry = ops_.prec(lexeme);

    if (consumed_name && entry.is_none()) {
        return lookahead_;
    }

    if (entry.is_none()) {
        throw token_exception_unrecognized_operator(tok.pos(), tok.lexeme());
    }

    if (entry.precedence <= 999) {
        prec = SYMBOL_OP_999;
    } else if (entry.precedence <= 1000) {
        prec = SYMBOL_OP_1000;
    } else {
        prec = SYMBOL_OP_1200;
    }

    lookahead_ = sym(current_state_, tok, prec);

    if (!consumed_name) {
        tokenizer().consume_token();
    }

    return lookahead_;
  }

  // start :- ...

  ext<cell> reduce_start__subterm_1200_full_stop(args_t &args)
  {
    result_ = args[0].result();
    accept_ = true;
    return result_;
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
    auto arg_list = args[1].result();
    size_t num_args = heap_.list_length(arg_list);
    auto fname = args[0].token().lexeme();
    auto farity = num_args;

    con_cell f = heap_.functor(fname, farity);
    cell fstr = heap_.new_str(f);

    size_t index = 0;
    cell lst = arg_list;
    while (lst != empty_list_) {
        heap_.set_arg(fstr, index, heap_.arg(lst, 0));
	lst = heap_.arg(lst, 1);
	index++;
    }

    return ext<cell>(heap_,fstr);
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
      term_tokenizer::token var_token = args[0].token();
      const std::string &var_name = var_token.lexeme();

      // Check if we have seen this name before
      auto found = name_var_map_.find(var_name);
      if (found != name_var_map_.end()) {
	  return found->second;
      }

      // We didn't find this variable, so we create a new one.

      auto ref = heap_.new_ref();

      // Register this ref cell so we can give the same name for it
      // when printing.
      var_name_map_[ref] = var_name;
      name_var_map_[var_name] = ref;

      return ref;
  }

  // arguments :- ...

  ext<cell> reduce_arguments__subterm_999(args_t &args)
  {
    // Construct list with single element

    con_cell empty_list("[]", 0);
    ext<cell> ext_empty_list(heap_, empty_list);
    return reduce_binary_op(".", args[0].result(), ext_empty_list);
  }

  ext<cell> reduce_arguments__subterm_999_comma_arguments(args_t &args)
  {
    return reduce_binary_op(".", args[0].result(), args[2].result());
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
    return args[0].result();
  }

  ext<cell> reduce_number__inf(args_t &args)
  {
    return ext<cell>();
  }

  ext<cell> reduce_number__nan(args_t &args)
  {
    return ext<cell>();
  }

  // unsigned_number :- ...

  ext<cell> reduce_unsigned_number__natural_number(args_t &args)
  {
    auto &tok = args[0].token();
    int_cell val(boost::lexical_cast<int>(tok.lexeme()));
    return ext<cell>(heap_, val);
  }

  ext<cell> reduce_unsigned_number__unsigned_float(args_t &args)
  {
    return ext<cell>();
  }

  // atom :- ...

  ext<cell> reduce_atom__name(args_t &args)
  {
    con_cell con = heap_.atom(args[0].token().lexeme());
    return ext<cell>(heap_, con);
  }

public:
  term_parser_interim(term_tokenizer &tokenizer, heap &h, term_ops &ops)
    : tokenizer_(tokenizer), heap_(h), ops_(ops),
      is_debug_(false),
      empty_list_(con_cell("[]", 0)),
      dotted_pair_(con_cell(".", 2)) {
    current_state_ = 0;
    predefined_symbols_[","] = SYMBOL_COMMA;
    predefined_symbols_["."] = SYMBOL_FULL_STOP;
    predefined_symbols_["inf"] = SYMBOL_INF;
    predefined_symbols_["{"] = SYMBOL_LBRACE;
    predefined_symbols_["["] = SYMBOL_LBRACKET;
    predefined_symbols_["("] = SYMBOL_LPAREN;
    predefined_symbols_["nan"] = SYMBOL_NAN;
    predefined_symbols_["}"] = SYMBOL_RBRACE;
    predefined_symbols_["]"] = SYMBOL_RBRACKET;
    predefined_symbols_[")"] = SYMBOL_RPAREN;
    predefined_symbols_["|"] = SYMBOL_VBAR;
  }

  void set_debug(bool dbg) { is_debug_ = dbg; }

  bool is_accept() const { return accept_; }
  bool is_error() const { return error_; }
  ext<cell> get_result() const { return result_; }
  void init() {
      current_state_ = 0;
      accept_ = false;
      error_ = false;
      result_ = ext<cell>();
  }
  bool is_eof() {
      skip_whitespace();
      return lookahead().ordinal() == SYMBOL_EOF;
  }

  const std::string & get_var_name(ext<cell> &cell);

  void for_each_var_name(std::function<void (const ext<cell> &ref, const std::string &name)> f) const {
      for (auto e : var_name_map_) {
	  const ext<cell> &ref = e.first;
	  const std::string &name = e.second;
	  f(ref,name);
      }
  }
};

class term_parser_impl : public term_parser_gen<term_parser_interim, term_tokenizer, heap, term_ops>
{
public:
  term_parser_impl(term_tokenizer &tokenizer, heap &h, term_ops &ops)
    : term_parser_gen<term_parser_interim, term_tokenizer, heap, term_ops>(tokenizer, h, ops) { }
  ~term_parser_impl() = default;

  void process_next()
  {
    if (lookahead_.ordinal() == SYMBOL_UNKNOWN) {
        lookahead_ = next_symbol();
    }

    if (is_debug_) {
      std::cout << "In state " << current_state() << " SYM " << lookahead_.ordinal() << " lexeme " << lookahead_.token().lexeme() << "\n";
    }

    process_state();

    if (accept_) {
	lookahead_ = old_lookahead_;
	stack_.pop_back();
	lookahead_.set_old_state(0);
    }
  }
};


term_parser::term_parser(term_tokenizer &tok, heap &h, term_ops &ops)
{
  impl_ = new term_parser_impl(tok, h, ops);
}

term_parser::~term_parser()
{
  delete impl_;
}

void term_parser::set_debug(bool dbg)
{
    impl_->set_debug(dbg);
}

ext<cell> term_parser::parse()
{
  impl_->init();
  while (!impl_->is_accept() && !impl_->is_error()) {
    impl_->process_next();
  }
  return impl_->get_result();
}

bool term_parser::is_eof()
{
    return impl_->is_eof();
}

bool term_parser::is_error()
{
    return impl_->is_error();
}

void term_parser::for_each_var_name(std::function<void (const ext<cell> &ref, const std::string &name)> f) const {
    impl_->for_each_var_name(f);
}

}}


