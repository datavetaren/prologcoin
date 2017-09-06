#include "term_env.hpp"
#include "term.hpp"
#include "term_ops.hpp"
#include "term_tokenizer.hpp"
#include "term_parser.hpp"
#include "term_emitter.hpp"

namespace prologcoin { namespace common {

class term_env_impl {
public:

  term_env_impl()
  {
    heap_ = new heap();
    heap_owner_ = true;
    ops_ = new term_ops();
    ops_owner_ = true;
    register_hb_ = 0;
    register_h_ = 0;
  }

  ~term_env_impl()
  {
    var_naming_.clear();
    if (heap_owner_) {
      delete heap_;
    }
    if (ops_owner_) {
      delete ops_;
    }
  }

  void sync_with_heap() { register_h_ = heap_->size(); }

  heap & get_heap() { return *heap_; }
  term_ops & get_ops() { return *ops_; }

  term parse(const std::string &term_expr);
  std::string to_string(const cell &c, term_emitter::style style = term_emitter::STYLE_TERM) const;
  std::string safe_to_string(const cell &c, term_emitter::style style = term_emitter::STYLE_TERM) const;
  std::string status() const;

  term empty_list() const { return term(*heap_, heap_->empty_list()); }

  inline size_t heap_size() const { return register_h_; }
  inline cell deref(cell c) const { return heap_->deref(c); }

  bool is_list(cell t) const;
  bool is_dotted_pair(cell t) const;
  bool is_empty_list(cell t) const;
  bool is_comma(cell t) const;
  bool is_string(cell t) const;
  std::string get_string(cell t) const;

  bool equal(cell a, cell b) const;
  int  functor_standard_order(con_cell a, con_cell b) const;
  int  standard_order(cell a, cell b) const;
  bool unify(cell a, cell b);
  bool unify_helper(cell a, cell b);
  void bind(cell a, cell b);
  bool direction();
  cell copy(cell c);
  void set_last_choice_heap(size_t at_index);

  inline std::string atom_name(con_cell f)  const { return heap_->atom_name(f); }
  inline std::string atom_name(cell c)  const { return atom_name(functor(c)); }
  inline bool is_atom(cell c) const { return is_functor(c) && functor(c).arity() == 0; }

  inline con_cell functor(const std::string &name, size_t arity) { return heap_->functor(name, arity); }
  inline con_cell functor(cell c) const { return heap_->functor(c); }
  inline bool is_functor(cell c) const
  {
      c = deref(c);
      return c.tag() == tag_t::CON || c.tag() == tag_t::STR;
  }
  term new_term(con_cell functor, const std::initializer_list<term> &args);
  term new_term(con_cell functor);
  term new_dotted_pair(term &a, term &b);
  con_cell to_atom(con_cell functor);
  con_cell to_functor(con_cell atom, size_t arity);
  size_t list_length(const term &lst) const;
   //  inline cell arg(cell c, size_t index) const { return heap_->arg0(c, index); }
  inline term arg(term t, size_t index) const { return heap_->arg(t, index); }
  inline void set_arg(term &t, size_t index, term &arg) { heap_->set_arg(t, index, arg); }
  inline size_t allocate_stack(size_t num_cells) { size_t at_index = stack_.size(); stack_.resize(at_index+num_cells); return at_index; }
  inline void ensure_stack(size_t at_index, size_t num_cells) { if (at_index + num_cells > stack_.size()) allocate_stack(at_index+num_cells-stack_.size()); }
  inline cell * stack_ref(size_t at_index) { return &stack_[at_index]; }
  inline void push(cell c) const { stack_.push_back(c); }
  inline cell pop() const { cell c = stack_.back(); stack_.pop_back(); return c; }
  inline void temp_push(cell c) { temp_.push_back(c); }
  inline cell temp_pop() { cell c = temp_.back(); temp_.pop_back(); return c; }
  inline size_t temp_depth() { return temp_.size(); }
  inline void temp_reset() { temp_.clear(); }

  inline size_t stack_depth() const { return stack_.size(); }
  inline void trim_stack(size_t depth) const { stack_.resize(depth); }

  inline void trim_heap(size_t new_size) { heap_->trim(new_size); register_h_ = new_size; }

  inline void trail(size_t index) {
    if (index < register_hb_) {
      // Only record variable bindings that happen before
      // the latest choice point. Otherwise we just trim the
      // heap and wr're done
      push_trail(index);
    }
  }

  inline void push_trail(size_t i) { trail_.push_back(i); }
  inline size_t pop_trail() {auto p=trail_.back(); trail_.pop_back(); return p;}
  inline size_t trail_depth() const { return trail_.size(); }
  void unwind_trail(size_t from, size_t to);
  void trim_trail(size_t to) { trail_.resize(to); }
  void tidy_trail(size_t from, size_t to);

  inline void clear_name(const term &ref) { var_naming_.erase(ref); }
  inline bool has_name(const term &ref) const { return var_naming_.count(ref) > 0; }
  inline void set_name(const term &ref, const std::string &name) { var_naming_[ref] = name; }

  inline term to_term(cell c) { return term(*heap_, c); }
  
private:
  heap *heap_;
  bool heap_owner_;
  term_ops *ops_;
  bool ops_owner_;

  size_t register_hb_; // Heap size at last choice point
  size_t register_h_;  // Current heap size

  mutable std::vector<cell> stack_;
  std::vector<cell> temp_;
  std::vector<size_t> trail_;
  std::unordered_map<term, std::string> var_naming_;
};

term term_env_impl::parse(const std::string &term_expr)
{
    std::stringstream ss(term_expr);
    term_tokenizer tokenizer(ss);
    term_parser parser(tokenizer, *heap_, *ops_);
    term r = parser.parse();

    // Once parsing is done we'll copy over the var-name bindings
    // so we can pretty print the variable names.
    parser.for_each_var_name( [&](const term  &ref,
				  const std::string &name)
			      { var_naming_[ref] = name; } );
    register_h_ = heap_->size();
    return r;
}

std::string term_env_impl::to_string(const cell &c, term_emitter::style style) const
{
    cell dc = deref(c);
    std::stringstream ss;
    term_emitter emitter(ss, *heap_, *ops_);
    emitter.set_style(style);
    emitter.set_var_naming(var_naming_);
    emitter.print(dc);
    return ss.str();
}

std::string term_env_impl::safe_to_string(const cell &c, term_emitter::style style) const
{
    return to_string(c, style); // TODO: Change this to handle cyclic structures, etc.
}

std::string term_env_impl::status() const
{
    std::stringstream ss;
    ss << "term_env::status() { heap_size=" << register_h_ 
       << ",stack_size=" << stack_depth() << ",trail_size=" << trail_depth() <<"}";
    return ss.str();
}


//
// This is the basic unification algorithm for two terms.
//

bool term_env_impl::is_list(cell t) const
{
    return heap_->is_list(t);
}

bool term_env_impl::is_dotted_pair(cell t) const
{
    return heap_->is_dotted_pair(t);
}

bool term_env_impl::is_empty_list(cell t) const
{
    return heap_->is_empty_list(t);
}

bool term_env_impl::is_comma(cell t) const
{
    return heap_->is_comma(t);
}

term term_env_impl::new_term(con_cell f, const std::initializer_list<term> &args)
{
    size_t i = 0;
    auto r = heap_->new_str(f);
    for (auto arg : args) {
	heap_->set_arg(r, i, arg);
	i++;
    }
    register_h_ = heap_->size();
    return r;
}

term term_env_impl::new_term(con_cell f)
{
    auto r = heap_->new_str(f);
    register_h_ = heap_->size();
    return r;
}

term term_env_impl::new_dotted_pair(term &a, term &b)
{
    static const con_cell dotted_pair(".", 2);

    term pair = heap_->new_str(dotted_pair);
    heap_->set_arg(pair, 0, a);
    heap_->set_arg(pair, 1, b);
    register_h_ = heap_->size();
    return pair;
}

con_cell term_env_impl::to_atom(con_cell functor)
{
    return heap_->to_atom(functor);
}

con_cell term_env_impl::to_functor(con_cell atom, size_t arity)
{
    return heap_->to_functor(atom, arity);
}

size_t term_env_impl::list_length(const term &lst) const
{
    return heap_->list_length(lst);
}

bool term_env_impl::equal(cell a, cell b) const
{
    size_t d = stack_depth();

    push(b);
    push(a);

    while (stack_depth() > d) {
	a = deref(pop());
	b = deref(pop());

	if (a == b) {
	    continue;
	}
	
	if (a.tag() != b.tag()) {
	    trim_stack(d);
	    return false;
	}

	if (a.tag() != tag_t::STR) {
	    trim_stack(d);
	    return false;
	}

	con_cell fa = functor(a);
	con_cell fb = functor(b);

	if (fa != fb) {
	    trim_stack(d);
	    return false;
	}

	str_cell &astr = static_cast<str_cell &>(a);
	str_cell &bstr = static_cast<str_cell &>(b);

	size_t num_args = fa.arity();
	for (size_t i = 0; i < num_args; i++) {
	    auto ai = arg(astr, num_args-i-1);
	    auto bi = arg(bstr, num_args-i-1);
	    push(bi);
	    push(ai);
	}
    }

    return true;
}

int term_env_impl::functor_standard_order(con_cell a, con_cell b) const
{
    if (a == b) {
        return 0;
    }
    size_t arity_a = a.arity();
    size_t arity_b = b.arity();
    if (arity_a < arity_b) {
        return -1;
    } else if (arity_a > arity_b) {
        return 1;
    }
    auto name_a = atom_name(a);
    auto name_b = atom_name(b);
    return name_a.compare(name_b);
}

int term_env_impl::standard_order(cell a, cell b) const
{
    size_t d = stack_depth();

    push(b);
    push(a);

    while (stack_depth() > d) {
	a = deref(pop());
	b = deref(pop());

	if (a == b) {
  	    continue;
	}

	if (a.tag() != b.tag()) {
	    trim_stack(d);
	    if (a.tag() < b.tag()) {
	        return -1;
	    } else {
	        return 1;
	    }
	    return false;
	}

	switch (a.tag()) {
 	case tag_t::CON:
	  {
	    trim_stack(d);
	    const con_cell &fa = static_cast<const con_cell &>(a);
	    const con_cell &fb = static_cast<const con_cell &>(b);
	    return functor_standard_order(fa, fb);
     	    break;
	  }
	case tag_t::REF:
	case tag_t::INT:
	  {
	    trim_stack(d);
	    if (a.value() < b.value()) {
	        return -1;
	    } else {
  	        return 1;
	    }
	  }
	  break;
	default:
	  assert(a.tag() == tag_t::STR);
	  break;
	}

	// a and b tags are both STR

	con_cell fa = functor(a);
	con_cell fb = functor(b);

	if (fa != fb) {
	    trim_stack(d);
	    return functor_standard_order(fa, fb);
	}

	str_cell &astr = static_cast<str_cell &>(a);
	str_cell &bstr = static_cast<str_cell &>(b);

	size_t num_args = fa.arity();
	for (size_t i = 0; i < num_args; i++) {
	    auto ai = arg(astr, num_args-i-1);
	    auto bi = arg(bstr, num_args-i-1);
	    push(bi);
	    push(ai);
	}
    }

    return 0;
}

bool term_env_impl::unify(cell a, cell b)
{
    size_t start_trail = trail_depth();
    size_t start_stack = stack_depth();

    size_t old_register_hb = register_hb_;

    register_hb_ = register_h_;

    bool r = unify_helper(a, b);

    if (!r) {
      unwind_trail(start_trail, trail_depth());
      trim_trail(start_trail);
      trim_stack(start_stack);

      register_hb_ = old_register_hb;
      return false;
    }

    register_hb_ = old_register_hb;
    return true;
}

void term_env_impl::set_last_choice_heap(size_t at_index)
{
    register_hb_ = at_index;
}

cell term_env_impl::copy(cell c)
{
    std::unordered_map<cell, cell> var_map;

    size_t current_stack = stack_depth();

    push(c);
    push(int_cell(0));

    while (stack_depth() > current_stack) {
        bool processed = pop() == int_cell(1);
        c = pop();
        switch (c.tag()) {
	case tag_t::REF:
	  {
	    cell v;
	    auto search = var_map.find(c);
	    if (search == var_map.end()) {
	        v = heap_->new_ref();
		var_map[c] = v;
	    } else {
  	        v = search->second;
	    }
	    temp_push(v);
	    break;
	  }
	case tag_t::CON:
	case tag_t::INT:
	  temp_push(c);
	  break;

	case tag_t::STR:
	  { con_cell f = heap_->functor(c);
	    size_t num_args = f.arity();
	    if (processed) {
	      // Arguments on temp are the new arguments of STR cell
	      cell newstr = heap_->new_str(f);
	      for (size_t i = 0; i < num_args; i++) {
		  heap_->set_arg(newstr, num_args-i-1, temp_pop());
	      }
	      temp_push(newstr);
	    } else {
	      // First push STR cell as processed
	      push(c);
	      push(int_cell(1));
	      // Then the arguments to be processed (argN-1 ... arg0)
	      // We want to process arg0 first (that's why it is pushed last.)
	      for (size_t i = 0; i < num_args; i++) {
		push(heap_->arg(c, num_args-i-1));
		push(int_cell(0));
	      }
	    }
	  }
	  break;

	// TODO: Implement hese later...
	case tag_t::BIG: assert(false); break;
	}
    }

    register_h_ = heap_->size();

    return temp_pop();
}

// Bind 'a' to 'b'.
void term_env_impl::bind(cell a, cell b)
{
    // We know 'a' is a REF cell. REF cell are always on
    // heap in our version.
    ref_cell &rc = static_cast<ref_cell &>(a);
    size_t index = rc.index();
    cell &p = (*heap_)[index];
    p = b;
    trail(index);
}

void term_env_impl::unwind_trail(size_t from, size_t to)
{
    for (size_t i = from; i < to; i++) {
      size_t index = trail_[i];
      (*heap_)[index] = ref_cell(index);
    }
}

void term_env_impl::tidy_trail(size_t from, size_t to)
{
    size_t i = from;
    while (i < to) {
      if (trail_[i] < register_hb_) {
	  // This variable recording happened before the choice point.
	  // We can't touch it.
	  i++;
      } else {
	  // Remove this trail point, move one trail point we haven't
	  // visited to this location.
	  trail_[i] = trail_[to-1];
	  to--;
      }
    }
    // We're done. Trim the trail to the new end
    trail_.resize(to);
}

bool term_env_impl::unify_helper(cell a, cell b)
{
    size_t d = stack_depth();

    push(b);
    push(a);

    while (stack_depth() > d) {
        a = deref(pop());
	b = deref(pop());

	if (a == b) {
	    continue;
	}

	// If at least one of them is a REF, then bind it.
	if (a.tag() == tag_t::REF) {
  	    if (b.tag() == tag_t::REF) {
	      auto ra = static_cast<ref_cell &>(a);
	      auto rb = static_cast<ref_cell &>(b);
	      // It's more efficient to bind higher addresses
	      // to lower if there's a choice. That way we
	      // don't need to trail the bindings.
	      if (ra.index() < rb.index()) {
		bind(b, a);
	      } else {
		bind(a, b);
	      }
	      continue;
	    } else {
	      bind(a, b);
	      continue;
	    }
	} else if (b.tag() == tag_t::REF) {
	    bind(b, a);
	    continue;
	}

	// Check tags
	if (a.tag() != b.tag()) {
	    return false;
	}
	
	switch (a.tag()) {
	case tag_t::CON:
	case tag_t::INT:
	  if (a != b) {
	    return false;
	  }
	  break;
	case tag_t::STR: {
	  str_cell &astr = static_cast<str_cell &>(a);
	  str_cell &bstr = static_cast<str_cell &>(b);
	  con_cell f = functor(astr);
	  if (f != functor(bstr)) {
	    return false;
	  }
	  // Push pairwise args
	  size_t num_args = f.arity();
	  for (size_t i = 0; i < num_args; i++) {
	    auto ai = arg(astr, num_args-i-1);
	    auto bi = arg(bstr, num_args-i-1);
	    push(bi);
	    push(ai);
	  }
	  break;
	}
        // TODO: Implement these two later...
	case tag_t::BIG:assert(false); break;
	}
    }

    return true;
}

//
// term_env
//

term_env::term_env()
{
    impl_ = new term_env_impl();
}

term_env::~term_env()
{
    delete impl_;
}

void term_env::sync_with_heap()
{
    impl_->sync_with_heap();
}

heap & term_env::get_heap()
{
    return impl_->get_heap();
}

term_ops & term_env::get_ops()
{
    return impl_->get_ops();
}

term term_env::parse(const std::string &term_expr)
{
    return impl_->parse(term_expr);
}

std::string term_env::to_string(const term  &term, term_emitter::style style) const
{
    return impl_->to_string(term, style);
}

std::string term_env::safe_to_string(const term  &term, term_emitter::style style) const
{
    return impl_->safe_to_string(term, style);
}

std::string term_env::status() const
{
    return impl_->status();
}

std::string term_env::atom_name(con_cell f) const
{
    return impl_->atom_name(f);
}

std::string term_env::atom_name(const term &t) const
{
    return impl_->atom_name(t);
}

size_t term_env::stack_size() const
{
    return impl_->stack_depth();
}

size_t term_env::trail_size() const
{
    return impl_->trail_depth();
}

void term_env::unwind_trail(size_t from_addr, size_t to_addr)
{
    impl_->unwind_trail(from_addr, to_addr);
}

size_t term_env::heap_size() const
{
    return impl_->heap_size();
}

bool term_env::is_list(const term &t) const
{
    return impl_->is_list(t);
}

bool term_env::is_dotted_pair(const term &t) const
{
    return impl_->is_dotted_pair(t);
}

bool term_env::is_empty_list(const term &t) const
{
    return impl_->is_empty_list(t);
}

bool term_env::is_comma(const term &t) const
{
    return impl_->is_comma(t);
}

term term_env::empty_list() const
{
    return impl_->empty_list();
}

bool term_env::is_atom(const term &t) const
{
    return impl_->is_atom(t);
}

con_cell term_env::functor(const std::string &name, size_t arity)
{
    return impl_->functor(name, arity);
}

con_cell term_env::functor(const term &t)
{
    return impl_->functor(t);
}

bool term_env::is_functor(const term &t, con_cell f)
{
    return impl_->is_functor(t) && impl_->functor(t) == f;
}

bool term_env::is_functor(const term &t)
{
    return impl_->is_functor(t);
}

term term_env::arg(const term &t, size_t index)
{
    return impl_->arg(t, index);
}

void term_env::set_arg(term &t, size_t index, term &arg)
{
    return impl_->set_arg(t, index, arg);
}

term term_env::new_term(con_cell f, const std::initializer_list<term> &args)
{
    return impl_->new_term(f, args);
}

term term_env::new_term(con_cell f)
{
    return impl_->new_term(f);
}

term term_env::new_dotted_pair(term &a, term &b)
{
    return impl_->new_dotted_pair(a, b);
}

con_cell term_env::to_atom(con_cell functor)
{
    return impl_->to_atom(functor);
}

con_cell term_env::to_functor(con_cell atom, size_t arity)
{
    return impl_->to_functor(atom, arity);
}

size_t term_env::list_length(const term &lst) const
{
    return impl_->list_length(lst);
}

bool term_env::is_ground(const term &tt) const
{
    auto range = const_cast<term_env &>(*this).iterate_over(tt);
    for (auto t : range) {
	if (t.tag() == tag_t::REF) {
	    return false;
	}
    }
    return true;    
}

bool term_env::unify(term &a, term &b)
{
    return impl_->unify(a,b);
}

term term_env::copy(const term &t)
{
    return impl_->to_term(impl_->copy(t));
}

void term_env::set_last_choice_heap(size_t at_index)
{
    impl_->set_last_choice_heap(at_index);
}

bool term_env::equal(const term &a, const term &b)
{
    return impl_->equal(a,b);
}

int term_env::standard_order(const term &a, const term &b)
{
    return impl_->standard_order(a,b);
}

void term_env::push(const term &t)
{
    impl_->push(t);
}

size_t term_env::allocate_stack(size_t num_cells)
{
    return impl_->allocate_stack(num_cells);
}

void term_env::ensure_stack(size_t at_index, size_t num_cells)
{
    impl_->ensure_stack(at_index, num_cells);
}

cell * term_env::stack_ref(size_t at_index)
{
    return impl_->stack_ref(at_index);
}

term term_env::pop()
{
    return impl_->to_term(impl_->pop());
}

void term_env::clear_name(const term &ref)
{
    impl_->clear_name(ref);
}

bool term_env::has_name(const term &ref) const
{
    return impl_->has_name(ref);
}

void term_env::set_name(const term &ref, const std::string &name)
{
    impl_->set_name(ref, name);
}

term term_env::to_term(cell c) const
{
    return impl_->to_term(c);
}

void term_env::trim_heap(size_t new_size)
{
    impl_->trim_heap(new_size);
}

void term_env::trim_trail(size_t new_size)
{
    impl_->trim_trail(new_size);
}

void term_env::tidy_trail(size_t from, size_t to)
{
    impl_->tidy_trail(from, to);
}

term_dfs_iterator term_env::begin(const term &t)
{
    return term_dfs_iterator(*this, t);
}

term_dfs_iterator term_env::end(const term &t)
{
    return term_dfs_iterator(*this);
}

}}

