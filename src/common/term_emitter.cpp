#include "term_emitter.hpp"

namespace prologcoin { namespace common {

term_emitter::term_emitter(std::ostream &out, heap &h, term_ops &ops)
        : out_(out),
	  heap_(h),
	  ops_(ops),
          column_(0),
          line_(0),
          indent_level_(0),
	  scan_mode_(false)
{
    set_max_column(78);
}


void term_emitter::print(cell c)
{
    stack_.push_back(elem(c,0));
    print_from_stack();
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
    scan_mode_ = true;
    size_t siz = stack_.size();
    stack_.push_back(elem(c));
    print_from_stack(siz);
    stack_.resize(siz);
    scan_mode_ = false;
    size_t len = column_ - current_column;
    column_ = current_column;
    indent_level_ = current_indent_level;
    return len;
}

void term_emitter::emit_token(const std::string &str)
{
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
}
	
void term_emitter::emit_error(const term_emitter::elem &, const std::string &msg)
{
    std::string err = "?[" + msg + "]?";
    emit_token(err);
}

void term_emitter::push_functor_args(size_t index, size_t arity)
{
    for (size_t i = 0; i < arity; i++) {
	auto arg = heap_[arity+index-i]; // Push last argument first
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

void term_emitter::emit_functor(const term_emitter::elem &e)
{
    // Emit functor. Check if the entire functor with args fits
    if (!scan_mode_ && !at_beginning()) {
	size_t len = get_emit_length(e.cell_);
	if (will_wrap(len)) {
	    nl();
	}
    }

    auto str = static_cast<const str_cell &>(e.cell_);
    auto index = str.index();
    if (!heap_.in_range(index)) {
	std::string msg("STR cell out of range " + boost::lexical_cast<std::string>(index));
	emit_error(e, msg);
	return;
    }

    auto fc = heap_[index];
    if (fc.tag() != tag_t::CON) {
	std::string msg("STR cell not pointing at functor " + boost::lexical_cast<std::string>(index));
	emit_error(e, msg);
	return;
    }

    auto f = static_cast<const con_cell &>(fc);
    // First check if this element is an operator with precedence
    auto p = ops_.prec(f);

    if (p.precedence == 0) {
	// Expand this functor as usual
	emit_token(f.name());
	push_functor_args(index, f.arity());
	return;
    }

    // We have an operator (with precedence)
    if (p.precedence != 0) {
	switch (p.type) {
        case term_ops::XF:
	    break;
	case term_ops::YF:
	    break;
	case term_ops::XFX:
	    break;
	case term_ops::XFY:
	    break;
	case term_ops::YFX:
	    break;
	case term_ops::FY:
	    break;
	case term_ops::FX:
	    break;
	}
    }
}

void term_emitter::emit_ref(const term_emitter::elem &e)
{
    const ref_cell &ref = static_cast<const ref_cell &>(e.cell_);
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
	    emit_token(c.name());
	} else {
	    switch (e.cell_.tag()) {
	    case tag_t::CON: {
		const con_cell &c = static_cast<const con_cell &>(e.cell_);
		emit_token(c.name());
		break;
	    }
	    case tag_t::STR:
		emit_functor(e);
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


