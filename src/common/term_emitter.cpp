#include "term_emitter.hpp"
#include "token_chars.hpp"

namespace prologcoin { namespace common {

term_emitter::term_emitter(std::ostream &out, heap &h, term_ops &ops)
        : out_(out),
	  heap_(h),
	  ops_(ops),
          column_(0),
          line_(0),
          indent_level_(0),
	  last_char_('\0'),
	  scan_mode_(false),
	  dotted_pair_(".", 2),
	  empty_list_("[]", 0),
	  var_naming_(nullptr),
	  var_naming_owned_(false)
{
    set_max_column(78);
}

term_emitter::~term_emitter()
{
    if (var_naming_owned_ && var_naming_) {
        delete var_naming_;
    }
}

void term_emitter::print(cell c)
{
    stack_.push_back(elem(deref(c),0));
    print_from_stack();
}

void term_emitter::set_var_naming(const std::unordered_map<ext<cell>, std::string> &var_naming)
{
    var_naming_ = const_cast<std::unordered_map<ext<cell>, std::string> *>(&var_naming);
    var_naming_owned_ = false;
}

void term_emitter::set_var_name(const ext<cell> &c, const std::string &name)
{
    if (var_naming_ == nullptr) {
        var_naming_ = new std::unordered_map<ext<cell>, std::string>;
	var_naming_owned_ = true;
    }
    (*var_naming_)[c] = name;
}

void term_emitter::nl()
{
    out_ << '\n';
    line_++;
    column_ = 0;
    indent();
}

void term_emitter::set_max_column(size_t max_column)
{
    max_column_ = max_column;
}

void term_emitter::indent()
{
    if (indent_level_ == 0) {
	return;
    }

    size_t to_col = indent_table_[indent_level_-1];

    if (column_ < to_col) {
	size_t num_spaces = to_col - column_;
	std::string spaces = std::string(num_spaces, ' ');
	if (!scan_mode_) {
	    out_ << spaces;
	}
	column_ = to_col;
    }
}

void term_emitter::increment_indent_level()
{
    if (!scan_mode_) {
	mark_indent_column();
    }
    indent_level_++;
}

void term_emitter::decrement_indent_level()
{
    indent_level_--;
}

void term_emitter::mark_indent_column()
{
    if (indent_level_ >= indent_table_.size()) {
	indent_table_.resize(indent_level_+1, 0);
    }
    indent_table_[indent_level_] = column_;
}

bool term_emitter::will_wrap(size_t len) const
{
    return column_ + len >= max_column_;
}

bool term_emitter::at_beginning() const
{
    if (column_ == 0) {
	return true;
    }
    if (indent_level_ == 0) {
	return false;
    }
    if (column_ == indent_table_[indent_level_-1]) {
	return true;
    }
    return false;
}

size_t term_emitter::get_emit_length(cell c)
{
    size_t current_column = column_;
    size_t current_indent_level = indent_level_;
    char current_last_char = last_char_;
    scan_mode_ = true;
    size_t siz = stack_.size();
    stack_.push_back(elem(c));
    print_from_stack(siz);
    stack_.resize(siz);
    scan_mode_ = false;
    size_t len = column_ - current_column;
    column_ = current_column;
    indent_level_ = current_indent_level;
    last_char_ = current_last_char;
    return len;
}

void term_emitter::emit_char(char ch)
{
    if (!scan_mode_ && will_wrap(1)) {
	nl();
    }
    if (!scan_mode_) {
	out_ << ch;
    }
    column_++;
}

void term_emitter::emit_token(const std::string &str)
{
    static const char *exempt = "(),[]{} ";

    char next_char = str.empty() ? '\0' : str[0];

    if (strchr(exempt, last_char_) == nullptr &&
	strchr(exempt, next_char) == nullptr) {
	bool follow_char_alnum = isalnum(next_char);
	if (static_cast<bool>(isalnum(last_char_)) == follow_char_alnum) {
	    emit_char(' ');
	}
    }

    size_t len = str.size();
    if (will_wrap(len)) {
	if (!scan_mode_) {
	    nl();
	}
    }
    if (!scan_mode_) {
	out_ << str;
    }
    column_ += str.size();

    last_char_ = str.empty() ? '\0' : str[str.size()-1];
}
	
void term_emitter::emit_error(const std::string &msg)
{
    std::string err = "?[" + msg + "]?";
    emit_token(err);
}

void term_emitter::push_functor_args(size_t index, size_t arity)
{
    for (size_t i = 0; i < arity; i++) {
	// Push last argument first
	auto arg = deref(heap_[arity+index-i]);
	auto e = elem(arg);
	if (i == 0) {
	    auto rparen = elem(con_cell(")",0));
	    rparen.set_as_token(true);
	    rparen.set_at_end(true);
	    stack_.push_back(rparen);
	} 
	e.set_as_arg(true);
	stack_.push_back(e);
	if (i == arity - 1) {
	    auto lparen = elem(con_cell("(",0));
	    lparen.set_as_token(true);
	    lparen.set_at_begin(true);
	    stack_.push_back(lparen);
	} else {
	    auto space = elem(con_cell(" ",0));
	    space.set_as_token(true);
	    stack_.push_back(space);
	    auto comma = elem(con_cell(",",0));
	    comma.set_as_token(true);
	    stack_.push_back(comma);
	}
    }
}

size_t term_emitter::get_precedence(cell c) const
{
    if (c.tag() != tag_t::STR) {
	return 0;
    }
    auto str = static_cast<const str_cell &>(c);
    size_t index = str.index();
    if (!heap_.in_range(index)) {
	return 0;
    }
    auto fc = heap_[index];
    if (fc.tag() != tag_t::CON) {
	return 0;
    }
    auto p = ops_.prec(fc);
    return p.precedence;
}

std::string term_emitter::name_ref(size_t index) const
{
    static const char ALPHABET[]
    { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
      'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
      'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4',
      '5', '6', '7', '8', '9'
    };

    // First character must be [A-Z].
    std::string s;
    size_t i = 0;

    while (index >= 26) {
	size_t digit = index % sizeof(ALPHABET);
	s += ALPHABET[digit];
	i++;
	index /= sizeof(ALPHABET);
    }

    s += ALPHABET[index];

    std::reverse(s.begin(), s.end());

    return s;
}

void term_emitter::wrap_paren(const term_emitter::elem &e)
{
    auto rparen = elem(con_cell(")",0));
    rparen.set_as_token(true);
    rparen.set_at_end(true);
    stack_.push_back(rparen);

    stack_.push_back(e);

    auto lparen = elem(con_cell("(",0));
    lparen.set_as_token(true);
    lparen.set_at_begin(true);
    stack_.push_back(lparen);
}



bool term_emitter::atom_name_needs_quotes(const std::string &name) const
{
    auto first = name[0];
    if (token_chars::is_capital_letter(first) ||
	token_chars::is_underline_char(first) ||
	token_chars::is_digit(first)) {
        return true;
    }

    std::function<int (char ch)> categorize = [&](char ch)
      { if (token_chars::is_alpha(ch)) { return 0; }
	if (token_chars::is_punctuation_char(ch)) { return 1; }
	if (token_chars::is_solo_char(ch)) { return 2; }
	if (token_chars::is_symbol_char(ch)) { return 3; }
	return -1;
      };

    int cat = categorize(name[0]);

    for (auto ch : name) {
        if (token_chars::is_layout_char(ch)) { return true; }
	// Using mixed categories? Then quote it!
	if (categorize(ch) != cat) {
	    return true;
	}
    }

    return false;
}

void term_emitter::emit_atom_name(const std::string &name)
{
    if (atom_name_needs_quotes(name)) {
        emit_token("'" + name + "'");
    } else {
        emit_token(name);
    }
}

void term_emitter::emit_functor(const con_cell &f, size_t index)
{
    std::string name = heap_.atom_name(f);
    emit_atom_name(name);
    push_functor_args(index, f.arity());
}

std::tuple<bool, cell, size_t> term_emitter::check_functor(cell c)
{
    c = deref(c);
    auto str = static_cast<const str_cell &>(c);
    auto index = str.index();
    if (!heap_.in_range(index)) {
	std::string msg("STR cell out of range " + boost::lexical_cast<std::string>(index));
	emit_error(msg);
	return std::make_tuple(false, cell(), 0);
    }

    auto fc = heap_[index];
    if (fc.tag() != tag_t::CON) {
	std::string msg("STR cell not pointing at functor CON " + boost::lexical_cast<std::string>(index));
	emit_error(msg);
	return std::make_tuple(false, cell(), 0);
    }

    return std::make_tuple(true, fc, index);
}

bool term_emitter::is_begin_alphanum(con_cell f) const
{
    std::string name = heap_.atom_name(f);
    return name.length() > 0 && isalnum(name[0]);
}

bool term_emitter::is_end_alphanum(con_cell f) const
{
    std::string name = heap_.atom_name(f);
    return !name.empty() && isalnum(name[name.size()-1]);
}

bool term_emitter::is_begin_alphanum(cell f) const
{
    switch (f.tag()) {
    case tag_t::CON:return is_begin_alphanum(static_cast<const con_cell &>(f));
    case tag_t::STR:
	{
	    auto str = static_cast<const str_cell &>(f);
	    auto index = str.index();
	    if (!heap_.in_range(index)) {
		return false;
	    }
	    auto fc = heap_[index];
	    if (fc.tag() != tag_t::CON) {
		return false;
	    }
	    return is_begin_alphanum(static_cast<const con_cell &>(fc));
	}
    case tag_t::INT: return true;
    case tag_t::BIG: return true;
    case tag_t::REF: return true;
    default: return false;
    }
}

bool term_emitter::is_end_alphanum(cell f) const
{
    switch (f.tag()) {
    case tag_t::CON:return is_end_alphanum(static_cast<const con_cell &>(f));
    case tag_t::STR:
	{
	    auto str = static_cast<const str_cell &>(f);
	    auto index = str.index();
	    if (!heap_.in_range(index)) {
		return false;
	    }
	    auto fc = heap_[index];
	    if (fc.tag() != tag_t::CON) {
		return false;
	    }
	    return is_end_alphanum(static_cast<const con_cell &>(fc));
	}
    case tag_t::INT: return true;
    case tag_t::BIG: return true;
    case tag_t::REF: return true;
    default: return false;
    }
}

void term_emitter::emit_space()
{
    elem e(con_cell(" ",0));
    e.set_as_token(true);
    stack_.push_back(e);
}

void term_emitter::emit_xf(cell x, con_cell f, bool x_ok)
{
    bool op_is_alnum = is_begin_alphanum(f);

    stack_.push_back(elem(f));
    if (op_is_alnum || f.arity() == 1) {
	emit_space();
    }
    if (x_ok) {
	stack_.push_back(elem(x));
    } else {
	wrap_paren(elem(x));
    }
}

void term_emitter::emit_fx(con_cell f, cell x, bool x_ok)
{
    bool op_is_alnum = is_end_alphanum(f);

    if (x_ok) {
	stack_.push_back(elem(x));
    } else {
	wrap_paren(elem(x));
    }
    if (op_is_alnum || f.arity() == 1) {
	emit_space();
    }
    stack_.push_back(elem(f));
}

void term_emitter::emit_xfy(cell x, con_cell f, cell y, bool x_ok, bool y_ok)
{
    bool op_is_alnum = is_end_alphanum(f);

    emit_fx(f, y, y_ok);
    if (op_is_alnum) {
	emit_space();
    }
    if (x_ok) {
	stack_.push_back(elem(x));
    } else {
	wrap_paren(elem(x));
    }
}

void term_emitter::emit_functor_elem(const term_emitter::elem &e)
{
    // Emit functor. Check if the entire functor with args fits
    if (!scan_mode_ && !at_beginning()) {
	size_t len = get_emit_length(e.cell_);
	if (will_wrap(len)) {
	    nl();
	}
    }

    bool r; cell fc; size_t index;
    std::tie(r, fc, index) = check_functor(e.cell_);
    if (!r) {
	return;
    }

    auto str = static_cast<const str_cell &>(e.cell_);
    auto f = static_cast<const con_cell &>(fc);

    if (f == dotted_pair_ || f == empty_list_) {
        emit_list(str);
	return;
    }

    auto p = ops_.prec(f);
    auto f_prec = p.precedence;

    // No operator or arity == 0? Then emit functor and exit.
    if (f_prec == 0 || f.arity() == 0 || f.arity() > 2) {
	emit_functor(f, index);
	return;
    }

    // Arity must be 1 or 2

    // This is an operator. Extract 1st arg.
    auto x = heap_.arg(str, 0);
    auto x_prec = get_precedence(x);

    if (f.arity() == 1) {
	switch (p.type) {
	case term_ops::XF: emit_xf(x, f, x_prec < f_prec); return;
	case term_ops::YF: emit_xf(x, f, x_prec <= f_prec); return;
	case term_ops::FX: emit_fx(f, x, x_prec < f_prec); return;
	case term_ops::FY: emit_fx(f, x, x_prec <= f_prec); return;
	default: emit_functor(f, index); return;
	}
    }

    // Arity must be 2

    auto y = heap_.arg(str, 1);
    auto y_prec = get_precedence(y);

    switch (p.type) {
    case term_ops::XFX: emit_xfy(x,f,y,x_prec<f_prec,y_prec<f_prec);return;
    case term_ops::XFY: emit_xfy(x,f,y,x_prec<f_prec,y_prec<=f_prec);return;
    case term_ops::YFX: emit_xfy(x,f,y,x_prec<=f_prec,y_prec<f_prec);return;
    default: emit_functor(f, index); return;
    }
}

void term_emitter::emit_list(const cell lst0)
{
    cell lst = lst0;

    auto lbracket = elem(con_cell("[",0));
    lbracket.set_as_token(true);
    lbracket.set_at_begin(true);

    auto rbracket = elem(con_cell("]",0));
    rbracket.set_as_token(true);
    rbracket.set_at_end(true);

    auto comma = elem(con_cell(",",0));
    comma.set_as_token(true);

    auto vbar = elem(con_cell("|",0));
    vbar.set_as_token(true);

    size_t lst_index = stack_.size();

    stack_.push_back(lbracket);

    while (lst != empty_list_ && lst.tag() == tag_t::STR) {
        stack_.push_back(elem(heap_.arg(lst, 0)));
        lst = heap_.arg(lst, 1);
	if (lst.tag() == tag_t::STR) {
	  bool r; cell fc; size_t index;
	  std::tie(r, fc, index) = check_functor(lst);
	  if (!r) {
	    break;
	  }
	  if (fc == dotted_pair_) {
	    stack_.push_back(comma);
	  } else {
	    stack_.push_back(vbar);
	  }
	} else if (lst != empty_list_) {
	  stack_.push_back(vbar);
	}
    }

    if (lst != empty_list_) {
        stack_.push_back(lst);
    }

    stack_.push_back(rbracket);

    std::reverse(stack_.begin() + lst_index, stack_.end());
}

void term_emitter::emit_ref(const term_emitter::elem &e)
{
    const ref_cell &ref = static_cast<const ref_cell &>(e.cell_);

    if (var_naming_ != nullptr) {
	ext<cell> search(heap_, ref);
	auto it = var_naming_->find(search);
	if (it != var_naming_->end()) {
	    const std::string &name = it->second;
	    emit_token(name);
	    return;
	}
    }

    emit_token(name_ref(ref.index()));
}

void term_emitter::emit_int(const term_emitter::elem &e)
{
    const int_cell &i = static_cast<const int_cell &>(e.cell_);
    emit_token(boost::lexical_cast<std::string>(i.value()));
}

void term_emitter::print_from_stack(size_t top)
{
    while (!stack_.empty() && stack_.size() > top) {

	if (scan_mode_ && column_ >= max_column_) {
	    return;
	}

	auto e = *(stack_.end() - 1);

	stack_.pop_back();

	if (e.as_token()) {
	    const con_cell &c = static_cast<const con_cell &>(e.cell_);
	    emit_token(heap_.atom_name(c));
	} else {
	    switch (e.cell_.tag()) {
	    case tag_t::CON: {
		const con_cell &c = static_cast<const con_cell &>(e.cell_);
		emit_atom_name(heap_.atom_name(c));
		break;
	    }
	    case tag_t::STR:
		emit_functor_elem(e);
		break;
	    case tag_t::INT:
		emit_int(e);
		break;
	    case tag_t::REF:
		emit_ref(e);
		break;
	    }
	}

	if (e.at_begin()) {
	    increment_indent_level();
	}

	if (e.at_end()) {
	    decrement_indent_level();
	}
    }
}

}}


