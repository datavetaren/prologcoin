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
#include <boost/noncopyable.hpp>

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
// In Prologcoin we have a (supposed) common heap among all nodes
// in the system. However, when we send a term from one node to
// another, some parts may refer to the common heap whereas other
// parts don't yet have definitive addresses. Once the term is
// committed to the (common) heap, these addresses will be definitive.
// Therefore, we need tentitive (relative) addresses for some cells.
//
// We use a separate tag GBL as a "REF cell" referring to the global heap.
// This makes unification (among other things) slightly more complicated
// but I like the clear distinction between local heaps and global heaps.
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
    enum kind_t { REF = 0, INT = 1, BIG = 2, CON = 3, STR = 4 };

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

    inline cell(const heap &, const cell other) : raw_value_(other.raw_value_) { }

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
    heap_index_out_of_range_exception(size_t index, size_t max)
	: term_exception( std::string("Heap index ") + boost::lexical_cast<std::string>(index) + " exceeded " + boost::lexical_cast<std::string>(max)) { }
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

    std::string str () const;

    inline operator std::string () const {
	return str();
    }

private:
    bool is_direct() const {
	return ((value() >> 60) & 0x1) != 0;
    }

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

public:
    inline int_cell(size_t val) : cell(tag_t::INT, val) { }
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

void blaha();

//
// heap_block
//
// We don't want to keep _all_ heap blocks in memory. We can
// cache those that are frequent.
//
class heap_block : private boost::noncopyable {
public:
    static const size_t MAX_SIZE = 1024*1024;

    inline heap_block() : index_(0), offset_(0),
			  size_(0), cells_(new cell[MAX_SIZE]) { }
    inline heap_block(size_t index, size_t offset)
        : index_(index), offset_(offset),
	  size_(0), cells_(new cell[MAX_SIZE]) { }
    inline ~heap_block() { delete [] cells_; }

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

    inline void check_index(size_t index) const
    {
	if (index >= size()) {
	    throw heap_index_out_of_range_exception(index, size());
	}
    }

    inline cell & operator [] (size_t addr)
    {
	return find_block(addr)[addr];
    }

    inline const cell & operator [] (size_t addr) const
    {
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
        if (s.tag() == tag_t::CON) {
	    return static_cast<const con_cell &>(s);
        }
        if (s.tag() != tag_t::STR) {
	    throw expected_str_cell_exception(s);
        }
	return functor(static_cast<const str_cell &>(s));
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
	    return c == empty_list_;
	} else if (c.tag() == tag_t::STR) {
   	    return functor(c) == empty_list_;
        } else {
	    return false;
        }
    }

    inline bool is_dotted_pair(const cell c) const
    {
	if (c.tag() == tag_t::CON) {
	    return c == dotted_pair_;
	} else if (c.tag() == tag_t::STR) {
	    return functor(c) == dotted_pair_;
	} else {
	    return false;
	}
    }

    inline bool is_comma(const cell c) const
    {
	if (c.tag() == tag_t::CON) {
	    return c == comma_;
	} else if (c.tag() == tag_t::STR) {
	    return functor(c) == comma_;
	} else {
	    return false;
	}
    }

    cell deref(cell c) const;

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
	size_t arity = con.arity();
	cell *p;
	size_t index;
	std::tie(p, index) = allocate(tag_t::STR, arity + 2);
	static_cast<ptr_cell &>(*p).set_index(index+1);
	p[1] = con;
	for (size_t i = 0; i < arity; i++) {
	    p[i+2] = ref_cell(index+i+2);
	}
	return term(*this, *p);
    }

    inline str_cell new_con0(con_cell con)
    {
        cell *p;
	size_t index;
        std::tie(p, index) = allocate(tag_t::CON, 1);
	*p = con;
	return str_cell(index);
    }

    inline str_cell new_str0(con_cell con)
    {
        cell *p;
        size_t index;
        std::tie(p, index) = allocate(tag_t::STR, 2);
	static_cast<ptr_cell &>(*p).set_index(index+1);
	p[1] = con;
	return str_cell(index);
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
        term t = new_str0(dotted_pair_);
	return t;
    }

    inline term new_dotted_pair(const term a, const term b)
    {
	term t = new_str(dotted_pair_);
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
	    static_cast<ref_cell &>(*p).set_index(index+i);
	}
    }

    inline term empty_list() const
    {
	return empty_list_;
    }

    inline con_cell empty_list_con() const
    {
        return empty_list_;
    }

    inline size_t external_ptr_count() const
    {
	return external_ptrs_.size();
    }

    void print_status(std::ostream &out) const;

    void print(std::ostream &out) const;
    void print(std::ostream &out, size_t from, size_t to) const;

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

    inline std::pair<cell *, size_t> allocate(tag_t::kind_t tag, size_t n) {
	if (head_block_->can_allocate(n)) {
	    heap_block *block = head_block_;
	    size_t addr = block->allocate(n);
	    ptr_cell new_cell(tag, addr);
	    cell *p = &(*block)[addr];
	    *p = new_cell;
	    size_ = addr + n;
	    return std::make_pair(p, addr);
	}
	new_block();
	return allocate(tag, n);
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

#ifdef DEBUG_TERM
    mutable std::unordered_map<cell *, size_t> external_ptrs_;
    static size_t id_counter_;
#else
    mutable std::unordered_set<cell *> external_ptrs_;
#endif
    mutable size_t external_ptrs_max_;

    mutable std::vector<std::string> atom_index_to_name_table_;
    mutable std::unordered_map<std::string, size_t> atom_name_to_index_table_;

    con_cell empty_list_;
    con_cell dotted_pair_;
    con_cell comma_;

    template<typename T> friend class ext;
};

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

namespace std {
    template<> struct hash<prologcoin::common::cell> {
        size_t operator()(const prologcoin::common::cell& k) const {
	    return hash<uint64_t>()(k.value());
	}
    };

    template<> struct hash<prologcoin::common::ref_cell> {
        size_t operator()(const prologcoin::common::ref_cell& k) const {
	    return hash<uint64_t>()(k.index());
	}
    };

    template<> struct hash<prologcoin::common::con_cell> {
	size_t operator()(const prologcoin::common::con_cell& k) const {
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


