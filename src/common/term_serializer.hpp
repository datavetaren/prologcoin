#pragma once

#ifndef _common_term_serializer_hpp
#define _common_term_serializer_hpp

#include <memory>
#include <vector>
#include <queue>
#include <set>
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
    serializer_exception_unexpected_data(const cell c, size_t offset,
					 const std::string &expect) :
	serializer_exception("Unexpected data " + c.hex_str() + "(" + c.str() + ") at offset " + boost::lexical_cast<std::string>(offset) + "; expected " + expect) { }
};

class serializer_exception_unexpected_end : public serializer_exception
{
public:
    serializer_exception_unexpected_end(size_t offset,
					const std::string &context) :
	serializer_exception("Unexpected end of buffer at offset " + boost::lexical_cast<std::string>(offset) + "; while " + context) { }
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

class serializer_exception_missing_index : public serializer_exception
{
public:
    serializer_exception_missing_index(const cell c) :
	serializer_exception("Missing index entry for " + c.str()) { }
};

class serializer_exception_illegal_cell : public serializer_exception
{
public:
    serializer_exception_illegal_cell(const cell c, size_t offset,
				  const std::string &why) :
	serializer_exception("Illegal cell " + c.str() + " at offset "
			     + boost::lexical_cast<std::string>(offset)
			     + "; " + why) { }
};

class serializer_exception_dangling_pointer : public serializer_exception
{
public:
    serializer_exception_dangling_pointer(const cell c, size_t offset) :
	serializer_exception("Dangling pointer for " + c.str() 
			     + " at offset "
			     + boost::lexical_cast<std::string>(offset)) { }

};

class serializer_exception_illegal_functor : public serializer_exception
{
public:
    serializer_exception_illegal_functor(const cell f, size_t f_offset,
					 const cell s, size_t s_offset) :
	serializer_exception("Illegal functor " + f.str() + " at offset "
			     + boost::lexical_cast<std::string>(f_offset)
			     + " for " + s.str() + " at offset "
			     + boost::lexical_cast<std::string>(s_offset)) {}
};

class serializer_exception_illegal_dat : public serializer_exception
{
public:
    serializer_exception_illegal_dat(const cell d, size_t d_offset,
				     const cell s, size_t s_offset) :
	serializer_exception("Illegal data " + d.str() + " at offset "
			     + boost::lexical_cast<std::string>(d_offset)
			     + " for " + s.str() + " at offset "
			     + boost::lexical_cast<std::string>(s_offset)) {}
};

class serializer_exception_missing_argument : public serializer_exception
{
public:
    serializer_exception_missing_argument(const cell f, size_t f_offset,
					  const cell s, size_t s_offset) :
	serializer_exception("Missing argument for " + f.str() 
			     + " at offset "
			     + boost::lexical_cast<std::string>(f_offset)
			     + " due to " + s.str() + " at offset "
			     + boost::lexical_cast<std::string>(s_offset)) { }

};

class serializer_exception_dat_too_small : public serializer_exception
{
public:
    serializer_exception_dat_too_small(const cell d, size_t d_offset) :
	serializer_exception("DAT cell " + d.str() + " at offset "
			     + boost::lexical_cast<std::string>(d_offset)
			     + " is too small. Minimum is 1.") { }
};


class serializer_exception_dat_too_big : public serializer_exception
{
public:
    serializer_exception_dat_too_big(const cell d, size_t d_offset, size_t n_bytes) :
	serializer_exception("DAT cell " + d.str() + " at offset "
			     + boost::lexical_cast<std::string>(d_offset)
			     + " has a size that exceeds length of "
			       "serialized data (max "
			     + boost::lexical_cast<std::string>(n_bytes)
			     + " bytes)") { }
};

class serializer_exception_erroneous_argument : public serializer_exception
{
public:
    serializer_exception_erroneous_argument(const cell f, size_t f_offset,
					    const cell s, size_t s_offset) :
	serializer_exception("Erroneous argument " + f.str() 
			     + " at offset "
			     + boost::lexical_cast<std::string>(f_offset)
			     + " due to " + s.str() + " at offset "
			     + boost::lexical_cast<std::string>(s_offset)) { }

};

class serializer_exception_self_reference : public serializer_exception
{
public:
    serializer_exception_self_reference(const cell c, size_t offset) :
	serializer_exception("Self reference for " + c.str() 
			     + " at offset "
			     + boost::lexical_cast<std::string>(offset)) { }

};

class serializer_exception_cyclic_reference : public serializer_exception
{
public:
    serializer_exception_cyclic_reference(const cell c, size_t offset,
					  const std::string &path) :
	serializer_exception("Cyclic reference for " + c.str() 
			     + " at offset "
			     + boost::lexical_cast<std::string>(offset)
			     + "[" + path + "]") { }
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

namespace test {
class test_term_serializer;
}

class term_serializer {
public:
    typedef std::vector<uint8_t> buffer_t;

    term_serializer(term_env &env);
    ~term_serializer();

    void write(buffer_t &bytes, const term t);
    term read(const buffer_t &bytes);
    term read(const buffer_t &bytes, size_t n);

    void print_buffer(const buffer_t &bytes, size_t n);

    static inline cell read_cell(const buffer_t &bytes, size_t from_offset, const std::string &context)
        { cell::value_t raw_value = 0;
	  if (from_offset + 8 > bytes.size()) {
	      throw serializer_exception_unexpected_end(from_offset, context);
	  }
	  for (size_t i = 0; i < 8; i++) {
	      raw_value <<= 8;
	      raw_value |= read_byte(bytes, from_offset+7-i);
	  }
	  cell c = cell(raw_value);
	  return c;
	}

    static inline void write_cell(buffer_t &bytes, size_t offset, const untagged_cell c)
        { auto v = c.raw_value();
	  if (offset == bytes.size()) {
	      bytes.resize(offset+8);
	  }
          for (size_t i = 0; i < 8; i++) {
	      bytes[offset+i] = static_cast<uint8_t>(v & 0xff);
	      v >>= 8;
          }
        }

private:
    static inline uint8_t read_byte(const buffer_t &bytes, size_t from_offset)
        { return bytes[from_offset]; }

    void set_used(size_t heap_start, std::vector<bool> &used, size_t heap_index);
    bool is_used(size_t heap_start, std::vector<bool> &used, size_t heap_index);

    void integrity_check(size_t heap_start, size_t heap_end,
			 size_t old_hdr_size, size_t new_hdr_size,
			 std::vector<bool> &used);

    friend class test::test_term_serializer;

    inline size_t cell_count(size_t offset)
        { return offset / sizeof(cell); }
    inline size_t cell_count(buffer_t &bytes)
        { return cell_count(bytes.size()); }

    inline void write_int_cell(buffer_t &bytes, size_t offset, const int_cell c)
        { write_cell(bytes, offset, c); }

    inline void write_con_cell(buffer_t &bytes, size_t offset,const con_cell c)
        { if (c.is_direct()) {
	      write_cell(bytes, offset, c);
	  } else {
	      auto remap = remapped_term(c, cell_count(offset));
	      write_cell(bytes, offset, remap);
	  }
	}

    inline void write_ref_cell(buffer_t &bytes, size_t offset, const ref_cell c)
    { write_cell(bytes, offset, remapped_term(c.unwatch(), cell_count(offset))); }

    void write_str_cell(buffer_t &bytes, size_t offset, const str_cell c);
    void write_big_cell(buffer_t &bytes, size_t offset, const big_cell c);

    inline bool is_indexed(const term t)
        { return term_index_.is_indexed(t); }
    inline term remapped_term(term t, size_t cell_index)
        { switch (t.tag()) {
  	    case tag_t::RFW:
	      return ptr_cell(tag_t::REF,
			      index_term(reinterpret_cast<ref_cell &>(t).unwatch(),
					 cell_index));
	    case tag_t::REF:
	    case tag_t::BIG:
	    case tag_t::STR: {
		return ptr_cell(t.tag(), index_term(t, cell_index));
		}
	    case tag_t::CON: {
		auto f = reinterpret_cast<const con_cell &>(t);
		if (f.is_direct()) {
		    return t;
		} else {
		    auto cc = con_cell(index_term(f.to_atom(), cell_index), f.arity());
		    return cc;
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

    term read(const buffer_t &bytes, size_t n, size_t heap_start,
	      std::vector<bool> &used, size_t &offset,
	      size_t &old_hdr_size, size_t &new_hdr_size);
    void read_all_header(const buffer_t &bytes, size_t &offset);
    void read_index(const buffer_t &bytes, size_t &offset, cell c);
    std::string read_encoded_string(const buffer_t &bytes, size_t &offset);

    term_env &env_;

    indexor<term> term_index_;
    std::unordered_map<cell,cell> new_to_old_;
    std::vector<std::pair<size_t, term> > stack_;
    std::vector<term> temp_stack_;
    std::unordered_set<term> temp_set_;
};

}}

#endif
