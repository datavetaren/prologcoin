#pragma once

#ifndef _common_term_serializer_hpp
#define _common_term_serializer_hpp

#include <memory>
#include <vector>
#include <queue>
#include "term_env.hpp"

namespace prologcoin { namespace common {

class serializer_exception : public std::runtime_error
{
public:
    serializer_exception(const std::string &msg) : 
	std::runtime_error(msg) { }
};

class serializer_exception_unexpected_data : public serializer_exception
{
public:
    serializer_exception_unexpected_data(const cell c, size_t offset) :
	serializer_exception("Unexpected data " + c.hex_str() + " at offset " + boost::lexical_cast<std::string>(offset)) { }
};

class serializer_exception_unsupported_version : public serializer_exception
{
public:
    serializer_exception_unsupported_version(const con_cell c) :
	serializer_exception("Unsupported version '" + c.name() + "'") { }
};

class serializer_exception_unexpected_index : public serializer_exception
{
public:
    serializer_exception_unexpected_index(const ptr_cell pc,
					  size_t expected_index) :
	serializer_exception("Unexpected index " + pc.str()
		     + "; expected index: " 
		     + boost::lexical_cast<std::string>(expected_index)) { }
};

class serializer_exception_unexpected_functor : public serializer_exception
{
public:
    serializer_exception_unexpected_functor(const con_cell f, size_t offset) :
	serializer_exception("Unexpected functor " + f.str()
			     + " at offset "
			     + boost::lexical_cast<std::string>(offset)
			     + "; not seen before and is not indexed.") { }
};

template<typename T> class indexor {
public:
    inline size_t to_index(const T &t, size_t new_id)
    {
	size_t index = 0;
	auto it = T_to_id_.find(t);
	if (it == T_to_id_.end()) {
	    index = new_id;
	    T_to_id_[t] = index;
	} else {
	    index = it->second;
	}
	return index;
    }

    inline size_t & operator [](const T &t)
    {
	return T_to_id_[t];
    }

    inline bool is_indexed(const T &t)
    {
	return T_to_id_.find(t) != T_to_id_.end();
    }

    inline void clear()
    {
	T_to_id_.clear();
    }

private:
    std::unordered_map<T, size_t> T_to_id_;
};

class term_serializer {
public:
    typedef std::vector<uint8_t> buffer_t;

    term_serializer(term_env &env);
    ~term_serializer();

    void write(buffer_t &bytes, const term t);
    term read(buffer_t &bytes);

    void print_buffer(buffer_t &bytes);

private:
    inline size_t cell_count(size_t offset)
        { return offset / sizeof(cell); }
    inline size_t cell_count(buffer_t &bytes)
        { return cell_count(bytes.size()); }
    inline uint8_t read_byte(buffer_t &bytes, size_t from_offset)
        { return bytes[from_offset]; }

    inline cell read_cell(buffer_t &bytes, size_t from_offset)
        { cell::value_t raw_value = 0;
	  for (size_t i = 0; i < 8; i++) {
	      raw_value <<= 8;
	      raw_value |= read_byte(bytes, from_offset+7-i);
	  }
	  return cell(raw_value);
	}

    inline void write_cell(buffer_t &bytes, size_t offset, const cell c)
        { auto v = c.raw_value();
	  if (offset == bytes.size()) {
	      bytes.resize(offset+8);
	  }
          for (size_t i = 0; i < 8; i++) {
	      bytes[offset+i] = static_cast<uint8_t>(v & 0xff);
	      v >>= 8;
          }
        }
    inline void write_int_cell(buffer_t &bytes, size_t offset, const int_cell c)
        { write_cell(bytes, offset, c); }

    inline void write_con_cell(buffer_t &bytes, size_t offset,const con_cell c)
        { if (c.is_direct()) {
	      write_cell(bytes, offset, c);
	  } else {
	      write_cell(bytes, offset, remapped_term(c, cell_count(offset)));
	  }
	}

    inline void write_ref_cell(buffer_t &bytes, size_t offset, const ref_cell c)
        { write_cell(bytes, offset, remapped_term(c, cell_count(offset))); }

    void write_str_cell(buffer_t &bytes, size_t offset, const str_cell c);

    inline bool is_indexed(const term t)
        { return term_index_.is_indexed(t); }
    inline term remapped_term(const term t, size_t cell_index)
        { switch (t.tag()) {
	    case tag_t::REF:
	    case tag_t::STR: {
		auto p = static_cast<const ptr_cell &>(t);
		return ptr_cell(t.tag(), index_term(t, cell_index));
		}
	    case tag_t::CON: {
		auto f = static_cast<const con_cell &>(t);
		if (f.is_direct()) {
		    return t;
		} else {
		    return con_cell(index_term(f, cell_index), f.arity());
		}
	        }
	    case tag_t::INT: return t;
	    default: assert("Not yet implemented" == nullptr); return t;
	    }
	}

    inline size_t index_term(const term t, size_t cell_index)
        { return term_index_.to_index(t, cell_index); }

    void write_encoded_string(buffer_t &bytes, const std::string &str);
    void write_all_header(buffer_t &bytes, const term t);

    term read(buffer_t &bytes, size_t &offset);
    void read_all_header(buffer_t &bytes, size_t &offset);
    void read_index(buffer_t &bytes, size_t &offset, cell c);
    std::string read_encoded_string(buffer_t &bytes, size_t &offset);
    // term read_arg(buffer_t &bytes, size_t &offset);

    term_env &env_;

    indexor<term> term_index_;

    std::vector<std::pair<size_t, term> > stack_;

    // std::vector<term> stack_;
    // std::queue<term> queue_;
};

}}

#endif
