#include <iomanip>
#include "term.hpp"
#include "term_ops.hpp"

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
    assert(use_compacted(name, arity));
    size_t n = name.length();
    value_t v = static_cast<value_t>(1) << 60;
    for (size_t i = 0; i < n; i++) {
	v |= static_cast<value_t>((name[i] & 0x7f)) << (53-i*8);
    }
    v |= arity;
    set_value(v);
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

heap::heap() 
  : size_(0),
    external_ptrs_max_(0),
    empty_list_("[]", 0),
    dotted_pair_(".", 2)
{
    new_block(0);
}



size_t heap::list_length(const cell lst0) const
{
    size_t n = 0;

    cell lst = lst0;
    while (lst != empty_list_) {
      n++;
      if (lst.tag() != tag_t::STR) {
	  break;
      }
      con_cell f = functor(lst);
      if (f != dotted_pair_) {
	  break;
      }
      lst = arg(lst, 1);
    }
    if (lst != empty_list_) {
      n++;
    }
    return n;
}

bool heap::check_functor(const cell c) const
{
    if (c.tag() != tag_t::STR) {
        return false;
    }
    auto str = static_cast<const str_cell &>(c);
    auto index = str.index();
    if (!in_range(index)) {
        return false;
    }

    auto fc = get(index);
    if (fc.tag() != tag_t::CON) {
        return false;
    }

    return true;
}

size_t heap::resolve_atom_index(const std::string &name) const
{
    auto found = atom_name_to_index_table_.find(name);
    if (found == atom_name_to_index_table_.end()) {
        // Not found. Create a new entry.
        size_t index = atom_index_to_name_table_.size();
        atom_index_to_name_table_.push_back(name);
	atom_name_to_index_table_[name] = index;
	return index;
    }
    return found->second;
}

//
// Dereference chain of REF cells.
//
// TODO: What do to with GBL? Perhaps we just treat them specially?
// GBL cells point to global heap, which then have REF cells. It feels
// good to have a firewall between the two. Yet, some extra logic is
// needed to manually "go through" that firewall. Perhaps another helper
// function would do, e.g. deref_global(c)
//
cell heap::deref(cell c) const
{
    while (c.tag() == tag_t::REF) {
      auto &rc = static_cast<ref_cell &>(c);
      size_t index = rc.index();
      cell referred = get(index);
      if (referred == c) {
	return c;
      }
      c = referred;
    }
    return c;
}

void heap::print(std::ostream &out) const
{
    out << std::setw(8) << " " << std::setw(0) << "  ." << std::string(27, '-') << "." << "\n";
    for (size_t i = 0; i < size_; i++) {
	out << std::setw(8) << i << std::setw(0) << ": " << get(i).str() << "\n";
    }
    out << std::setw(8) << " " << std::setw(0) << "  `" << std::string(27, '-') << "´" << "\n";
}

void heap::print_status(std::ostream &out) const
{
    out << "Heap status: Size: " << size_ << " External refs: " << external_ptr_count() << " (at most it was " << external_ptrs_max_ << ")\n";
}



}}
