#pragma once

#ifndef _common_term_emitter_hpp
#define _common_term_emitter_hpp

#include <vector>
#include "term.hpp"
#include "term_ops.hpp"

namespace prologcoin { namespace common {

//
// term_emitter
//
// This class emits a term into a sequence of ASCII characters.
//
class term_emitter {
public:
    term_emitter(std::ostream &out, heap &h, term_ops &ops);

    void print(cell c);

    void set_max_column( size_t max_column );

    std::string name_ref(size_t index) const;

private:
    void nl();
    void indent();
    void emit_token(const std::string &str);
    size_t get_precedence(cell c) const;

    typedef unsigned int flags_t;
    static const flags_t NEED_PAREN = 1 << 0;
    static const flags_t AT_BEGIN = 1 << 1;
    static const flags_t AT_END = 1 << 2;
    static const flags_t IN_LIST = 1 << 3;
    static const flags_t AS_ARG = 1 << 4;
    static const flags_t AS_TOKEN = 1 << 5;

    struct elem {
	cell cell_;
	flags_t flags;

	inline elem() : cell_(0), flags(0) { }
	inline elem(cell c) : cell_(c), flags(0) { }
	inline elem(cell c, flags_t f) : cell_(c), flags(f) { }
	inline elem(const elem &other) : cell_(other.cell_), flags(other.flags) { }

	inline bool need_paren() const { return (flags & NEED_PAREN) != 0; }
	inline bool at_begin() const { return (flags & AT_BEGIN) != 0; }
	inline bool at_end() const { return (flags & AT_END) != 0; }
	inline bool in_list() const { return (flags & IN_LIST) != 0; }
	inline bool as_arg() const { return (flags & AS_ARG) != 0; }
	inline bool as_token() const { return (flags & AS_TOKEN) != 0; }

	void set_flag(flags_t flag, bool b) {
	    flags = ((flags &~flag) | flag*(int)b); }

	void set_need_paren(bool b) { set_flag(NEED_PAREN, b); }
	void set_at_begin(bool b) { set_flag(AT_BEGIN, b); }
	void set_at_end(bool b) { set_flag(AT_END, b); }
	void set_in_list(bool b) { set_flag(IN_LIST, b); }
	void set_as_arg(bool b) { set_flag(AS_ARG, b); }
	void set_as_token(bool b) { set_flag(AS_TOKEN, b); }
    };

    void emit_error(const std::string &msg);

    std::tuple<bool, cell, size_t> check_functor(cell c);
    bool is_begin_alphanum(con_cell c) const;
    bool is_end_alphanum(con_cell c) const;
    bool is_begin_alphanum(cell c) const;
    bool is_end_alphanum(cell c) const;
    void emit_char(char ch);
    void emit_space();
    void emit_xf(cell x, con_cell f, bool wrap_x);
    void emit_fx(con_cell f, cell x, bool wrap_x);
    void emit_xfy(cell x, con_cell f, cell y, bool wrap_x, bool wrap_y);
    void emit_functor_elem(const elem &a);
    void emit_functor(const con_cell &f, size_t index);
    void push_functor_args(size_t index, size_t arity);
    void emit_ref(const elem &a);
    void emit_int(const elem &a);
    void increment_indent_level();
    void decrement_indent_level();
    void wrap_paren(const term_emitter::elem &e);

    void mark_indent_column();
    bool will_wrap(size_t len) const;
    bool at_beginning() const;
    bool exceeding_half() const;

    size_t get_emit_length(cell c);

    void print_from_stack(size_t top = 0);

    std::ostream &out_;
    heap &heap_;
    term_ops &ops_;

    size_t column_;
    size_t line_;
    size_t indent_level_;
    size_t max_column_;
    char last_char_;
    bool scan_mode_;

    std::vector<size_t> indent_table_;

    std::vector<elem> stack_;
};

}}

#endif
