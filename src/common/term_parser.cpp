#include <boost/multiprecision/cpp_int.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include "term_env.hpp"
#include "term_parser.hpp"
#include "term_parser_gen.hpp"
#include "term_emitter.hpp"

namespace prologcoin { namespace common {

static const int BASE_STANDARD_CONV [128] = 
      { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 00
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 10
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 20
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, // 30
	-1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, // 40
        25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1, // 50
        -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, // 60
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1  // 70
      };
static const int BASE_58_CONV [128] =
      { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 00
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 10
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 20
	-1,  0,  1,  2,  3,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, // 30
	-1,  9, 10, 11, 12, 13, 14, 15, 16, -1, 17, 18, 19, 20, 21, -1, // 40
	22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1, -1, // 50
	-1, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46, // 60
	47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, -1, -1, -1, -1, -1  // 70
      };

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
  con_cell empty_list_;
  con_cell dotted_pair_;

  struct sym {
      sym()
	: ordinal_(SYMBOL_UNKNOWN),
	  result_(),
	  pos_(),
	  token_(),
          old_state_(-1),
          precedence_(0) { }

      sym( int old_state, const sym &other )
        : ordinal_(other.ordinal_),
	  result_(other.result_),
	  pos_(other.pos_),
	  token_(other.token_),
          old_state_(old_state),
          precedence_(other.precedence_) { }

      sym( int old_state, const term_tokenizer::token &t, symbol_t ordinal )
        : ordinal_(ordinal),
  	  result_(),
	  pos_(),
  	  token_(t),
          old_state_(old_state),
	  precedence_(0)
          { }

      sym( int old_state, symbol_t ord, const term result, const term pos)
	: ordinal_(ord),
	  result_(result),
	  pos_(pos),
	  token_(),
	  old_state_(old_state),
          precedence_(0) { }

      sym( const term result, const term pos)
	  : ordinal_(SYMBOL_UNKNOWN),
	    result_(result),
	    pos_(pos),
	    token_(),
	    old_state_(0),
	    precedence_(0) { }

    inline void clear() { ordinal_ = SYMBOL_UNKNOWN; }

    inline int old_state() const { return old_state_; }
    inline int precedence() const { return precedence_; }

    inline symbol_t ordinal() const { return ordinal_; }
    inline const term_tokenizer::token & token() const { return token_; }
    inline const term result() const { return result_; }
    inline const term pos() const { return pos_; }
    
    inline void set_ordinal(symbol_t ord) { ordinal_ = ord; }
    inline void set_token(const term_tokenizer::token &tok ) { token_ = tok; }
    inline void set_result(const term result) { result_ = result; }
    inline void set_pos(const term pos) { pos_ = pos; }
    inline void set_old_state(int no) { old_state_ = no; }
    inline void set_precedence(int prec) { precedence_ = prec; }

    private:
      symbol_t ordinal_;
      term result_;
      term pos_; // Position information mirroring result
      term_tokenizer::token token_;
      int old_state_;
      int precedence_;
  };

  std::vector<sym> stack_;
  sym lookahead_;
  sym old_lookahead_;

  std::vector<term_tokenizer::token> comments_;

  term result_;
  term positions_;
  bool accept_;
  bool error_;
  bool check_mode_;
  bool track_positions_;

  typedef std::vector<sym> args_t;
  args_t args_;

  typedef std::unordered_map<term, std::string> var_name_map_type;
  typedef std::unordered_map<std::string, term> name_var_map_type;

  var_name_map_type var_name_map_;
  name_var_map_type name_var_map_;

  friend class term_parser_gen<term_parser_interim, term_tokenizer, heap, term_ops>;

protected:
  inline int current_state() const { return current_state_; }

  inline const sym & lookahead() const { return lookahead_; }

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
    lookahead_.set_pos(result.pos());
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

  void parse_error(const std::vector<std::string> &desc,
		   const std::vector<int> &expected) {
    if (!check_mode_ && is_debug_) {
      std::cout << "parse_error(): at state " << current_state_ << "\n";
    }
    error_ = true;
    if (!check_mode_) {
	throw term_parse_exception(tokenizer().line_string(), lookahead_.token(), desc, expected, "Unexpected " + lookahead_.token().lexeme());
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
    positions_ = args[0].pos();
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

  inline term make_pos(const token_position &pos, size_t arity)
  {
      term term_pos = heap_.new_str(heap_.functor("pos", 2+arity));
      heap_.set_arg(term_pos, 0, int_cell(pos.line()));
      heap_.set_arg(term_pos, 1, int_cell(pos.column()));
      return term_pos;
  }

  inline void set_pos(term term_pos, size_t arg, term pos_arg)
  {
      heap_.set_arg(term_pos, 2+arg, pos_arg);
  }

  sym reduce_unary_op(const std::string &opname,
		      const token_position &pos,
		      term operand, term operand_pos)
  {   
    if (check_mode_) return sym();

    con_cell unop(opname, 1);
    auto newstr = heap_.new_str(unop);
    heap_.set_arg(newstr, 0, operand);

    term term_pos;
    if (track_positions()) {
	term_pos = make_pos(pos, 1);
	set_pos(term_pos, 0, operand_pos);
    }
    
    sym s(newstr, term_pos);
    s.set_precedence(ops_.prec(unop).precedence);
    return s;
  }

  sym reduce_binary_op(const std::string &opname,
		       const token_position &pos,
		       term operand0, term operand0_pos,
		       term operand1, term operand1_pos)
  {   
    con_cell binop(opname, 2);
    auto newstr = heap_.new_str(binop);
    heap_.set_arg(newstr, 0, operand0);
    heap_.set_arg(newstr, 1, operand1);

    term term_pos;
    if (track_positions()) {
	term_pos = make_pos(pos, 2);
	set_pos(term_pos, 0, operand0_pos);
	set_pos(term_pos, 1, operand1_pos);
    }

    sym s(newstr, term_pos);
    s.set_precedence(ops_.prec(binop).precedence);
    return s;
  }

  sym reduce_args(term arg, term arg_pos,
		  term rest, term rest_pos)
  {
      auto newstr = heap_.new_str(con_cell(".",2));
      heap_.set_arg(newstr, 0, arg);
      heap_.set_arg(newstr, 1, rest);

      term term_pos;
      if (track_positions()) {
	  term_pos = heap_.new_str(con_cell(".",2));
	  heap_.set_arg(term_pos, 0, arg_pos);
	  heap_.set_arg(term_pos, 1, rest_pos);
      }
      sym s(newstr, term_pos);
      return s;
  }

  sym reduce_unary_op_subterm(args_t &args)
  {
     return reduce_unary_op(args[0].token().lexeme(),
			    args[0].token().pos(),
			    args[1].result(),
			    args[1].pos());
  }

  sym reduce_binary_op_subterm(args_t &args)
  {
     return reduce_binary_op(args[1].token().lexeme(),
			     args[1].token().pos(),
			     args[0].result(),
			     args[0].pos(),
			     args[2].result(),
			     args[2].pos());
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
    auto arg_list_pos = args[1].pos();
    size_t num_args = heap_.list_length(arg_list);
    auto fname = args[0].token().lexeme();
    auto farity = num_args;

    con_cell f = heap_.functor(fname, farity);
    term fstr = heap_.new_str(f);
    term fstr_pos;

    if (track_positions()) {
	fstr_pos = make_pos(args[0].token().pos(), farity);
    }

    size_t index = 0;
    term lst = arg_list;
    term lst_pos = arg_list_pos;
    while (lst != empty_list_) {
        heap_.set_arg(fstr, index, heap_.arg(lst, 0));
	lst = heap_.arg(lst, 1);
	if (track_positions()) {
	    set_pos(fstr_pos, index, heap_.arg(lst_pos, 0));
	    lst_pos = heap_.arg(lst_pos, 1);
	}
	index++;
    }

    return sym(fstr, fstr_pos);
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

    return reduce_unary_op("{}", args[0].token().pos(),
			   args[1].result(), args[1].pos());
  }

  sym reduce_term_0__list(args_t &args)
  {
    if (check_mode_) return sym();

    return args[0];
  }

  sym reduce_term_0__string(args_t &args)
  {
    if (check_mode_) return sym();

    std::string lexeme = args[0].token().lexeme();
    bool first = true;

    term lst;
    for (auto ch : lexeme) {
	str_cell p = heap_.new_str0(dotted_pair_);
	if (first) { lst = p; first = false; }
	heap_.new_cell0(int_cell(ch));
    }
    if (!first) {
	heap_.new_cell0(empty_list_);
    } else {
	lst = empty_list_;
    }
    if (track_positions()) {
	return sym(lst, make_pos(args[0].token().pos(),0));
    } else {
	return sym(lst, term());
    }
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

      bool is_new = var_name[0] == '_';

      if (!is_new) {
          // Check if we have seen this name before
          auto found = name_var_map_.find(var_name);
          if (found != name_var_map_.end()) {
	      if (track_positions()) {
		  return sym(found->second, make_pos(var_token.pos(), 0));
	      } else {
		  return sym(found->second, term());
	      }
          }
      }

      // We didn't find this variable, so we create a new one.

      auto ref = heap_.new_ref();

      // Register this ref cell so we can give the same name for it
      // when printing.
      var_name_map_[ref] = var_name;
      name_var_map_[var_name] = ref;

      if (track_positions()) {
	  return sym(ref, make_pos(var_token.pos(), 0));
      } else {
	  return sym(ref, term());
      }
  }

  // arguments :- ...

  sym reduce_arguments__subterm_999(args_t &args)
  {
    if (check_mode_) return sym();

    // Construct list with single element

    con_cell empty_list("[]", 0);
    term ext_empty_list = empty_list;
    term ext_empty_list_pos = empty_list;

    return reduce_args(args[0].result(), args[0].pos(),
	  	       ext_empty_list, ext_empty_list_pos);
  }

  sym reduce_arguments__subterm_999_comma_arguments(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_args(args[0].result(), args[0].pos(),
		       args[2].result(), args[2].pos());
  }

  // list :- ...

  sym reduce_list__lbracket_rbracket(args_t &args)
  {
    if (check_mode_) return sym();

    con_cell empty_list("[]", 0);
    if (track_positions()) {
	return sym(empty_list, make_pos(args[0].token().pos(), 0));
    } else {
	return sym(empty_list, term());
    }
  }

  sym reduce_list__lbracket_listexpr_rbracket(args_t &args)
  {
    if (check_mode_) return sym();

    return args[1];
  }

  // listexpr :- ...

  sym reduce_listexpr__subterm_999(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_binary_op(".", args[0].token().pos(),
			         args[0].result(),
  			         args[0].pos(),
			         heap_.empty_list(),
			         heap_.empty_list());
  }

  sym reduce_listexpr__subterm_999_comma_listexpr(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_binary_op(".", args[1].token().pos(),
			         args[0].result(),
			         args[0].pos(),
			         args[2].result(),
			         args[2].pos());
  }
  
  sym reduce_listexpr__subterm_999_vbar_subterm_999(args_t &args)
  {
    if (check_mode_) return sym();

    return reduce_binary_op(".", args[1].token().pos(),
			         args[0].result(),
			         args[0].pos(),
			         args[2].result(),
			         args[2].pos());
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

    return args[0];
  }

  sym reduce_number__nan(args_t &args)
  {
    if (check_mode_) return sym();

    return args[0];
  }

  // unsigned_number :- ...

  void get_number_and_base(const std::string &str, std::string &num, int &base)
  {
      base = 10;
      size_t n = str.size();
      for (size_t i = 0; i < n; i++) {
	  auto ch = str[i];
	  if (ch == '\'') {
	      try {
		  base = boost::lexical_cast<int>(str.substr(0,i));
	      } catch (std::runtime_error &ex) {
		  base = -1;
		  num = str.substr(0,i);
		  return;
	      }
	      num = str.substr(i+1);
	      return;
	  }
      }
      // No quote, so it's an ordinary base 10 number
      num = str;
  }

  static inline int get_base_digit(const char ch, int base)
  {
      uint8_t code = static_cast<uint8_t>(ch);
      if (code >= 128) {
	  return -1;
      }
      if (base == 58) {
	  return BASE_58_CONV[code];
      } else {
	  return BASE_STANDARD_CONV[code];
      }
  }

  static inline bool is_inside_int(const std::string &num, int base)
  {
      using namespace boost::multiprecision;

      uint128_t current = 0;
      for (auto ch : num) {
	  int d = get_base_digit(ch, base);
	  current = current*base + d;
	  if (current > int_cell::max().value()) {
	      return false;
	  }
      }
      return true;
  }

  static inline int64_t get_int(const std::string &num, int base)
  {
      int64_t current = 0;
      for (auto ch : num) {
	  int d = get_base_digit(ch, base);
	  current = current*base + d;
      }
      return current;
  }

  static inline void to_cpp_int(const std::string &num, int base,
				boost::multiprecision::cpp_int &out)
  {
      out = 0;
      for (auto ch : num) {
	  int d = get_base_digit(ch, base);
	  out *= base;
	  out += d;
      }
  }

  sym reduce_unsigned_number__natural_number(args_t &args)
  {
    using namespace boost::multiprecision;
    if (check_mode_) return sym();

    auto &tok = args[0].token();
    auto &lexeme = tok.lexeme();

    int base = 0;
    std::string number;

    // Check if there's a base
    get_number_and_base(lexeme, number, base);
    if (is_inside_int(number, base)) {
	int_cell val(get_int(number, base));
	if (track_positions()) {
	    return sym(val, make_pos(tok.pos(),0));
	} else {
	    return sym(val, term());
	}
    } else {
	// Construct a bignum
	cpp_int val;
	to_cpp_int(number, base, val);
	big_cell big = heap_.new_big(val);
	if (track_positions()) {
	    return sym(big, make_pos(tok.pos(),0));
	} else {
	    return sym(big, term());
	}
    }
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
    if (track_positions()) {
	return sym(con, make_pos(args[0].token().pos(), 0));
    } else {
	return sym(con, term());
    }
  }

  sym reduce_atom__empty_brace(args_t &args)
  {
    if (check_mode_) return sym();

    con_cell con = heap_.atom(args[0].token().lexeme());
    if (track_positions()) {
	return sym(con, make_pos(args[0].token().pos(), 0));
    } else {
	return sym(con, term());
    }
  }

  sym reduce_atom__empty_list(args_t &args)
  {
    if (check_mode_) return sym();

    con_cell con = heap_.atom(args[0].token().lexeme());
    if (track_positions()) {
	return sym(con, make_pos(args[0].token().pos(), 0));
    } else {
	return sym(con, term());
    }
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
    predefined_symbols_["{"] = SYMBOL_LBRACE;
    predefined_symbols_["["] = SYMBOL_LBRACKET;
    predefined_symbols_["("] = SYMBOL_LPAREN;
    predefined_symbols_["}"] = SYMBOL_RBRACE;
    predefined_symbols_["]"] = SYMBOL_RBRACKET;
    predefined_symbols_[")"] = SYMBOL_RPAREN;
    predefined_symbols_["|"] = SYMBOL_VBAR;
    track_positions_ = false;
  }

  ~term_parser_interim() {
      stack_.clear();
      args_.clear();
      var_name_map_.clear();
      name_var_map_.clear();
      result_ = term();
      positions_ = term();
  }

  inline term_tokenizer & tokenizer() { return tokenizer_; }

  void set_debug(bool dbg) { is_debug_ = dbg; }

  bool is_accept() const { return accept_; }
  bool is_error() const { return error_; }
  term get_result() const { return result_; }
  term positions() const { return positions_; }
  int line(const term pos) const {
      auto line_term = heap_.arg(pos, 0);
      return static_cast<int>(reinterpret_cast<const int_cell &>(line_term).value());
  }
  int column(const term pos) const {
      auto col_term = heap_.arg(pos, 1);
      return static_cast<int>(reinterpret_cast<const int_cell &>(col_term).value());
  }
  term position_arg(const term pos, size_t arg) const { return heap_.arg(pos, 2+arg); }
  bool track_positions() const { return track_positions_; }
  void set_track_positions(bool b) { track_positions_ = b; }

  void init() {
      comments_.clear();
      current_state_ = 0;
      accept_ = false;
      error_ = false;
      check_mode_ = false;
      result_ = term();
      positions_ = term();
  }

  bool is_eof() {
      skip_whitespace();
      return lookahead().ordinal() == SYMBOL_EOF;
  }

  const std::vector<term_tokenizer::token> & get_comments() const
  {
      return comments_;
  }

  const std::string & get_var_name(term &cell);

  void for_each_var_name(std::function<void (const term ref, const std::string &name)> f) const {
      for (auto e : var_name_map_) {
	  const term ref = e.first;
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
        throw token_exception_unrecognized_operator(tokenizer().line_string(), tok.pos(), tok.lexeme());
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

  std::vector<std::string> get_expected(const term_parse_exception &ex) const {
    std::vector<std::string> lexemes;
    std::unordered_map<int, std::string> sym_to_str;
    for (auto x : predefined_symbols_) {
	sym_to_str[static_cast<int>(x.second)] = x.first;
    }
    for (auto sym_index : ex.expected_symbols()) {
	auto sym = static_cast<symbol_t>(sym_index);
	auto it = sym_to_str.find(sym_index);
	if (it != sym_to_str.end()) {
	    lexemes.push_back(it->second);
	} else {
	    lexemes.push_back(symbol_name(sym));
	}
    }
    return lexemes;
  }

  void parse_next()
  {
      while (!is_accept() && !is_error()) {
	  process_next();
      }
  }
};

term_parser::term_parser(term_tokenizer &tok, term_env &env)
{
  impl_ = new term_parser_impl(tok, env, env);
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

term term_parser::parse()
{
  impl_->init();
  impl_->parse_next();
  return impl_->get_result();
}

term term_parser::positions() const
{
    return impl_->positions();
}

int term_parser::line(const term pos) const
{
    return impl_->line(pos);
}

int term_parser::column(const term pos) const
{
    return impl_->column(pos);
}

term term_parser::position_arg(const term pos, size_t arg) const
{
    return impl_->position_arg(pos, arg);
}

bool term_parser::track_positions() const
{
    return impl_->track_positions();
}

void term_parser::set_track_positions(bool b)
{
    impl_->set_track_positions(b);
}

term_tokenizer & term_parser::tokenizer()
{
    return impl_->tokenizer();
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

std::vector<std::string> term_parser::get_expected(const term_parse_exception &ex) const
{
    return impl_->get_expected(ex);
}

bool term_parser::is_eof()
{
    return impl_->is_eof();
}

bool term_parser::is_error()
{
    return impl_->is_error();
}

void term_parser::for_each_var_name(std::function<void (const term ref, const std::string &name)> f) const {
    impl_->for_each_var_name(f);
}

void term_parser::clear_var_names()
{
    impl_->clear_var_names();
}

}}


