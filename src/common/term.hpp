#pragma once

#ifndef _common_Term_hpp
#define _common_Term_hpp

#include <stdint.h>
#include <algorithm>
#include <vector>
#include <memory>
#include <unordered_set>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>

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
// In Prologcoin we have a (supposed) common heap among all nodes
// in the system. However, when we send a term from one node to
// another, some parts may refer to the common heap whereas other
// parts don't yet have definitive addresses. Once the term is
// committed to the (common) heap, these addresses will be definitive.
// Therefore, we need tentitive (relative) addresses for some cells.
// We'll use the upper bit (bit 63) to represent a relative reference.
//
// You can also view this as if the upper bit 63 tells whether the
// reference is refering to a secondary (tentative) heap or not.
// 
//

namespace prologcoin { namespace common {

class term_emitter;

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

class ref_cell; // Forward

template<tag_t::kind_t T> struct enum_to_cell_type {};

template<> struct enum_to_cell_type<tag_t::REF> {
    typedef ref_cell cell_type;
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

    inline bool operator == (const cell other) const { return value() == other.value(); }
    inline bool operator != (const cell other) const { return value() != other.value(); }

    std::string str() const;

private:
    value_t raw_value_;
};

//
// Exceptions
//

class term_exception : public ::std::runtime_error {
public:
    term_exception(const std::string &msg) : ::std::runtime_error(msg) { }
};

class heap_index_out_of_range_exception : public term_exception {
public:
    heap_index_out_of_range_exception(size_t index, size_t max)
	: term_exception( std::string("Heap index ") + boost::lexical_cast<std::string>(index) + " exceeded " + boost::lexical_cast<std::string>(max)) { }
};

class expected_con_cell_exception : public term_exception {
public:
    expected_con_cell_exception(size_t index, cell c)
	: term_exception( std::string("Expected CON cell at index ") + boost::lexical_cast<std::string>(index) + "; was " + c.tag().str()) { }
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

    inline std::string str() const {
	std::string s = boost::lexical_cast<std::string>(index());
	return "|" + std::string(std::max(0,20 - static_cast<int>(s.length())), ' ') + s + " : " + static_cast<std::string>(tag()) + " |";
    }

    inline operator std::string () const {
	return str();
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

    inline bool operator == (const int other) const {
	return value() == other;
    }

    inline std::string str () const {
	std::string s = boost::lexical_cast<std::string>(value());
	return "|" + std::string(std::max(0,20 - static_cast<int>(s.length())), ' ') + s + " : " + static_cast<std::string>(tag()) + " |";
    }

   inline operator std::string () const {
       return str();
   }
};

//
// heap_block
//
// We don't want to keep _all_ heap blocks in memory. We can
// cache those that are frequent.
//
class heap_block : private boost::noncopyable {
public:
    inline heap_block() : index_(0), offset_(0) { }
    inline heap_block(size_t index, size_t offset)
        : index_(index), offset_(offset) { }

    inline size_t index() const { return index_; }
    inline size_t offset() const { return offset_; }

    inline cell & operator [] (size_t offset) {
	return cells_[offset - offset_];
    }

    inline const cell & operator [] (size_t offset) const {
	return cells_[offset - offset_];
    }

    inline void allocate(size_t n) {
	size_t top = cells_.size();
	cells_.resize(top + n);
    }

private:
    size_t index_;
    size_t offset_;
    std::vector<cell> cells_;
};

class cell_ptr {
public:
    inline cell_ptr(heap_block &block, ptr_cell pcell) :
	block_(block), pcell_(pcell) { }

    inline size_t index() const {
	return pcell_.index();
    }

    inline size_t index(int delta) const {
	return pcell_.index() + delta;
    }

    inline cell arg(size_t i) const {
	return block_[pcell_.index() - block_.offset() + i];
    }

    inline cell & arg(size_t i) {
	return block_[pcell_.index() - block_.offset() + i];
    }

    inline cell & operator [] (size_t index) { return arg(index); }
    inline cell & operator * () { return arg(0); }

private:
    heap_block &block_;
    ptr_cell pcell_;
};

class heap; // Forward

//
// Externally typed cell references.
//
// We keep track of which heap the cell comes from. Also the heap
// gets notified so that whenever a heap GC happens the address can be
// updated.
// 

template<typename T> class ext {
public:
    inline ext(const heap &h, cell ptr) : heap_(h), ptr_(ptr)
        { ext_register(h, &ptr_); }
    inline ~ext() { ext_unregister(heap_, &ptr_); }

    inline operator T & () { return static_cast<T &>(ptr_); }

private:
    inline void ext_register(const heap &h, cell *p);
    inline void ext_unregister(const heap &h, cell *p);

    const heap &heap_;
    cell ptr_;
};

//
// heap
//
// This is just a stack of heap_blocks.
//

class heap {
public:
    inline heap() : size_(0) { new_block(0); }

    inline size_t size() const { return size_; }

    inline void check_index(size_t index) const
    {
	if (index >= size()) {
	    throw heap_index_out_of_range_exception(index, size());
	}
    }

    inline con_cell functor(const str_cell &s) const
    {
	size_t index = s.index();
	cell c = get(index);
	if (c.tag() != tag_t::CON) {
	    throw expected_con_cell_exception(index, c);
	}
	const con_cell &cc = static_cast<const con_cell &>(c);
	return cc;
    }

    inline ext<cell> arg(const str_cell &s, size_t index) const
    {
	return ext<cell>(*this, get(s.index() + index + 1));
    }

    void set_arg(const str_cell &s, size_t index, cell c)
    {
	size_t i = s.index() + index + 1;
	(*this)[i] = c;
    }

    inline ext<str_cell> new_str(con_cell con)
    {
	size_t index = size_;
	size_t arity = con.arity();
	cell *p = allocate(tag_t::STR, arity + 2);
	static_cast<ptr_cell &>(*p).set_index(index+1);
	p[1] = con;
	for (size_t i = 0; i < arity; i++) {
	    p[i+2] = ref_cell(index+i+2);
	}
	return ext<str_cell>(*this, *p);
    }

    inline ext<ref_cell> new_ref()
    {
	size_t index = size_;
	cell *p = allocate(tag_t::REF, 1);
	static_cast<ref_cell &>(*p).set_index(index);
	return ext<ref_cell>(*this, *p);
    }

    void print(std::ostream &out) const;

private:
    friend class term_emitter;

    // Memory at base of size isz moved to a new address, new_base.
    void update_external_ptrs(void *base, size_t siz, void *new_base);

    struct _block_compare
    {
	bool operator () (const std::unique_ptr<heap_block> &left,
			  size_t right) {
	    return left->offset() < right;
	}
    };

    inline void new_block(size_t offset)
    {
	std::unique_ptr<heap_block> p(new heap_block(blocks_.size(), offset));
	blocks_.push_back(std::move(p));
    }

    inline size_t find_block_index(size_t index) const
    {
	auto found =
	    std::lower_bound(blocks_.begin(),
			     blocks_.end(),
			     index,
			     _block_compare());
	if (found != blocks_.begin()) {
	    --found;
	}
	auto block_index = (*found)->index();
	return block_index;
    }

    inline heap_block & find_block(size_t index)
    {
	return *blocks_[find_block_index(index)];
    }

    inline const heap_block & find_block(size_t index) const
    {
	return *blocks_[find_block_index(index)];
    }

    inline const bool in_range(size_t index) const
    {
	return index < size();
    }

    inline cell * allocate(tag_t::kind_t tag, size_t n) {
	std::unique_ptr<heap_block> &block = blocks_.back();
	block->allocate(n);
	ptr_cell new_cell(tag, size_);
	cell *p = &(*block)[size_];
	*p = new_cell;
	size_ += n;
	return p;
    }

    inline cell & operator [] (size_t index)
    {
	return find_block(index)[index];
    }

    inline const cell & operator [] (size_t index) const
    {
	return get(index);
    }

    inline const cell & get(size_t index) const
    {
	check_index(index);
	return find_block(index)[index];
    }

    inline void register_ext(cell *p) const
    {
        external_ptrs_.insert(p);
    }

    inline void unregister_ext(cell *p) const
    {
        external_ptrs_.erase(p);
    }

    typedef std::vector<std::unique_ptr<heap_block> >::iterator block_iterator;

    size_t size_;
    std::vector<std::unique_ptr<heap_block> > blocks_;
    mutable std::unordered_set<cell *> external_ptrs_;

    template<typename T> friend class ext;
};

//
//  register and unregister for ref.
//

template<typename T> void ext<T>::ext_register(const heap &h, cell *p)
{
    h.register_ext(p);
}

template<typename T> void ext<T>::ext_unregister(const heap &h, cell *p)
{
    h.unregister_ext(p);
}

} }

#endif


