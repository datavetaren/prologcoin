#include "term_env.hpp"
#include "term.hpp"
#include "term_ops.hpp"
#include "term_tokenizer.hpp"
#include "term_parser.hpp"
#include "term_emitter.hpp"

namespace prologcoin { namespace common {

#if 0
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
#endif

bool term_utils::equal(term a, term b, uint64_t &cost)
{
    size_t d = stack_size();
    uint64_t cost_tmp = 0;

    push(b);
    push(a);

    while (stack_size() > d) {
	uint64_t cost_deref1 = 0, cost_deref2 = 0;
	a = deref_with_cost(pop(), cost_deref1);
	cost_tmp += cost_deref1;
	b = deref_with_cost(pop(), cost_deref2);
	cost_tmp += cost_deref2;

	if (a == b) {
	    continue;
	}
	
	if (a.tag() != b.tag()) {
	    trim_stack(d);
	    cost = cost_tmp;
	    return false;
	}

	if (a.tag() != tag_t::STR) {
	    trim_stack(d);
	    cost = cost_tmp;
	    return false;
	}

	con_cell fa = functor(a);
	con_cell fb = functor(b);

	if (fa != fb) {
	    trim_stack(d);
	    cost = cost_tmp;
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

    cost = cost_tmp;
    return true;
}

uint64_t term_utils::hash(term t)
{
    size_t d = stack_size();

    push(t);

    uint64_t h = 0;

    while (stack_size() > d) {
	t = deref(pop());

	switch (t.tag()) {
	case tag_t::CON:
	case tag_t::INT:
	case tag_t::REF:
	    h += t.raw_value();
	    break;
        case tag_t::STR: {
	    con_cell f = functor(t);
	    h += f.raw_value();
	    size_t n = f.arity();
	    for (size_t i = 0; i < n; i++) {
	        push(arg(t, i));
	    }
	  }
	  break;
	}
    }

    return h;
}

uint64_t term_utils::cost(term t)
{
    size_t d = stack_size();

    push(t);

    uint64_t cost_acc = 0;

    while (stack_size() > d) {
	uint64_t cost_tmp = 0;
	// The cost of deref is at least 1. If ref chains are
	// longer the cost will be bigger. Thus, when the stack
	// based DFS is completed we'll have a cost that this
	// approximately the size of the term.
	t = deref_with_cost(pop(), cost_tmp);
	cost_acc += cost_tmp;

	switch (t.tag()) {
	case tag_t::CON:
	case tag_t::INT:
	case tag_t::REF:
	    break;
        case tag_t::STR: {
	    con_cell f = functor(t);
	    size_t n = f.arity();
	    for (size_t i = 0; i < n; i++) {
	        push(arg(t, i));
	    }
	  }
	  break;
	}
    }

    return cost_acc;
}

bool term_utils::unify(term a, term b, uint64_t &cost)
{
    size_t start_trail = trail_size();
    size_t start_stack = stack_size();
    size_t old_register_hb = get_register_hb();

    // Record all bindings so we can undo them in case
    // unification fails.
    set_register_hb(heap_size());

    bool r = unify_helper(a, b, cost);

    if (!r) {
      unwind_trail(start_trail, trail_size());
      trim_trail(start_trail);
      trim_stack(start_stack);

      set_register_hb(old_register_hb);
      return false;
    }

    set_register_hb(old_register_hb);
    return true;
}

bool term_utils::unify_helper(term a, term b, uint64_t &cost)
{
    size_t d = stack_size();

    uint64_t cost_tmp = 0;

    push(b);
    push(a);

    while (stack_size() > d) {
	
	// The cost of deref is at least 1 (if the ref chains are longer
	// the cost will be bigger.)
	// So every iteration of this stack based unification loop
	// will add 2 to the accumulated cost.

	uint64_t cost_deref1 = 0, cost_deref2 = 0;
        a = deref_with_cost(pop(), cost_deref1);
	cost_tmp += cost_deref1;
	b = deref_with_cost(pop(), cost_deref2);
	cost_tmp += cost_deref2;

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
		bind(rb, a);
	      } else {
		bind(ra, b);
	      }
	      continue;
	    } else {
	      auto ra = static_cast<ref_cell &>(a);
	      bind(ra, b);
	      continue;
	    }
	} else if (b.tag() == tag_t::REF) {
	    auto rb = static_cast<ref_cell &>(b);
	    bind(rb, a);
	    continue;
	}

	// Check tags
	if (a.tag() != b.tag()) {
	    cost = cost_tmp;
	    return false;
	}
	
	switch (a.tag()) {
	case tag_t::CON:
	case tag_t::INT:
	  if (a != b) {
	    cost = cost_tmp;
	    return false;
	  }
	  break;
	case tag_t::STR: {
	  str_cell &astr = static_cast<str_cell &>(a);
	  str_cell &bstr = static_cast<str_cell &>(b);
	  con_cell f = functor(astr);
	  if (f != functor(bstr)) {
	    cost = cost_tmp;
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

    cost = cost_tmp;

    return true;
}

int term_utils::functor_standard_order(con_cell a, con_cell b)
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

int term_utils::standard_order(term a, term b, uint64_t &cost)
{
    size_t d = stack_size();

    uint64_t cost_tmp = 0;

    push(b);
    push(a);

    while (stack_size() > d) {
	uint64_t cost_deref1 = 0, cost_deref2 = 0;
	a = deref_with_cost(pop(), cost_deref1);
	cost_tmp += cost_deref1;
	b = deref_with_cost(pop(), cost_deref2);
	cost_tmp += cost_deref2;

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
	    cost = cost_tmp;
	    return false;
	}

	switch (a.tag()) {
 	case tag_t::CON:
	  {
	    trim_stack(d);
	    const con_cell &fa = static_cast<const con_cell &>(a);
	    const con_cell &fb = static_cast<const con_cell &>(b);
	    // Can never be equal as it would have triggered if (a == b)...
	    int cmp = functor_standard_order(fa, fb);
	    cost = cost_tmp;
	    return cmp;
	  }
	case tag_t::REF:
	case tag_t::INT:
	  {
	    trim_stack(d);
	    if (a.value() < b.value()) {
		cost = cost_tmp;
	        return -1;
	    } else {
		cost = cost_tmp;
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
	    cost = cost_tmp;
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

    cost = cost_tmp;
    return 0;
}

term term_utils::copy(term c, heap &src, uint64_t &cost)
{
    std::unordered_map<term, term> var_map;

    size_t current_stack = stack_size();
    
    uint64_t cost_tmp = 0;

    push(src.deref(c));
    push(int_cell(0));

    while (stack_size() > current_stack) {
	cost_tmp++;

        bool processed = pop() == int_cell(1);
        c = pop();
        switch (c.tag()) {
	case tag_t::REF:
	  {
	    cell v;
	    auto search = var_map.find(c);
	    if (search == var_map.end()) {
	        v = new_ref();
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
	  { con_cell f = src.functor(c);
	    size_t num_args = f.arity();
	    if (processed) {
	      // Arguments on temp are the new arguments of STR cell
	      cell newstr = new_term(f);
	      for (size_t i = 0; i < num_args; i++) {
		  set_arg(newstr, num_args-i-1, temp_pop());
	      }
	      temp_push(newstr);
	    } else {
	      // First push STR cell as processed
	      push(c);
	      push(int_cell(1));
	      // Then the arguments to be processed (argN-1 ... arg0)
	      // We want to process arg0 first (that's why it is pushed last.)
	      for (size_t i = 0; i < num_args; i++) {
		push(src.arg(c, num_args-i-1));
		push(int_cell(0));
	      }
	    }
	  }
	  break;

	// TODO: Implement hese later...
	case tag_t::BIG: assert(false); break;
	}
    }

    cost = cost_tmp;

    return temp_pop();
}

}}

