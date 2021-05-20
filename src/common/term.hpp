#pragma once

#ifndef _common_Term_hpp
#define _common_Term_hpp

#include <stdint.h>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <boost/lexical_cast.hpp>
#include <bitset>

#include <boost/noncopyable.hpp>
#include <iostream>
#include <boost/multiprecision/cpp_int.hpp>

// #define DEBUG_TERM

//
// term
//
// A term comes from mathematics, where the more commonly known "formula"
// consists of terms. For example, the left hand side of '=', X + 3, is a
// term. Likewise, the right hand side '=', f(17, Y), is another term.
//
// X + 3 = f(17, Y)
//
// Terms can be decomposed into terms, and the grammar is quite simple:
//
// term ::= constant
//        | functor(<term>, <term>, ..., <term>)
//        | Variable
//
// Terms can be efficiently represented with cells. A cell is typically
// a fixed sized integer word (preferbly matching the machine's word
// size.) In our case a cell is represented with an (unsigned) 64-bit
// integer.
//
// The lower 3 bits of this cell represents a TAG (which gives us 8
// combinations.) In WAM (Warren's Abstract Machine) the cells of
// REF, INT, CON, STR are usually enough to represent anything.
// However, other datatypes can be added such as DBL (double),
// BIG (bignums), ...
//
// The cell type REF (reference) is used to represent variables that
// can be bound. For unbound variables, following a chain of REF cells
// always ends up at a REF cell pointing at itself. For example,
//
// The term: p(Z, h(Z,W), f(W)) as:
//
// Heap
//
// [0]:    | 1    STR |
// [1]:    | h/2  CON |    (functor h with 2 args)
// [2]:    | 2    REF |
// [3]:    | 3    REF |
// [4]:    | 5    STR |
// [5]:    | f/1  CON |    (functor f with 1 arg)
// [6]:    | 3    REF |
// [7]:    | 8    STR | <--- p(Z, h(Z,W), f(W))
// [8]:    | p/3  CON |    (functor p with 3 args)
// [9]:    | 2    REF |
// [10]:   | 1    STR |
// [11]:   | 5    STR |
//
// This example is taken from the book
// "Warren's Abstract Machine (A Tutorial Reconstruction)"
// by Hassan Ait-Kaci
//
// (An excellent book in my opinion that tells how you can compile
//  Prolog programs into an efficient representation.)
//

namespace prologcoin { namespace common {

class term_emitter;
class heap;

//
// tag
//
// This represents the tag of the cell which is encoded in the
// lower 3 bits.
//
class tag_t {
public:
    enum kind_t { REF = 0, RFW = 1, INT = 2, BIG = 3, CON = 4, STR = 5, DAT = 6, FWD = 7 };

    inline tag_t( kind_t k ) : kind_(k) { }

    inline bool operator == (kind_t k) const { return kind_ == k; }

    inline kind_t kind() const { return kind_; }

    inline bool is_ref() const { return (static_cast<unsigned int>(kind_) & 6) == 0; }

    inline operator uint64_t () const { return kind_; }

    inline operator std::string () const {
	return str();
    }

    inline std::string str() const {
	switch (kind_) {
	case REF: return "REF";
	case RFW: return "RFW";
	case CON: return "CON";
	case STR: return "STR";
	case INT: return "INT";
	case BIG: return "BIG";
	case DAT: return "DAT";
	case FWD: return "FWD";
	default : return "???";
	}
    }

private:
    kind_t kind_;
};

class ref_cell; // Forward

//
// cell
//
class untagged_cell {
public:
    static const size_t CELL_NUM_BITS = 64;

    typedef uint64_t value_t;

    inline untagged_cell() = default;
   
    inline untagged_cell(const untagged_cell &other) = default;

    inline untagged_cell(value_t raw_value) : raw_value_(raw_value) { }

    inline value_t raw_value() const { return raw_value_; }

    // inline operator value_t () const { return raw_value_; }

    // inline operator value_t & () { return raw_value_; }

protected:
    inline void set_raw_value(value_t v) { raw_value_ = v; }

private:
    value_t raw_value_;
};

class cell_byte {
public:
    typedef uint8_t value_t;
    typedef value_t type;

private:
    inline void set_byte(size_t index, value_t v)
    { auto byte_off = index % sizeof(untagged_cell);
      auto bit_off = 8*byte_off;
      auto bit_mask = ~(static_cast<untagged_cell::value_t>(0xff) << bit_off);
      auto bit_val = static_cast<untagged_cell::value_t>(v) << bit_off;
      cached_ = (cached_.raw_value() & bit_mask) | bit_val;
      dirty_ = true;
    }

    inline value_t get_byte(size_t index) const
    { auto byte_off = index % sizeof(untagged_cell);
      auto bit_off = 8*byte_off;
      return static_cast<value_t>(cached_.raw_value() >> bit_off);
    }

    static inline untagged_cell * flush_and_invalidate(const cell_byte &cb0) {
	cell_byte &cb = const_cast<cell_byte &>(cb0);
	auto *r = cb.base_;
	if (!cb.dirty_) {
	    cb.base_ = nullptr;	    
	    return r;
	}
	if (cb.base_) cb.base_[cb.index_/sizeof(untagged_cell)] = cb.cached_;
	cb.base_ = nullptr;
	cb.dirty_ = false;
	return r;
    }

public:
    inline cell_byte(untagged_cell *base, size_t i, size_t n) :
	base_(base), index_(i), n_(n), cached_(*base), dirty_(false) { }

    cell_byte(const cell_byte &other) = default;
    cell_byte(cell_byte &&other) = default;

    inline ~cell_byte() { if (base_ && dirty_) base_[index_/sizeof(untagged_cell)] = cached_; }

    inline void set_address(untagged_cell *base, size_t index) {
	flush_and_invalidate(*this);
        base_ = base;
	index_ = index;
	cached_ = *base_;
    }

    inline void operator = (const cell_byte &other) {
	if (base_ && dirty_) base_[index_/sizeof(untagged_cell)] = cached_;
	base_ = other.base_;
	index_ = other.index_;
	cached_ = other.cached_;
	dirty_ = other.dirty_;
    }
	
    inline void operator ++ () {
	if (base_ && ((index_+1) % sizeof(untagged_cell)) == 0) {
	    if (dirty_) {
		base_[index_/sizeof(untagged_cell)] = cached_;
		dirty_ = false;
	    }
	    if ((index_+1) < n_) {	    
		cached_ = base_[(index_+1)/sizeof(untagged_cell)];
	    }
	}
	index_++;
    }

    inline void operator >>= (size_t n)
    { set_byte(index_, get_byte(index_) >> n); }

    inline void operator = (value_t v)
    { set_byte(index_, v); }

    inline operator value_t () const
    { return get_byte(index_); }

    untagged_cell *base_;
    size_t index_;
    size_t n_;
    untagged_cell cached_;
    bool dirty_;
};

}}

namespace boost {
    template<> struct make_unsigned<prologcoin::common::cell_byte> {
	typedef prologcoin::common::cell_byte type;
    };
}

namespace prologcoin { namespace common {

class cell : public untagged_cell {
public:
    static const int TAG_SIZE_BITS = 3;

    inline cell(const heap &, const cell other) : untagged_cell(other.raw_value()) { }

    inline cell() = default;

    inline cell(value_t raw_value) : untagged_cell(raw_value) { }

    inline cell(tag_t t) : untagged_cell(static_cast<value_t>(t) & 0x7) { }

    inline cell(tag_t t, value_t v)
                        : untagged_cell((static_cast<value_t>(t) & 0x7) |
				     (v << 3)) { }

    inline cell(const cell &other) = default;

    inline tag_t tag() const { return tag_t(static_cast<tag_t::kind_t>(raw_value()&0x7));}

    // inline operator value_t () { return raw_value() >> 3; }

    inline value_t value() const { return raw_value() >> 3; }

    inline void set_value(value_t v) { set_raw_value((v << 3) | (raw_value() & 0x7)); }

    inline int64_t value_signed() const { return static_cast<int64_t>(raw_value()) >> 3; }

    inline bool operator == (const cell other) const { return raw_value() == other.raw_value(); }
    inline bool operator != (const cell other) const { return raw_value() != other.raw_value(); }
    inline bool operator < (const cell other) const { return raw_value() < other.raw_value(); }
    inline bool operator <= (const cell other) const { return raw_value() <= other.raw_value(); }
    inline bool operator > (const cell other) const { return raw_value() > other.raw_value(); }
    inline bool operator >= (const cell other) const { return raw_value() >= other.raw_value(); }

    inline operator bool () const { return raw_value() != 0; }

    std::string str() const;
    std::string inner_str() const;
    std::string boxed_str() const;
    std::string boxed_str_dat() const;

    std::string hex_str() const;
    std::string hex_str0() const;

protected:
    static std::string hex_str(uint64_t value, bool skip_leading_0s = true);
};

class term : public cell {
public:
   term() : cell(0) { }
   term(cell c) : cell(c) {}
   term(const term &other) = default;
   term(const heap &src, cell c) : cell(src,c) { }
   term(heap &src, cell c) : cell(src,c) { }   
};

//
// Exceptions
//

class term_exception : public std::runtime_error {
public:
    term_exception(const std::string &msg) : std::runtime_error(msg) { }
};

class heap_index_out_of_range_exception : public term_exception {
public:
    heap_index_out_of_range_exception(size_t index, size_t max_sz);
};

class expected_con_cell_exception : public term_exception {
public:
    expected_con_cell_exception(size_t index, cell c)
	: term_exception( std::string("Expected CON cell at index ") + boost::lexical_cast<std::string>(index) + "; was " + c.tag().str()) { }
};

class expected_str_cell_exception : public term_exception {
public:
    expected_str_cell_exception(cell c)
      : term_exception( std::string("Expected STR cell; was " + c.tag().str())) { }
};

class coin_security_exception : public term_exception {
public:
    coin_security_exception();
};

//
// ptr_cell this is not a real cell, but any class that uses the upper
// bits for referencing another cell is inheriting from this class:
//
//  Integer value        REF 
// [xxxxxxxxxxxxxxxxxxxx 000]
//  bits 63-3       bits 0-2
//
//

class ptr_cell : public cell {
public:
    inline ptr_cell(const ptr_cell &other) : cell( other ) { }
    inline ptr_cell(tag_t tag, size_t index) : cell( tag, static_cast<value_t>(index) ) { }

    inline operator size_t () const { return static_cast<value_t>(value()); }

    inline size_t index() const { return (size_t)*this; }

    void set_index( size_t index ) { set_value(static_cast<value_t>(index)); }

    inline bool operator < (const ptr_cell other) const {
	return index() < other.index();
    }

    inline std::string inner_str() const {
	return boost::lexical_cast<std::string>(index());
    }

    inline bool is_ptr_cell(cell c) {
      auto tag = c.tag();
      return tag == tag_t::REF ||
             tag == tag_t::RFW ||
             tag == tag_t::STR ||
             tag == tag_t::BIG;
    }
};

//
// REF cells
// Like in WAM / Warren's Abstract Machine.
//
class ref_cell : public ptr_cell {
public:
    inline ref_cell() : ptr_cell(tag_t::REF, 0) { }
    inline ref_cell(size_t index) : ptr_cell(tag_t::REF, index) { }
    inline ref_cell(size_t index, bool) : ptr_cell(tag_t::RFW, index) { }
    inline ref_cell watch() const { return ref_cell(index(), true); }
    inline ref_cell unwatch() const { return ref_cell(index()); }
    inline bool watched() const { return tag() == tag_t::RFW; }
};

//
// STR cells
// Like in WAM / Warren's Abstract Machine.
//
class str_cell : public ptr_cell {
public:
    inline str_cell(ptr_cell pcell) : ptr_cell(pcell) { }
    inline str_cell(size_t index) : ptr_cell(tag_t::STR, index) { }
};

//
// FWD cells
// Forwading cells used for cycle detection during unification.
class fwd_cell : public ptr_cell {
public:
    inline fwd_cell(ptr_cell pcell) : ptr_cell(pcell) { }
    inline fwd_cell(size_t index) : ptr_cell(tag_t::FWD, index) { }
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
    static const size_t MAX_ARITY = 8192 - 1;
    static const size_t MAX_NAME_LENGTH = 256;
  
    inline con_cell() : cell(tag_t::CON) { }
    inline con_cell(const con_cell &other) : cell(other) { }
    inline con_cell( size_t atom_index, size_t arity ) : cell(tag_t::CON)
    {
        set_value((atom_index << 13) | arity);
    }

    con_cell( const std::string &name, size_t arity );

    static inline bool use_compacted( const std::string &name, size_t arity)
    {
        return name.length() <= 7 && arity <= 31;
    }    

    size_t arity() const
    {
        if (is_direct()) {
    	    // Only 5 bits for arity
	    return static_cast<size_t>(value() & 0x1f);
        } else {
	    // 5+8=13 bits for arity
	    return static_cast<size_t>(value() & ((1 << 12)-1));
        }
    }

    con_cell to_atom() const
    {
	con_cell c = *this;
	c.set_value(value() & ~(0x1f));
	return c;
    }

    inline size_t atom_index() const
    {
        assert(!is_direct());
	return static_cast<size_t>(value() >> 13);
    }

    std::string name() const;
    size_t name_length() const;
    std::string name_and_arity() const;

    std::string inner_str () const;

    // inline operator std::string () const {
    //	return xxxstr();
    // }

    bool is_direct() const {
	return ((value() >> 60) & 0x1) != 0;
    }

private:
    uint8_t get_name_byte(size_t index) const;

    friend class heap;
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

protected:
    inline int_cell(tag_t t, int64_t val) : cell(t, val) { }

public:
    inline int_cell(size_t val) : cell(tag_t::INT, val) { }
    inline int_cell(int64_t val) : cell(tag_t::INT, val) { }
    inline int_cell(int val) : cell(tag_t::INT, val) { }

    inline int_cell(const int_cell &other) : cell(other) { }

    static inline int64_t saturate(int64_t val) {
	if (val < min().value()) {
	    return min().value();
	} else if (val > max().value()) {
	    return max().value();
	} else {
	    return val;
	}
    }

    static inline uint64_t saturate(uint64_t val) {
	if (val > static_cast<uint64_t>(max().value())) {
	    return static_cast<uint64_t>(max().value());
	} else {
	    return val;
	}
    }

    inline operator T () const {
	return static_cast<T>(value_signed());
    }

    inline T value() const {
	return value_signed();
    }

    inline int_cell operator + (const int_cell other ) const {
	return bin_op( std::plus<T>(), *this, other );
    }

    inline int_cell & operator ++ () {
	set_value(value() + 1);
	return *this;
    }

    inline int_cell & operator -- () {
	set_value(value() - 1);
	return *this;
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

    inline bool operator == (const int other) const {
	return value() == other;
    }

    inline bool operator == (const int_cell &other) const {
	return value() == other.value();
    }

    inline bool operator != (const int_cell &other) const {
	return value() != other.value();
    }

    inline bool operator > (const int_cell &other) const {
	return value() > other.value();
    }

    inline bool operator < (const int_cell &other) const {
	return value() < other.value();
    }

    inline bool operator >= (const int_cell &other) const {
	return value() >= other.value();
    }

    inline bool operator <= (const int_cell &other) const {
	return value() <= other.value();
    }

    inline int_cell negate() const {
	return int_cell(saturate(-value()));
    }

    inline int_cell abs() const {
        if (is_negative()) {
	    return int_cell(saturate(-value()));
	} else {
	    return *this;
	}
    }

    inline int_cell sign() const {
	return is_negative() ? int_cell(-1) : int_cell(1);
    }
    
    inline bool is_negative() const {
	return value() < 0;
    }

    inline bool is_zero() const {
	return value() == 0;
    }

    static inline const int_cell max() {
	static const int_cell max_ = int_cell(std::numeric_limits<int64_t>::max() >> cell::TAG_SIZE_BITS);
	return max_;
    }

    static inline const int_cell min() {
	static const int_cell min_ = int_cell(std::numeric_limits<int64_t>::min() >> cell::TAG_SIZE_BITS);
	return min_;
    }

    std::string inner_str () const;
    std::string hex_str() const;

    inline operator std::string () const {
       return str();
    }

    static int_cell encode_str(const std::string &str, bool has_more);

    static int_cell encode_str(const std::string &str, size_t from,
			       size_t to, bool has_more);

private:
    friend class term_serializer;

    bool is_char_chunk() const;
    bool is_last_char_chunk() const;
    std::string as_char_chunk() const;

    inline bool is_valid_char(uint8_t ch) const
    { return ch >= 1 && ch <= 127; }
};

//
// BIG
// This is to use a binary blob of data. Like STR it points at the
// beginning of the block which first contains the number of cells.
//
class big_cell : public ptr_cell {
public:
    inline big_cell(const big_cell &other) : ptr_cell(other) { }
    inline big_cell(size_t index) : ptr_cell(tag_t::BIG, index) { }
};

class dat_cell : public int_cell {
private:
    static const size_t CELL_NUM_BITS_HALF = CELL_NUM_BITS / 2;

    static const size_t NUM_SIZE_BITS = CELL_NUM_BITS_HALF - TAG_SIZE_BITS;
    
public:
    static const size_t CELL_NUM_BYTES_HALF = CELL_NUM_BITS / 2 / 8;

    // Lower 4 bytes (half cell) = number of bits
    // (if negative then the number lives separately, not
    //  directly on heap.)
    // Upper 4 bytes (half cell) = binary data

    inline dat_cell(size_t num_bits)
        : int_cell(tag_t::DAT, static_cast<int64_t>(num_bits)) { }

    inline size_t num_bits() const
    { auto mask = (static_cast<untagged_cell::value_t>(1) << NUM_SIZE_BITS) - 1;
      auto v = value();
      auto n = v & mask;
      return n;
    }

    // [32] means reserved for 32 bits storing size.
    //
    // 288 bits = 5 cells (fits within [32] + 32 + 64 + 64 + 64 + 64)
    // 256 bits = 5 cells (fits within [32] + 32 + 64 + 64 + 64 + 32)
    // 224 bits = 4 cells (fits within [32] + 32 + 64 + 64 + 64)
    //
    // This means that 33 bytes (typical bitcoin private key, 32 bytes
    // + 1 byte for meta data = 264 bits) can be stored within 5 cells.
    // (6 cells if you count the BIG reference.)
    //
    // Not optimal, but it is convenient to have everything represented
    // as terms.
    //
    inline size_t num_half_cells() const
    { return 1 + (num_bits() + CELL_NUM_BITS_HALF - 1) / CELL_NUM_BITS_HALF; }

    inline size_t num_cells() const
    { return (num_half_cells() + 1) / 2; }

    std::string inner_str() const;    
};

class big_header : public dat_cell {
public:
    inline big_header(size_t num_bits) : dat_cell(num_bits) { }
};

template<typename T> class big_iterator_base : public T {
public:
    big_iterator_base(heap &h, size_t index, size_t num);
    big_iterator_base(heap &h, size_t index, size_t num, bool);
    big_iterator_base(const big_iterator_base<T> &other);
    big_iterator_base<T> & operator ++();

    inline int operator - (const big_iterator_base<T> &other) const
    { return i_ - other.i_; }

    inline bool operator == (const big_iterator_base<T> &other) const {
	return i_ == other.i_;
    }
    inline bool operator != (const big_iterator_base<T> &other) const {
	return ! operator == (other);
    }
    
protected:
    heap &heap_;
    size_t index_;
    int i_;
    int end_;
    cell_byte cell_byte_;
};

class big_iterator : public big_iterator_base<std::iterator<std::random_access_iterator_tag, cell_byte> > {
    using it = std::iterator<std::random_access_iterator_tag, cell_byte>;
public:
    inline big_iterator(heap &h, size_t index, size_t num) : big_iterator_base<it>(h, index, num) { }
    inline big_iterator(heap &h, size_t index, size_t num, bool b) : big_iterator_base<it>(h, index, num, b) { }
    inline big_iterator(const big_iterator &other) : big_iterator_base<it>(other) { }

    cell_byte & operator * ();
};

class const_big_iterator : public big_iterator_base<std::iterator<std::random_access_iterator_tag, cell_byte> > {
    using it = std::iterator<std::random_access_iterator_tag, cell_byte>;
public:
    inline const_big_iterator(const heap &h, size_t index, size_t num) : big_iterator_base<it>(const_cast<heap &>(h), index, num) { }
    inline const_big_iterator(const heap &h, size_t index, size_t num, bool b) : big_iterator_base<it>(const_cast<heap &>(h), index, num, b) { }
    inline const_big_iterator(const big_iterator &other) : big_iterator_base<it>(other) { }

    const cell_byte & operator * () const;
};

class big_inserter {
public:
    big_inserter(heap &heap, big_cell big);

    inline cell_byte & operator * () {
	return *it_;
    }

    inline void operator ++ () {
	++it_;
    }

private:
    big_iterator it_;
};

class heap; // Forward
    
//
// heap_block
//
// We don't want to keep _all_ heap blocks in memory. We can
// cache those that are frequent.
//
class heap_block : private boost::noncopyable {
public:

    friend class garbage_collector;
    static const size_t MAX_SIZE = 8192; // 64k

    inline heap_block(heap &h) : heap_block(h, 0) { }
    inline heap_block(heap &h, size_t index)
        : heap_(h), index_(index), offset_(index*MAX_SIZE),
	  size_(0), changed_(false) {
    }
    ~heap_block() = default;

    inline const cell * cells() const { return &cells_[0]; }
    inline cell * cells() { return &cells_[0]; }  

    inline bool has_changed() const {
        return changed_;
    }
    inline void clear_changed() {
        changed_ = false;
    }

    bool is_head_block() const;

    inline size_t index() const { return index_; }
    inline size_t offset() const { return offset_; }
    inline size_t size() const { return size_; }
    inline bool is_full() const { return size() == MAX_SIZE; }

    inline void set_index(size_t new_index) {
      index_ = new_index;
      offset_ = new_index * MAX_SIZE;
    }

    inline cell & operator [] (size_t addr) {
        if (!changed_) { changed_ = true; modified(); }
	return cells_[addr - offset_];
    }

    inline const cell & operator [] (size_t addr) const {
	return cells_[addr - offset_];
    }

    inline const cell & get(size_t index) const {
        return cells_[index];
    }

    inline cell & get(size_t index) {
        return cells_[index];
    }

    inline void set(size_t index, cell c) {
        cells_[index] = c;
    }

    inline bool can_allocate(size_t n) const {
	return size_ + n <= MAX_SIZE;
    }

    inline size_t allocate(size_t n) {
	size_t addr = offset_ + size_;
	size_ += n;
	return addr;
    }

    inline size_t allocate0(size_t n) {
	memset(&cells_[size_], 0, n*sizeof(untagged_cell));
	return allocate(n);
    }
    
    inline void trim(size_t n) {
	size_ = n;
    }

    // Will transform REF into RFW if value is true and
    //      transform RFW into REF if value is false.
    inline void watch(size_t addr, bool value) {
        cell &c = cells_[addr - offset_];
	if (c.tag() == tag_t::REF && value) {
	    ref_cell &r = static_cast<ref_cell &>(c);
	    r = ref_cell(r.index(), true);
	} else if (c.tag() == tag_t::RFW && !value) {
	    ref_cell &r = static_cast<ref_cell &>(c);
	    r = ref_cell(r.index());
	}
    }

    inline bool watched(size_t addr) const {
        auto c = cells_[addr - offset_];
	return c.tag() == tag_t::RFW;
    }

    void modified();
    
private:
    heap &heap_;
    size_t index_;
    size_t offset_;
    size_t size_;
    bool changed_;
    cell cells_[MAX_SIZE];
};

//
// Externally typed cell references.
//
// We keep track of which heap the cell comes from. Also the heap
// gets notified so that whenever a heap GC happens the address can be
// updated.
// 

#if 0
template<typename T> class ext {
public:
    inline ext() : heap_(nullptr), ptr_()
    {
#ifdef DEBUG_TERM
	id_ = 0;
#endif
    }
    inline ext(const heap &h, cell ptr) : heap_(&h), ptr_(ptr)
    {
#ifdef DEBUG_TERM
	id_ = ext_register(h, &ptr_); 
#else
	ext_register(h, &ptr_);
#endif
    }
    inline ~ext() { if (heap_ != nullptr) ext_unregister(*heap_, &ptr_); }

    inline ext(const ext<T> &other)
    {
	if (other.heap_ != nullptr) {
	    heap_ = other.heap_;
	    ptr_ = other.ptr_;
#ifdef DEBUG_TERM
            id_ = ext_register(*heap_, &ptr_);
#else
            ext_register(*heap_, &ptr_);
#endif
	} else {
	    heap_ = nullptr;
	}
    }

    inline void operator = (const ext<T> &other)
    {
	if (other.heap_ != nullptr) {
	    bool was_null = heap_ == nullptr;
	    heap_ = other.heap_;
	    ptr_ = other.ptr_;
	    if (was_null) {
#ifdef DEBUG_TERM
   	        id_ = ext_register(*heap_, &ptr_);
#else
	        ext_register(*heap_, &ptr_);
#endif
            }
	} else {
	    if (heap_ != nullptr) {
		ext_unregister(*heap_, &ptr_);
		heap_ = nullptr;
	    }
	}
    }

    inline operator T ();
    inline operator const T & () const;
    inline T operator * () const;
    inline T deref() const;
    inline const T * operator -> () const;
    inline T * operator -> ();

    inline bool operator == (const ext<T> &other) const {
	return heap_ == other.heap_ && ptr_ == other.ptr_;
    }

    inline bool is_void() const {
	return heap_ == nullptr;
    }

private:
#ifdef DEBUG_TERM
    inline size_t ext_register(const heap &h, cell *p);
#else
    inline void ext_register(const heap &h, cell *p);
#endif
    inline void ext_unregister(const heap &h, cell *p);

    const heap *heap_;
    mutable cell ptr_;
#ifdef DEBUG_TERM
    mutable size_t id_;
#endif
};
#endif

//
// heap
//
// This is just a stack of heap_blocks.
//

class heap {
public:
    heap();
    ~heap();

    void reset();

    inline heap & get_heap() { return *this; }
    inline const heap & get_heap() const { return *this; }

    inline size_t size() const { return size_; }

    inline size_t num_blocks() const { return (size()+heap_block::MAX_SIZE-1)/heap_block::MAX_SIZE; }

    void trim(size_t new_size) {
	trim_fn_(*this, trim_fn_context_, new_size);
    }

protected:
    void internal_trim(size_t new_size);

public:

    inline void set_size(size_t new_size) {
        size_ = new_size;
    }

    inline void coin_security_check(con_cell c) const {
        if (coin_security_enabled_ && c == COIN) {
	    throw coin_security_exception();
        }
    }

    friend class garbage_collector;

    class disabled_coin_security;
    friend class disabled_coin_security;

    class disabled_coin_security {
    public:
        inline disabled_coin_security(heap &h) : heap_(h), old_(h.coin_security_enabled_)
            { heap_.coin_security_enabled_ = false; }
        inline ~disabled_coin_security()
            { heap_.coin_security_enabled_ = old_; }
    private:
        heap &heap_;
        bool old_;
    };

    // Only a couple of builtins will use this.
    inline disabled_coin_security disable_coin_security() {
        return disabled_coin_security(*this);
    }

    inline void check_index(size_t index) const
    {
	if (index >= size()) {
	    throw heap_index_out_of_range_exception(index, size());
	}
    }

    inline cell & operator [] (size_t addr)
    {
	auto &block = find_block(addr);
	return block[addr];
    }

    inline const cell & operator [] (size_t addr) const
    {
	return get(addr);
    }

    inline untagged_cell & untagged_at(size_t addr) {
	return (*this)[addr];
    }

    inline const untagged_cell & untagged_at(size_t addr) const {
	return get(addr);
    }
  
    size_t list_length(const cell lst) const;

    inline con_cell atom(const std::string &name) const
    {
        if (name.length() > 7) {
	    return con_cell(resolve_atom_index(name), 0);
	}
	return con_cell(name, 0);
    }

    inline std::string atom_name(con_cell cell) const
    {
        if (cell.is_direct()) {
	    return cell.name();
        } else {
	    size_t index = cell.atom_index();
	    auto it = atom_index_to_name_table_.find(index);
	    if (it == atom_index_to_name_table_.end()) {
	        load_atom_name_fn_(const_cast<heap &>(*this), load_atom_name_fn_context_, index);
		it = atom_index_to_name_table_.find(index);
	    }
  	    return it->second;
	}
    }

    inline bool is_dollar_atom_name(con_cell cell) const
    {
        if (cell.is_direct()) {
	    return cell.name_length() >= 1 && ((cell.get_name_byte(0) & 0x7f) == '$');
        } else {
	    std::string name = atom_name(cell);
	    return !name.empty() && name[0] == '$';
	}
    }
  
    inline bool is_string(term lst) const
    {
	if (is_empty_list(lst)) {
	    return false;
	}
	bool contain_printables = false;
	while (is_dotted_pair(lst)) {
	    term head = arg(lst, 0);
	    if (head.tag() != tag_t::INT) {
		return false;
	    }
	    auto val = reinterpret_cast<const int_cell &>(head).value();
	    if (val < 0 || val > 255) {
		return false;
	    }
	    if (val >= ' ') {
		contain_printables = true;
	    }
	    lst = arg(lst, 1);
	}
	return contain_printables;
    }

    inline bool is_prefer_string(term lst) const
    {
	if (is_empty_list(lst)) {
	    return false;
	}
        size_t printables = 0;
	while (is_dotted_pair(lst)) {
	    term head = arg(lst, 0);
	    if (head.tag() != tag_t::INT) {
		return false;
	    }
	    auto val = reinterpret_cast<const int_cell &>(head).value();
	    if (val < 0 || val > 255) {
		return false;
	    }
	    if (val >= ' ') {
		printables++;
	    }
	    lst = arg(lst, 1);
	}
	return printables >= 7;
    }

    inline std::string list_to_string(term lst) const
    {
	std::string s;

	while (is_dotted_pair(lst)) {
	    term head = arg(lst, 0);
	    lst = arg(lst, 1);
	    if (head.tag() != tag_t::INT) {
		continue;
	    }
	    auto val = reinterpret_cast<const int_cell &>(head).value();
	    if (val >= 0 && val <= 255) {
		s += static_cast<char>(val);
	    }
	}
	return s;
    }

    bool is_name(con_cell cell, const std::string &name) const;

    inline con_cell functor(const std::string &name, size_t arity)
    {
	auto name_len = name.length();
        if (name_len > 7) {
	    assert(name_len < con_cell::MAX_NAME_LENGTH);
   	    return con_cell(resolve_atom_index(name), arity);
	}
	
        return con_cell(name, arity);
    }

    inline con_cell to_atom(con_cell c)
    {
	if (c.is_direct()) {
	    con_cell c2 = c;
	    c2.set_value(c.value() & ~(0x1f));
	    return c2;
	} else {
	    std::string name = atom_name(c);
	    return functor(name, 0);
	}
    }

    inline con_cell to_functor(con_cell atom, size_t arity)
    {
	if (arity >= 32) {
	    return functor(atom_name(atom), arity);
	} else {
	    if (atom.is_direct()) {
		con_cell f = atom;
		f.set_value(f.value() | arity);
		return f;
	    } else {
		return functor(atom_name(atom), arity);
	    }
	}
    }

    size_t resolve_atom_index(const std::string &name) const;

    inline con_cell functor(const term s) const
    {
	term ds = deref(s);
        if (ds.tag() == tag_t::CON) {
	    return reinterpret_cast<const con_cell &>(ds);
        }
        if (ds.tag() != tag_t::STR) {
	    throw expected_str_cell_exception(ds);
        }
	return functor(reinterpret_cast<const str_cell &>(ds));
    }

    inline con_cell functor(const str_cell &s) const
    {
	size_t index = s.index();
	check_index(index);
	cell c = get(index);
	if (c.tag() != tag_t::CON) {
	    throw expected_con_cell_exception(index, c);
	}
	const con_cell &cc = static_cast<const con_cell &>(c);
	return cc;
    }
    
    bool is_list(const cell c) const;

    inline bool is_empty_list(const cell c) const
    {
	if (c.tag() == tag_t::CON) {
	    return c == EMPTY_LIST;
	} else if (c.tag() == tag_t::STR) {
   	    return functor(c) == EMPTY_LIST;
        } else {
	    return false;
        }
    }

    inline bool is_dotted_pair(const cell c) const
    {
	if (c.tag() == tag_t::CON) {
  	    return c == DOTTED_PAIR;
	} else if (c.tag() == tag_t::STR) {
	    return functor(c) == DOTTED_PAIR;
	} else {
	    return false;
	}
    }

    inline bool is_comma(const cell c) const
    {
	if (c.tag() == tag_t::CON) {
	    return c == COMMA;
	} else if (c.tag() == tag_t::STR) {
	    return functor(c) == COMMA;
	} else {
	    return false;
	}
    }

    cell deref(cell c) const;
    cell deref_with_cost(cell c, uint64_t &cost) const;

    inline term arg(const cell c, size_t index) const
    {
	auto dc = deref(c);
        const str_cell &s = static_cast<const str_cell &>(dc);
	return term(*this, deref(get(s.index() + index + 1)));
    }

    void set_arg(cell str, size_t index, const cell c)
    {
	auto dc = deref(str);
        str_cell &s = static_cast<str_cell &>(dc);
	size_t i = s.index() + index + 1;
	(*this)[i] = c;
    }

    inline term new_str(con_cell con)
    {
        coin_security_check(con);
	size_t arity = con.arity();
	cell *p;
	size_t index;
	std::tie(p, index) = allocate(tag_t::STR, 2+ arity);
	static_cast<ptr_cell &>(*p).set_index(index+1);
	p[1] = con;
	for (size_t i = 0; i < arity; i++) {
	    p[i+2] = ref_cell(index+i+2);
	}
	return term(*this, *p);
    }

    inline str_cell new_con0(con_cell con)
    {
        coin_security_check(con);
        cell *p;
	size_t index;
	size_t arity = con.arity();
	ensure_allocate(1+arity);
        std::tie(p, index) = allocate(tag_t::CON, 1);
	*p = con;
	return str_cell(index);
    }

    inline str_cell new_str0(con_cell con)
    {
        coin_security_check(con);
        cell *p;
        size_t index;
	size_t arity = con.arity();
	ensure_allocate(2+arity);
        std::tie(p, index) = allocate(tag_t::STR, 2);
	static_cast<ptr_cell &>(*p).set_index(index+1);
	p[1] = con;
	return str_cell(index+1);
    }

    inline size_t num_bits(const big_cell &big) const {
	auto &hdr = reinterpret_cast<const big_header &>(get(big.index()));
	return hdr.num_bits();
    }

    inline size_t num_bytes(const big_cell &big) const {
	return (num_bits(big) + 7) / 8;
    }

    inline size_t num_cells(const big_cell &big) const {
	auto &hdr = reinterpret_cast<const big_header &>(get(big.index()));
	return hdr.num_cells();
    }

    inline big_iterator begin(const big_cell &big) {
	auto dc = deref(big);
	big_cell &b = reinterpret_cast<big_cell &>(dc);
	auto &hdr = reinterpret_cast<const big_header &>(untagged_at(b.index()));
	size_t n = (hdr.num_bits() + 7) / 8;
	return big_iterator(*this, b.index(), n);
    }

    inline big_iterator end(const big_cell &big) {
	auto dc = deref(big);
	big_cell &b = reinterpret_cast<big_cell &>(dc);
	auto &hdr = reinterpret_cast<const big_header &>(untagged_at(b.index()));
	size_t n = (hdr.num_bits() + 7) / 8;
	return big_iterator(*this, b.index(), n, true);
    }

    inline const_big_iterator begin(const big_cell &big) const {
	auto dc = deref(big);
	big_cell &b = reinterpret_cast<big_cell &>(dc);
	auto &hdr = reinterpret_cast<const big_header &>(untagged_at(b.index()));
	size_t n = (hdr.num_bits() + 7) / 8;
	return const_big_iterator(*this, b.index(), n);
    }

    inline const_big_iterator end(const big_cell &big) const {
	auto dc = deref(big);
	big_cell &b = reinterpret_cast<big_cell &>(dc);
	auto &hdr = reinterpret_cast<const big_header &>(untagged_at(b.index()));
	size_t n = (hdr.num_bits() + 7) / 8;
	return const_big_iterator(*this, b.index(), n, true);
    }

    inline big_cell new_big(const boost::multiprecision::cpp_int &i, size_t nbits0)
    {
	using namespace boost::multiprecision;
	size_t nbits = nbits0 == 0 ? (i ? msb(i)+1 : 1) : nbits0;
	nbits = ((nbits + 7) / 8) * 8;
	big_cell big = new_big(nbits);
	set_big(big, i);
	return big;
    }

    // Big nums are special and can span over multiple heap blocks
    inline big_cell new_big(size_t num_bits)
    {
	auto num_bytes = (num_bits + 7) / 8;
	auto num_cells = (num_bytes <= 4) ? 1 : 1+(((num_bytes-4) + sizeof(cell) - 1) / sizeof(cell));

	big_header header(num_bits);

	cell *p;
	size_t index;
	size_t n = num_cells;
	// We pass true as second argument to indicate that a bignum can
	// span across multiple heap blocks (even for small bignums as
	// a small bignum can go across the heap block boundary.)
	std::tie(p, index) = allocate(tag_t::DAT, n, true);
	*p = header;
	return big_cell(index);
    }

    bool big_equal(big_cell big1, big_cell big2, uint64_t &cost) const;
    int big_compare(big_cell big1, big_cell big2, uint64_t &cost) const;

    void get_big(cell big, uint8_t *bytes, size_t n) const;
    void set_big(cell big, const uint8_t *bytes, size_t n);

    big_header get_big_header(cell big) const {
      auto dc = deref(big);
      big_cell &b = reinterpret_cast<big_cell &>(dc);
      auto &hdr = reinterpret_cast<const big_header &>(untagged_at(b.index()));
      return hdr;
    }

    static std::string big_to_string(const boost::multiprecision::cpp_int &i, size_t base, size_t nbits, size_t limit = std::numeric_limits<size_t>::max());
  
    std::string big_to_string(cell big, size_t base, bool capital = false, size_t limit = std::numeric_limits<size_t>::max()) const;
    std::string big16_to_string(cell big, size_t limit) const;    
    inline size_t num_cells(const boost::multiprecision::cpp_int &i)
    {
	using namespace boost::multiprecision;
	return (msb(i) + untagged_cell::CELL_NUM_BITS) /
	    untagged_cell::CELL_NUM_BITS;
    }

    void set_big(cell big, const boost::multiprecision::cpp_int &i);

    inline void get_big(cell big, boost::multiprecision::cpp_int &i,
			size_t &nbits) const
    {
	using namespace boost::multiprecision;
	
	cell dc = deref(big);
	auto &b = reinterpret_cast<const big_cell &>(dc);
	nbits = num_bits(b);
	import_bits(i, begin(b), end(b), 8);
    }
    
    inline size_t new_cell0(cell c, bool has_tag = true)
    {
        cell *p;
        size_t index;
	if(has_tag) {
	  if(c.tag() == tag_t::CON) {
	    auto &con = reinterpret_cast<con_cell &>(c);
	    ensure_allocate(1+con.arity());
	  }
	}
        std::tie(p, index) = allocate(tag_t::STR, 1);
	*p = c;
	return index;
    }

    inline void new_dat_cell(cell c)
    {
        cell *p;
        size_t index;
	std::tie(p, index) = allocate(tag_t::DAT, 1, true);
	*p = c;
    }
    
    inline term new_dotted_pair()
    {
        term t = new_str0(DOTTED_PAIR);
	return t;
    }

    inline term new_dotted_pair(const term a, const term b)
    {
	term t = new_str(DOTTED_PAIR);
	set_arg(t, 0, a);
	set_arg(t, 1, b);
	return t;
    }

    inline term new_ref()
    {
	size_t index;
	cell *p;
	std::tie(p, index) = allocate(tag_t::REF, 1);
	static_cast<ref_cell &>(*p).set_index(index);
	return term(*this, *p);
    }

    inline void new_ref(size_t cnt)
    {
	for (size_t i = 0; i < cnt; i++) {
	    new_ref();
	}
    }

    inline size_t external_ptr_count() const
    {
	return external_ptrs_.size();
    }

    void print_status(std::ostream &out) const;

    void print(std::ostream &out) const;
    void print(std::ostream &out, size_t from, size_t to) const;

    inline void add_watched(size_t addr) {
        if (std::find(watched_.begin(), watched_.end(), addr) == watched_.end()) {
           watched_.push_back(addr);
	}
    }

    inline bool has_watched() const {
        return !watched_.empty();
    }

    inline void clear_watched() {
        watched_.clear();
    }

    inline void watch(size_t addr, bool b) {
        find_block(addr).watch(addr, b);
    }

    inline const std::vector<size_t> & watched() const {
        return watched_;
    }

    inline bool watched(size_t addr) const {
        return find_block(addr).watched(addr);
    }  
  
    bool check_term(const term t, std::string *name = nullptr) const;
private:
    friend class term_emitter;

protected:
    inline size_t find_block_index(size_t addr) const
    {
	return addr / heap_block::MAX_SIZE;
    }

    inline size_t last_block_index() const
    {
	size_t hsz = size();
	return hsz == 0 ? 0 : find_block_index(hsz-1);
    }

public:
    static const size_t NEW_BLOCK = static_cast<size_t>(-1);

    static inline heap_block & get_block_default(heap &h, void * /*context*/, size_t block_index)
    {
        if (block_index == NEW_BLOCK) {
  	    new_block_default(h);
	    block_index = h.blocks_.size();
	}
        return *h.blocks_[block_index];
    }

    static inline void modified_block_default(heap_block & /*block*/, void * /* context */) {
        // Do nothing
    }

    static inline size_t new_atom_default(const heap &h, void * /* context */, const std::string &name) {
	// This is a simple way of returning new identifiers for atoms
	// but it is not deterministic. For the global Prolog interpreter
	// we need to query the database.
	return h.atom_name_to_index_table_.size();
    }

    static inline void load_atom_name_default(heap &h, void * /* context */, size_t index)
    {
    }

    static inline void load_atom_index_default(heap &, void * /* context */, const std::string &name)
    {
    }

    static inline void trim_default(heap &h, void * /* context */, size_t new_size) {
	h.internal_trim(new_size);
    }

    inline void set_atom_index(const std::string &name, size_t index)
    {
	atom_name_to_index_table_[name] = index;
	atom_index_to_name_table_[index] = name;
    }

    inline void clear_atom_index(const std::string &name, size_t index)
    {
	atom_name_to_index_table_.erase(name);
	atom_index_to_name_table_.erase(index);
    }

    inline heap_block * get_head_block() {
        return head_block_;
    }
  
    inline void set_head_block(heap_block *h) {
        head_block_ = h;
        size_ = h->index() * heap_block::MAX_SIZE + h->size();
    }

private:

    static inline void new_block_default(heap &h)
    {
        heap_block *block = new heap_block(h, h.blocks_.size());
	h.set_head_block(block);
	h.blocks_.push_back(block);
    }

    inline heap_block * find_block_from_index(size_t index) const
    {
        return blocks_[index];
    }

    inline heap_block & find_block(size_t addr)
    {
        return get_block_fn_(*this, get_block_fn_context_, find_block_index(addr));
    }

    inline const heap_block & find_block(size_t addr) const
    {
        return get_block_fn_(const_cast<heap &>(*this), get_block_fn_context_, find_block_index(addr));
    }

    inline const bool in_range(size_t addr) const
    {
	return addr < size();
    }

    inline heap_block * ensure_allocate(size_t n, bool span = false) {
	if (!span) {
	    if (head_block_ == nullptr || !head_block_->can_allocate(n)) {
		get_block_fn_(*this, get_block_fn_context_, NEW_BLOCK);
	    }
	    return head_block_;
	} else {
	    heap_block *block = nullptr;
	    while (n > 0) {
		if (head_block_ == nullptr || head_block_->is_full()) {
		    get_block_fn_(*this, get_block_fn_context_, NEW_BLOCK);
		}
		if (!block) block = head_block_;
		size_t next_alloc = std::min(n, heap_block::MAX_SIZE - head_block_->size());
		head_block_->allocate0(next_alloc);
		n -= next_alloc;
	    }
	    return block;
	}
    }

    inline std::pair<cell *, size_t> allocate(tag_t::kind_t tag, size_t n, bool span = false) {
	size_t addr = size_;
	heap_block *block = ensure_allocate(n, span);
	if (!span) {
	    block = head_block_;
	    addr = block->allocate(n);
	}
	size_ = head_block_->offset() + head_block_->size();
	ptr_cell new_cell(tag, addr);
	cell *p = &(*block)[addr];
	*p = new_cell;
	return std::make_pair(p, addr);
    }

    inline const cell & get(size_t addr) const
    {
	check_index(addr);
	return find_block(addr)[addr];
    }

    inline cell arg0(const cell &c, size_t index) const
    {
        const str_cell &s = static_cast<const str_cell &>(c);
	return get(s.index() + index + 1);
    }

#ifdef DEBUG_TERM
    inline size_t register_ext(cell *p) const
#else
    inline void register_ext(cell *p) const
#endif
    {
	if (external_ptrs_.size() > external_ptrs_max_) {
	    external_ptrs_max_ = external_ptrs_.size();
	}
#ifdef DEBUG_TERM
	size_t id = id_counter_++;
        external_ptrs_.insert(std::make_pair(p, id));
	return id;
#else
        external_ptrs_.insert(p);
#endif
    }

    inline void unregister_ext(cell *p) const
    {
#ifdef DEBUG_TERM
	assert(external_ptrs_.erase(p) == 1);
#else
        external_ptrs_.erase(p);
#endif
    }

    bool check_functor(const cell c) const;

    size_t size_;
    std::vector<heap_block *> blocks_;
    heap_block * head_block_;
    std::vector<size_t> watched_;

    bool coin_security_enabled_;

#ifdef DEBUG_TERM
    mutable std::unordered_map<cell *, size_t> external_ptrs_;
    static size_t id_counter_;
#else
    mutable std::unordered_set<cell *> external_ptrs_;
#endif
    mutable size_t external_ptrs_max_;

    mutable std::unordered_map<size_t, std::string> atom_index_to_name_table_;
    mutable std::unordered_map<std::string, size_t> atom_name_to_index_table_;

public:
    static const con_cell EMPTY_LIST;
    static const con_cell DOTTED_PAIR;
    static const con_cell COMMA;
    static const con_cell COIN;

    template<typename T> friend class ext;

    typedef heap_block & (*get_block_fn)(heap &h, void *context, size_t block_index);
    typedef void (*modified_block_fn)(heap_block &block, void *context);
    typedef size_t (*new_atom_fn)(const heap &h, void *context, const std::string &atom_name);
    typedef void (*load_atom_name_fn)(heap &h, void *context, size_t atom_index);
    typedef void (*load_atom_index_fn)(heap &h, void *context, const std::string &atom_name);
    typedef void (*trim_fn)(heap &h, void *context, size_t new_size);
  
    inline void setup_get_block_fn(get_block_fn fn, void *context) { get_block_fn_ = fn; get_block_fn_context_ = context; }
    inline void setup_modified_block_fn(modified_block_fn fn, void *context) { modified_block_fn_ = fn; modified_block_fn_context_ = context; }
    inline void setup_new_atom_fn(new_atom_fn fn, void *context) { new_atom_fn_ = fn; new_atom_fn_context_ = context; }
    inline void setup_load_atom_name_fn(load_atom_name_fn fn, void *context) { load_atom_name_fn_ = fn; load_atom_name_fn_context_ = context; }    
    inline void setup_load_atom_index_fn(load_atom_index_fn fn, void *context) { load_atom_index_fn_ = fn; load_atom_index_fn_context_ = context; }
    inline void setup_trim_fn(trim_fn fn, void *context) { trim_fn_ = fn; trim_fn_context_ = context; }

private:
    get_block_fn get_block_fn_;
    void *get_block_fn_context_;

    friend class heap_block;
    modified_block_fn modified_block_fn_;
    void *modified_block_fn_context_;

    new_atom_fn new_atom_fn_;
    void *new_atom_fn_context_;

    load_atom_name_fn load_atom_name_fn_;
    void *load_atom_name_fn_context_;

    load_atom_index_fn load_atom_index_fn_;
    void *load_atom_index_fn_context_;    

    trim_fn trim_fn_;
    void *trim_fn_context_;
};

inline void heap_block::modified() {
    heap_.modified_block_fn_(*this, heap_.modified_block_fn_context_);
}

inline bool heap_block::is_head_block() const {
    return this == heap_.head_block_;
}

template<typename T> inline big_iterator_base<T>::big_iterator_base(heap &h, size_t index, size_t num) 
 : heap_(h),
   index_(index),
   i_(4),
   end_(num+4),
   cell_byte_(&h.untagged_at(index), 4, num+4) { }

template<typename T> inline big_iterator_base<T>::big_iterator_base(heap &h, size_t index, size_t num, bool b)
 : heap_(h),
   index_(index),
   i_(num+4),
   end_(num+4),
   cell_byte_(&h.untagged_at(index), 4, num+4) { }

template<typename T> inline big_iterator_base<T>::big_iterator_base(const big_iterator_base<T> &it)
 : heap_(it.heap_),
   index_(it.index_),
   i_(it.i_),
   end_(it.end_),
   cell_byte_(it.cell_byte_) { }

template<typename T> inline big_iterator_base<T> & big_iterator_base<T>::operator ++()
{
    i_++;
    if (i_ % 8 == 0) {
	index_++;
	if ((index_ % heap_block::MAX_SIZE) == 0) {
	    // Are we stepping over a heap block boundary?
	    // Then we need to switch base address for cell_byte
	    cell_byte_.set_address(&heap_.untagged_at(index_), 0);
	} else {
	    ++cell_byte_;	    
	}
    } else {
	++cell_byte_;
    }
    return *this;
}

inline cell_byte & big_iterator::operator * ()
{
    return cell_byte_;
}

inline const cell_byte & const_big_iterator::operator * () const
{
    return cell_byte_;
}

inline big_inserter::big_inserter(heap &h, big_cell big) :
    it_(h.begin(big))
{
}

//
//  register and unregister for ref.
//

#if 0

#ifdef DEBUG_TERM
template<typename T> size_t ext<T>::ext_register(const heap &h, cell *p)
{
    return h.register_ext(p);
}
#else
template<typename T> void ext<T>::ext_register(const heap &h, cell *p)
{
    h.register_ext(p);
}
#endif

template<typename T> void ext<T>::ext_unregister(const heap &h, cell *p)
{
    h.unregister_ext(p);
}

template<typename T> T ext<T>::deref() const
{
    cell c = heap_->deref(ptr_);
    ptr_ = c;
    return ext<T>(*heap_, c);
}

template<typename T> T ext<T>::operator * () const
{
    return deref();
}

template<typename T> const T * ext<T>::operator -> () const
{
    deref();
    return &ptr_;
}

template<typename T> T * ext<T>::operator -> ()
{
    deref();
    return &ptr_;
}

template<typename T> ext<T>::operator T ()
{
    return ptr_;
}

template<typename T> ext<T>::operator const T & () const
{
    return ptr_;
}

#endif

} }

namespace boost {
    template<> struct make_unsigned<prologcoin::common::untagged_cell> {
	typedef prologcoin::common::untagged_cell::value_t type;
    };
}

namespace std {
    template<> struct hash<prologcoin::common::cell> {
        size_t operator()(const prologcoin::common::cell k) const {
	    return hash<uint64_t>()(k.value());
	}
    };

    template<> struct hash<prologcoin::common::ref_cell> {
        size_t operator()(const prologcoin::common::ref_cell k) const {
	    return hash<uint64_t>()(k.index());
	}
    };

    template<> struct hash<prologcoin::common::int_cell> {
        size_t operator()(const prologcoin::common::int_cell k) const {
	    return hash<uint64_t>()(k.value());
	}
    };

    template<> struct hash<prologcoin::common::con_cell> {
	size_t operator()(const prologcoin::common::con_cell k) const {
	    return hash<uint64_t>()(k.value());
	}
    };

    template<> struct hash<prologcoin::common::term> {
        size_t operator()(const prologcoin::common::term& k) const {
	    auto &c = reinterpret_cast<const prologcoin::common::cell &>(k);
	    return hash<uint64_t>()(c.value());
	}
    };

}

namespace prologcoin { namespace common {

// For term naming
class naming_map {
public:
    void clear_names() {
	ref_2_name_.clear();
    }
    const std::string & get_name(ref_cell r) const {
	static std::string empty;
	auto it = ref_2_name_.find(r);
	if (it == ref_2_name_.end()) {
	    return empty;
	}
	return it->second;
    }
    void set_name(ref_cell r, const std::string &name) {
	ref_2_name_[r] = name;
    }
    void clear_name(ref_cell r) {
	ref_2_name_.erase(r);
    }
    bool has_name(ref_cell r) const {
	return ref_2_name_.count(r);
    }
    size_t size() const {
	return ref_2_name_.size();
    }
    void trim_names(size_t heap_limit) {
	ref_cell r(heap_limit);
	auto it_end = ref_2_name_.end();
	for (auto it = ref_2_name_.lower_bound(r); it != it_end; ++it) {
	    ref_2_name_.erase(it);
	}
    }
    
private:
    std::map<ref_cell, std::string> ref_2_name_;
};

} }

#endif

