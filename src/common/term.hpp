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

    inline untagged_cell(const untagged_cell &other) : raw_value_(other.raw_value_) { }
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
    { auto bit_off = untagged_cell::CELL_NUM_BITS-(8*(index+1));
      auto bit_mask = ~(static_cast<untagged_cell::value_t>(0xff) << bit_off);
      auto bit_val = static_cast<untagged_cell::value_t>(v) << bit_off;
      cached_ = (cached_.raw_value() & bit_mask) | bit_val;
    }

    inline value_t get_byte(size_t index) const
    { auto bit_off = untagged_cell::CELL_NUM_BITS-(8*(index+1));
      return static_cast<value_t>(cached_.raw_value() >> bit_off);
    }

    static inline untagged_cell * flush_and_invalidate(const cell_byte &cb0) {
	cell_byte &cb = const_cast<cell_byte &>(cb0);
	if (cb.cell_) *cb.cell_ = cb.cached_;
	auto *r = cb.cell_;
	cb.cell_ = nullptr;
	return r;
    }

public:
    inline cell_byte(untagged_cell &c, size_t index) :
	cell_(&c), index_(index), cached_(c) { }

    inline cell_byte(const cell_byte &other)
	: cell_(flush_and_invalidate(other)),
	  index_(other.index_),
	  cached_(other.cached_) { }

    inline cell_byte(cell_byte &&other) :
	cell_(flush_and_invalidate(other)),
	index_(other.index_),
	cached_(other.cached_) { }

    inline ~cell_byte() { if (cell_) *cell_ = cached_; }

    inline void operator = (const cell_byte &other) {
	*cell_ = cached_;
	cell_ = other.cell_;
	index_ = other.index_;
	cached_ = other.cached_;
    }
	
    inline void operator ++ () {
	index_++;
    }

    inline void set_address(untagged_cell &c, size_t index) {
	if (cell_) *cell_ = cached_;
	cell_ = &c;
	index_ = index;
	cached_ = *cell_;
    }

    inline void operator >>= (size_t n)
    { set_byte(index_, get_byte(index_) >> n); }

    inline void operator = (value_t v)
    { set_byte(index_, v); }

    inline operator value_t () const
    { return get_byte(index_); }

    untagged_cell *cell_;
    size_t index_;
    untagged_cell cached_;
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

    inline cell() : untagged_cell(0) { }

    inline cell(value_t raw_value) : untagged_cell(raw_value) { }

    inline cell(tag_t t) : untagged_cell(static_cast<value_t>(t) & 0x7) { }

    inline cell(tag_t t, value_t v)
                        : untagged_cell((static_cast<value_t>(t) & 0x7) |
				     (v << 3)) { }

    inline cell(const cell &other) : untagged_cell(other) { }

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

typedef cell term;

//
// Exceptions
//

class term_exception : public std::runtime_error {
public:
    term_exception(const std::string &msg) : std::runtime_error(msg) { }
};

class heap_index_out_of_range_exception : public term_exception {
public:
    heap_index_out_of_range_exception(size_t index, size_t max_sz)
	: term_exception( std::string("Heap index ") + boost::lexical_cast<std::string>(index) + " exceeded " + boost::lexical_cast<std::string>(max_sz-1)) { }
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

public:
    static const size_t CELL_NUM_BYTES_HALF = CELL_NUM_BITS / 2 / 8;

    // Lower 4 bytes (half cell) = number of bits
    // Upper 4 bytes (half cell) = binary data
    inline dat_cell(size_t num_bits)
        : int_cell(tag_t::DAT, static_cast<int64_t>(num_bits)) { }

    inline size_t num_bits() const
    { auto mask = (static_cast<untagged_cell::value_t>(1) << (CELL_NUM_BITS_HALF - TAG_SIZE_BITS)) - 1;
      return value() & mask;
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
 
//
// heap_block
//
// We don't want to keep _all_ heap blocks in memory. We can
// cache those that are frequent.
//
class heap_block : private boost::noncopyable {
public:
    static const size_t MAX_SIZE = 1024*128;

    inline heap_block() : index_(0), offset_(0),
			  size_(0), cells_(nullptr) { init_cells(); }
    inline heap_block(size_t index, size_t offset)
        : index_(index), offset_(offset),
	  size_(0), cells_(nullptr) { init_cells(); }
    inline ~heap_block() { free_cells(); }

    inline void init_cells() {
	cells_ = reinterpret_cast<cell *>(new uint64_t[MAX_SIZE]);
    }

    inline void free_cells() {
	delete reinterpret_cast<uint64_t *>(cells_);
    }

    inline size_t index() const { return index_; }
    inline size_t offset() const { return offset_; }

    inline cell & operator [] (size_t addr) {
	return cells_[addr - offset_];
    }

    inline const cell & operator [] (size_t addr) const {
	return cells_[addr - offset_];
    }

    inline bool can_allocate(size_t n) const {
	return size_ + n < MAX_SIZE;
    }

    inline size_t allocate(size_t n) {
	size_t addr = offset_ + size_;
	size_ += n;
	return addr;
    }

    inline void trim(size_t n) {
	size_ = n;
    }

    inline void fill() {
	size_ = MAX_SIZE;
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

private:
    size_t index_;
    size_t offset_;
    size_t size_;
    cell *cells_;
};

class heap; // Forward

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

    inline heap & get_heap() { return *this; }
    inline const heap & get_heap() const { return *this; }

    inline size_t size() const { return size_; }

    void trim(size_t new_size);

    inline void coin_security_check(con_cell c) const {
        if (coin_security_enabled_ && c == COIN) {
	    throw coin_security_exception();
        }
    }

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
  	    return atom_index_to_name_table_[cell.atom_index()];
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
	    auto val = static_cast<const int_cell &>(head).value();
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
	    auto val = static_cast<const int_cell &>(head).value();
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
	    auto val = static_cast<const int_cell &>(head).value();
	    if (val >= 0 && val <= 255) {
		s += static_cast<char>(val);
	    }
	}
	return s;
    }

    bool is_name(con_cell cell, const std::string &name) const;

    inline con_cell functor(const std::string &name, size_t arity)
    {
        if (name.length() > 7) {
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
	    return static_cast<const con_cell &>(ds);
        }
        if (ds.tag() != tag_t::STR) {
	    throw expected_str_cell_exception(ds);
        }
	return functor(static_cast<const str_cell &>(ds));
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

    inline big_cell new_big(size_t num_bits)
    {
	auto num_bytes = (num_bits + 7) / 8;
	auto num_cells = (num_bytes <= 4) ? 1 : 1+(((num_bytes-4) + sizeof(cell) - 1) / sizeof(cell));

	big_header header(num_bits);

	cell *p;
	size_t index;
	size_t n = num_cells;
	ensure_allocate(n+1);
	std::tie(p, index) = allocate(tag_t::BIG, n+1);
	static_cast<big_cell &>(*p).set_index(index+1);
	p[1] = header;
	// 0 the data to ensure a consistent state (across all platforms)
	memset(&p[2], 0, sizeof(cell)*(n-1));
	return big_cell(index+1);
    }

    bool big_equal(big_cell big1, big_cell big2, uint64_t &cost) const;

    void get_big(cell big, uint8_t *bytes, size_t n) const;
    void set_big(cell big, const uint8_t *bytes, size_t n);

    static std::string big_to_string(const boost::multiprecision::cpp_int &i, size_t base, size_t nbits);
  
    std::string big_to_string(cell big, size_t base, bool capital = false) const;

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
    
    inline void new_cell0(cell c)
    {
        cell *p;
        size_t index;
        std::tie(p, index) = allocate(tag_t::STR, 1);
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
	size_t index;
	cell *p;
	std::tie(p, index) = allocate(tag_t::REF, cnt);
	for (size_t i = 0; i < cnt; i++) {
	    static_cast<ref_cell &>(p[i]).set_index(index+i);
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
  
private:
    friend class term_emitter;

    inline size_t new_block()
    {
	heap_block *last_block = blocks_.back();
	size_t last_offset = last_block->offset();
	size_t new_offset = last_offset + heap_block::MAX_SIZE;
	new_block(new_offset);
	last_block->fill();
	size_ = new_offset;
	return new_offset;
    }

    inline void new_block(size_t offset)
    {
	heap_block *block = new heap_block(blocks_.size(), offset);
	head_block_ = block;
	blocks_.push_back(block);
    }

    inline size_t find_block_index(size_t addr) const
    {
	return addr / heap_block::MAX_SIZE;
    }

    inline heap_block & find_block(size_t addr)
    {
	return *blocks_[find_block_index(addr)];
    }

    inline const heap_block & find_block(size_t addr) const
    {
	return *blocks_[find_block_index(addr)];
    }

    inline const bool in_range(size_t addr) const
    {
	return addr < size();
    }

    inline void ensure_allocate(size_t n) {
	if (!head_block_->can_allocate(n)) {
	    new_block();
	}
    }

    inline std::pair<cell *, size_t> allocate(tag_t::kind_t tag, size_t n) {
	ensure_allocate(n);
	heap_block *block = head_block_;
	size_t addr = block->allocate(n);
	ptr_cell new_cell(tag, addr);
	cell *p = &(*block)[addr];
	*p = new_cell;
	size_ = addr + n;
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

    mutable std::vector<std::string> atom_index_to_name_table_;
    mutable std::unordered_map<std::string, size_t> atom_name_to_index_table_;

public:
    static const con_cell EMPTY_LIST;
    static const con_cell DOTTED_PAIR;
    static const con_cell COMMA;
    static const con_cell COIN;

    template<typename T> friend class ext;
};

template<typename T> inline big_iterator_base<T>::big_iterator_base(heap &h, size_t index, size_t num) 
 : heap_(h),
   index_(index),
   i_(0),
   end_(num),
   cell_byte_(h.untagged_at(index), 0) { }

template<typename T> inline big_iterator_base<T>::big_iterator_base(heap &h, size_t index, size_t num, bool b)
 : heap_(h),
   index_(index),
   i_(num),
   end_(num),
   cell_byte_(h.untagged_at(index), 0) { }

template<typename T> inline big_iterator_base<T>::big_iterator_base(const big_iterator_base<T> &it)
 : heap_(it.heap_),
   index_(it.index_),
   i_(it.i_),
   end_(it.end_),
   cell_byte_(it.cell_byte_) { }

template<typename T> inline big_iterator_base<T> & big_iterator_base<T>::operator ++()
{
    i_++;
    ++cell_byte_;
    if (i_ < static_cast<int>(dat_cell::CELL_NUM_BYTES_HALF)) {
	return *this;
    }
    size_t ii = static_cast<size_t>(i_ - static_cast<int>(dat_cell::CELL_NUM_BYTES_HALF));
    if ((ii % sizeof(cell)) != 0) {
	return *this;
    }
    size_t iic = ii / sizeof(cell) + 1;
    cell_byte_.set_address(heap_.untagged_at(index_+iic), 0);
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

#if 0
    template<> struct hash<prologcoin::common::term> {
        size_t operator()(const prologcoin::common::term& k) const {
	    auto &c = static_cast<const prologcoin::common::cell &>(*k);
	    return hash<uint64_t>()(c.value());
	}
    };
#endif

}

#endif


