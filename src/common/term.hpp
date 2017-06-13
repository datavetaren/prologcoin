#pragma once

#ifndef _common_Term_hpp
#define _common_Term_hpp

#include <stdint.h>
#include <algorithm>
#include <boost/lexical_cast.hpp>

namespace prologcoin { namespace common {

//
// tag
//
// This represents the tag of the cell which is encoded in the
// lower 3 bits.
//
class tag_t {
public:
    enum kind_t { REF = 0, CON = 1, STR = 2, INT = 3, BIG = 4 };

    inline tag_t( kind_t k ) : kind_(k) { }

    inline bool operator == (kind_t k) const { return kind_ == k; }

    inline kind_t kind() const { return kind_; }

    inline operator uint64_t () const { return kind_; }

    inline operator std::string () const {
	switch (kind_) {
	case REF: return "REF";
	case CON: return "CON";
	case STR: return "STR";
	case INT: return "INT";
	case BIG: return "BIG";
	default : return "???";
	}
    }

    inline std::string str() const {
	return static_cast<std::string>(*this);
    }

private:
    kind_t kind_;
};

//
// cell
//
class cell {
public:
    typedef uint64_t value_t;

    inline cell() : raw_value_(0) { }

    inline cell(value_t raw_value) : raw_value_(raw_value) { }

    inline cell(tag_t t) : raw_value_(static_cast<value_t>(t) & 0x7) { }

    inline cell(tag_t t, value_t v)
                        : raw_value_((static_cast<value_t>(t) & 0x7) |
				     (v << 3)) { }

    inline cell(const cell &other) : raw_value_(other.raw_value_) { }

    inline tag_t tag() const { return tag_t(static_cast<tag_t::kind_t>(raw_value_&0x7));}

    inline value_t value() const { return raw_value_ >> 3; }

    inline void set_value(value_t v) { raw_value_ = (v << 3) | (raw_value_ & 0x7); }

    inline int64_t value_signed() const { return static_cast<int64_t>(raw_value_) >> 3; }

private:
    value_t raw_value_;
};

//
// REF cells (61 bits index)
// Like in WAM / Warren's Abstract Machine but with integer "pointers"
//
//  Integer value        REF 
// [xxxxxxxxxxxxxxxxxxxx 000]
//  bits 63-3       bits 0-2
//
//
class ref_cell : public cell {
public:
    inline ref_cell(size_t index) : cell( tag_t::REF, static_cast<value_t>(index) ) { }

    inline operator size_t () const { return static_cast<value_t>(value()); }

    inline size_t index() const { return (size_t)*this; }

    void set_index( size_t index ) { set_value(static_cast<value_t>(index)); }

    inline std::string str() const {
	std::string s = boost::lexical_cast<std::string>(index());
	return "|" + std::string(std::max(0,20 - static_cast<int>(s.length())), ' ') + s + " : " + static_cast<std::string>(tag()) + " |";
    }

    inline operator std::string () const {
	return str();
    }
};

// 
// CON cells (61 bits)
//
// If high (63rd) bit is set, then this encodes the name of the constant
// directly (7 bytes for constant string).
//
// If bit63==1 -->
//          f        o        o            t     arity CON
//     [ xxxxxxxx xxxxxxxx xxxxxxxx ... xxxxxxxx yyyyy 001]
// If bit63==0 -->
//            Index into const table         arity     CON
//     [ xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx yyyyyyyy yyyyy 001]
//                    48 bits               13 bits

class con_cell : public cell {
public:
    con_cell( const std::string &name, size_t arity );

    size_t arity() const;

    std::string name() const;
    size_t name_length() const;
    std::string name_and_arity() const;

    std::string str () const;

    inline operator std::string () const {
	return str();
    }

private:
    bool is_direct() const {
	return ((value() >> 60) & 0x1) != 0;
    }

    uint8_t get_name_byte(size_t index) const;
};

//
// INT cells (61 bits)
// Like in WAM / Warren's Abstract Machine
//

class int_cell : public cell {
private:
    typedef int64_t T;

    inline int_cell bin_op( const std::function<T(const T, const T)> &f,
			    const int_cell a,
			    const int_cell b) const {
	return int_cell(f((T)a,(T)b));
    }

public:
    inline int_cell(int64_t val) : cell(tag_t::INT, val) { }
    inline int_cell(int val) : cell(tag_t::INT, val) { }

    inline int_cell(const int_cell &other) : cell(other) { }

    inline operator T () const {
	return static_cast<T>(value_signed());
    }

    inline T value() const {
	return value_signed();
    }

    inline int_cell operator + (const int_cell other ) const {
	return bin_op( std::plus<T>(), *this, other );
    }

    inline int_cell operator - (const int_cell other ) const {
	return bin_op( std::minus<T>(), *this, other );
    }

    inline int_cell operator * (const int_cell other) const {
	return bin_op( std::multiplies<T>(), *this, other );
    }

    inline int_cell operator / (const int_cell other) const {
	return bin_op( std::divides<T>(), *this, other );
    }

    inline std::string str () const {
	std::string s = boost::lexical_cast<std::string>(value());
	return "|" + std::string(std::max(0,20 - static_cast<int>(s.length())), ' ') + s + " : " + static_cast<std::string>(tag()) + " |";
    }

   inline operator std::string () const {
       return str();
   }
};

} }

#endif


