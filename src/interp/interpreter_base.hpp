#pragma once

#ifndef _interp_interpreter_base_hpp
#define _interp_interpreter_base_hpp

#include <istream>
#include <vector>
#include <stack>
#include <tuple>
#include <map>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/thread.hpp>
#include "../common/term_env.hpp"
#include "../common/term_tokenizer.hpp"
#include "../common/merkle_trie.hpp"
#include "../common/utime.hpp"
#include "../common/spinlock.hpp"
#include "builtins.hpp"
#include "file_stream.hpp"
#include "arithmetics.hpp"
#include "locale.hpp"
#include "source_element.hpp"
#include "interpreter_exception.hpp"

extern "C" void frotz();

namespace prologcoin { namespace interp {
class interpreter_base;

typedef std::pair<qname, common::cell> functor_index;

class predicate;
    
class managed_clause {
public:
    inline managed_clause()
        : clause_(), cost_(0), ordinal_(0) { }
    inline managed_clause(common::term cl, uint64_t cost, size_t ord)
        : clause_(cl), cost_(cost), ordinal_(ord) { }
    inline managed_clause(const managed_clause &other) = default;

    inline common::term clause() const {
	return clause_;
    }

    inline bool is_erased() const {
        return clause_ == common::term();
    }

    inline void erase() {
        clause_ = common::term();
    }
  
    inline size_t ordinal() const {
        return ordinal_;
    }

    inline uint64_t cost() const {
	return cost_;
    }

private:
    friend class predicate;
    void set_ordinal(size_t ord) { ordinal_ = ord; }

    common::term clause_;
    size_t cost_;
    size_t ordinal_;
};

typedef std::vector<managed_clause> managed_clauses;

enum clause_position { FIRST_CLAUSE, LAST_CLAUSE };
    
class predicate {
public:
  inline predicate() = default;
  inline predicate(const predicate &other) = default;
  inline predicate(const qname &qn) : qname_(qn), id_(0), with_vars_(false), num_clauses_(0),was_compiled_(false),ok_to_compile_(true), performance_count_(0) { }
  inline const qname & qualified_name() const { return qname_; }

  inline const std::vector<managed_clause> & clauses() const { return clauses_; }

  inline bool empty() const { return num_clauses_ == 0; }
  
  void add_clause(interpreter_base &interp,
		  common::term clause,
		  clause_position pos = LAST_CLAUSE);

  size_t get_term_id(interpreter_base &interp, common::term first_arg) const;
  const std::vector<managed_clause> & get_clauses(interpreter_base &interp, size_t term_id) const;
  const std::vector<managed_clause> & get_clauses(interpreter_base &interp, common::term first_arg) const;

  inline const std::vector<managed_clause> & get_clauses() const
      { return clauses_; }

  inline size_t num_clauses() const { return num_clauses_; }

  inline void clear()
  {
      clauses_.clear();
      indexed_.clear();
      with_vars_ = false;
      clear_cache();
      num_clauses_ = 0;
  }

  inline void clear_cache()
  {
      filtered_.clear();
      term_id_.clear();
  }

  inline bool was_compiled() const {
      return was_compiled_;
  }
    
  inline void set_was_compiled(bool on) {
      was_compiled_ = on;
  }

  inline bool ok_to_compile() const {
      return ok_to_compile_;
  }

  inline void set_ok_to_compile(bool on) {
      ok_to_compile_ = on;
  }

  bool matching_clauses(interpreter_base &interp, common::term head);
  bool remove_clauses(interpreter_base &interp, common::term head, bool all);

  size_t id() const { return id_; }

  size_t performance_count() const { return performance_count_; }

private:
    bool matched_indexed_clause(interpreter_base &interp, common::term head);
    managed_clause remove_indexed_clause(interpreter_base &interp, common::term head);
    friend class interpreter_base;
  
    void set_id(size_t identifier) { id_ = identifier; }
  
    qname qname_;
    size_t id_;
    mutable std::vector<managed_clause> clauses_;
    mutable std::unordered_map<common::term, std::vector<managed_clause> > indexed_;
    mutable std::vector<std::vector<managed_clause> > filtered_;
    mutable std::unordered_map<common::term, size_t> term_id_;
    bool with_vars_;
    size_t num_clauses_;
    bool was_compiled_;
    bool ok_to_compile_;
    mutable size_t performance_count_;
};

class module_meta {
public:
    inline module_meta() : changed_(false) { }

    module_meta(const module_meta &other) = delete;

    inline common::con_cell get_name() const { return name_; }
    inline void set_name(common::con_cell name) { name_ = name; }
  
    inline void set_source_elements(const std::vector<source_element> &elems)
    { source_elements_ = elems; }

    inline const std::vector<source_element> & get_source_elements() const
    { return source_elements_; }

    inline void changed() { changed_ = true; }
    inline bool has_changed() const { return changed_; }
    inline void clear_changed() { changed_ = false; }
  
private:
    common::con_cell name_;
    std::vector<source_element> source_elements_;
    bool changed_;
};
    
}}

namespace std {
    template<> struct hash<prologcoin::interp::qname> {
	size_t operator()(const prologcoin::interp::qname &k) const {
	    return k.first.raw_value() + 
		   17*k.second.raw_value();
	}
    };
    template<> struct hash<prologcoin::interp::functor_index> {
	size_t operator()(const prologcoin::interp::functor_index &k) const {
	    return k.first.first.raw_value() + 
	   	   17*k.first.second.raw_value() +
	           131*k.second.raw_value();
	}
    };
}

namespace prologcoin { namespace interp {
class wam_instruction_base;

// Contemplated this be a union, but it's better to ensure
// portability. And it might be good to have access to both
// the instruction pointer as well as the label.
class code_point {
public:
    inline code_point() : wam_code_(nullptr), term_code_(common::ref_cell(0)){}
    explicit inline code_point(const common::term t) : wam_code_(nullptr), term_code_(t)
    { static const common::con_cell el = common::con_cell("[]",0); module_ = el; }
    explicit inline code_point(const common::con_cell l) : wam_code_(nullptr), term_code_(l)
    { static const common::con_cell el = common::con_cell("[]",0); module_ = el; }
    explicit inline code_point(const common::int_cell i) : wam_code_(nullptr), term_code_(i)
    { static const common::con_cell el = common::con_cell("[]",0); module_ = el; }
    inline code_point(const code_point &other) = default;
    inline code_point(wam_instruction_base *i)
        : wam_code_(i), term_code_(common::ref_cell(0))
    { 
	static const common::con_cell WAM = common::con_cell("$WAM",0);
	module_ = WAM;
    }

    inline code_point(const common::con_cell module,
		      const common::con_cell name)
        : bn_(nullptr), module_(module), term_code_(name) { }

    inline code_point(const common::con_cell name, builtin_fn f, bool is_recursive)
      : bn_(f), term_code_(name) {
        static const common::con_cell BUILTIN = common::con_cell("$BN",0);
        static const common::con_cell BUILTIN_R = common::con_cell("$BNR",0);
        module_ = is_recursive ? BUILTIN_R : BUILTIN;
    }

    inline static code_point fail() {
        return code_point();
    }

    inline void reset()
    { static const common::con_cell el = common::con_cell("[]",0);
      wam_code_ = nullptr;
      module_ = el;
      term_code_ = fail_term_;
    }

    inline bool is_fail() const { return term_code_ == fail_term_; }

    inline bool has_wam_code() const {
	static const common::con_cell WAM = common::con_cell("$WAM",0);
	return module_ == WAM; 
    }

    inline bool is_builtin() const {
	static const common::con_cell BUILTIN = common::con_cell("$BN",0);
	static const common::con_cell BUILTIN_R = common::con_cell("$BNR",0);	
        return module_ == BUILTIN || module_ == BUILTIN_R;
    }

    inline bool is_builtin_recursive() const {
	static const common::con_cell BUILTIN_R = common::con_cell("$BNR",0);	      
        return module_ == BUILTIN_R;
    }

    inline wam_instruction_base * wam_code() const { return wam_code_; }
    inline builtin_fn bn() const { return bn_; }
    inline const common::con_cell & module() const { return module_; }
    inline const common::term & term_code() const { return term_code_; }
    inline const common::int_cell & label() const { return reinterpret_cast<const common::int_cell &>(term_code_); }
    inline const common::con_cell & name() const { return reinterpret_cast<const common::con_cell &>(term_code_); }

    inline const qname qn() const { return qname(module(), name()); }

    inline void set_wam_code(wam_instruction_base *p) {
	static const common::con_cell WAM = common::con_cell("$WAM",0);
	static const common::con_cell el = common::con_cell("[]",0);
	if (p == nullptr) {
	    module_ = el;
	    term_code_ = fail_term_;
	} else {
	    module_ = WAM;
	}
	wam_code_ = p;
    }
    inline void set_term_code(const common::term t) { term_code_ = t; }

    inline void set_qn(const qname &qn) {
	module_ = qn.first;
	term_code_ = qn.second;
    }

    std::string to_string(const interpreter_base &interp) const;

private:
    static const common::term fail_term_;

    union {
        wam_instruction_base *wam_code_;
        builtin_fn bn_;
    };
    common::con_cell module_;
    common::term term_code_;
};

// Saved wrapped environment
struct environment_base_t;

enum environment_kind_t { ENV_NAIVE = 0, ENV_WAM = 1, ENV_FROZEN = 2 };

struct environment_saved_t {
    uint64_t saved;

    environment_saved_t(environment_base_t *e, environment_kind_t k)
    {
        saved = reinterpret_cast<uint64_t>(e) + static_cast<uint64_t>(k);
    }

    environment_base_t * ce0()
    {
        return reinterpret_cast<environment_base_t *>(saved & ~0x3);
    }

    environment_kind_t kind()
    {
        return static_cast<environment_kind_t>(saved & 0x3);
    }

    std::pair<environment_base_t *, environment_kind_t> ce()
    {
        return std::make_pair(ce0(), kind());
    }
};

// Base data type for stack frames
struct environment_base_t {
    environment_saved_t   ce; // Continuation environment
    code_point            cp; // Continuation point
};

// Data type for stack frames with Y variables (for WAM)
struct environment_t : public environment_base_t {
    common::term          yn[]; // Y variables
};

struct meta_context;

struct choice_point_t {
    environment_saved_t   ce;
    meta_context          *m;
    code_point            cp;
    choice_point_t       *b;
    code_point            bp;
    size_t                tr;
    size_t                h; 
    choice_point_t       *b0;
    common::term          qr; // Only used for naive interpreter (for now)
    common::con_cell      pr; // Only used for naive interpreter (for now)
    size_t                arity;
    common::term          ai[];
};

// Standard environment (for naive interpreter)
struct environment_naive_t : public environment_base_t {
    choice_point_t       *b0;
    common::term          qr;
    common::con_cell      pr;
};

struct environment_frozen_t : public environment_naive_t {
    code_point            p; // restore p when deallocating
    size_t                num_extra; // Number of extra cells
    common::term          extra[];
};

typedef union {
    environment_t *e;
    environment_naive_t *ee;
    environment_frozen_t *ef;
    choice_point_t *cp;
    common::term term;
} word_t;

//
// The purpose of the meta context is to store environments &
// choice points upon recursive invocation of the interpreter.
//
class meta_reason_t {
public:
    enum meta_reason_enum_t {
	META_UNKNOWN = 0,
	META_RETURN = 1,
	META_BACKTRACK = 2,
	META_DELETE = 3
    } enum_;
    inline meta_reason_t(meta_reason_enum_t e) : enum_(e) { }
    inline bool operator == (meta_reason_enum_t t) const {
	return enum_ == t;
    }
    inline std::string str() const {
	switch (enum_) {
	case META_UNKNOWN: return "META_UNKNOWN";
	case META_RETURN: return "META_RETURN";
	case META_BACKTRACK: return "META_BACKTRACK";
	case META_DELETE: return "META_DELETE";
	}
    }
};

struct meta_context;
	
typedef bool (*meta_fn)(interpreter_base &, const meta_reason_t &reason);
struct meta_context {
    meta_context(interpreter_base &i, meta_fn fn);

    inline void * operator new (size_t n, word_t *where) {
	return reinterpret_cast<void *>(where);
    }

    // Silence warning
    inline void operator delete (void *, word_t *) {
    }

    size_t size_in_words;
    meta_fn fn;
    meta_context *old_m;
    choice_point_t *old_top_b;
    choice_point_t *old_b;
    choice_point_t *old_b0;
    environment_base_t *old_top_e;
    environment_base_t *old_e;
    environment_kind_t old_e_kind;
    code_point old_p;
    code_point old_cp;
    common::term old_qr;
    size_t old_hb;
};

class interpreter;

template<environment_kind_t K> struct env_type_from_kind { };
template<> struct env_type_from_kind<ENV_NAIVE> { typedef environment_naive_t * type; };
template<> struct env_type_from_kind<ENV_WAM> { typedef environment_t * type; };
template<> struct env_type_from_kind<ENV_FROZEN> { typedef environment_frozen_t * type; };
  

// Used with set_managed_data() and get_managed_data() which can be
// used to store additional data associated with the interpreter.
//
// The destructor (via delete) is called when the interpreter is destroyed.
    
class managed_data {
public:
    virtual ~managed_data() { }
};

class wam_compiler;

class interpreter_base : public common::term_env {
    friend class builtins;
    friend class builtins_opt;
    friend class builtins_fileio;
    friend class arithmetics;
    friend struct meta_context;
    friend class interpreter;
    friend struct new_instance_context;
    friend class remote_execution_proxy;
    friend class predicate;

private:
    static const size_t STACK_BASE = 0x80000000000000;
    static const size_t MAX_STACK_SIZE = 1024*1024;
    static const size_t MAX_STACK_SIZE_WORDS = MAX_STACK_SIZE / sizeof(word_t);
    static const size_t MAX_STACK_FRAME_WORDS = 4096 / sizeof(word_t);

public:
    typedef common::term term;
    typedef common::cell cell;
    typedef common::con_cell con_cell;
    typedef common::int_cell int_cell;

    interpreter_base(const std::string &name);
    virtual ~interpreter_base();

    static const size_t MAX_ARGS = 256;

    inline void heap_set_size(size_t sz) {
        term_env::heap_set_size(sz);
	set_register_hb(sz);
    }

    inline con_cell current_module() const { return current_module_; }
    void set_current_module(con_cell m);
  
    inline const locale & current_locale() const { return locale_; }
    inline locale & current_locale() { return locale_; }

    inline bool is_debug() const { return debug_; }
    inline void set_debug(bool dbg) { debug_ = dbg; arith_.set_debug(dbg); }
    inline void debug_check() { debug_check_fn_(); }
    inline void set_debug_check_fn(std::function<void ()> fn)
        { debug_check_fn_ = fn; }
    void set_debug_enabled();

    inline std::string to_string(const term t, const common::emitter_options &opt) const {
        return term_env::to_string(t, opt);      
    }
  
    inline std::string to_string(const term t) const {
        return term_env::to_string(t);
    }
  
    inline std::string to_string(const qname &qn) const {
        std::string s = to_string(qn.first);
        s += ":";
        s += to_string(qn.second);
	return s;
    }

    void reset();
    void total_reset();

    bool is_track_cost() const { return track_cost_; }
    void set_track_cost(bool b) { track_cost_ = b; }

    void enable_file_io();
    const std::string & get_current_directory() const;
    void set_current_directory(const std::string &dir);
    std::string get_full_path(const std::string &path) const;
    void close_all_files();
    bool is_file_id(size_t id) const;
    void reset_files();

    inline arithmetics & arith() { return arith_; }

    inline void load_builtin(con_cell f, builtin b)
        { qname qn(current_module_, f);
	  load_builtin(qn, b);
	}

    inline void load_builtin(con_cell module, con_cell f, builtin b)
        { qname qn(module, f);
	  load_builtin(qn, b);
	}

    void load_clause(const std::string &str);
    void load_clause(std::istream &is);
    void load_clause(term t, clause_position pos);

    con_cell clause_module(term clause)
    {
        auto head = clause_head(clause);
	if (is_functor(head) && functor(head) == COLON) {
	    term mod = arg(head, 0);
	    if (mod.tag() != common::tag_t::CON) {
	        return current_module();
	    } else {
	        return reinterpret_cast<con_cell &>(mod);
	    }
	} else {
	    return current_module();
	}
    }

    con_cell clause_predicate(term clause)
    {
        auto head = clause_head(clause);
	if (is_functor(head)) {
	    auto fun = functor(head);
	    if (fun == COLON) {
	        term fun1 = arg(head, 1);
	        if (is_functor(fun1)) {
	            return functor(fun1);
	        } else {
	            return EMPTY_LIST;
	        }
	    }
	    return fun;
	} else {
	    return EMPTY_LIST;
	}
    }

    term clause_head(term clause)
    {
        auto f = functor(clause);
	if (f == IMPLIED_BY) {
	    return arg(clause, 0);
	} else {
	    return clause;
	}
    }
  
    term clause_body(term clause)
    {
        auto f = functor(clause);
        if (f == IMPLIED_BY) {
	    return arg(clause, 1);
	} else {
            return EMPTY_LIST;
	}
    }
  
    term clause_first_arg(term clause)
    {
        term head = clause_head(clause);
	if (!is_functor(head)) {
	    return EMPTY_LIST;
	}
	auto f = functor(head);
	if (f == COLON) {
	     head = arg(head, 1);
	     if (!is_functor(head)) {
	         return EMPTY_LIST;
	     }
	     f = functor(head);
	}
	if (f.arity() <= 0) {
	    return EMPTY_LIST;      
	}
	return arg(head, 0);
    }

    term arg_index(term arg)
    {
        switch (arg.tag()) {
	case common::tag_t::CON:
	case common::tag_t::INT:
  	    return arg;
	case common::tag_t::REF:
	case common::tag_t::RFW:
	    return term();
	case common::tag_t::STR:
	    return functor(arg);
	case common::tag_t::BIG:
	    return get_big_header(arg);
	default:
	    assert(false);
	    return term();
        }
    }

    struct none {
        void operator () (term t)  { }
    };
      
    template<typename F = none> void load_program(const std::string &str, F f = F())
    {
	std::stringstream ss(str);
	load_program<F>(ss, f);
    }

    template<typename F = none> void load_program(std::istream &in, F f = F())
    {
	using namespace prologcoin::common;

	term_tokenizer tok(in);
	term_parser parser(tok, *this);
    
	std::vector<term> clauses;

	std::vector<source_element> source_list;

	std::unordered_set<con_cell> seen_predicates;

	while (!parser.is_eof()) {
	    parser.clear_var_names();

	    // The check for EOF may skip comments which is stored in the
	    // parser.
	    for (auto &comment : parser.get_comments()) {
	        if (!comment.is_whitespace()) {
	            source_list.push_back(source_element(comment));
		}
	    }

	    // Calling parse() restes the comment list to empty in the parser
	    auto clause = parser.parse();
	    auto first_pos = parser.first_non_whitespace_token().pos();
	    auto last_pos = parser.last_non_whitespace_token().pos();

	    for (auto &comment : parser.get_comments()) {
	        if (!comment.is_whitespace() && comment.pos().line() < first_pos.line()) {
		    source_list.push_back(source_element(comment));
		}
	    }

	    auto pred = clause_predicate(clause);

	    if (pred == ACTION_BY) {
	        source_list.push_back(source_element(clause));
	    } else if (seen_predicates.count(pred) == 0) {
	        seen_predicates.insert(pred);
		source_list.push_back(source_element(pred));
	    }

	    for (auto &comment : parser.get_comments()) {
	        if (!comment.is_whitespace() && comment.pos().line() >= last_pos.line()) {
	            source_list.push_back(source_element(comment));
		}
	    }

	    parser.clear_comments();

	    // Once parsing is done we'll copy over the var-name bindings
	    // so we can pretty print the variable names.
	    parser.for_each_var_name( [&](ref_cell ref,
					  const std::string &name)
				      { set_name(ref, name); } );

	    clauses.push_back(clause);
	}

	term clause_list = EMPTY_LIST;
	for (auto clause : boost::adaptors::reverse(clauses)) {
	    clause_list = new_dotted_pair(clause, clause_list);
	}

	con_cell primary_module = current_module();
	load_program<F>(clause_list, f, primary_module);

	module_meta &mm = module_meta_db_[primary_module];
        mm.set_source_elements(source_list);
	mm.clear_changed();
    }

    template<typename F = none> void load_program(term clauses, F f = F()) {
        con_cell dummy;
        load_program<F>(clauses, f, dummy);
    }
  
    template<typename F = none> void load_program(term clauses, F f , con_cell &primary_module)
    {
        syntax_check_program(clauses);

	con_cell current_mod = current_module();
	primary_module = current_mod;

	bool first_mod = true;

	std::unordered_set<qname> seen;
	
	for (auto clause : list_iterator(*this, clauses)) {
	    auto mod = clause_module(clause);
	    auto pn = clause_predicate(clause);
	    qname qn{mod, pn};
	    auto &pred = get_predicate(qn);
	    if (!seen.count(qn)) {
	        pred.clear();
		seen.insert(qn);
	    }
	    load_clause(clause, LAST_CLAUSE);
	    f(clause);
	    if (current_mod != current_module() && first_mod) {
	        // First module change!
	        current_mod = current_module();
		first_mod = false;
		primary_module = current_mod;
	    }
	}
    }

    void import_predicate(const qname &qn);
    void use_module(con_cell module);
  
    void save_program(con_cell module, std::ostream &out);
    void save_predicate(const qname &qn, std::ostream &out);
    void save_clause(term t, std::ostream &out);
    void save_comment(const common::term_tokenizer::token &comment, std::ostream &out);


    inline const predicate & get_predicate(size_t id) const
        {
          static const predicate NOT_FOUND = predicate();
	  if (id == 0) return NOT_FOUND;
	  return get_predicate(program_predicates_[id-1]);
	}
  
    inline const predicate & get_predicate(con_cell f) const
        { return get_predicate(std::make_pair(current_module_, f)); }

  
    inline const predicate & get_predicate(con_cell module, con_cell f) const
        { return get_predicate(std::make_pair(module, f)); }

    inline const predicate & get_predicate(const qname &pn) const {
	  return const_cast<interpreter_base *>(this)->get_predicate(pn);
        }

    inline void clear_predicate(const qname &qn) {
	auto it = program_db_.find(qn);
	if (it != program_db_.end()) {
	    program_db_.erase(it);
	    auto preds = module_db_[qn.first];
	    auto it2 = std::find(preds.begin(), preds.end(), qn);
	    if (it2 != preds.end()) {
		preds.erase(it2);
	    }
	}
    }

    inline const predicate * internal_get_predicate(const qname &qn) const {
	auto it = program_db_.find(qn);
	if (it == program_db_.end()) {
	    return nullptr;
	}
	return &(it->second);
    }

    inline predicate * internal_get_predicate(const qname &qn) {
	auto it = program_db_.find(qn);
	if (it == program_db_.end()) {
	    return nullptr;
	}
	return &(it->second);
    } 

    inline predicate & get_predicate(const qname &qn)
        {
	    auto it = program_db_.find(qn);
	    if (it != program_db_.end()) {
		auto &pred = it->second;
		if (pred.empty()) {
		    load_predicate(qn);
		}
		return it->second;
	    }

	    size_t id = program_predicates_.size() + 1;
	    program_predicates_.push_back(qn);
	    predicate &pred = program_db_[qn];
	    pred = predicate(qn);
	    pred.set_id(id);

	    // Enables a client to fill the predicate with clauses
	    // (from a database)
	    load_predicate(qn);

	    return pred;
	}

    virtual size_t num_predicates() const
    {
	return program_db_.size();
    }

    inline size_t num_clauses() const
    {
	size_t n = 0;
	for (auto &p : program_db_) {
	    auto &predicate = p.second;
	    n += predicate.clauses().size();
	}
	return n;
    }

    inline void restore_predicate(const predicate &p)
    {
	const qname &qn = p.qualified_name();
	program_db_[qn] = p;
    }  

    inline const module_meta & get_module_meta(con_cell name)
    {
        return module_meta_db_[name];
    }
  
    inline const std::vector<qname> & get_module(const con_cell name)
        { auto it = module_db_.find(name);
	  if (it == module_db_.end()) {
	      module_meta_db_[name].set_name(name);
	      return module_db_[name];
	  }
	  return it->second;
	}

    inline bool is_existing_module(const con_cell name)
        { return module_db_.find(name) != module_db_.end(); }

    qname gen_predicate(const con_cell module, size_t arity);

    inline const std::vector<qname> & get_predicates() const
        { return program_predicates_; }

    inline bool has_updated_predicates() const {
        return has_updated_predicates_;
    }

    virtual void updated_predicate_pre(const qname &qn) { }
    virtual void updated_predicate_post(const qname &qn) { }
    virtual void load_predicate(const qname &qn) { }
    virtual size_t unique_predicate_id(const con_cell module) {
	return program_predicates_.size() + 1;
    }

    void internal_updated_predicate_post(const qname &qn) {
        updated_predicates_.insert(qn);
	has_updated_predicates_ = true;
	module_meta_db_[qn.first].changed();
	updated_predicate_post(qn);
    }
  
    inline const std::unordered_set<qname> & get_updated_predicates() const
        { return updated_predicates_; }

    inline bool is_updated_predicate(const qname &pn) const
        { return updated_predicates_.find(pn) != updated_predicates_.end(); }
    inline void clear_updated_predicates()
        { updated_predicates_.clear();
	  has_updated_predicates_ = false;
	}
    inline void clear_updated_predicate(const qname &qn) {
        auto it = updated_predicates_.find(qn);
	if (it != updated_predicates_.end()) {
	    updated_predicates_.erase(it);
	    if (updated_predicates_.empty()) {
  	        has_updated_predicates_ = false;
	    }
	}
    }
 
    std::string to_string_cp(const code_point &cp) const
        { return cp.to_string(*this); }

    void print_db() const;
    void print_db(std::ostream &out) const;
    void print_predicate(std::ostream &out, const qname &qn) const;
    void print_predicate(std::ostream &out, const qname &qn, bool &do_nl_p) const;
    void print_profile() const;
    void print_profile(std::ostream &out) const;

    class list_iterator : public common::term_iterator {
    public:
	list_iterator(Env &env, const common::term t)
            : common::term_iterator(env, t) { }

	list_iterator begin() {
	    return *this;
	}

	list_iterator end() {
	    return list_iterator(env(), interpreter_base::EMPTY_LIST);
	}
    };

    inline const code_point & get_code(const qname &qn)
    {
        return code_db_[qn];
    }

    inline void set_code(const qname &qn, const code_point &cp)
    {
        code_db_[qn] = cp;
    }
    
protected:
    inline std::unordered_map<qname, code_point> & code_db() {
	return code_db_;
    }
public:
			       
    inline builtin & get_builtin(const qname &qn)
    {
	static builtin empty_bn_;

        auto it = builtins_.find(qn);
        if (it == builtins_.end()) {
	    return empty_bn_;
        } else {
	    return it->second;
	}
    }

    inline bool is_builtin(const qname &qn) const
    { return builtins_.find(qn) != builtins_.end(); }

    inline uint64_t accumulated_cost() const
        { return accumulated_cost_; }

    inline void set_maximum_cost(uint64_t cost) { maximum_cost_ = cost; }

    inline bool unify(term a, term b)
       { using namespace prologcoin::common;
	 uint64_t cost = 0;
	 bool ok = common::term_env::unify(a, b, cost);
	 add_accumulated_cost(cost);
	 return ok;
       }

    inline bool can_unify(term a, term b) {
        using namespace prologcoin::common;
	size_t old_hb = get_register_hb();
	set_register_hb(heap_size());
	size_t current_tr = trail_size();
	bool r = unify(a, b);
	if (r) {
	    unwind(current_tr);
	}
	set_register_hb(old_hb);
	return r;
    }

    inline int standard_order(const term a, const term b)
       { uint64_t cost = 0;
	int cmp = common::term_env::standard_order(a, b, cost);
	add_accumulated_cost(cost);
	return cmp;
       }

    inline term copy(term t, term_env &src)
       { uint64_t cost = 0;
         term c = common::term_env::copy(t, src, cost);
	 add_accumulated_cost(cost);
	 return c;
       }

    inline term copy(term t)
       { uint64_t cost = 0;
         term c = common::term_env::copy(t, cost);
	 add_accumulated_cost(cost);
	 return c;
       }

    inline term copy_without_names(term t)
       { uint64_t cost = 0;
         term c = common::term_env::copy_without_names(t, cost);
	 add_accumulated_cost(cost);
	 return c;
       }

    inline term copy_except_big(term t)
       { uint64_t cost = 0;
         term c = common::term_env::copy_except_big(t, cost);
	 add_accumulated_cost(cost);
	 return c;
       }
  
    inline managed_data * get_managed_data(common::con_cell key)
       { auto it = managed_data_.find(key);
	 if (it == managed_data_.end()) {
	     return nullptr;
	 } else {
  	     return it->second;
	 }
       }

    inline void set_managed_data(common::con_cell key, managed_data *data)
       { managed_data_[key] = data; }

    inline meta_context * get_current_meta_context()
        { return register_m_; }

    inline const meta_context * get_current_meta_context() const
        { return register_m_; }

    template<typename T> inline T * get_current_meta_context()
        { return reinterpret_cast<T *>(get_current_meta_context()); }

    inline bool has_meta_context() const
        { return register_m_ != nullptr; }

protected:
    friend class wam_interpreter;
  
    template<typename T> inline size_t words() const
    { return sizeof(T)/sizeof(word_t); }

    template<typename T> inline word_t * base(T *t) const
    { return reinterpret_cast<word_t *>(t); }

    inline bool is_stack(common::ref_cell ref) const
    {
        return ref.index() >= STACK_BASE;
    }

    inline bool is_stack(size_t ref) const
    {
	return ref >= STACK_BASE;
    }

    inline const word_t * to_stack(common::ref_cell ref) const
    {
        return &stack_[ref.index() - STACK_BASE];
    }

    inline word_t * to_stack(common::ref_cell ref)
    {
        return &stack_[ref.index() - STACK_BASE];
    }
  
    inline const word_t * to_stack(size_t ref) const
    {
	return &stack_[ref - STACK_BASE];
    }

    inline word_t * to_stack(size_t ref)
    {
	return &stack_[ref - STACK_BASE];
    }

    inline size_t to_stack_addr(word_t *p)
    {
        return static_cast<size_t>(p - stack_) + STACK_BASE;
    }
    inline size_t to_stack_relative_addr(word_t *p)
    {
        return static_cast<size_t>(p - stack_);
    }

    typedef size_t (*num_y_fn_t)(interpreter_base *interp, bool use_previous);
    typedef void (*save_state_fn_t)(interpreter_base *interp);
    typedef void (*restore_state_fn_t)(interpreter_base *interp);

    inline num_y_fn_t num_y_fn()
    {
        return num_y_fn_;
    }

    inline void set_num_y_fn(num_y_fn_t num_y_fn)
    {
        num_y_fn_ = num_y_fn;
    }

    inline save_state_fn_t save_state_fn()
    {
        return save_state_fn_;
    }

    inline restore_state_fn_t restore_state_fn()
    {
        return restore_state_fn_;
    }

    inline void set_save_restore_state_fns( save_state_fn_t ffn, restore_state_fn_t rfn)
    {
        save_state_fn_ = ffn;
	restore_state_fn_ = rfn;
    }

    inline term & a(size_t i)
    {
        return register_ai_[i];    
    }

    inline term & y(size_t i)
    {
        return e()->yn[i];
    }

    inline term * args()
    {
        return &register_ai_[0];
    }

    inline size_t num_of_args()
    {
        return num_of_args_;
    }

    inline void set_num_of_args(size_t n)
    {
        num_of_args_ = n;
    }

    static inline size_t num_y(interpreter_base *interp, bool)
    {
        if (interp->e_kind() == ENV_FROZEN) {
	    size_t n = (sizeof(environment_frozen_t)-sizeof(environment_base_t))/sizeof(term);
  	    // Extra cells 
  	    n += interp->ef()->num_extra;
	    return n;
        }  else {
	    return (sizeof(environment_naive_t)-sizeof(environment_base_t))/sizeof(term);
	}
    }

    static inline void save_state(interpreter_base *interp)
    {
        // Nothing being ordinary needs to be done in the naive interpreter as
        // it is not using any registers (except argument registers) but you
        // cannot terminate execution in the naive interpreter while those registers
        // are being manipulated.
        //
        // The purpose for save_state() is to deal with frozen closures, that can be
        // woken up (like an interrupt) after unification. Then we need to execute
        // some code before returning to the normal continuation point.

        interp->ef()->num_extra = 0;
	interp->set_cp( code_point(EMPTY_LIST) );
	interp->set_p( code_point(EMPTY_LIST) );
    }

    static inline void restore_state(interpreter_base *)
    {
        // See save_state
    }

    inline state_context save_term_state() {
        return term_env::save_state();
    }

    inline void restore_term_state(state_context &ctx) {
        term_env::restore_state(ctx);
    }
  
    inline code_point & p()
    {
        return register_p_;
    }

    inline void set_p(const code_point &p1)
    {
        register_p_ = p1;
    }

    inline void set_cp(const code_point &cp)
    {
        register_cp_ = cp;
    }

    inline code_point & cp()
    {
	return register_cp_;
    }

    inline void set_register_hb(choice_point_t *cp) {
	bool is_null = reinterpret_cast<void *>(cp) == nullptr;
	if (is_null) {
	    set_register_hb(top_hb());
	} else {
	    set_register_hb(cp->h);
	}
    }

public:
    inline void set_register_hb(size_t h) {
	term_env::set_register_hb(h);
    }
protected:

    inline bool has_late_choice_point() 
    {
        return b() > b0();
    }

    inline void cut_direct()
    {
	set_b(b0());
	set_register_hb(b());
	tidy_trail();
    }
    inline void cut()
    {
	if (b() > b0()) {
	    cut_direct();
	}
    }

    void unwind_to_top_choice_point();

    inline void set_top_e()
    {
        register_top_e_ = register_e_;
    }

    inline void set_top_e(environment_base_t *e)
    {
        register_top_e_ = e;
    }

    inline environment_base_t * e0()
    {
        return register_e_;
    }

    inline const environment_base_t * e0() const
    {
        return register_e_;
    }

    inline environment_t * e()
    {
        return reinterpret_cast<environment_t *>(e0());
    }

    inline environment_naive_t * ee()
    {
        return reinterpret_cast<environment_naive_t *>(e0());
    }

    inline environment_frozen_t * ef()
    {
        return reinterpret_cast<environment_frozen_t *>(e0());
    }  

    inline environment_kind_t e_kind()
    {
        return register_e_kind_;
    }

    inline environment_saved_t save_e()
    {
        return environment_saved_t(e0(), e_kind());
    }

    inline void set_e(environment_saved_t saved)
    {
        std::tie(register_e_, register_e_kind_) = saved.ce();
    }

    inline void set_e(environment_base_t *e, environment_kind_t k)
    {
        register_e_ = e;
	register_e_kind_ = k;
    }

    inline void set_ee(environment_naive_t *ee)
    {
        register_e_ = ee;
	register_e_kind_ = ENV_NAIVE;
    }

    inline environment_base_t * top_e() const
    {
        return register_top_e_;
    }

    inline choice_point_t * b() const
    {
        return register_b_;
    }

    // Choice point is WAM based if continuation environment is WAM based.
    inline bool b_is_wam() const
    {
	if (b() == nullptr) {
	    return false;
	} else {
	    return b()->bp.has_wam_code();
	}
    }

    inline void set_b(choice_point_t *b)
    {
        register_b_ = b;
    }

    inline choice_point_t * b0() const
    {
        return register_b0_;
    }

    inline void set_b0(choice_point_t *b)
    {
        register_b0_ = b;
    }

    inline choice_point_t * top_b() const
    {
        return register_top_b_;
    }

    inline void set_top_b(choice_point_t *b)
    {
        register_top_b_ = b;
    }

    inline size_t top_hb() const
    {
	return register_top_hb_;
    }

    inline void set_top_hb(size_t hb)
    {
	register_top_hb_ = hb;
    }

    inline size_t top_tr() const
    {
	return register_top_tr_;
    }

    inline void set_top_tr(size_t tr)
    {
	register_top_tr_ = tr;
    }

    inline meta_context * m() const { return register_m_; }
    inline void set_m(meta_context *m) { register_m_ = m; }

    inline void set_top_fail(bool b)
    {
        top_fail_ = b;
	set_complete(true);
    }
  
    inline bool is_top_fail() const
    {
        return top_fail_;
    }

    inline void set_complete(bool b)
    {
        complete_ = b;
    }

    inline bool is_complete() const
    {
        return complete_;
    }

    struct stack_frame_visitor {
	virtual void visit_naive_environment(environment_naive_t *e) { }
	virtual void visit_wam_environment(environment_t *e) { }
	virtual void visit_frozen_environment(environment_frozen_t *e) { }
	virtual void visit_choice_point(choice_point_t *cp) { }
	virtual void visit_meta_context(meta_context *m) { }
    };

    void foreach_stack_frame(stack_frame_visitor &cb);

    // Allocate on stack so that we don't overwrite any data of a previous
    // stack frame.
    inline word_t * allocate_stack(bool use_previous)
    {
	word_t *new_s;

	// Is the meta context on top?
	if (base(m()) > base(e0()) && base(m()) > base(b())) {
	    new_s = base(m()) + m()->size_in_words;
	} else if (base(e0()) > base(b())) {
	    auto n = words<term>()*(num_y_fn()(this, use_previous)) + words<environment_base_t>();
	    new_s = base(e0()) + n;
	} else {
	    if (b() == nullptr) {
	        new_s = stack_;
  	    } else {
	        new_s = base(b()) + words<term>()*b()->arity + words<choice_point_t>();
	    }
	}
	if (to_stack_relative_addr(new_s) + MAX_STACK_FRAME_WORDS
	    >= MAX_STACK_SIZE_WORDS) {
	    // There seems to be a weird compiler bug on Mac OSX / clang
	    // If you substitute max_stack_size you'll get an unresolved
	    // symbol linker error.
	    auto const max_stack_size = MAX_STACK_SIZE;
	    throw interpreter_exception_stack_overflow("Exceeded maximum stack size (" + boost::lexical_cast<std::string>(max_stack_size) + " bytes.)");
	}

	return new_s;
    }

    inline size_t program_stack_size() {
	word_t *new_s;

	// Is the meta context on top?
	if (base(m()) > base(e0()) && base(m()) > base(b())) {
	    new_s = base(m()) + m()->size_in_words;
	} else if (base(e0()) > base(b())) {
	    auto n = words<term>()*(num_y_fn()(this, true)) + words<environment_base_t>();
	    new_s = base(e0()) + n;
	} else {
	    if (b() == nullptr) {
	        new_s = stack_;
  	    } else {
	        new_s = base(b()) + words<term>()*b()->arity + words<choice_point_t>();
	    }
	}

	return (new_s - stack_)*sizeof(word_t);
    }
  
    template<environment_kind_t K, typename T = typename env_type_from_kind<K>::type> T allocate_environment();

    inline void deallocate_environment()
    {
        // std::cout << "[before] deallocate_environment: e=" << e() << " p=" << to_string_cp(p()) << " cp=" << to_string_cp(cp()) << "\n";

        switch (e_kind()) {
	case ENV_FROZEN: {
  	    restore_state_fn_(this);
	    environment_frozen_t *ef1 = ef();
	    set_cp(ef1->cp);
	    set_e(ef1->ce);
	    set_b0(ef1->b0);
	    set_qr(ef1->qr);
	    set_pr(ef1->pr);
	    set_p(ef1->p);
	    break;
	    }
	case ENV_NAIVE: {
	    environment_naive_t *ee1 = ee();
	    set_cp(ee1->cp);
	    set_e(ee1->ce);
	    set_b0(ee1->b0);
	    set_qr(ee1->qr);
	    set_pr(ee1->pr);
	    break;
   	    }
	case ENV_WAM:
  	    set_cp(e0()->cp);
	    set_e(e0()->ce);
	    break;
	}
	    
	// std::cout << "[after]  deallocate_environment: e=" << e() << " p=" << to_string_cp(p()) << " cp=" << to_string_cp(cp()) << "\n";
	
    }

    inline void allocate_choice_point(const code_point &cont)
    {
        word_t *new_b0 = allocate_stack(true);
	auto *new_b = reinterpret_cast<choice_point_t *>(new_b0);
	new_b->arity = num_of_args_;
	for (size_t i = 0; i < num_of_args_; i++) {
	    new_b->ai[i] = a(i);
	}
	new_b->ce = save_e();
	new_b->m = register_m_;
	new_b->cp = register_cp_;
	new_b->b = register_b_;
	new_b->bp = cont;
	new_b->tr = trail_size();
	new_b->h = heap_size();
	new_b->b0 = register_b0_;
	new_b->qr = register_qr_;
	new_b->pr = register_pr_;
	register_b_ = new_b;
	set_register_hb(heap_size());

	// std::cout << cnt << ": allocate_choice_point(): b=" << new_b << " trail_size=" << trail_size() << "\n";

        // std::cout << "allocate_choice_point b=" << new_b << " (prev=" << new_b->b << ")\n";
	// std::cout << "allocate_choice_point saved qr=" << to_string(register_qr_) << "\n";
    }

    inline void deallocate_and_proceed()
    {
	if (e0() != top_e()) {
	    bool is_frozen_env = e_kind() == ENV_FROZEN;
	    deallocate_environment();
	    if (is_frozen_env) {
	        // Do not attempt to modify P (with CP) after restored from
	        // a frozen environment, as we attempt to execute exactly where
	        // we left off.
	        return;
	    }
	} else {
	    set_cp(code_point(EMPTY_LIST));
	}
	set_p(cp());
	set_cp(code_point(EMPTY_LIST));
    }
    
    inline void proceed_and_deallocate()
    {
        set_p(cp());
	if (e0() != top_e()) {
	    deallocate_environment();
	    if (is_debug()) {
		std::cout << "interpreter_base::proceed_and_deallocate(): p="
			  << to_string_cp(p()) << " cp="
			  << to_string_cp(cp()) << " e=" << e0() << "\n";
	    }
	} else {
	    set_cp(code_point(EMPTY_LIST));
	    if (is_debug()) {
		std::cout << "interpreter_base::proceed_and_deallocate(): p="
			  << to_string_cp(p()) << " cp="
			  << to_string_cp(cp()) << " e=" << e0() << "\n";
	    }
	}
    }

    void prepare_execution();

    inline const term qr() const
        { return register_qr_; }

    inline void set_qr(term qr)
        { register_qr_ = qr; }

    inline void set_pr(common::con_cell pr)
        { register_pr_ = pr; }

    choice_point_t * reset_to_choice_point(choice_point_t *b);

    void unwind(size_t current_tr);

    term get_first_arg()
    {
        if (num_of_args_ == 0) {
	    return EMPTY_LIST;
	}
	return deref(a(0));
    }

    term get_first_arg(term goal)
    {
	if (goal.tag() == common::tag_t::CON) {
	    // This is a functor constant (probably set by WAM)
	    // we need to use the argument register instead
	    return get_first_arg();
	}
        if (!is_functor(goal)) {
	    return EMPTY_LIST;
        }
	auto f = functor(goal);
	if (f == COLON) {
	    goal = arg(goal, 1);
	    if (!is_functor(goal)) {
	        return EMPTY_LIST;
	    }
	    f = functor(goal);
	}
	if (f.arity() <= 0) {
	    return EMPTY_LIST;
	}
	return arg(goal, 0);
    }

    void abort(const interpreter_exception &ex);
    bool definitely_inequal(const term a, const term b);

    template<typename T, typename... Args> inline T * new_meta_context(meta_fn fn, Args... args) {
        T *context = new(allocate_stack(true)) T(*this, fn, args...);
	context->size_in_words = sizeof(T) / sizeof(word_t);

	if (is_debug()) {
	    std::cout << "interpreter_base::new_meta_context(): ---- e=" << context->old_e << " ----\n";
	}

	register_m_ = context;

	return context;
    }

    inline void release_last_meta_context()
    {
	auto *context = get_current_meta_context();
	register_top_b_ = context->old_top_b;
	register_b_ = context->old_b;
	register_top_e_ = context->old_top_e;
	register_e_ = context->old_e;
	register_p_ = context->old_p;
	register_cp_ = context->old_cp;
	set_qr(context->old_qr);
	set_register_hb(context->old_hb);
        register_m_ = register_m_->old_m;
	
	if (is_debug()) {
	    std::cout << "interpreter_base::release_meta_context(): ---- e=" << e() << " ----\n";
	}
    }

    inline term_env & secondary_env()
        { return secondary_env_; }

    inline void reset_accumulated_cost(uint64_t value = 0)
        { accumulated_cost_ = value; }

    inline void add_accumulated_cost(uint64_t cost)
        { accumulated_cost_ += cost;
          if (accumulated_cost_ >= maximum_cost_) {
	      throw interpreter_exception_out_of_funds(
                        "Not enough funds to complete. Maximum was "
			+ boost::lexical_cast<std::string>(maximum_cost_) + ".");
	  }
        }

public:
    file_stream & new_file_stream(const std::string &path);
    void close_file_stream(size_t id);
    file_stream & get_file_stream(size_t id);
    file_stream & standard_output();
    void tell_standard_output(file_stream &fs);
    void told_standard_output();
    bool has_told_standard_outputs();
    void load_builtins_file_io();

protected:
    void tidy_trail();
    
private:
    void load_builtin(const qname &qn, builtin b);
    void load_builtins();

    void init();

    inline choice_point_t * get_last_choice_point()
    {
        return b();
    }

    inline void move_cut_point_to_last_choice_point()
    {
        set_b0(b());
    }

    void syntax_check_program(term clauses);
    void syntax_check_clause(term clause);
    void syntax_check_head(term clause, term head);
    void syntax_check_body(term clause, term body);
    void syntax_check_goal(term clause, term goal);

    void preprocess_freeze(term term);
    void preprocess_freeze_body(term term);
    term rewrite_freeze_body(term freeze_var, term freeze_body);

    // Useful for meta predicates as scratch area to temporarily
    // copy terms.
    term_env secondary_env_;

    bool debug_;
    bool track_cost_;

    std::unordered_map<qname, code_point> code_db_;
    std::unordered_map<qname, builtin> builtins_;
    std::unordered_map<qname, predicate> program_db_;
    std::unordered_map<con_cell, std::vector<qname> > module_db_;
    std::unordered_map<con_cell, std::unordered_set<qname> > module_db_set_;
    std::vector<qname> program_predicates_;
    bool has_updated_predicates_;
    std::unordered_set<qname> updated_predicates_;
    std::unordered_map<con_cell, module_meta>  module_meta_db_;

    // Stack is emulated at heap offset >= 2^59 (3 bits for tag, remember!)
    // (This conforms to the WAM standard where addr(stack) > addr(heap))

    word_t    *stack_;

    bool top_fail_;
    bool complete_;

    code_point register_p_;
    code_point register_cp_;

    environment_kind_t register_e_kind_;    // What kind of environment 'register_e_' is
    environment_base_t *register_e_;        // Points to current environment
    environment_base_t *register_top_e_;

    choice_point_t *register_b_;
    choice_point_t *register_b0_;
    choice_point_t *register_top_b_;
    size_t register_top_hb_;
    size_t register_top_tr_;    

    // This is for recursive invocation of the interpreter. These meta
    // frames are stored on stack (like environments and choice points.)
    meta_context *register_m_;

    term register_ai_[MAX_ARGS];

    size_t num_of_args_;

    num_y_fn_t num_y_fn_;
    save_state_fn_t save_state_fn_;
    restore_state_fn_t restore_state_fn_;

    term register_qr_;     // Current query 
    con_cell register_pr_; // Current predicate (for profiling)

public:
    static const con_cell COMMA;
    static const con_cell EMPTY_LIST;
    static const con_cell IMPLIED_BY;
    static const con_cell ACTION_BY;
    static const con_cell USER_MODULE;
    static const con_cell COLON;

private:

    std::string current_dir_; // Current directory

    std::unordered_map<size_t, file_stream *> open_files_;
    size_t file_id_count_;
    file_stream * standard_output_;
    std::stack<file_stream *> standard_output_stack_;

    arithmetics arith_;

    std::unordered_map<common::con_cell, uint64_t> profiling_;

    std::function<void ()> debug_check_fn_;

    // Keep track of accumulated cost while interpreter is executing
    uint64_t accumulated_cost_;

    // Maximum cost allowed
    uint64_t maximum_cost_;

    // Locale
    locale locale_;

    // Current module (can be changed with builtin module/1/2)
    common::con_cell current_module_;
  
    std::unordered_map<common::con_cell, managed_data *> managed_data_;

    // If password is in persistent mode
    bool persistent_password_;

    bool is_persistent_password() const { return persistent_password_; }
    void set_persistent_password(bool p) { persistent_password_ = p; }

protected:
    void clear_secret();

private:
    std::map<size_t, term> frozen_closures_;

protected:
    // Closure management is virtual as that, although not the most efficient,
    // it's the easiest way to override its behavior, e.g. accessing a database
    // with caching.

    static const size_t MAX_FROZEN_CLOSURES = static_cast<size_t>(-1);

    inline void internal_set_frozen_closure(size_t index, term closure) {
	heap_watch(index, true);
	trail(index);
	if (is_debug()) {
	    std::cout << "Set frozen closure: " << index << std::endl;
        }
    }

    inline void internal_clear_frozen_closure(size_t index) {
	heap_watch(index, false);
	if (is_debug()) {
	    std::cout << "Clear frozen closure: " << index << std::endl;
	}
    }

    inline size_t internal_num_frozen_closures() const {
	return frozen_closures_.size();
    }

    virtual void set_frozen_closure(size_t index, term closure) {
	internal_set_frozen_closure(index, closure);
        frozen_closures_.insert(std::make_pair(index, closure));
    }

    virtual void clear_frozen_closure(size_t index) {
	internal_clear_frozen_closure(index);
        frozen_closures_.erase(index);
    }

    virtual term get_frozen_closure(size_t index) {
	auto it = frozen_closures_.find(index);
	if (it == frozen_closures_.end()) {
	    return EMPTY_LIST;
	}
	return it->second;
    }

    virtual size_t num_frozen_closures() const {
	return internal_num_frozen_closures();
    }

    virtual void get_frozen_closures(size_t from_addr, size_t to_addr,
				     size_t max_closures,
			  std::vector<std::pair<size_t, term> > &closures)
    {
	bool from_end = from_addr == heap_size();

	if (from_end) {
	    auto it = frozen_closures_.rbegin();
	    auto it_end = frozen_closures_.rend();
	    size_t k = max_closures;
	    while (k > 0 && it != it_end) {
		auto heap_address = it->first;
		if (heap_address < to_addr) {
		    break;
		}
		closures.push_back(*it);
		k--;
		++it;
	    }
	} else {
	    auto it = frozen_closures_.lower_bound(from_addr);
	    auto it_end = frozen_closures_.end();
	    size_t k = max_closures;
	    while (k > 0 && it != it_end) {
		auto heap_address = it->first;
		if (heap_address >= to_addr) {
		    break;
		}
		closures.push_back(*it);
		k--;
		++it;
	    }
	}
    }

    inline void unwind_frozen_closures(size_t a, size_t b) {
        for (size_t i = a; i < b; i++) {
            size_t addr = trail_get(i);
            if (heap_watched(addr)) {
	        clear_frozen_closure(addr);
            }
        }
    }

public:
    inline void clear_all_frozen_closures() {
        std::vector<size_t> addr;
	auto it = frozen_closures_.begin();
        for (; it != frozen_closures_.end(); ++it) {
	    auto k = it->first;
	    addr.push_back(k);
	    heap_watch(k, false);
        }
	frozen_closures_.clear();
    }

private:
    void trim_heap(size_t new_size) = delete;

protected:
    inline void trim_heap_safe(size_t new_size) {
        size_t current_size = heap_size();
	// And we need to remove any pending frozen closures
	std::vector<std::pair<size_t, term> > closures;
	get_frozen_closures(new_size, current_size, MAX_FROZEN_CLOSURES, closures);
	for (auto &p : closures) {
   	     size_t addr = p.first;
	     clear_frozen_closure(addr);
	}
	// Never shrink the heap beyond the limiter.
	// The limiter is set as the "safe point." For example,
	// when program DB is loaded with new clauses, we need to
	// ensure the heap never trims away that code, so every
	// time the program DB is modified we need to bump up the
	// heap limiter. The heap limiter can be trimmed at next
	// next garbage collection cycle.
	// size_t orig_size = new_size;
	if (new_size < heap_limiter_) {
	    new_size = heap_limiter_;
	}
	// std::cout << "trim_heap: new_size=" << new_size << " requested=" << orig_size << " limit=" << heap_limiter_ << std::endl;
        term_env::trim_heap(new_size);
    }

    void check_frozen();
    void check_delayed();

    void heap_limit() {
	heap_limiter_ = heap_size();
    }

    size_t get_heap_limit() const {
	return heap_limiter_;
    }
    
    void heap_limit(size_t heap_sz) {
	heap_limiter_ = heap_sz;
    }

private:
    size_t heap_limiter_;

public:
    struct delayed_t {
	delayed_t(interpreter_base &i, term q, term e, std::function<void()> processed_fn = nullptr)
	    : interp(i), query(q), else_do(e), timeout(std::numeric_limits<size_t>::max()), timeout_at(), result(), result_src(nullptr), ready(false), failed(false), processed_fn(nullptr) { }
	delayed_t(const delayed_t &other) = default;
	interpreter_base &interp;
	term query;
	term else_do;
	size_t timeout;
	common::utime timeout_at;
	term result;
	term_env *result_src;
	bool ready;
	bool failed;
	std::function<void()> processed_fn;
	std::string standard_out;

	bool has_timeout() const {
	    return timeout != std::numeric_limits<size_t>::max();
	}
	
	bool has_expired() const {
	    if (!has_timeout()) {
		return false;
	    }
	    return common::utime::now() > timeout_at;
	}

	void clear_timeout() {
	    timeout = std::numeric_limits<size_t>::max();
        }

	void set_failed() {
	    failed = true;
        }

	void set_timeout_millis(size_t dt) {
	    timeout = dt;
	    timeout_at = common::utime::now() + common::utime::ms(dt);
	}
    };
    
    void add_delayed(delayed_t *dt) {
	delayed_.push_back(dt);
	if (dt->has_timeout()) {
	    boost::lock_guard<common::spinlock> lockit2(delayed_fast_lock_);
	    if (delayed_next_timeout_.is_zero() ||
		dt->timeout_at < delayed_next_timeout_) {
		delayed_next_timeout_ = dt->timeout_at;
	    }
	}
    }
    void delayed_ready(delayed_t *dt) {
	boost::unique_lock<common::spinlock> lockit(delayed_fast_lock_);
	dt->ready = true;
	delayed_ready_++;
    }
    void process_delayed( const std::function<void (const delayed_t &)> &fn ) {
	delayed_next_timeout_.set_zero();
	for (auto it = delayed_.begin(); it != delayed_.end();) {
	    auto *d = *it;
	    if (d->has_expired()) {
		d->set_failed();
		d->clear_timeout();
		fn(*d);
		if (d->processed_fn) d->processed_fn();
	    } else if (d->ready) {
		if (!d->failed) {
		    fn(*d);
		    if (d->processed_fn) d->processed_fn();
		}
		delete d;
		it = delayed_.erase(it);
		boost::lock_guard<common::spinlock> lockit2(delayed_fast_lock_);
		delayed_ready_--;
	    } else {
		if (d->has_timeout()) {
		    if (delayed_next_timeout_.is_zero() ||
			d->timeout_at < delayed_next_timeout_) {
			delayed_next_timeout_ = d->timeout_at;
		    }
		}
		++it;
	    }
	}
    }

private:
    size_t delayed_ready_;
    common::spinlock delayed_fast_lock_;
    common::utime delayed_next_timeout_;
    std::vector<delayed_t *> delayed_;

    std::string name_;
};

inline void predicate::add_clause(interpreter_base &interp, common::term clause0, clause_position pos)  {
    performance_count_++;
    managed_clause clause(clause0, interp.cost(clause0), clauses_.size());
    switch (pos) {
    case FIRST_CLAUSE: clauses_.insert(clauses_.begin(), clause); break;
    case LAST_CLAUSE: clauses_.push_back(clause); break;
    }
    auto first_arg_index = interp.arg_index(interp.clause_first_arg(clause0));

    if (first_arg_index.tag().is_ref()) {
        with_vars_ = true;
	indexed_.clear();
        filtered_.clear();
	term_id_.clear();
    }
    if (!with_vars_) {
        auto &idx = indexed_[first_arg_index];
	if (pos == FIRST_CLAUSE) {
	    idx.insert(idx.begin(), clause);
	} else {
	    idx.push_back(clause);
	}
    }
    clear_cache();
    num_clauses_++;
}

inline bool predicate::matched_indexed_clause(interpreter_base &interp, common::term head) {
    auto arg_index = interp.arg_index(interp.clause_first_arg(head));
    auto &idx = indexed_[arg_index];
    for (auto it = idx.begin(); it != idx.end();) {
        auto idx_clause = (*it).clause();
        auto idx_clause_head = interp.clause_head(idx_clause);
        if (interp.can_unify(idx_clause_head, head)) {
	    return true;
        } else {
            ++it;
        }
    }
    return false;
}

inline managed_clause predicate::remove_indexed_clause(interpreter_base &interp, common::term head)
{
    auto arg_index = interp.arg_index(interp.clause_first_arg(head));
    auto &idx = indexed_[arg_index];
    for (auto it = idx.begin(); it != idx.end();) {
        performance_count_++;
        auto &mclause = (*it);
        auto idx_clause = mclause.clause();
        auto idx_clause_head = interp.clause_head(idx_clause);
        if (interp.can_unify(idx_clause_head, head)) {
            it = idx.erase(it);
	    return mclause;
        } else {
            ++it;
        }
    }
    return managed_clause(common::term(), 0, 0);
}

inline bool predicate::matching_clauses(interpreter_base &interp, common::term head) {
    auto arg = interp.clause_first_arg(head);
    if (with_vars_ || arg.tag().is_ref()) {
	for (auto it = clauses_.begin(); it != clauses_.end();) {
            performance_count_++;
	    auto mclause = *it;
	    if (mclause.is_erased()) {
	        ++it;
	        continue;
	    }
	    auto clause = mclause.clause();
	    auto clause_head = interp.clause_head(clause);
	    if (interp.can_unify(clause_head, head)) {
		return true;
	    } else {
	        ++it;
	    }
	}
	return false;
    } else {
	return matched_indexed_clause(interp, head);
    }
}

inline bool predicate::remove_clauses(interpreter_base &interp, common::term head, bool all)
{
    auto arg = interp.clause_first_arg(head);
    bool found = false;
    if (with_vars_ || arg.tag().is_ref()) {
	for (auto it = clauses_.begin(); it != clauses_.end();) {
            performance_count_++;
	    auto mclause = *it;
	    if (mclause.is_erased()) {
	        ++it;
	        continue;
	    }
	    auto clause = mclause.clause();
	    auto clause_head = interp.clause_head(clause);
	    if (interp.can_unify(clause_head, head)) {
	        if (!found) {
	            found = true;
		    filtered_.clear();
		    term_id_.clear();
		}
		it = clauses_.erase(it);
		remove_indexed_clause(interp, clause_head);
		num_clauses_--;
		if (!all) break;
	    } else {
	        ++it;
	    }
	}
    } else {
        // Update index and remove clause
        bool cont = false;
        do {
    	    auto mclause = remove_indexed_clause(interp, head);
	    cont = all;
	    if (mclause.clause() != common::term()) {
	        found = true;
	        clauses_[mclause.ordinal()].erase();
		num_clauses_--;
	    } else {
	        cont = false;
	    }
	} while (cont);
    }
    if (!found && !all) {
        return false;
    }
    clear_cache();
    return true;
}

inline size_t predicate::get_term_id(interpreter_base &interp, common::term arg_index) const
{
    auto it = term_id_.find(arg_index);
    performance_count_++;
    if (it == term_id_.end()) {
        size_t new_id = filtered_.size();
	term_id_[arg_index] = new_id;
	filtered_.resize(filtered_.size()+1);
	auto &v = filtered_[new_id];
	bool is_unknown = with_vars_ || arg_index.tag().is_ref();
	auto &src_clauses = is_unknown ? clauses_ : indexed_[arg_index];
	for (auto &mclause : src_clauses) {
            performance_count_++;
	    if (mclause.is_erased()) {
	        continue;
	    }
	    auto farg_index = interp.arg_index(interp.clause_first_arg(mclause.clause()));
	    if (!interp.definitely_inequal(farg_index, arg_index)) {
	        v.push_back(mclause);
	    }
	}
	return new_id;
    }
    return it->second;
}

inline const std::vector<managed_clause> & predicate::get_clauses(interpreter_base &interp, size_t term_id) const {
    return filtered_[term_id];
}

inline const std::vector<managed_clause> & predicate::get_clauses(interpreter_base &interp, common::term first_arg) const {
    static const std::vector<managed_clause> NOT_FOUND;
    performance_count_++;
    auto arg_index = interp.arg_index(first_arg);
    size_t term_id = get_term_id(interp, arg_index);
    return filtered_[term_id];
}

template<> inline environment_naive_t * interpreter_base::allocate_environment<ENV_NAIVE>()
{
    auto new_ee = reinterpret_cast<environment_naive_t *>(allocate_stack(true));
    new_ee->ce = save_e();
    new_ee->cp = cp();
    new_ee->b0 = b0();
    new_ee->qr = register_qr_;
    new_ee->pr = register_pr_;
    
    set_ee(new_ee);
    return new_ee;
}

template<> inline environment_t * interpreter_base::allocate_environment<ENV_WAM>()
{
    auto new_e = reinterpret_cast<environment_t *>(allocate_stack(true));
    new_e->ce = save_e();
    new_e->cp = cp();
    set_e(new_e, ENV_WAM);
    return new_e;
}

template<> inline environment_frozen_t * interpreter_base::allocate_environment<ENV_FROZEN>()
{
    auto new_ef = reinterpret_cast<environment_frozen_t *>(allocate_stack(false));
    new_ef->ce = save_e();
    new_ef->cp = cp();
    new_ef->b0 = b0();
    new_ef->qr = register_qr_;
    new_ef->pr = register_pr_;
    new_ef->p = p();
    
    set_e(new_ef, ENV_FROZEN);
    
    save_state_fn_(this);
    
    return new_ef;
}

inline void interpreter_base::check_delayed()
{
    process_delayed( [this](const delayed_t &dt) {
	if (dt.result_src) {
	    term result_copy = copy( dt.result, *dt.result_src);
	    unify(dt.query, result_copy);
	} else {
	    // Remote execution failed, so execute else clause (if present)
	    if (dt.else_do != EMPTY_LIST) {
		allocate_environment<ENV_FROZEN, environment_frozen_t *>();
		allocate_environment<ENV_NAIVE, environment_naive_t *>();
		set_cp(code_point(EMPTY_LIST));
		set_p(code_point(dt.else_do));
		set_qr(dt.else_do);
	    }
	}
    });
}

inline void interpreter_base::check_frozen()
{
    bool do_check_delayed = false;
    {
	boost::lock_guard<common::spinlock> lockit(delayed_fast_lock_);
	if (delayed_ready_) {
	    do_check_delayed = true;
	} else if (!delayed_next_timeout_.is_zero() &&
		 common::utime::now() > delayed_next_timeout_) {
	    do_check_delayed = true;
	}
    }
    if (do_check_delayed) check_delayed();
    auto &watched = heap_watched();
    size_t n = watched.size();
    if (n == 0) {
        return;
    }
    // Go in reverse order so the first watched is pushed last.
    if (is_debug()) {
        std::cout << "Check frozen closures (n=" << n << ")" << std::endl;
    }
    for (size_t i = 0; i < n; i++) {
	auto addr = watched[n-i-1];
	auto h = heap_get(addr);
	auto cl = get_frozen_closure(addr);
	if (!h.tag().is_ref()) {
	    clear_frozen_closure(addr);
	    if (cl != EMPTY_LIST) {
		allocate_environment<ENV_FROZEN, environment_frozen_t *>();
		allocate_environment<ENV_NAIVE, environment_naive_t *>();
		set_cp(code_point(EMPTY_LIST));
		set_p(code_point(cl));
		set_qr(cl);
	    }
	    heap_watch(addr, false);
	} else {
	    // This could be a ref-ref binding where newer binds to older
	    // Move index for frozen closure to the oldest one.
	    term h2 = deref(h);
	    if (h2.tag().is_ref()) {
	        auto new_addr = reinterpret_cast<common::ref_cell &>(h2).index();
		if (new_addr != addr) {
	  	    if (is_debug()) {
		      std::cout << "Move frozen closure: from=" << addr << " to=" << new_addr << std::endl;
		    }
		    set_frozen_closure(new_addr, cl);
		    clear_frozen_closure(addr);
		    heap_watch(addr, false);
		}
	    }
	}
    }
    heap_clear_watched();
}

}}

#endif
