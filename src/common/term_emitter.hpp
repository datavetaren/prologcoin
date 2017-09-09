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
  enum style { STYLE_TERM, STYLE_PROGRAM };

    term_emitter(std::ostream &out, const heap &h, const term_ops &ops);
    ~term_emitter();

    void set_style( style s );

    void set_var_naming(const std::unordered_map<term, std::string> &var_naming);

    void set_var_name(const term &cell, const std::string &name);

    void print(cell c);
    void nl();

    void set_max_column( size_t max_column );

    std::string name_ref(size_t index) const;

private:
    cell deref(cell c) const { return heap_.deref(c); }

    void indent();
    void emit_token(const std::string &str);
    size_t get_precedence(cell c) const;

    typedef unsigned int flags_t;
    static const flags_t AT_BEGIN = 1 << 0;
    static const flags_t AT_END = 1 << 1;
    static const flags_t AS_TOKEN = 1 << 2;
    static const flags_t INDENT_INC = 1 << 3;
    static const flags_t INDENT_DEC = 1 << 4;
    static const flags_t SET_INDENT = 1 << 5;
    static const flags_t IS_DEF = 1 << 6;
    static const flags_t HAS_PAREN = 1 << 7;
    static const flags_t CHECK_PAREN = 1 << 8;
    static const flags_t SKIP_FUNCTOR = 1 << 9;

    struct elem {
	cell cell_;
	flags_t flags;

	inline elem() : cell_(0), flags(0) { }
	inline elem(cell c) : cell_(c), flags(0) { }
	inline elem(cell c, flags_t f) : cell_(c), flags(f) { }
	inline elem(const elem &other) : cell_(other.cell_), flags(other.flags) { }

	inline bool at_begin() const { return (flags & AT_BEGIN) != 0; }
	inline bool at_end() const { return (flags & AT_END) != 0; }
	inline bool as_token() const { return (flags & AS_TOKEN) != 0; }
        inline bool is_indent_inc() const { return (flags & INDENT_INC) != 0; }
        inline bool is_indent_dec() const { return (flags & INDENT_DEC) != 0; }
	inline bool is_set_indent() const { return (flags & SET_INDENT) != 0; }
	inline bool is_def() const { return (flags & IS_DEF) != 0; }
	inline bool has_paren() const { return (flags & HAS_PAREN) != 0; }
	inline bool check_paren() const { return (flags & CHECK_PAREN) != 0; }
        inline bool is_skip_functor() const { return (flags & SKIP_FUNCTOR) != 0; }

	void set_flag(flags_t flag, bool b) {
	    flags = ((flags &~flag) | flag*(int)b); }

	void set_at_begin(bool b) { set_flag(AT_BEGIN, b); }
	void set_at_end(bool b) { set_flag(AT_END, b); }
	void set_as_token(bool b) { set_flag(AS_TOKEN, b); }
	void set_indent_inc(bool b) { set_flag(INDENT_INC, b); }
	void set_indent_dec(bool b) { set_flag(INDENT_DEC, b); }
	void set_indent(bool b) { set_flag(SET_INDENT,b); }
	void set_is_def(bool b) { set_flag(IS_DEF,b); }
	void set_has_paren(bool b) { set_flag(HAS_PAREN,b); }
	void set_check_paren(bool b) { set_flag(CHECK_PAREN,b); }
        void set_skip_functor(bool b) { set_flag(SKIP_FUNCTOR,b); }
    };

    void emit_error(const std::string &msg);

    std::tuple<bool, cell, size_t> check_functor(cell c);
    bool is_begin_alphanum(con_cell c) const;
    bool is_end_alphanum(con_cell c) const;
    bool is_begin_alphanum(cell c) const;
    bool is_end_alphanum(cell c) const;

    void emit_char(char ch);
    void emit_space();
    void emit_space4();
    void emit_dot();
    void emit_str(const std::string &str);
    void emit_nl();
    void emit_indent_increment();
    void emit_indent_decrement();
    void emit_set_indent(size_t new_column);
    void emit_xf(bool is_def, cell x, con_cell f, bool wrap_x);
    void emit_fx(bool is_def, con_cell f, cell x, bool wrap_x);
    void emit_xfy(bool is_def, cell x, con_cell f, cell y, bool wrap_x, bool wrap_y);
    void emit_functor_elem(const elem &a);
    void emit_functor_elem_helper(const elem &a);
    void emit_list(const cell lst);
    bool atom_name_needs_quotes(const std::string &name) const;
    void emit_atom_name(const std::string &name);
    void emit_functor(const term_emitter::elem &e, const con_cell &f, size_t index);
    void emit_functor_name(const con_cell &f);
    void emit_functor_args(const con_cell &f, size_t index, bool with_paren = true);
    void push_functor_args(size_t index, size_t arity, bool with_paren);
    void emit_ref(const elem &a);
    void emit_int(const elem &a);
    void increment_indent_level();
    void decrement_indent_level();
    void wrap_paren(const term_emitter::elem &e);
    void wrap_curly(const term_emitter::elem &e);
    bool check_wrap_paren(const term_emitter::elem &e, size_t prec_low, size_t prec_high = 1201);

    void mark_indent_column();
    bool will_wrap(size_t len) const;
    bool at_beginning() const;
    bool exceeding_half() const;

    size_t get_emit_length(cell c);

    void print_from_stack(size_t top = 0);

    std::ostream &out_;
    const heap &heap_;
    const term_ops &ops_;

    size_t column_;
    size_t line_;
    size_t indent_level_;
    size_t max_column_;
    char last_char_;
    bool scan_mode_;
    bool first_def_;

    std::vector<size_t> indent_table_;

    std::vector<elem> stack_;

    con_cell dotted_pair_;
    con_cell empty_list_;

    std::unordered_map<term, std::string> *var_naming_;
    bool var_naming_owned_;

    style style_;
};

}}

#endif
