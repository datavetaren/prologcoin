#include "term_env.hpp"
#include "term_parser.hpp"
#include "term_parser_gen.hpp"
#include "term_emitter.hpp"

namespace prologcoin { namespace common {

//
// This is a custom shift/reduce parser.
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
          old_state_(-1),
          precedence_(0) { }

      sym( int old_state, const sym &other )
        : ordinal_(other.ordinal_),
	  result_(other.result_),
	  token_(other.token_),
          old_state_(old_state),
          precedence_(other.precedence_) { }

      sym( int old_state, const term_tokenizer::token &t, symbol_t ordinal )
        : ordinal_(ordinal),
  	  result_(),
  	  token_(t),
          old_state_(old_state),
	  precedence_(0)
          { }

      sym( int old_state, symbol_t ord, const ext<cell> &result)
	: ordinal_(ord),
	  result_(result),
	  token_(),
	  old_state_(old_state),
          precedence_(0) { }

      sym( const ext<cell> &result )
	  : ordinal_(SYMBOL_UNKNOWN),
	    result_(result),
	    token_(),
	    old_state_(0),
	    precedence_(0) { }

    inline void clear() { ordinal_ = SYMBOL_UNKNOWN; }

    inline int old_state() const { return old_state_; }
    inline int precedence() const { return precedence_; }

    inline symbol_t ordinal() const { return ordinal_; }
    inline const term_tokenizer::token & token() const { return token_; }
    inline const ext<cell> & result() const { return result_; }
    
    inline void set_ordinal(symbol_t ord) { ordinal_ = ord; }
    inline void set_token(const term_tokenizer::token &tok ) { token_ = tok; }
    inline void set_result(const ext<cell> &result) { result_ = result; }
    inline void set_old_state(int no) { old_state_ = no; }
    inline void set_precedence(int prec) { precedence_ = prec; }

    private:
      symbol_t ordinal_;
      ext<cell> result_;
      term_tokenizer::token token_;
      int old_state_;
      int precedence_;
  };

  std::vector<sym> stack_;
  sym lookahead_;
  sym old_lookahead_;

  std::vector<term_tokenizer::token> comments_;

  ext<cell> result_;
  bool accept_;
  bool error_;
  bool check_mode_;

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
    if (check_mode_) {
      return args_;
    }
    args_.resize(numArgs);
    while (numArgs > 0) {
      current_state_ = stack_.back().old_state();
      args_[numArgs-1] = stack_.back();
      stack_.pop_back();
      numArgs--;
    }
    return args_;
  }

  void reduce( symbol_t ord, const sym &result ) { 
    if (check_mode_) {
      return;
    }
    old_lookahead_ = lookahead_;
    lookahead_.set_ordinal(ord);
    lookahead_.set_result(result.result());
    lookahead_.set_token(result.token());
    lookahead_.set_precedence(result.precedence());
    lookahead_.set_old_state(current_state_);
    if (is_debug_) {
	std::cout << "reduce(): " << lookahead_.ordinal() << "\n";
    }
    stack_.push_back(lookahead_);
  }

  void shift_and_goto_state(int new_state) {
    if (check_mode_) {
      return;
    }
    if (is_debug_) {
      std::cout << "shift_and_goto_state(): " << new_state << "\n";
    }
    stack_.push_back(lookahead_);
    lookahead_.clear();
    current_state_ = new_state;
  }

  void goto_state(int new_state) {
    if (check_mode_) {
	return;
    }
    if (is_debug_) {
      std::cout << "goto_state(): " << new_state << "\n";
    }
    current_state_ = new_state;
    lookahead_ = old_lookahead_;
    old_lookahead_.clear();
  }

  void parse_error() {
    if (!check_mode_ && is_debug_) {
      std::cout << "parse_error(): at state " << current_state_ << "\n";
    }
    error_ = true;
    if (!check_mode_) {
	throw term_parse_error_exception(lookahead_.token(), "Unexpected " + lookahead_.token().lexeme());
    }
  }

  void skip_whitespace()
  {
    while (tokenizer().peek_token().type()
	   == term_tokenizer::TOKEN_LAYOUT_TEXT) {
      auto &tok = tokenizer().peek_token();
      comments_.push_back(tok);
      tokenizer().consume_token();
    }
  }

  bool is_last_char_alpha(const term_tokenizer::token &token) const
  {
    return token_chars::is_alpha(token.lexeme()[token.lexeme().length()-1]);
  }

  // --- check for reduction operations ---

  bool check_op_fx(sym la)
  {
      int op_f = stack_.end()[-2].precedence();

      int op_g = la.precedence();
      int op_arg = stack_.back().precedence();

      if (op_g >= op_f) {
	  return op_arg < op_f;
      } else {
	  return false;
      }
  }

  bool check_op_fy(sym la)
  {
      int op_f = stack_.end()[-2].precedence();

      int op_g = la.precedence();
      int op_arg = stack_.back().precedence();

      if (op_g > op_f) {
	  return op_arg <= op_f;
      } else {
	  return false;
      }
  }

  bool check_op_xfx(sym la)
  {
      int op_arg_0 = stack_.end()[-3].precedence();
      int op_f = stack_.end()[-2].precedence();
      int op_arg_1 = stack_.back().precedence();

      int op_g = la.precedence();
      
      if (op_g >= op_f) {
	  return (op_arg_0 < op_f) && (op_arg_1 < op_f);
      } else {
	  return false;
      }
  }

  bool check_op_xfy(sym la)
  {
      int op_arg_0 = stack_.end()[-3].precedence();
      int op_f = stack_.end()[-2].precedence();
      int op_arg_1 = stack_.back().precedence();

      int op_g = la.precedence();
      
      if (op_g > op_f) {
	  return (op_arg_0 < op_f) && (op_arg_1 <= op_f);
      } else {
	  return false;
      }
  }

  bool check_op_yfx(sym la)
  {
      int op_arg_0 = stack_.end()[-3].precedence();
      int op_f = stack_.end()[-2].precedence();
      int op_arg_1 = stack_.back().precedence();

      int op_g = la.precedence();

      if (op_g >= op_f) {
	  return (op_arg_0 <= op_f) && (op_arg_1 < op_f);
      } else {
	  return false;
      }
  }

  bool check_op_xf(sym la)
  {
      (void) la;

      int op_f = stack_.back().precedence();
      int op_arg = stack_.end()[-2].precedence();

      return op_arg < op_f;
  }

  bool check_op_yf(sym la)
  {
      (void) la;

      int op_f = stack_.back().precedence();
      int op_arg = stack_.end()[-2].precedence();

      return op_arg <= op_f;
  }

  // start :- ...

  sym reduce_start__subterm_1200_full_stop(args_t &args)
  {
    if (check_mode_) return sym();

    result_ = args[0].result();
    accept_ = true;
    return args[0];
  }

  // subterm_1200 :- subterm_n

  sym reduce_subterm_1200__subterm_n(args_t &args)
  {
    if (check_mode_) return sym();

    return args[0];
  }

  // subterm_999 :- subterm_n

  sym reduce_subterm_999__subterm_n(args_t &args)
  {
    if (check_mode_) return sym();

    return args[0];
  }

  // subterm_n :- term_n

  sym reduce_subterm_n__term_n(args_t &args)
  {
    if (check_mode_) return sym();

    return args[0];
  }

  // term_n :- ...

  // Helper methods...
  sym reduce_unary_op(const std::string &opname, ext<cell> operand)
  {   
    if (check_mode_) return sym();

    con_cell unop(opname, 1);
    auto newstr = heap_.new_str(unop);
    heap_.set_arg(newstr, 0, operand);
    sym s(newstr);
    s.set_precedence(ops_.prec(unop).precedence);
    return s;
  }

  sym reduce_binary_op(const std::string &opname,
		       ext<cell> operand0, ext<cell> operand1)
  {   
    con_cell binop(opname, 2);
    auto newstr = heap_.new_str(binop);
    heap_.set_arg(newstr, 0, operand0);
    heap_.set_arg(newstr, 1, operand1);
    sym s(newstr);
    s.set_precedence(ops_.prec(binop).precedence);
    return s;
  }

  sym reduce_unary_op_subterm(args_t &args)
  {
    return reduce_unary_op(args[0].token().lexeme(), args[1].result());
  }

  sym reduce_binary_op_subterm(args_t &args)
  {
    return reduce_binary_op(args[1].token().lexeme(),
			    args[0].result(),
			    args[2].result());
  }

  // term_n :- ...

  sym reduce_term_n__op_fx_subterm_n(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_unary_op_subterm(args);
  }

  sym reduce_term_n__op_fy_subterm_n(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_unary_op_subterm(args);
  }

  sym reduce_term_n__subterm_n_op_xfx_subterm_n(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_binary_op_subterm(args);
  }

  sym reduce_term_n__subterm_n_op_xfy_subterm_n(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_binary_op_subterm(args);
  }

  sym reduce_term_n__subterm_n_op_yfx_subterm_n(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_binary_op_subterm(args);
  }

  sym reduce_term_n__subterm_n_op_xf(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_unary_op_subterm(args);
  }

  sym reduce_term_n__subterm_n_op_yf(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_unary_op_subterm(args);
  }

  sym reduce_term_n__term_0(args_t &args)
  {
    if (check_mode_) return sym();

    return args[0];
  }

  // term_0 :- ...

  sym reduce_term_0__functor_lparen_arguments_rparen(args_t &args)
  {
    if (check_mode_) return sym();

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

    return sym(ext<cell>(heap_,fstr));
  }

  inline sym remove_precedence(sym &s)
  {
      s.set_precedence(0);
      return s;
  }

  sym reduce_term_0__lparen_subterm_1200_rparen(args_t &args)
  {
    if (check_mode_) return sym();

    return remove_precedence(args[1]);
  }

  sym reduce_term_0__lbrace_subterm_1200_rbrace(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_unary_op("{}", args[1].result());
  }

  sym reduce_term_0__list(args_t &args)
  {
    if (check_mode_) return sym();

    return args[0];
  }

  sym reduce_term_0__string(args_t &args)
  {
    if (check_mode_) return sym();

    return args[0];
  }

  sym reduce_term_0__constant(args_t &args)
  {
    if (check_mode_) return sym();

    return args[0];
  }

  sym reduce_term_0__variable(args_t &args)
  {
    if (check_mode_) return sym();

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

      return sym(ref);
  }

  // arguments :- ...

  sym reduce_arguments__subterm_999(args_t &args)
  {
    if (check_mode_) return sym();

    // Construct list with single element

    con_cell empty_list("[]", 0);
    ext<cell> ext_empty_list(heap_, empty_list);
    return reduce_binary_op(".", args[0].result(), ext_empty_list);
  }

  sym reduce_arguments__subterm_999_comma_arguments(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_binary_op(".", args[0].result(), args[2].result());
  }

  // list :- ...

  sym reduce_list__lbracket_rbracket(args_t &args)
  {
    if (check_mode_) return sym();

    con_cell empty_list("[]", 0);
    return sym(ext<cell>(heap_, empty_list));
  }

  sym reduce_list__lbracket_listexpr_rbracket(args_t &args)
  {
    if (check_mode_) return sym();

    return args[1];
  }

    /*
  ext<cell> build_list(cell c)
  {
      con_cell f_comma(",",2);
      con_cell f_dot(".",2);
      con_cell f_empty_list("[]",0);

      auto lst = heap_.new_str(f_dot);
      heap_.set_arg(lst, 0, f_empty_list);
      heap_.set_arg(lst, 1, f_empty_list);
      cell last = lst;

      while (c.tag() == tag_t::STR && heap_.functor(c) == f_comma) {
	  heap_.set_arg(last, 0, heap_.arg(c, 0));
	  auto newlst = heap_.new_str(f_dot);
	  heap_.set_arg(last, 1, newlst);
	  heap_.set_arg(newlst, 0, f_empty_list);
	  heap_.set_arg(newlst, 1, f_empty_list);
	  last = newlst;

	  c = heap_.arg(c, 1);
      }

      heap_.set_arg(last, 0, c);

      return lst;
  }
    */

  // listexpr :- ...

  sym reduce_listexpr__subterm_999(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_binary_op(".", args[0].result(), heap_.empty_list());
  }

  sym reduce_listexpr__subterm_999_comma_listexpr(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_binary_op(".", args[0].result(), args[2].result());
  }
  
  sym reduce_listexpr__subterm_999_vbar_subterm_999(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_binary_op(".", args[0].result(), args[2].result());
  }

  // constant :- ...

  sym reduce_constant__atom(args_t &args)
  {
    if (check_mode_) return sym();

    return args[0];
  }

  sym reduce_constant__number(args_t &args)
  {
    if (check_mode_) return sym();

    return args[0];
  }

  // number :- ...

  sym reduce_number__unsigned_number(args_t &args)
  {
    if (check_mode_) return sym();

    return args[0];
  }

  sym reduce_number__inf(args_t &args)
  {
    if (check_mode_) return sym();

    return sym();
  }

  sym reduce_number__nan(args_t &args)
  {
    if (check_mode_) return sym();

    return sym();
  }

  // unsigned_number :- ...

  sym reduce_unsigned_number__natural_number(args_t &args)
  {
    if (check_mode_) return sym();

    auto &tok = args[0].token();
    int_cell val(boost::lexical_cast<int>(tok.lexeme()));
    return sym(ext<cell>(heap_, val));
  }

  sym reduce_unsigned_number__unsigned_float(args_t &args)
  {
    if (check_mode_) return sym();

    return sym();
  }

  // atom :- ...

  sym reduce_atom__name(args_t &args)
  {
    if (check_mode_) return sym();

    con_cell con = heap_.atom(args[0].token().lexeme());
    return sym(ext<cell>(heap_, con));
  }

  sym reduce_atom__empty_brace(args_t &args)
  {
    if (check_mode_) return sym();

    con_cell con = heap_.atom(args[0].token().lexeme());
    return sym(ext<cell>(heap_, con));
  }

  sym reduce_atom__empty_list(args_t &args)
  {
    if (check_mode_) return sym();

    con_cell con = heap_.atom(args[0].token().lexeme());
    return sym(ext<cell>(heap_, con));
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

  ~term_parser_interim() {
      stack_.clear();
      args_.clear();
      var_name_map_.clear();
      name_var_map_.clear();
      result_ = term();
  }

  void set_debug(bool dbg) { is_debug_ = dbg; }

  bool is_accept() const { return accept_; }
  bool is_error() const { return error_; }
  ext<cell> get_result() const { return result_; }
  void init() {
      comments_.clear();
      current_state_ = 0;
      accept_ = false;
      error_ = false;
      check_mode_ = false;
      result_ = ext<cell>();
  }
  bool is_eof() {
      skip_whitespace();
      return lookahead().ordinal() == SYMBOL_EOF;
  }

  const std::vector<term_tokenizer::token> & get_comments() const
  {
      return comments_;
  }

  const std::string & get_var_name(ext<cell> &cell);

  void for_each_var_name(std::function<void (const ext<cell> &ref, const std::string &name)> f) const {
      for (auto e : var_name_map_) {
	  const ext<cell> &ref = e.first;
	  const std::string &name = e.second;
	  f(ref,name);
      }
  }

  void clear_var_names()
  {
      var_name_map_.clear();
      name_var_map_.clear();
  }
};

class term_parser_impl : public term_parser_gen<term_parser_interim, term_tokenizer, heap, term_ops>
{
public:
  term_parser_impl(term_tokenizer &tokenizer, heap &h, term_ops &ops)
    : term_parser_gen<term_parser_interim, term_tokenizer, heap, term_ops>(tokenizer, h, ops) { }
  ~term_parser_impl() = default;

  symbol_t to_symbol(const term_ops::op_entry &entry)
  {
    symbol_t symt;
    switch (entry.type) {
    case term_ops::XF: symt = SYMBOL_OP_XF; break;
    case term_ops::YF: symt = SYMBOL_OP_YF; break;
    case term_ops::FX: symt = SYMBOL_OP_FX; break;
    case term_ops::FY: symt = SYMBOL_OP_FY; break;
    case term_ops::XFY: symt = SYMBOL_OP_XFY; break;
    case term_ops::YFX: symt = SYMBOL_OP_YFX; break;
    case term_ops::XFX: symt = SYMBOL_OP_XFX; break;
    default: symt = SYMBOL_UNKNOWN; break;
    }
    return symt;
  }

  bool check_empty_brace(const term_tokenizer::token &token)
  {
      tokenizer().consume_token();
      skip_whitespace();
      if (tokenizer().peek_token().lexeme() == "}") {
	  tokenizer().consume_token();
	  symbol_t symt = SYMBOL_EMPTY_BRACE;
	  term_tokenizer::token tok(token);
	  tok.set_lexeme("{}");
	  lookahead_ = sym(current_state_, tok, symt);
	  return true;
      } else {
	  return false;
      }
  }

  bool check_empty_list(const term_tokenizer::token &token)
  {
      tokenizer().consume_token();
      skip_whitespace();
      if (tokenizer().peek_token().lexeme() == "]") {
	  tokenizer().consume_token();
	  symbol_t symt = SYMBOL_EMPTY_LIST;
	  term_tokenizer::token tok(token);
	  tok.set_lexeme("[]");
	  lookahead_ = sym(current_state_, tok, symt);
	  return true;
      } else {
	  return false;
      }
  }

  const sym & lookahead() const
  {
      return lookahead_;
  }

  const sym next_symbol()
  {
    skip_whitespace();

    auto tok = tokenizer().peek_token();
    auto lexeme = tok.lexeme();

    // std::cout << "Next_symbol(): " << lexeme << " " << tok.type() << "\n";

    bool consumed_name = false;

    symbol_t symt = SYMBOL_UNKNOWN;

    if (!tok.is_quoted()) {
	symt = predefined_symbols_[lexeme];
	// COMMA is a complicated parse symbol in Prolog.
	// We use it so it becomes easy to define a conflict free
	// meaningful grammar, but is is also an operator with
	// precedence 1000. The grammar COMMA takes precedence, but
	// in case it would yield a parse error we dispatch to the
	// standard operator class.
	if (symt == SYMBOL_COMMA) {
	    sym s(current_state_, tok, symt);
	    if (!check(s)) {
		symt = SYMBOL_UNKNOWN;
	    }
	}

	// Is this is an empty '{}' brace construction?
	if (symt == SYMBOL_LBRACE) {
	    lookahead_ = sym(current_state_, tok, symt);
	    check_empty_brace(tok);
	    return lookahead_;
	}

	if (symt == SYMBOL_LBRACKET) {
	    lookahead_ = sym(current_state_, tok, symt);
	    check_empty_list(tok);
	    return lookahead_;
	}
    }

    if (symt != SYMBOL_UNKNOWN) {
        lookahead_ = sym(current_state_, tok, symt);
        tokenizer().consume_token();
        return lookahead_;
    }

    switch (tok.type()) {
    case term_tokenizer::TOKEN_UNKNOWN:
    case term_tokenizer::TOKEN_PUNCTUATION_CHAR:
	// Should already be handled unless it was a COMMA
	if (lexeme == ",") {
	    break;
	}
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

    auto candidates = ops_.prec(lexeme);

    if (consumed_name && candidates.empty()) {
        return lookahead_;
    }

    if (candidates.empty()) {
        throw token_exception_unrecognized_operator(tok.pos(), tok.lexeme());
    }

    // Pick first candidate that doesn't yield parse error.
    // (Note that entries are sorted in precedence order.)

    term_ops::op_entry entry;
    for (auto e : candidates) {
	entry = e;
	symt = to_symbol(e);
	sym s(current_state_, tok, symt);
	if (check(s)) {
	    break;
	}
    }

    lookahead_ = sym(current_state_, tok, symt);
    lookahead_.set_precedence(entry.precedence);

    if (!consumed_name) {
        tokenizer().consume_token();
    }

    return lookahead_;
  }

  bool check(sym symbol) {
      old_lookahead_ = lookahead_;
      lookahead_ = symbol;
      check_mode_ = true;
      process_state();
      check_mode_ = false;
      lookahead_ = old_lookahead_;
      bool r = !error_;
      error_ = false;
      return r;
  }

  void process_next()
  {
    if (lookahead_.ordinal() == SYMBOL_UNKNOWN) {
        lookahead_ = next_symbol();
    }

    if (is_debug_) {
	std::cout << "In state " << current_state() << " SYM " << lookahead_.ordinal() << " lexeme " << lookahead_.token().lexeme() << " " << "@ " << lookahead_.token().pos().str() << "\n";
    }

    process_state();

    if (accept_) {
	lookahead_ = old_lookahead_;
	stack_.pop_back();
	lookahead_.set_old_state(0);
    }
  }
};

term_parser::term_parser(term_tokenizer &tok, term_env &env)
{
  impl_ = new term_parser_impl(tok, env.get_heap(), env.get_ops());
}

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

const term_tokenizer::token & term_parser::lookahead() const
{
    return impl_->lookahead().token();
}

const std::vector<term_tokenizer::token> & term_parser::get_comments() const
{
    return impl_->get_comments();
}

std::string term_parser::get_comments_string() const
{
    std::string str;
    for (auto &tok : get_comments()) {
	str += tok.lexeme();
    }
    return str;
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

void term_parser::clear_var_names()
{
    impl_->clear_var_names();
}

}}


