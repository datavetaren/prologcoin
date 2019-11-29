#pragma once

#ifndef _common_term_env_hpp
#define _common_term_env_hpp

#include <iterator>
#include <map>
#include <stack>
#include "term.hpp"
#include "term_emitter.hpp"
#include "term_parser.hpp"
#include "term_tokenizer.hpp"

namespace prologcoin { namespace common {

//
// This is the environment that terms need when dealing with
// some basic operations.  It consists of a heap, a constant table
// (if constant names are bigger than 6 characters,) a stack and a trail.
//
// One of the core operations is unificiation. Variables may become
// bound during the unification process and thus need to be recorded so
// they can become unbound.
//
class term_exception_not_list : public std::runtime_error {
public:
    inline term_exception_not_list(term t)
       : std::runtime_error("term is not a list"), term_(t) { }
    inline ~term_exception_not_list() noexcept(true) { }

    inline term get_term() { return term_; }

private:
    term term_;
};

template<typename HT, typename ST, typename OT> class term_env_dock;

template<typename HT, typename ST, typename OT>
class term_iterator_templ : public std::iterator<std::forward_iterator_tag,
						 term,
						 term,
						 const term *,
						 const term &> {
public:
    typedef term_env_dock<HT,ST,OT> Env;
    inline term_iterator_templ(Env &env, const term t)
	 : env_(env), current_(t) { elem_ = first_of(t); }

    inline bool operator == (const term_iterator_templ &other) const;
    inline bool operator != (const term_iterator_templ &other) const
        { return ! operator == (other); }

    inline term_iterator_templ & operator ++ ()
        { term_iterator_templ &it = *this; it.advance(); return it; }
    inline reference operator * () const { return elem_; }
    inline pointer operator -> () const { return &elem_; }

    inline Env & env() { return env_; }

private:
    inline term first_of(const term t);
    void advance();

    Env &env_;
    term current_;
    term elem_;
};

template<typename HT, typename ST, typename OT>
class term_dfs_iterator_templ : public std::iterator<std::forward_iterator_tag,
						     term,
						     term,
						     const term *,
						     const term &> {
public:
    typedef term_env_dock<HT,ST,OT> Env;

    inline term_dfs_iterator_templ(Env &env, const term t)
           : env_(env) { elem_ = first_of(t); }

    inline term_dfs_iterator_templ(Env &env)
	   : env_(env) { }

    inline bool operator == (const term_dfs_iterator_templ &other) const;
    inline bool operator != (const term_dfs_iterator_templ &other) const
        { return ! operator == (other); }

    inline term_dfs_iterator_templ & operator ++ ()
        { term_dfs_iterator_templ &it = *this;
	  it.advance(); return it; }
    inline reference operator * () const { return elem_; }
    inline pointer operator -> () const { return &elem_; }
    inline size_t depth() const { return stack_.size(); }

    inline Env & env() { return env_; }

private:
    inline term first_of(const term t);
    void advance();

    Env &env_;
    term elem_;

    struct dfs_pos {
	dfs_pos(const term &p, size_t i, size_t n) :
	    parent(p), index(i), arity(n) { }

	term parent;
	size_t index;
	size_t arity;
    };

    std::vector<dfs_pos> stack_;
};

template<typename HT, typename ST, typename OT>
class const_term_dfs_iterator_templ : public term_dfs_iterator_templ<HT,ST,OT>
{
public:
    typedef term_env_dock<HT,ST,OT> Env;

    inline const_term_dfs_iterator_templ(const Env &env, const term t)
  	   : term_dfs_iterator_templ<HT,ST,OT>(const_cast<Env &>(env), t) { }

    inline const_term_dfs_iterator_templ(const Env &env)
  	   : term_dfs_iterator_templ<HT,ST,OT>(const_cast<Env &>(env)) { }

    inline bool operator == (const const_term_dfs_iterator_templ &other) const;
    inline bool operator != (const const_term_dfs_iterator_templ &other) const
        { return ! operator == (other); }

    inline const_term_dfs_iterator_templ & operator ++ ()
        { return static_cast<const_term_dfs_iterator_templ<HT,ST,OT> &>(
		   term_dfs_iterator_templ<HT,ST,OT>::operator ++ ()); }

    inline const Env & env() const { return term_dfs_iterator_templ<HT,ST,OT>::env(); }
};

template<typename T> class heap_dock : public T {
public:
    inline heap_dock() { }

    // Heap management
    inline void heap_set(size_t index, term t)
        { T::get_heap()[index] = t; }
    inline term heap_get(size_t index)
        { return T::get_heap()[index]; }
    inline untagged_cell heap_get_untagged(size_t index)
        { return T::get_heap().untagged_at(index); }

    // Term management
    inline term new_ref()
        { return T::get_heap().new_ref(); }
    inline void new_ref(size_t cnt)
        { T::get_heap().new_ref(cnt); }
    inline term deref(const term t) const
        { return T::get_heap().deref(t); }
    inline term deref_with_cost(const term t, uint64_t &cost) const
        { return T::get_heap().deref_with_cost(t, cost); }
    inline con_cell functor(const term t) const
        { return T::get_heap().functor(t); }
    inline term arg(const term t, size_t index) const
        { return T::get_heap().arg(t, index); }
    inline void set_arg(term t, size_t index, const term arg)
        { return T::get_heap().set_arg(t, index, arg); }
    inline untagged_cell get_big(term t, size_t index) const
        { return T::get_heap().get_big(t, index); }
    inline size_t num_bits(big_cell b) const
        { return T::get_heap().num_bits(b); }
    inline bool big_equal(term t1, term t2, uint64_t &cost) const
        { return T::get_heap().big_equal(static_cast<big_cell &>(t1),
					 static_cast<big_cell &>(t2),
					 cost);
	}
    inline void get_big(term t, boost::multiprecision::cpp_int &i,
			size_t &nbits) const
        { T::get_heap().get_big(t, i, nbits); }
    inline void get_big(term t, uint8_t *bytes, size_t n) const
        { T::get_heap().get_big(t, bytes, n); }
    inline void set_big(term t, size_t index, const untagged_cell cell)
        {  T::get_heap().set_big(t, index, cell); }
    inline void set_big(term t, const boost::multiprecision::cpp_int &i)
        { T::get_heap().set_big(t, i); }
    inline void set_big(term t, const uint8_t *bytes, size_t n)
        { T::get_heap().set_big(t, bytes, n); }
    inline void trim_heap(size_t new_size)
        { return T::get_heap().trim(new_size); }
    inline size_t heap_size() const
        { return T::get_heap().size(); }

    // Term creation
    inline con_cell functor(const std::string &name, size_t arity)
        { return T::get_heap().functor(name, arity); }
    inline term new_term(con_cell functor)
        { return T::get_heap().new_str(functor); }
    inline term new_term_con(con_cell functor)
        { return T::get_heap().new_con0(functor); }
    inline term new_term_str(con_cell functor)
        { return T::get_heap().new_str0(functor); }
    inline term new_big(size_t nbits)
        { return T::get_heap().new_big(nbits); }

    inline void new_term_copy_cell(term t)
        { T::get_heap().new_cell0(t); }
    inline term new_term(con_cell functor, const std::vector<term> &args)
        { term t = new_term(functor);
          size_t i = 0;
	  for (auto arg : args) {
	      T::get_heap().set_arg(t, i, arg);
	      i++;
	  }
	  return t;
        }
    inline term new_term(con_cell functor, std::initializer_list<term> args)
        { term t = new_term(functor);
          size_t i = 0;
	  for (auto arg : args) {
	      T::get_heap().set_arg(t, i, arg);
	      i++;
	  }
	  return t;
	}
    inline term new_dotted_pair()
        { return T::get_heap().new_dotted_pair(); }
    inline term new_dotted_pair(term a, term b)
        { return T::get_heap().new_dotted_pair(a,b); }
    inline con_cell to_atom(con_cell functor)
        { return T::get_heap().to_atom(functor); }
    inline con_cell to_functor(con_cell atom, size_t arity)
        { return T::get_heap().to_functor(atom, arity); }

    // Term functions
    inline size_t list_length(term lst)
        { return T::get_heap().list_length(lst); }
    inline std::string atom_name(con_cell f) const
        { return T::get_heap().atom_name(f); }

    // Term predicates
    inline bool is_dotted_pair(term t) const
       { return T::get_heap().is_dotted_pair(t); }
    inline bool is_empty_list(term t) const
       { return T::get_heap().is_empty_list(t); }
    inline bool is_comma(term t) const
       { return T::get_heap().is_comma(t); }
    inline bool is_list(term t) const
       { return T::get_heap().is_list(t); }

    // Watch addresses
    inline void heap_watch(size_t addr, bool b)
       { T::get_heap().watch(addr, b); }
    inline const std::vector<size_t> & heap_watched() const
       { return T::get_heap().watched(); }
    inline bool heap_watched(size_t addr) const
       { return T::get_heap().watched(addr); }  
    inline void heap_clear_watched()
       { T::get_heap().clear_watched(); }

    // Disable coin security
    inline typename heap::disabled_coin_security disable_coin_security()
       { return T::get_heap().disable_coin_security(); }
};

class heap_bridge
{
public:
    inline heap_bridge() : heap_(nullptr) { }

    inline heap & get_heap() { return *heap_; }
    inline const heap & get_heap() const { return *heap_; }
    inline void set_heap(heap &h) { heap_ = &h; }
private:
    heap * heap_;
};

class heap_proxy : public heap_dock<heap_bridge> 
{
public:
    inline heap_proxy(heap &h) { set_heap(h); }
};

class stacks {
public:
    inline stacks & get_stacks() { return *this; }
    inline const stacks & get_stacks() const { return *this; }
    inline std::vector<term> & get_stack() { return stack_; }
    inline const std::vector<term> & get_stack() const { return stack_; }
    inline std::vector<size_t> & get_trail() { return trail_; }
    inline const std::vector<size_t> & get_trail() const { return trail_; }
    inline std::vector<term> & get_temp() { return temp_; }
    inline const std::vector<term> & get_temp() const { return temp_; }
    inline size_t get_register_hb() const { return register_hb_; }
    inline void set_register_hb(size_t hb) { register_hb_ = hb; }

private:
    std::vector<term> stack_;
    std::vector<size_t> trail_;
    std::vector<term> temp_;
    size_t register_hb_;
};

template<typename T> class stacks_dock : public T {
public:
  inline stacks_dock() { }

  inline stacks & get_stacks()
      { return T::get_stacks(); }

  inline size_t allocate_stack(size_t num_cells)
      { size_t at_index = T::get_stack().size();
	T::get_stack().resize(at_index+num_cells);
	return at_index;
      }
  inline void ensure_stack(size_t at_index, size_t num_cells)
      { if (at_index + num_cells > T::get_stack().size()) {
	    allocate_stack(at_index+num_cells-T::get_stack().size());
	}
      }
  inline term * stack_ref(size_t at_index)
      { return &T::get_stack()[at_index]; }

  inline void push(const term t)
      { T::get_stack().push_back(t); }
  inline term pop()
      { term t = T::get_stack().back(); T::get_stack().pop_back(); return t; }
  inline size_t stack_size() const
      { return T::get_stack().size(); }
  inline void trim_stack(size_t new_size)
      { T::get_stack().resize(new_size); }

  inline void push_trail(size_t i)
      { T::get_trail().push_back(i); }
  inline size_t pop_trail()
      { auto p = T::get_trail().back(); T::get_trail().pop_back(); return p; }
  inline size_t trail_size() const
      { return T::get_trail().size(); }
  inline void trim_trail(size_t to)
      { T::get_trail().resize(to); }
  inline size_t trail_get(size_t i) const
      { return T::get_trail()[i]; }
  inline void trail_set(size_t i, size_t val)
      { T::get_trail()[i] = val; }
  inline void trail(size_t index)
      { if (index < T::get_register_hb()) {
	    push_trail(index);
	}
      }

  inline void temp_push(const term t)
      { T::get_temp().push_back(t); }
  inline term temp_pop()
      { auto t = T::get_temp().back(); T::get_temp().pop_back(); return t; }
};

class stacks_bridge
{
public:
    inline stacks_bridge() : stacks_(nullptr) { }

    inline stacks & get_stacks() { return *stacks_; }
    inline const stacks & get_stacks() const { return *stacks_; }
    inline void set_stacks(stacks &s) { stacks_ = &s; }

    inline std::vector<term> & get_stack() { return get_stacks().get_stack(); }
    inline const std::vector<term> & get_stack() const { return get_stacks().get_stack(); }
    inline std::vector<size_t> & get_trail() { return get_stacks().get_trail(); }
    inline const std::vector<size_t> & get_trail() const { return get_stacks().get_trail(); }

    inline std::vector<term> & get_temp() { return get_stacks().get_temp(); }
    inline const std::vector<term> & get_temp() const { return get_stacks().get_temp(); }

    inline size_t get_register_hb() { return get_stacks().get_register_hb(); }
    inline void set_register_hb(size_t index) { get_stacks().set_register_hb(index); }

private:
    stacks *stacks_;
};

class stacks_proxy : public stacks_dock<stacks_bridge> 
{
public:
    inline stacks_proxy(stacks &s) { set_stacks(s); }
};

template<typename T> class ops_dock : public T
{
public:
    ops_dock() { }
    ops_dock(T &t) : T(t) { }
};

class ops_bridge
{
public:
    inline ops_bridge(term_ops &ops) : ops_(&ops) { }

    inline term_ops & get_ops() { return *ops_; }
    inline const term_ops & get_ops() const { return *ops_; }

    inline void set_ops(term_ops &ops) { ops_ = &ops; }

private:
    term_ops *ops_;
};

class ops_proxy : public ops_dock<ops_bridge> 
{
public:
    inline ops_proxy(term_ops &ops)
        : ops_dock<ops_bridge>(*this) { set_ops(ops); }
};

typedef std::unordered_map<term, std::string> naming_map;

class term_utils : public heap_proxy, stacks_proxy, ops_proxy {
public:
    term_utils(heap &h, stacks &s, term_ops &o) : heap_proxy(h), stacks_proxy(s), ops_proxy(o) { }

    bool unify(term a, term b, uint64_t &cost);
    term copy(const term t, naming_map &names, uint64_t &cost);
    term copy(const term t, naming_map &names,
	      heap &src, naming_map &src_names, uint64_t &cost);
    bool equal(term a, term b, uint64_t &cost);
    uint64_t hash(term t);
    uint64_t cost(term t);

    // Return -1, 0 or 1 when comparing standard order for 'a' and 'b'
    int standard_order(const term a, const term b, uint64_t &cost);

    std::string list_to_string(const term t, heap &src);
    term string_to_list(const std::string &str);
    bool is_string(const term t, heap &src);

    std::vector<std::string> get_expected(const term_parse_exception &ex);
    std::vector<std::string> get_expected_simplified(const term_parse_exception &ex);

    std::vector<std::string> to_error_messages(const token_exception &ex);
    std::vector<std::string> to_error_messages(const term_parse_exception &ex);

    void error_excerpt(const std::string &line, size_t column,
		       std::vector<std::string> &msgs);
    

private:
    void restore_cells_after_unify(std::stack<size_t> &visited);
    bool unify_helper(term a, term b, uint64_t &cost);
    int functor_standard_order(con_cell a, con_cell b);

    inline void bind(const ref_cell &a, term b)
    {
        size_t index = a.index();
        heap_set(index, b);
        trail(index);
    }

    inline void unwind_trail(size_t from, size_t to)
    {
        for (size_t i = from; i < to; i++) {
  	    size_t index = trail_get(i);
            heap_set(index, ref_cell(index));
        }
    }
};

template<typename HT, typename ST, typename OT> class term_env_dock
  : public heap_dock<HT>, public stacks_dock<ST>, public ops_dock<OT>
{
public:
  inline term_env_dock() { }

  inline bool is_atom(const term t) const
      { return is_functor(t) && heap_dock<HT>::functor(t).arity() == 0; }

  inline std::string atom_name(const term t) const
      { return heap_dock<HT>::atom_name(heap_dock<HT>::functor(t)); }

  inline bool is_functor(const term t) const
      { auto tt = heap_dock<HT>::deref(t).tag(); return tt == tag_t::CON || tt == tag_t::STR; }
	 
  inline bool is_functor(const term t, con_cell f)
     { return is_functor(t) && heap_dock<HT>::functor(t) == f; }

  inline bool is_ground(const term t) const
     {
        auto range = const_cast<term_env_dock<HT,ST,OT> &>(*this).iterate_over(t);
        for (auto t1 : range) {
   	    if (t1.tag() == tag_t::REF) {
	        return false;
	    }
        }
        return true;    
     }

  inline naming_map & var_naming()
     { return var_naming_; }
  inline void clear_name(const term t)
     { var_naming_.erase(t); }
  inline bool has_name(const term t) const
     { return var_naming_.find(t) != var_naming_.end(); }
  inline void set_name(const term t, const std::string &name)
     { var_naming_[t] = name; }
  inline const std::string & get_name(const term t) const
     { auto it = var_naming_.find(t);
       if (it == var_naming_.end()) {
	   static const std::string empty;
	   return empty;
       } else {
	   return it->second;
       }
     }

  inline void unwind_trail(size_t from, size_t to)
  {
      for (size_t i = from; i < to; i++) {
	  size_t index = stacks_dock<ST>::trail_get(i);
          heap_dock<HT>::heap_set(index, ref_cell(index));
      }
  }

  inline void tidy_trail(size_t from, size_t to)
  {
      size_t i = from;
      while (i < to) {
   	  if (stacks_dock<ST>::trail_get(i) < stacks_dock<ST>::get_register_hb()) {
   	      // This variable recording happened before the choice point.
	      // We can't touch it.
	      i++;
          } else {
	      // Remove this trail point, move one trail point we haven't
	      // visited to this location.
  	      stacks_dock<ST>::trail_set(i, stacks_dock<ST>::trail_get(to-1));
	      to--;
          }
      }

      // We're done. Trim the trail to the new end
      stacks_dock<ST>::trim_trail(to);
  }

  inline bool unify(term a, term b, uint64_t &cost)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.unify(a, b, cost);
  }

  inline term copy(term t, uint64_t &cost)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.copy(t, var_naming(), heap_dock<HT>::get_heap(),
			var_naming(), cost);
  }

  inline term copy(term t, term_env_dock<HT,ST,OT> &src, uint64_t &cost)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.copy(t, var_naming(), src.get_heap(), src.var_naming(), cost);
  }

  inline std::string list_to_string(term t, term_env_dock<HT,ST,OT> &src)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.list_to_string(t, src.get_heap());
  }

  inline std::string list_to_string(term t)
  {
      return list_to_string(t, *this);
  }

  inline term string_to_list(const std::string &str)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.string_to_list(str);
  }

  inline bool is_string(term t)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.is_string(t, *this);
  }

  inline uint64_t hash(term t)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.hash(t);
  }

  inline uint64_t cost(const term t)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.cost(t);
  }

  inline bool equal(const term a, const term b, uint64_t &cost)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.equal(a, b, cost);
  }

  inline void bind(const ref_cell &a, term b)
  {
      size_t index = a.index();
      heap_dock<HT>::heap_set(index, b);
      stacks_dock<ST>::trail(index);
  }

  inline int standard_order(const term a, const term b, uint64_t &cost)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.standard_order(a, b, cost);
  }

  class term_range {
  public:
      inline term_range(term_env_dock &env, const term t) : env_(env), term_(t) { }

      inline term_dfs_iterator_templ<HT,ST,OT> begin() { return env_.begin(term_); }
      inline term_dfs_iterator_templ<HT,ST,OT> end() { return env_.end(term_); }
  private:
      term_env_dock &env_;
      term term_;
  };

  class const_term_range {
  public:
      inline const_term_range(const term_env_dock &env, const term t)
           : env_(env), term_(t) { }

      inline const_term_dfs_iterator_templ<HT,ST,OT> begin() const
      { return env_.begin(term_); }
      inline const_term_dfs_iterator_templ<HT,ST,OT> end() const
      { return env_.end(term_); }
  private:
      const term_env_dock &env_;
      term term_;
  };

  inline term_range iterate_over(const term t)
      { return term_range(*this, t); }

  inline const_term_range iterate_over(const term t) const
      { return const_term_range(*this, t); }

  inline std::string status() const
  { 
    std::stringstream ss;
    ss << "term_env::status() { heap_size=" << heap_dock<HT>::heap_size()
       << ",stack_size=" << stacks_dock<ST>::stack_size() << ",trail_size="
       << stacks_dock<ST>::trail_size() <<"}";
    return ss.str();
  }

  inline term parse(std::istream &in)
  {
      term_tokenizer tokenizer(in);
      term_parser parser(tokenizer, heap_dock<HT>::get_heap(),
			 ops_dock<OT>::get_ops());
      term r = parser.parse();

      // Once parsing is done we'll copy over the var-name bindings
      // so we can pretty print the variable names.
      parser.for_each_var_name( [this](const term ref,
				    const std::string &name)
			     { this->var_naming_[ref] = name; } );
      return r;
  }
 
  inline term parse(const std::string &str)
  {
      std::stringstream ss(str);
      return parse(ss);
  }

  inline std::string to_string(const term t) const
  {
      emitter_options default_opt;
      return to_string(t, default_opt);
  }

  inline std::string to_string(const term t,const emitter_options &opt) const
  {
      term t1 = heap_dock<HT>::deref(t);
      std::stringstream ss;
      term_emitter emitter(ss, heap_dock<HT>::get_heap(),
			   ops_dock<OT>::get_ops());
      emitter.set_options(opt);
      emitter.set_var_naming(var_naming_);
      emitter.print(t1);
      return ss.str();
  }

  inline std::string safe_to_string(const term t, const emitter_options &opt) const
  {
      return to_string(t, opt);
  }

  inline std::string safe_to_string(const term t) const
  {
      emitter_options default_opt;
      return to_string(t, default_opt);
  }

  const_big_iterator begin(const big_cell b)
  {
      return heap_dock<HT>::begin(b);
  }

  const_big_iterator end(const big_cell b)
  {
      return heap_dock<HT>::end(b);
  }  

  term_dfs_iterator_templ<HT,ST,OT> begin(const term t)
  {
      return term_dfs_iterator_templ<HT,ST,OT>(*this, t);
  }
  
  term_dfs_iterator_templ<HT,ST,OT> end(const term t)
  {
      return term_dfs_iterator_templ<HT,ST,OT>(*this);
  }

  const_term_dfs_iterator_templ<HT,ST,OT> begin(const term t) const
  {
      return const_term_dfs_iterator_templ<HT,ST,OT>(*this, t);
  }
  
  const_term_dfs_iterator_templ<HT,ST,OT> end(const term t) const
  {
      return const_term_dfs_iterator_templ<HT,ST,OT>(*this);
  }

  struct state_context {
      state_context(size_t tr, size_t st, size_t hb) :
	  trail_(tr), stack_(st), hb_(hb) { }
      size_t trail_;
      size_t stack_;
      size_t hb_;
  };

  state_context capture_state() {
      return state_context(stacks_dock<ST>::trail_size(), stacks_dock<ST>::stack_size(), stacks_dock<ST>::get_register_hb());
  }

  void restore_state(state_context &context) {
      unwind_trail(context.trail_, stacks_dock<ST>::trail_size());
      stacks_dock<ST>::trim_trail(context.trail_);
      stacks_dock<ST>::trim_stack(context.stack_);
      stacks_dock<ST>::set_register_hb(context.hb_);
  }

  std::vector<std::pair<std::string, term> > find_vars(const term t0) {
      std::vector<std::pair<std::string, term> > vars;
      std::unordered_set<term> seen;
      // Record all vars for this query
      std::for_each( begin(t0),
		     end(t0),
		     [this,&seen,&vars](const term t) {
			 if (t.tag() == tag_t::REF) {
			     const std::string name = this->to_string(t);
			     if (!seen.count(t)) {
				 vars.push_back(std::make_pair(name,t));
				 seen.insert(t);
			     }
			 }
		     } );
      return vars;
  }

  std::vector<term> prettify_var_names(const term t0)
  {
      std::vector<term> touched;

      std::map<term, size_t> count_occurrences;
      std::for_each(begin(t0),
		  end(t0),
		  [this,&count_occurrences] (const term t) {
		    if (t.tag() == tag_t::REF) {
			if (!this->has_name(t)) {
			    ++count_occurrences[t];
			}
		    }
		  }
		  );

      // Those vars with a singleton occurrence will be named
      // '_'.
      size_t named_var_count = 0;
      for (auto v : count_occurrences) {
	  touched.push_back(v.first);
	  if (v.second == 1) {
	      set_name(v.first, "_");
	  } else { // v.second > 1
	      std::string name = "G_" + boost::lexical_cast<std::string>(
						 named_var_count);
	      named_var_count++;
	      set_name(v.first, name);
	  }
      }

      return touched;
  }

  inline std::vector<std::string> get_expected(const term_parse_exception &ex)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.get_expected(ex);
  }

  inline std::vector<std::string> get_expected_simplified(const term_parse_exception &ex)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.get_expected_simplified(ex);
  }

  inline std::vector<std::string> to_error_messages(const token_exception &ex)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.to_error_messages(ex);
  }

  inline std::vector<std::string> to_error_messages(const term_parse_exception &ex)
  {
      term_utils utils(heap_dock<HT>::get_heap(), stacks_dock<ST>::get_stacks(), ops_dock<OT>::get_ops());
      return utils.to_error_messages(ex);
  }

  inline std::string to_string_debug(const term t) const
  {
      return to_string(t);
  }

private:
  std::unordered_map<term, std::string> var_naming_;
};

template<typename HT, typename ST, typename OT>
inline bool term_iterator_templ<HT,ST,OT>::operator == (const term_iterator_templ<HT,ST,OT> &other) const
{
    uint64_t cost = 0;
    return env_.equal(current_, other.current_, cost);
}

template<typename HT, typename ST, typename OT>
inline term term_iterator_templ<HT,ST,OT>::first_of(const term t)
{
    if (env_.is_empty_list(t)) {
	return t;
    } else {
	if (!env_.is_dotted_pair(t)) {
	    throw term_exception_not_list(t);
	}
	return env_.arg(t, 0);
    }
}

template<typename HT, typename ST, typename OT>
inline void term_iterator_templ<HT,ST,OT>::advance()
{
    term t = env_.deref(current_);
    if (t.tag() != common::tag_t::STR) {
	throw term_exception_not_list(current_);
    }
    current_ = env_.arg(current_, 1);
    elem_ = first_of(current_);
}

template<typename HT, typename ST, typename OT>
inline bool term_dfs_iterator_templ<HT,ST,OT>::operator == (const term_dfs_iterator_templ<HT,ST,OT> &other) const
{
    return elem_ == other.elem_ && stack_.size() == other.stack_.size();
}

template<typename HT, typename ST, typename OT>
inline bool const_term_dfs_iterator_templ<HT,ST,OT>::operator == (const const_term_dfs_iterator_templ<HT,ST,OT> &other) const
{
    return term_dfs_iterator_templ<HT,ST,OT>::operator == (other);
}

template<typename HT, typename ST, typename OT>
inline term term_dfs_iterator_templ<HT,ST,OT>::first_of(const term t)
{
    term p = env_.deref(t);
    while (p.tag() == tag_t::STR) {
	con_cell f = env_.functor(p);
	size_t arity = f.arity();
	if (arity > 0) {
	    stack_.push_back(dfs_pos(p, 0, arity));
	    p = env_.arg(p, 0);
	} else {
	    return p;
	}
    }
    return p;
}

template<typename HT, typename ST, typename OT>
inline void term_dfs_iterator_templ<HT,ST,OT>::advance()
{
    if (stack_.empty()) {
	elem_ = term();
	return;
    }

    size_t new_index = ++stack_.back().index;
    if (new_index == stack_.back().arity) {
	elem_ = stack_.back().parent;
	stack_.pop_back();
	return;
    }
    elem_ = first_of(env_.arg(stack_.back().parent, new_index));
}

typedef term_iterator_templ<heap, stacks, term_ops> term_iterator;

class term_env : public term_env_dock<heap, stacks, term_ops>
{
public:
     term_env() : term_env_dock() { }
};

//
// This is handy for having unordered sets with equal terms.
// (Good for common sub-term recognition.) We also need this for
// good register allocation in the WAM compiler.
//
class eq_term {
public:
    inline eq_term(const eq_term &other)
        : env_(other.env_), term_(other.term_) { }
    inline eq_term(term_env &env, term t) : env_(env), term_(t) { }

    inline bool operator == (const eq_term &other) const {
        uint64_t cost = 0;
	bool eq = env_.equal(term_, other.term_, cost);
	return eq;
    }

    inline uint64_t hash() const {
        return env_.hash(term_);
    }

private:
    term_env &env_;
    term term_;
};

}}

namespace std {
    template<> struct hash<prologcoin::common::eq_term> {
        size_t operator()(const prologcoin::common::eq_term& eqt) const {
 	    return eqt.hash();
	}
    };
}

#endif
