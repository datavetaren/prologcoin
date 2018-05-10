#include <iomanip>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "term.hpp"
#include "term_ops.hpp"

namespace prologcoin { namespace common {

#ifdef DEBUG_TERM
size_t heap::id_counter_ = 0;
#endif

std::string cell::str() const
{
    return inner_str() + ":" + tag().str();
}

std::string cell::inner_str() const
{
    switch (tag()) {
    case tag_t::REF: return static_cast<const ref_cell &>(*this).inner_str();
    case tag_t::CON: return static_cast<const con_cell &>(*this).inner_str();
    case tag_t::STR: return static_cast<const str_cell &>(*this).inner_str();
    case tag_t::INT: return static_cast<const int_cell &>(*this).inner_str();
    case tag_t::BIG: return static_cast<const big_cell &>(*this).inner_str();
    case tag_t::DAT: return static_cast<const dat_cell &>(*this).inner_str();
    default: return "?";
    }
    
}

std::string cell::boxed_str() const
{
    std::string s = inner_str();
    return "|" + std::string(std::max(0,20 - static_cast<int>(s.length())),
	     ' ') + s + " : " + static_cast<std::string>(tag()) + " |";
}

std::string cell::boxed_str_dat() const
{
    std::string s = hex_str0();
    return "|" + std::string(std::max(0,20 - static_cast<int>(s.length())),
			     ' ') + s + " : ... |";
}

std::string cell::hex_str() const
{
    return hex_str(raw_value());
}

std::string cell::hex_str0() const
{
    return hex_str(raw_value(), false);
}

std::string cell::hex_str(uint64_t value, bool skip_leading_0s)
{
    std::string s = "0x";
    if (value == 0 && skip_leading_0s) {
	s += "0";
	return s;
    }

    // Skip leading 0s
    size_t m = 60;
    bool leading0 = skip_leading_0s;
    for (size_t i = 0; i < 16; i++, m -= 4) {
	auto digit = (value >> m) & 0xf;
	if (leading0 && digit == 0) {
	    continue;
	}
	leading0 = false;
	if (digit < 10) {
	    s += ('0' + digit);
	} else {
	    s += ('a' + digit - 10);
	}
    }
    return s;
}

con_cell::con_cell(const std::string &name, size_t arity) : cell(tag_t::CON)
{
    assert(use_compacted(name, arity));
    size_t n = name.length();
    value_t v = static_cast<value_t>(1) << 60;
    for (size_t i = 0; i < n; i++) {
        value_t ch = (name[i] & 0x7f) | 0x80;
	v |= static_cast<value_t>(ch << (53-i*8));
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
	bool is_last = (b & 0x80) == 0;
	if (is_last) {
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

std::string con_cell::inner_str() const
{
    std::string s = (is_direct() ? name() : "[" + boost::lexical_cast<std::string>(atom_index()) + "]");
    if (arity() > 0) s += "/" + boost::lexical_cast<std::string>(arity());
    return s;
}

	//    return "|" + std::string(std::max(0,20 - static_cast<int>(s.length())), ' ') + s + " : " + static_cast<std::string>(tag()) + " |";
	//}

std::string int_cell::hex_str() const
{
    return cell::hex_str(value());
}

bool int_cell::is_char_chunk() const
{
    // Check if lowest 5 bits is 00000 or 00001. Then it encodes
    // a character chunk.
    auto v = value();
    auto lowest_5_bits = static_cast<uint8_t>(v) & 0x1f;
    if (lowest_5_bits != 0 && lowest_5_bits != 1) {
	return false;
    }
    for (size_t i = 0; i < 7; i++) {
	auto ch = static_cast<uint8_t>((v >> ((6-i)*8 + 5)) & 0xff);
	if (ch == 0) {
	    return true;
	}
	if (!is_valid_char(ch)) {
	    return false;
	}
    }
    return true;
}

std::string int_cell::as_char_chunk() const
{
    auto v = value();
    size_t i;
    std::string s = "";
    for (i = 0; i < 7; i++) {
	auto ch = (v >> ((6-i)*8 + 5)) & 0xff;
	if (ch == 0) {
	    break;
	}
	s += static_cast<char>(ch);
    }
    return s;
}

bool int_cell::is_last_char_chunk() const
{
    auto v = value();
    auto lowest_5_bits = static_cast<uint8_t>(v) & 0x1f;
    return lowest_5_bits == 0;
}

int_cell int_cell::encode_str(const std::string &str, bool has_more)
{
    return encode_str(str, 0, str.size(), has_more);
}

int_cell int_cell::encode_str(const std::string &str, size_t from,
			      size_t to, bool has_more)
{
    assert(to-from <= 7);

    size_t end = to;
    if (end > str.size()) {
	end = str.size();
    }

    int64_t v = 0;

    for (size_t i = from; i < end; i++) {
	v |= (static_cast<int64_t>(1) << (5+(6-(i-from))*8)) *
	      static_cast<uint8_t>(str[i]);
    }

    if (has_more) v |= 1;

    return int_cell(v);
}

std::string int_cell::inner_str() const
{
    std::string s;
    auto v = value();
    if (v >= -1000000 && value() <= 1000000) {
	s = boost::lexical_cast<std::string>(v);
    } else if (is_char_chunk()) {
	s = "'" + as_char_chunk() + "'";
	while (s.size() < 8) s += " ";
	if (!is_last_char_chunk()) {
	    s += "...";
	}
    } else {
	s = hex_str();
    }
    return s;
}

std::string dat_cell::inner_str() const
{
    std::stringstream ss;
    std::string s = cell::hex_str(raw_value() >> CELL_NUM_BITS_HALF, false);
    s = "0x" + s.substr(10);
    ss << std::left << std::setw(14) << s << std::right << " " << num_bits();
    return ss.str();
}

heap::heap() 
  : size_(0),
    external_ptrs_max_(0),
    empty_list_("[]", 0),
    dotted_pair_(".", 2),
    comma_(",", 2)
{
    new_block(0);
}

heap::~heap()
{
#ifdef DEBUG_TERM
    if (external_ptrs_.size() > 0) {
	std::cerr << "Warning: Heap destroyed while external pointers exist.\n";
	for (auto p : external_ptrs_) {
	    std::cout << "  " << p.first << " id=" << p.second << "\n";
	}
	assert(external_ptrs_.size() == 0);
    }
#endif
    for (auto *b : blocks_) {
	delete b;
    }
}

void heap::trim(size_t new_size)
{
    size_t heap_end = new_size > 0 ? new_size - 1 : 0;
    size_t block_index = find_block_index(heap_end);
    auto &block = find_block(heap_end);
    block.trim(new_size - block.offset());
    size_ = new_size;
    if (block_index+1 < blocks_.size()) {
	for (size_t i = block_index+1; i < blocks_.size(); i++) {
	    delete blocks_[i];
	}
	blocks_.resize(block_index+1);
	head_block_ = &block;
    }
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

bool heap::is_name(con_cell c, const std::string &name) const
{
    if (c.is_direct()) {
        return c.name() == name;
    } else {
        return atom_name(c) == name;
    }
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

cell heap::deref_with_cost(cell c, uint64_t &cost) const
{
    uint64_t cost_tmp = 1;
    while (c.tag() == tag_t::REF) {
      auto &rc = static_cast<ref_cell &>(c);
      size_t index = rc.index();
      cell referred = get(index);
      if (referred == c) {
        cost = cost_tmp;
	return c;
      }
      c = referred;
      cost_tmp++;
    }
    cost = cost_tmp;
    return c;
}

bool heap::is_list(const cell c) const
{
    cell l = deref(c);
    while (l != empty_list_) {
	if (!check_functor(l)) {
	    return false;
	}
	con_cell f = functor(l);
	if (f != dotted_pair_) {
	    return false;
	}
	l = arg(l, 1);
    }
    return true;
}

std::string heap::big_to_string(const boost::multiprecision::cpp_int &i, size_t base, size_t nbits) const
{
    using namespace boost::multiprecision;

    static const char BASE_STD[37] = "0123456789abcdefghijklmnopqrstuvwxyz";
    static const char BASE_58[59] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

    assert((base >= 2 && base <= 36) || base == 58);

    std::string s;
    const char *base_char = (base == 58) ? BASE_58 : BASE_STD;

    // Only pad with whole bit size in base58 to make it compatible
    // with bitcoin representation.

    cpp_int val = i;
    cpp_int maxval = 1;
    // When we use base 16 or base 58 we pad with 0s to make it compatible
    // with the common bitcoin representations.
    if (base == 16 || base == 58) {
	maxval <<= (nbits-1);
    } else {
	maxval = val;
    }

    while (maxval) {
	size_t digit = static_cast<size_t>(val % base);
	val -= digit;
	val /= base;
	maxval /= base;
	char ch = base_char[digit];
	s += ch;
    }
    std::reverse(s.begin(), s.end());
    return s;
}

std::string heap::big_to_string(cell big, size_t base, bool capital) const
{
    using namespace boost::multiprecision;

    cpp_int val;
    size_t nbits = 0;
    get_big(big, val, nbits);
    std::string s = big_to_string(val, base, nbits);
    if (capital) {
	boost::to_upper(s);
    }
    return s;
}

void heap::set_big(cell big, const uint8_t *bytes, size_t n)
{
    auto dc = deref(big);
    big_cell &b = static_cast<big_cell &>(dc);
    size_t index = b.index();
    check_index(index);
    auto bi = begin(b);
    for (size_t i = 0; i < n; i++, ++bi) {
	*bi = bytes[i];
    }
}

void heap::set_big(cell big, const boost::multiprecision::cpp_int &i)
{
    using namespace boost::multiprecision;

    cell dc = deref(big);
    big_cell &b = reinterpret_cast<big_cell &>(dc);
    size_t nbits = num_bits(b);
    big_inserter bi(*this, b);
    if (i != 0) {
	// It needs to be in the right most position of the bignum
	// (the bignum is in big endian form)
	auto nbytes = (nbits - ((msb(i) + 7) / 8)*8) / 8;
	while (nbytes) {
	    ++bi;
	    nbytes--;
	}
    }
    export_bits(i, bi, 8);
}

void heap::get_big(cell big, uint8_t *bytes, size_t n) const
{
    auto dc = deref(big);
    big_cell &b = static_cast<big_cell &>(dc);
    size_t index = b.index();
    check_index(index);
    check_index(index+(n+sizeof(cell)-1)/sizeof(cell)-1);
    auto bi = begin(b);
    for (size_t i = 0; i < n; i++, ++bi) {
	bytes[i] = *bi;
    }
}

void heap::print(std::ostream &out) const
{
    print(out, 0, size_);
}

void heap::print(std::ostream &out, size_t from, size_t to) const
{
    out << std::setw(8) << " " << std::setw(0) << "  ." << std::string(27, '-') << "." << std::endl;
    size_t dat_num = 0;
    for (size_t i = from; i < to; i++) {
	out << std::setw(8) << i << std::setw(0) << ": ";
	cell c = get(i);
	if (dat_num > 0) {
	    out << c.boxed_str_dat();
	    dat_num--;
	} else {
	    out << c.boxed_str();
	    if (c.tag() == tag_t::DAT) {
		dat_num = reinterpret_cast<dat_cell &>(c).num_cells() - 1;
	    }
	}
	out << std::endl;
    }
    out << std::setw(8) << " " << std::setw(0) << "  `" << std::string(27, '-') << "´" << std::endl;
}

void heap::print_status(std::ostream &out) const
{
    out << "Heap status: Size: " << size_ << " External refs: " << external_ptr_count() << " (at most it was " << external_ptrs_max_ << ")\n";
}

}}
