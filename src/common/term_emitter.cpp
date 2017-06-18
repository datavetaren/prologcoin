#include "term_emitter.hpp"

namespace prologcoin { namespace common {

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

void term_emitter::set_indent_width(size_t indent_width)
{
    indent_token_ = std::string(' ', indent_width);
}

void term_emitter::set_max_column(size_t max_column)
{
    max_column_ = max_column;
}

void term_emitter::indent()
{
    std::string indent_token(' ', indent_width_);

    for (auto i = 0; i < indent_level_; i++, column_ += indent_width_) {
	out_ << indent_token_;
    }
}

void term_emitter::increment_indent_level()
{
    indent_level_++;
}

void term_emitter::decrement_indent_level()
{
    indent_level_--;
}

void term_emitter::emit_token(const std::string &str)
{
    size_t len = str.size();
    if (column_ + len >= max_column_) {
	nl();
    }
    column_ += str.size();
    out_ << str;
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
	    stack_.push_back(rparen);
	    e.set_at_end(true);
	} 
	if (i == arity - 1) {
	    e.set_at_begin(true);
	} 
	e.set_as_arg(true);
	stack_.push_back(e);
	if (i == arity - 1) {
	    auto lparen = elem(con_cell("(",0));
	    lparen.set_as_token(true);
	    stack_.push_back(lparen);
	} else {
	    auto comma = elem(con_cell(", ",0));
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
	increment_indent_level();
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

void term_emitter::print_from_stack()
{
    (void)heap_;

    while (!stack_.empty()) {
	auto e = *(stack_.end() - 1);

	stack_.pop_back();

	if (e.as_token()) {
	    const con_cell &c = static_cast<const con_cell &>(e.cell_);
	    emit_token(c.name());
	    continue;
	}

	switch (e.cell_.tag()) {
	case tag_t::CON: {
	    const con_cell &c = static_cast<const con_cell &>(e.cell_);
	    emit_token(c.name());
	    break;
	    }
	case tag_t::STR:
	    emit_functor(e);
	    continue;
	case tag_t::REF:
	    emit_ref(e);
	    break;
	}

	if (e.at_end()) {
	    decrement_indent_level();
	}
    }
}

}}


