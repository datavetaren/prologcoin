#include <iomanip>
#include "term.hpp"

namespace prologcoin { namespace common {

std::string cell::str() const
{
    switch (tag()) {
    case tag_t::REF: return static_cast<const ref_cell &>(*this).str();
    case tag_t::CON: return static_cast<const con_cell &>(*this).str();
    case tag_t::STR: return static_cast<const str_cell &>(*this).str();
    case tag_t::INT: return static_cast<const int_cell &>(*this).str();
    default: return "?";
    }
}

con_cell::con_cell(const std::string &name, size_t arity) : cell(tag_t::CON)
{
    assert(name.length() <= 7);
    assert(arity <= 31);
    size_t n = name.length();
    value_t v = static_cast<value_t>(1) << 60;
    for (size_t i = 0; i < n; i++) {
	v |= static_cast<value_t>((name[i] & 0x7f)) << (53-i*8);
    }
    v |= arity;
    set_value(v);
}

size_t con_cell::arity() const
{
    if (is_direct()) {
	// Only 5 bits for arity
	return static_cast<size_t>(value() & 0x1f);
    } else {
	// 5+8=13 bits for arity
	return static_cast<size_t>(value() & ~(1 << 12));
    }
}

size_t con_cell::name_length() const
{
    assert(is_direct());

    value_t v = value();

    for (size_t i = 0; i < 7; i++) {
	uint8_t b = (v >> (53-i*8)) & 0xff;
	if (b == 0) {
	    return i;
	}
    }
    return 7;
}

uint8_t con_cell::get_name_byte(size_t index) const
{
    assert(is_direct());

    value_t v = value();
    uint8_t b = (v >> (53-index*8)) & 0xff;
    return b;
}

std::string con_cell::name() const
{
    assert(is_direct());

    size_t n = name_length();
    std::string s(n, ' ');
    for (size_t i = 0; i < n; i++) {
	s[i] = static_cast<char>(get_name_byte(i) & 0x7f);
    }
    return s;
}

std::string con_cell::name_and_arity() const
{
    assert(is_direct());

    return name() + "/" + boost::lexical_cast<std::string>(arity());
}

std::string con_cell::str() const
{
    std::string s = (is_direct() ? name() : "[" + boost::lexical_cast<std::string>(value()) + "]");
    if (arity() > 0) s += "/" + boost::lexical_cast<std::string>(arity());

    return "|" + std::string(std::max(0,20 - static_cast<int>(s.length())), ' ') + s + " : " + static_cast<std::string>(tag()) + " |";
}

void heap::print(std::ostream &out) const
{
    out << std::setw(8) << " " << std::setw(0) << "  ." << std::string(27, '-') << "." << "\n";
    for (size_t i = 0; i < size_; i++) {
	out << std::setw(8) << i << std::setw(0) << ": " << get(i).str() << "\n";
    }
    out << std::setw(8) << " " << std::setw(0) << "  `" << std::string(27, '-') << "´" << "\n";
}

}}
