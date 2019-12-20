#include <iomanip>
#include <queue>
#include "term_serializer.hpp"

namespace prologcoin { namespace common {

term_serializer::term_serializer(term_env &env) : env_(env)
{
}

term_serializer::~term_serializer()
{
}

void term_serializer::write(buffer_t &bytes, const term t)
{
    write_all_header(bytes, t);

    size_t offset = bytes.size();

    write_cell(bytes, offset, t);

    stack_.push_back(std::make_pair(offset, t));

    while (!stack_.empty()) {
	size_t offset;
	term t1;

	std::tie(offset, t1) = stack_.back();
	stack_.pop_back();

	t1 = env_.deref(t1);

	switch (t1.tag()) {
	case tag_t::CON:
	    write_con_cell(bytes,offset,reinterpret_cast<const con_cell &>(t1));
	    break;
	case tag_t::INT:
	    write_int_cell(bytes,offset,reinterpret_cast<const int_cell &>(t1));
	    break;
	case tag_t::RFW:
	    t1 = reinterpret_cast<ref_cell &>(t1).unwatch();
	    // Fall through
	case tag_t::REF:
	    write_ref_cell(bytes,offset,reinterpret_cast<const ref_cell &>(t1));
	    break;
	case tag_t::STR:
	    write_str_cell(bytes,offset,reinterpret_cast<const str_cell &>(t1));
	    break;
	case tag_t::BIG:
	    write_big_cell(bytes,offset,reinterpret_cast<const big_cell &>(t1));
	    break;
	}
    }
}

void term_serializer::write_encoded_string(buffer_t &bytes, const std::string &str) {
    // Write a series of INT cells, with 7 bytes
    // of data, the lower 5 bits tells (before tag)
    // tells whether this continues or not.
    size_t n = str.size();
    for (size_t i = 0; i < n; i += 7) {
	auto ic = int_cell::encode_str(str, i, i+7, (i+7 < n));
	write_int_cell(bytes, bytes.size(), ic);
    }
}

void term_serializer::write_all_header(buffer_t &bytes,const term t)
{
    write_con_cell(bytes, bytes.size(), con_cell("ver1",0));
    write_con_cell(bytes, bytes.size(), con_cell("remap", 0));
    for (auto t1 : env_.iterate_over(t)) {
	switch (t1.tag()) {
	case tag_t::CON: case tag_t::STR: {
	    con_cell f = env_.functor(t1);
	    if (!f.is_direct() && !is_indexed(f)) {
		std::string name = env_.atom_name(f);
		write_con_cell(bytes, bytes.size(), f);
		write_encoded_string(bytes, name);
	    }
	    break;
	    }
        case tag_t::REF: case tag_t::RFW: { 
	    ref_cell v = static_cast<ref_cell &>(t1).unwatch();
	    if (!is_indexed(v) && env_.has_name(v)) {
		const std::string &name = env_.get_name(v);
		write_ref_cell(bytes, bytes.size(), v);
		write_encoded_string(bytes, name);
	    }
	    break;
	    }
	case tag_t::BIG: break;
        }
    }
    write_con_cell(bytes, bytes.size(), con_cell("pamer", 0));
}

void term_serializer::write_str_cell(buffer_t &bytes, size_t offset,
				     const str_cell c)
{
    // If we've seen this before, then we just reuse what we have
    if (is_indexed(c)) {
	write_cell(bytes, offset, remapped_term(c, 0));
	return;
    }

    auto f = env_.functor(c);
    write_cell(bytes, offset, remapped_term(c, cell_count(bytes)));
    write_con_cell(bytes, bytes.size(), f);
    size_t arity = f.arity();
    size_t new_offset = bytes.size();
    bytes.resize(bytes.size() + sizeof(cell)*arity);
    for (size_t i = 0; i < arity; i++) {
	term arg = env_.arg(c, arity-i-1);
	size_t arg_offset = new_offset + (arity-i-1)*sizeof(cell);
	stack_.push_back(std::make_pair(arg_offset,arg));
    }
}

void term_serializer::write_big_cell(buffer_t &bytes, size_t offset,
				     const big_cell c)
{
    // If we've seen this before, then we just reuse what we have
    if (is_indexed(c)) {
	write_cell(bytes, offset, remapped_term(c, 0));
	return;
    }

    // First write BIG ptr
    write_cell(bytes, offset, remapped_term(c, cell_count(bytes)));
    offset = bytes.size();

    auto index = reinterpret_cast<const big_cell &>(c).index();
    auto big_header = env_.heap_get(index);
    auto dat = reinterpret_cast<const dat_cell &>(big_header);
    auto num_dat = dat.num_cells();

    // Write DAT cell followed by untagged data
    bytes.resize(offset + sizeof(cell)*num_dat);
    for (size_t i = 0; i < num_dat; i++) {
	untagged_cell dc = env_.heap_get_untagged(index+i);
	write_cell(bytes, offset, dc);
	offset += sizeof(cell);
    }
}

term term_serializer::read(const buffer_t &bytes)
{
    return read(bytes, bytes.size());
}

term term_serializer::read(const buffer_t &bytes, size_t n)
{
    size_t offset = 0;
    size_t heap_start = env_.heap_size();
    size_t old_hdr_size = 0, new_hdr_size = 0;
    term t = read(bytes, n, offset, old_hdr_size, new_hdr_size);
    size_t heap_end = env_.heap_size();
    integrity_check(heap_start, heap_end, old_hdr_size, new_hdr_size);
    return t;
}

term term_serializer::read(const buffer_t &bytes, size_t n,
			   size_t &offset,
			   size_t &old_header_size,
			   size_t &new_header_size)
{
    term_index_.clear();

    size_t old_addr_base = offset;
    size_t new_addr_base = env_.heap_size();

    read_all_header(bytes, offset);

    size_t old_hdr_size = offset - old_addr_base;
    size_t new_hdr_size = env_.heap_size() - new_addr_base;

    size_t old_hdr = cell_count(old_addr_base + old_hdr_size);

    old_header_size = old_hdr_size;
    new_header_size = new_hdr_size;

    size_t num_dat = 0;

    while (offset < n) {
	cell c = read_cell(bytes, offset, "reading for term construction");

	// Process as untagged cells if num_dat > 0
	if (num_dat > 0) {
	    env_.new_cell0(c);
	    num_dat--;
	    offset += sizeof(cell);
	    continue;
	}

	switch (c.tag()) {
	case tag_t::INT: env_.new_cell0(c); break;
	case tag_t::CON: {
	    auto &con = reinterpret_cast<const con_cell &>(c);
	    if (con.is_direct()) {
		env_.new_cell0(c);
	    } else {
		if (!is_indexed(c)) {
		    throw serializer_exception_missing_index(c);
		}
		auto newcon = con_cell(index_term(c,0), con.arity());
		env_.new_cell0(newcon);
		new_to_old_[newcon] = c;
	    }
	    break;
   	    }
	case tag_t::RFW:
	    c = static_cast<ref_cell &>(c).unwatch();
	    // Fall through
	case tag_t::BIG:
	case tag_t::REF:
	case tag_t::STR: {
	    auto &pc = reinterpret_cast<const ptr_cell&>(c);
	    size_t new_addr;
	    if (pc.index() < old_hdr) {
		if (!is_indexed(c)) {
		    throw serializer_exception_missing_index(pc);
		}
		new_addr = index_term(c, 0);
	    } else {
		new_addr = pc.index() - old_hdr + new_addr_base + new_hdr_size;
	    }
	    cell new_cell = ptr_cell(c.tag(), new_addr);
	    env_.new_cell0(new_cell);
	    new_to_old_[new_cell] = c;
	    break;
   	    }
	case tag_t::DAT: {
	    auto &dc = reinterpret_cast<const dat_cell&>(c);
	    size_t nc = dc.num_cells();
	    if (dc.num_bits() < 1 || dc.num_cells() == 0) {
		throw serializer_exception_dat_too_small(c, offset);
	    }
	    if (offset + nc*sizeof(cell) > n) {
		throw serializer_exception_dat_too_big(c, offset, n);
	    }
	    env_.new_cell0(c);
	    num_dat = nc - 1;
	    break;
	    }
	}
	offset += sizeof(cell);
    }

    return env_.heap_get(new_addr_base + new_hdr_size);
}

void term_serializer::read_all_header(const buffer_t &bytes, size_t &offset)
{
    auto ver_t = read_cell(bytes, offset, "reading version");
    if (ver_t.tag() != tag_t::CON) {
	throw serializer_exception_unexpected_data(ver_t, offset, "version constant");
    }

    auto &v = reinterpret_cast<const con_cell &>(ver_t);
    if (v != con_cell("ver1",0)) {
	throw serializer_exception_unsupported_version(v);
    }

    offset += sizeof(cell);

    auto remap_c = read_cell(bytes, offset, "reading remap");

    if (remap_c.tag() != tag_t::CON) {
	throw serializer_exception_unexpected_data(remap_c, offset, "remap section");
    }

    const con_cell &remap_cc = static_cast<const con_cell &>(remap_c);
    if (remap_cc != con_cell("remap",0)) {
	throw serializer_exception_unexpected_data(remap_cc, offset, "remap section");
    }

    offset += sizeof(cell);

    bool cont = true;

    while (cont) {
	cell c = read_cell(bytes, offset, "reading remap index entry");
	offset += sizeof(cell);
	if (c == con_cell("pamer",0)) {
	    cont = false;
	} else {
	    switch (c.tag()) {
	    case tag_t::REF:
	    case tag_t::CON:
		read_index(bytes, offset, c);
		break;
	    default:
		throw serializer_exception_unexpected_data(c, offset, "ref/con in remap section");
	    }
	}
    }
}

std::string term_serializer::read_encoded_string(const buffer_t &bytes, size_t &offset)
{
    bool cont = true;
    std::string str;
    while (cont) {
	cell c = read_cell(bytes, offset, "reading encoded string");
	if (c.tag() != tag_t::INT) {
	    throw serializer_exception_unexpected_data(c, offset, "encoded string as INTs");
	}
	auto &ic = static_cast<const int_cell &>(c);
	if (!ic.is_char_chunk()){
	    throw serializer_exception_unexpected_data(c, offset, "encoded string as INTs");
	}
	str += ic.as_char_chunk();
	cont = !ic.is_last_char_chunk();
	offset += sizeof(cell);
    }
    return str;
}

void term_serializer::read_index(const buffer_t &bytes, size_t &offset, cell c)
{
    std::string name = read_encoded_string(bytes, offset);

    switch (c.tag()) {
    case tag_t::CON:
	index_term(c, env_.resolve_atom_index(name));
	break;
    case tag_t::REF: {
	auto t = env_.new_ref();
	auto &ref = reinterpret_cast<ref_cell &>(t);
	env_.set_name(ref, name);
	index_term(c, ref.index());
	break;
        }
    default:
	break;
    }
}

void term_serializer::integrity_check(size_t heap_start, size_t heap_end,
				      size_t old_hdr_size,
				      size_t new_hdr_size)
{
    std::vector<bool> checked;

    auto set_checked = [&](size_t heap_index) {
	size_t rel = heap_index - heap_start;
	if (checked.size() <= rel) {
	    checked.resize(rel+1);
	}
	checked[rel] = true;
    };

    auto is_checked = [&](size_t heap_index) {
	size_t rel = heap_index - heap_start;
	if (rel >= checked.size()) {
	    return false;
	}
	bool r = checked[rel];
	return r;
    };

    auto compute_old_index = [&](size_t heap_index) {
	return heap_index - heap_start - new_hdr_size
	       + cell_count(old_hdr_size);
    };

    auto compute_old_offset = [&](size_t heap_index) {
	return compute_old_index(heap_index) * sizeof(cell);
    };

    auto compute_old_cell = [&](cell new_cell) {
	auto it = new_to_old_.find(new_cell);
	if (it != new_to_old_.end()) {
	    return it->second;
	} else {
	    return new_cell;
	}
    };

    auto check_pointer = [&](ptr_cell ptrcell, size_t heap_index) {
	size_t index = ptrcell.index();
	if (index < heap_start || index >= heap_end) {
	    throw serializer_exception_dangling_pointer(
				compute_old_cell(ptrcell),
				compute_old_offset(heap_index));
	}
    };

    auto check_bignum = [&](ptr_cell bigcell, size_t heap_index) {
	size_t index = bigcell.index();
	auto c = env_.heap_get(index);
	if (c.tag() != tag_t::DAT) {
	    throw serializer_exception_illegal_dat(
		       compute_old_cell(c),
		       compute_old_offset(index),
		       compute_old_cell(bigcell), 
		       compute_old_offset(heap_index));
	}
    };

    auto check_functor = [&](str_cell strcell, size_t heap_index) {
	size_t index = strcell.index();
	auto c = env_.heap_get(index);
	if (c.tag() != tag_t::CON) {
	    throw serializer_exception_illegal_functor(
		       compute_old_cell(c),
		       compute_old_offset(index),
		       compute_old_cell(strcell), 
		       compute_old_offset(heap_index));
	}
    };

    auto check_functor_args = [&](str_cell strcell, size_t heap_index) {
	size_t index = strcell.index();
	auto c = env_.heap_get(index);
	auto f = reinterpret_cast<const con_cell &>(c);
	if (index + f.arity() >= heap_end) {
	    throw serializer_exception_missing_argument(
		       compute_old_cell(c),
		       compute_old_offset(index),
		       compute_old_cell(strcell),
		       compute_old_offset(heap_index));
	}
	// Arguments cannot be CON/n where n>0!
	size_t n = f.arity();
	for (size_t i = 0; i < n; i++) {
	    auto c = env_.heap_get(index+1+i);
	    switch (c.tag()) {
	    case tag_t::REF:
	    case tag_t::RFW:
	    case tag_t::INT:
	    case tag_t::STR:
	    case tag_t::BIG:
		break;
	    default:
		if (c.tag() == tag_t::CON) {
		    if (reinterpret_cast<con_cell &>(c).arity() == 0) {
			break;
		    }
		}
		throw serializer_exception_erroneous_argument(
			  compute_old_cell(c),
			  compute_old_offset(index+1+i),
			  compute_old_cell(strcell),
			  compute_old_offset(heap_index));
	    }
	}
    };

    auto arrow = [&](std::string &str) {
	if (!str.empty()) {
	    str += "->";
	}
    };

    auto compute_path = [&](size_t start, size_t end) {
	std::string path;
	size_t i = start;
	while (i != end) {
	    auto c = env_.heap_get(i);
	    auto &ref = static_cast<const ref_cell &>(c);
	    arrow(path);
	    path += compute_old_cell(ref).str();
	    i = ref.index();
	}
	arrow(path);
	path += compute_old_cell(env_.heap_get(i)).str();
	arrow(path);
	path += compute_old_cell(env_.heap_get(start)).str();
	return path;
    };

    auto check_ref_chain = [&](ref_cell refcell, size_t heap_index) {
	size_t index = heap_index;
	auto c = env_.heap_get(index);
	std::unordered_set<size_t> visit;
	while (c.tag().is_ref()) {
	    auto &ref = reinterpret_cast<const ref_cell &>(c);
	    check_pointer(ref, index);
	    visit.insert(index);
	    if (is_checked(index) || ref.index() == index) {
		std::for_each(visit.begin(), visit.end(), set_checked);
		return;
	    }
	    if (visit.find(ref.index()) != visit.end()) {
		// Ref cycle detected.
		throw serializer_exception_cyclic_reference(
			    compute_old_cell(refcell),
			    compute_old_offset(heap_index),
			    compute_path(heap_index, index));
	    }
	    index = ref.index();
	    c = env_.heap_get(index);
	}
    };

    for (size_t i = heap_start; i < heap_end; i++) {
	if (is_checked(i)) {
	    continue;
	}
	cell c = env_.heap_get(i);
	switch (c.tag()) {
	case tag_t::DAT: {
	    auto &datcell = reinterpret_cast<const dat_cell &>(c);
	    // Skip untagged cells after DAT cell
	    // (We've already checked the size of the DAT)
	    i += (datcell.num_cells()-1);
	    break;
	    }
	case tag_t::BIG: {
	    auto &bigcell = reinterpret_cast<const big_cell &>(c);
	    check_pointer(bigcell, i);
	    check_bignum(bigcell, i);
	    break;
	    }
	case tag_t::STR: {
	    auto &strcell = reinterpret_cast<const str_cell &>(c);
	    check_pointer(strcell, i);
	    check_functor(strcell, i);
	    check_functor_args(strcell, i);
	    break;
	    }
	case tag_t::REF: case tag_t::RFW: {
	    auto &refcell = reinterpret_cast<const ref_cell &>(c);
	    check_ref_chain(refcell, i);
	    break;
	    }
	}
    }
}

void term_serializer::print_buffer(const buffer_t &bytes, size_t n)
{
    size_t num_dat = 0;
    for (size_t i = 0; i < n; i += sizeof(cell::value_t)) {
	cell c = read_cell(bytes, i, "print_buffer");
	std::cout << "[" << std::setw(5) << cell_count(i) << "]: ";
	if (num_dat > 0) {
	    std::cout << c.boxed_str_dat();
	    num_dat--;
	} else {
    	    std::cout << c.boxed_str();
	    if (c.tag() == tag_t::DAT) {
		auto &dc = reinterpret_cast<const dat_cell &>(c);
		num_dat = dc.num_cells();
		assert(num_dat > 0);
		num_dat--;
	    }
	}
       std::cout << " [offset:" << std::setw(5) << i << "]\n";
    }
}

}}
