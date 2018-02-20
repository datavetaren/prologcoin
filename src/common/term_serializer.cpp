#include <iomanip>
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

	switch (t1.tag()) {
	case tag_t::CON:
	    write_con_cell(bytes,offset,reinterpret_cast<const con_cell &>(t1));
	    break;
	case tag_t::INT:
	    write_int_cell(bytes,offset,reinterpret_cast<const int_cell &>(t1));
	    break;
	case tag_t::REF:
	    write_ref_cell(bytes,offset,reinterpret_cast<const ref_cell &>(t1));
	    break;
	case tag_t::STR:
	    write_str_cell(bytes,offset,reinterpret_cast<const str_cell &>(t1));
	    break;
	case tag_t::BIG:
	    assert("TODO: Not implemented yet" == nullptr);
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
	int64_t v = 0;
	for (size_t j = 0; j < 7; j++) {
	    if (i+j < n) {
		v |= (static_cast<int64_t>(1) << (5+(6-j)*8)) *
		    static_cast<uint8_t>(str[i+j]);
	    }
	}
	if (i+7 < n) v |= 1;
	write_int_cell(bytes, bytes.size(), int_cell(v));
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
        case tag_t::REF: { 
	    ref_cell v = static_cast<ref_cell &>(t1);
	    if (!is_indexed(v) && env_.has_name(v)) {
		const std::string &name = env_.get_name(t1);
		write_ref_cell(bytes, bytes.size(), v);
		write_encoded_string(bytes, name);
	    }
	    break;
	    }
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
	term arg = env_.arg(c, i);
	size_t arg_offset = new_offset + i*sizeof(cell);
	stack_.push_back(std::make_pair(arg_offset,arg));
    }
}

term term_serializer::read(buffer_t &bytes)
{
    size_t offset = 0;
    return read(bytes, offset);
}

term term_serializer::read(buffer_t &bytes, size_t &offset)
{
    term_index_.clear();

    read_all_header(bytes, offset);

    size_t old_addr_base = cell_count(offset);
    size_t new_addr_base = env_.heap_size();

    while (offset < bytes.size()) {
	cell c = read_cell(bytes, offset);
	switch (c.tag()) {
	case tag_t::INT: env_.new_cell0(c); break;
	case tag_t::CON: {
	    auto &con = reinterpret_cast<const con_cell &>(c);
	    if (con.is_direct()) {
		env_.new_cell0(c);
	    } else {
		env_.new_cell0(con_cell(index_term(c,0), con.arity()));
	    }
	    break;
   	    }
	case tag_t::REF:
	case tag_t::STR: {
	    auto &pc = reinterpret_cast<const ptr_cell&>(c);
	    size_t new_addr;
	    if (pc.index() < old_addr_base) {
		// TODO: Error if index is missing (is_indexed(c) == false)
		new_addr = ref_cell(index_term(c, 0));
	    } else {
		new_addr = pc.index() - old_addr_base + new_addr_base;
	    }
	    cell new_cell = ptr_cell(c.tag(), new_addr);
	    env_.new_cell0(new_cell);
	    break;
   	    }
	case tag_t::BIG:
	    assert("To be implemented" == nullptr);
	    break;
	}
	offset += sizeof(cell);
    }

    return env_.heap_get(new_addr_base);
}

void term_serializer::read_all_header(buffer_t &bytes, size_t &offset)
{
    auto ver_t = read_cell(bytes, offset);
    if (ver_t.tag() != tag_t::CON) {
	throw serializer_exception_unexpected_data(ver_t, offset);
    }

    auto &v = reinterpret_cast<const con_cell &>(ver_t);
    if (v != con_cell("ver1",0)) {
	throw serializer_exception_unsupported_version(v);
    }

    offset += sizeof(cell);

    auto remap_c = read_cell(bytes, offset);

    if (remap_c.tag() != tag_t::CON) {
	throw serializer_exception_unexpected_data(ver_t, offset);
    }

    const con_cell &remap_cc = static_cast<const con_cell &>(remap_c);
    if (remap_cc != con_cell("remap",0)) {
	throw serializer_exception_unexpected_data(remap_cc, offset);
    }

    offset += sizeof(cell);

    bool cont = true;

    while (cont) {
	cell c = read_cell(bytes, offset);
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
		throw serializer_exception_unexpected_data(c, offset);
	    }
	}
    }
}

std::string term_serializer::read_encoded_string(buffer_t &bytes, size_t &offset)
{
    bool cont = true;
    std::string str;
    while (cont) {
	cell c = read_cell(bytes, offset);
	if (c.tag() != tag_t::INT) {
	    throw serializer_exception_unexpected_data(c, offset);
	}
	auto &ic = static_cast<const int_cell &>(c);
	if (!ic.is_char_chunk()){
	    throw serializer_exception_unexpected_data(c, offset);
	}
	str += ic.as_char_chunk();
	cont = !ic.is_last_char_chunk();
	offset += sizeof(cell);
    }
    return str;
}

void term_serializer::read_index(buffer_t &bytes, size_t &offset, cell c)
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

void term_serializer::print_buffer(buffer_t &bytes)
{
    for (size_t i = 0; i < bytes.size(); i += sizeof(cell::value_t)) {
	cell c = read_cell(bytes, i);
	std::cout << "[" << std::setw(5) << cell_count(i) << "]: " << c.str() << "\n";
    }
}

}}
