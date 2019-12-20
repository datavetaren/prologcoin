#include "builtins.hpp"
#include "interpreter_base.hpp"
#include "wam_interpreter.hpp"
#include <stdarg.h>
#include <boost/algorithm/string.hpp>
#include <memory>
#include <set>

namespace prologcoin { namespace interp {

    using namespace prologcoin::common;

    //
    // Profiling
    //

    bool builtins::profile_0(interpreter_base &interp, size_t arity, common::term args[])
    {
	interp.print_profile();
	return true;
    }

    //
    // debug_on/0
    //
    bool builtins::debug_on_0(interpreter_base &interp, size_t arity, common::term args[])
    {
	interp.set_debug(true);
	return true;
    }

    //
    // debug_check/0
    //
    bool builtins::debug_check_0(interpreter_base &interp, size_t arity, common::term args[])
    {
	interp.debug_check();
	return true;
    }	

    //
    // Simple
    //

    bool builtins::true_0(interpreter_base &interp, size_t arity, common::term args[])
    {
	return true;
    }

    bool builtins::fail_0(interpreter_base &interp, size_t arity, common::term args[])
    {
	return false;
    }

    //
    // Control flow
    //

    // TODO: This needs to be modified to use callbacks.
    bool builtins::operator_comma(interpreter_base &interp, size_t arity, common::term args[])
    {
        interp.allocate_environment<ENV_NAIVE>();
	interp.set_cp(code_point(args[1]));
	interp.allocate_environment<ENV_NAIVE>();
	interp.set_p(code_point(args[0]));
	interp.set_cp(interpreter_base::EMPTY_LIST);

	return true;
    }

    // TODO: This needs to be modified to use callbacks. Cut should be a primitive.
    bool builtins::operator_cut(interpreter_base &interp, size_t arity, common::term args[])
    {
	interp.cut();
        return true;
    }

    // TODO: This needs to be modified to use callbacks. Cut should be a primitive.
    bool builtins::operator_cut_if(interpreter_base &interp, size_t arity, common::term args[])
    {
	interp.deallocate_environment();
	interp.set_p(interp.cp());
	interp.set_cp(interp.e0()->cp);

	interp.cut_direct();
	auto *ch = interp.b();
	interp.set_b(ch->b);

        return true;
    }

    // TODO: This needs to be modified to use callbacks. 
    bool builtins::operator_disjunction(interpreter_base &interp, size_t arity, common::term args[])
    {
	static con_cell arrow("->", 2);

	// Check if this is an if-then-else
	term arg0 = args[0];
	if (interp.functor(arg0) == arrow) {
	    return operator_if_then_else(interp, arity, args);
	}

	term arg1 = args[1];
        interp.allocate_choice_point(code_point(arg1));
	interp.allocate_environment<ENV_NAIVE>();
	interp.set_p(code_point(arg0));
	return true;
    }

    // TODO: This needs to be modified to use callbacks.
    bool builtins::operator_if_then(interpreter_base &interp, size_t arity, common::term args[])
    {
	static con_cell cut_op_if("_!",0);

	interp.allocate_choice_point(code_point::fail());
	interp.move_cut_point_to_last_choice_point();
	term cut_if = cut_op_if;
	term arg0 = args[0];
	term arg1 = args[1];
	interp.allocate_environment<ENV_NAIVE>();
	interp.set_cp(code_point(arg1));
	interp.allocate_environment<ENV_NAIVE>();
	interp.set_cp(code_point(cut_if));
	interp.allocate_environment<ENV_NAIVE>();
	interp.set_p(code_point(arg0));
        return true;
    }

    // TODO: This needs to be modified to use callbacks.
    bool builtins::operator_if_then_else(interpreter_base &interp, size_t arity, common::term args[])
    {
	static con_cell cut_op_if("_!",0);

	term lhs = args[0];

	term cond = interp.arg(lhs, 0);
	term if_true = interp.arg(lhs, 1);
	term if_false = args[1];
	term cut_if = cut_op_if;

	// Go to 'C' the false clause if ((A->B) ; C) fails
	interp.allocate_choice_point(code_point(if_false));
	interp.move_cut_point_to_last_choice_point();
	interp.allocate_environment<ENV_NAIVE>();
	interp.set_cp(code_point(if_true));
	interp.allocate_environment<ENV_NAIVE>();
	interp.set_cp(code_point(cut_if));
	interp.allocate_environment<ENV_NAIVE>();
	interp.set_p(code_point(cond));
        return true;
    }

    //
    // Standard order, eqaulity and unification
    //

    bool builtins::operator_at_less_than(interpreter_base &interp, size_t arity, common::term args[])
    {
        return interp.standard_order(args[0], args[1]) < 0;
    }

    bool builtins::operator_at_equals_less_than(interpreter_base &interp, size_t arity, common::term args[])
    {
        return interp.standard_order(args[0], args[1]) <= 0;
    }

    bool builtins::operator_at_greater_than(interpreter_base &interp, size_t arity, common::term args[])
    {
        return interp.standard_order(args[0], args[1]) > 0;
    }

    bool builtins::operator_at_greater_than_equals(interpreter_base &interp, size_t arity, common::term args[])
    {
        return interp.standard_order(args[0], args[1]) >= 0;
    }

    bool builtins::operator_equals(interpreter_base &interp, size_t arity, common::term args[])
    {
        return interp.standard_order(args[0], args[1]) == 0;
    }

    bool builtins::operator_not_equals(interpreter_base &interp, size_t arity, common::term args[])
    {
        return interp.standard_order(args[0], args[1]) != 0;
    }

    bool builtins::compare_3(interpreter_base &interp, size_t arity, common::term args[])
    {
        term order = args[0];

	int c = interp.standard_order(args[1], args[2]);
	term op;
	if (c < 0) {
	    op = con_cell("<", 0);
	} else if (c > 0) {
	    op = con_cell(">", 0);
	} else {
	    op = con_cell("=", 0);
	}
	bool ok = interp.unify(order, op);
	return ok;
    }

    bool builtins::operator_unification(interpreter_base &interp, size_t arity, common::term args[])
    {
	term arg0 = args[0];
	term arg1 = args[1];
	bool r = interp.unify(arg0, arg1);
	return r;
    }

    bool builtins::operator_cannot_unify(interpreter_base &interp, size_t arity, common::term args[])
    {
	term lhs = args[0];
	term rhs = args[1];

	size_t current_tr = interp.trail_size();
	bool r = interp.unify(lhs, rhs);
	if (r) {
	    interp.unwind(current_tr);
	}
	return !r;
    }

    //
    // Type tests
    //
    
    bool builtins::var_1(interpreter_base &interp, size_t arity, common::term args[])
    {
        return args[0].tag().is_ref();
    }

    bool builtins::nonvar_1(interpreter_base &interp, size_t arity, common::term args[])
    {
	return !var_1(interp, arity, args);
    }

    bool builtins::integer_1(interpreter_base &interp, size_t arity, common::term args[])
    {
	return args[0].tag() == tag_t::INT;
    }

    bool builtins::number_1(interpreter_base &interp, size_t arity, common::term args[])
    {
	return integer_1(interp, arity, args);
    }

    bool builtins::atom_1(interpreter_base &interp, size_t arity, common::term args[])
    {
	term arg = args[0];
	
	switch (arg.tag()) {
	case tag_t::CON: return true;
	case tag_t::STR: {
	    con_cell f = interp.functor(arg);
	    return f.arity() == 0;
	}
	default: return false;
	}
    }

    bool builtins::compound_1(interpreter_base &interp, size_t arity, common::term args[])
    {
	term arg = args[0];
	if (arg.tag() != tag_t::STR) {
	    return false;
	}
	return interp.functor(arg).arity() > 0;
    }

    bool builtins::callable_1(interpreter_base &interp, size_t arity, common::term args[])
    {
	return atom_1(interp, arity, args) || compound_1(interp, arity, args);
    }

    bool builtins::atomic_1(interpreter_base &interp, size_t arity, common::term args[])
    {
	return atom_1(interp, arity, args);
    }

    bool builtins::ground_1(interpreter_base &interp, size_t arity, common::term args[])
    {
	term arg = args[0];
	return interp.is_ground(arg);
    }

    //
    // Character properties
    //

   bool builtins::upcase_atom_2(interpreter_base &interp, size_t arity, common::term args[])
   {
       term from = args[0];
       term to = args[1];

       std::string s;
       switch (from.tag()) {
         case tag_t::REF:
         case tag_t::RFW:
	     interp.abort(interpreter_exception_not_sufficiently_instantiated("upcase_atom/2: Arguments are not sufficiently instantiated"));
	     return false;
         case tag_t::INT: {
	     int_cell &ic = static_cast<int_cell &>(from);
	     s = boost::lexical_cast<std::string>(ic.value());
	     break;
         }
         case tag_t::CON:
         case tag_t::STR: {
	     auto f = interp.functor(from);
	     if (f.arity() != 0) {
		 std::string msg = "upcase_atom/2: "
		     "First argument was not 'atomic', found '"
		     + interp.to_string(from) + "'";
		 interp.abort(interpreter_exception_wrong_arg_type(msg));
	     }
	     s = interp.atom_name(f);
	     break;
         }
         default: {
	     std::string msg = "upcase_atom/2: "
		 "Unexpected first argument, found '"
		 + interp.to_string(from) + "'";
	     interp.abort(interpreter_exception_wrong_arg_type(msg));
	 }
       }
       boost::to_upper(s);
       term r = interp.atom(s);
       bool ok = interp.unify(to, r);
       return ok;
   }

    bool builtins::bytes_number_2(interpreter_base &interp, size_t arity, common::term args[]) {
        if (args[0].tag().is_ref() && args[1].tag().is_ref()) {
  	    std::string msg = "bytes_number/2: Not both arguments can be unbounded variables.";
	    interp.abort(interpreter_exception_not_sufficiently_instantiated(msg));
        }
	term charlst = args[0];
        if (!charlst.tag().is_ref()) {
  	    if (!interp.is_list(charlst)) {
	        interp.abort(interpreter_exception_wrong_arg_type("bytes_number/2: First argument must be a list of integers (in 0..255)"));
	    }
	    size_t n = interp.list_length(charlst);
	    if (n > 65536) {
	        std::stringstream msg;
	        msg << "bytes_number/2: List length exceeds 65536 elements";
		msg << "; was " << n;
		interp.abort(interpreter_exception_wrong_arg_type(msg.str()));
	    }

 	    std::unique_ptr<uint8_t> bytes( new uint8_t[n] );
	  
  	    size_t i = 0;
	    while (interp.is_dotted_pair(charlst)) {
	        term charelem = interp.arg(charlst,0);
		if (charelem.tag() != tag_t::INT ||
		    static_cast<int_cell &>(charelem).value() < 0 ||
		    static_cast<int_cell &>(charelem).value() > 255) {
  		    std::stringstream msg;
		    msg << "bytes_number/2: Element at position " << (i+1) << " is not an integer between 0 and 255; was ";
		    msg << interp.to_string(charelem);
		    interp.abort(interpreter_exception_wrong_arg_type(msg.str()));
		}
		auto v = static_cast<int_cell &>(charelem).value();
		(bytes.get())[i] = static_cast<uint8_t>(v);
		charlst = interp.arg(charlst, 1);
		i++;
	    }

	    // TODO: If n <= 4 we should try to make a standard integer (int_cell)
	    assert(n > 4);

	    term t = interp.new_big(8*n);
	    interp.set_big(t, bytes.get(), n);

	    return interp.unify(args[1], t);
        }

	// First argument is unbound, we need to construct a list.
	term bignum_term = args[1];
	if (bignum_term.tag() != tag_t::BIG && bignum_term.tag() != tag_t::INT) {
  	    std::stringstream msg;
	    msg << "bytes_number/2: Second argument must be an integer number; was ";
	    msg << interp.to_string(bignum_term);
	    interp.abort(interpreter_exception_wrong_arg_type(msg.str()));
	}
	if (bignum_term.tag() == tag_t::INT) {
	    // This is a 61 bit integer (which can be negative)
	    int64_t val = reinterpret_cast<int_cell &>(bignum_term).value();
	    uint8_t byt = (val >> 56) & 0xff;
	    auto r = interp.new_dotted_pair(int_cell(byt),
					    interpreter_base::EMPTY_LIST);
	    auto tail = r;
	    val >>= 8;
	    for (size_t i = 0; i < 7; i++) {
	        byt = static_cast<uint8_t>((val >> 56) & 0xff);
		auto nextLst = interp.new_dotted_pair(int_cell(byt),
						      interpreter_base::EMPTY_LIST);
		interp.set_arg(tail, 1, nextLst);
		tail = nextLst;
	    }
	    return interp.unify(r, args[0]);	    
	}
	
	// TODO: Should fix a range for bignum so we can use for (auto b : )
	const big_cell &bignum = reinterpret_cast<const big_cell &>(bignum_term);
	auto bend = interp.end(bignum);
	term r = interpreter_base::EMPTY_LIST;
	term tail = r;
        for (auto it = interp.begin(bignum); it != bend; ++it) {
	    uint8_t byt = *it;
	    auto nextLst = interp.new_dotted_pair(int_cell(byt),
						  interpreter_base::EMPTY_LIST);
	    if (interp.is_empty_list(tail)) {
	        tail = nextLst;
	        r = tail;
	    } else {
	        interp.set_arg(tail, 1, nextLst);
		tail = nextLst;
	    }
	}
        return interp.unify(r, args[0]);
    }

    bool builtins::cyclic_term_1(interpreter_base &interp, size_t arity, common::term args[]) {
      int i = 0;
      term t = args[0];
      std::list<std::pair<term, int> > workstack;
      std::set<untagged_cell::value_t> path;
      workstack.push_front(std::pair<term, int>(t, 0));
      while(workstack.size() > 0) {
	i++;
	if(i > 20) {
	  return false;
	}
	auto current = workstack.front();
	workstack.pop_front();
	auto current_term = current.first;
	auto current_index = current.second;
	auto current_tag = current_term.tag();
	if(path.count(current_term.raw_value()) > 0 && current_index == 0) {
	  return true;
	}
	path.insert(current_term.raw_value());
	switch (current_tag) {
	case tag_t::STR:
	case tag_t::CON:
	  {
	  auto tf = interp.functor(current_term);
	  auto arity = tf.arity();
	  if (current_index < arity) {
	    auto arg = interp.arg(current_term, current_index);
	    workstack.push_front(std::pair<term, int>(current_term, current_index+1));
	    workstack.push_front(std::pair<term, int>(arg, 0));
	  } else {
	    path.erase(current_term.raw_value());
	  }
	  break;
	}
	case tag_t::REF: case tag_t::RFW: {
	  {
	    term deref_term = interp.deref(current_term);
	    if (deref_term == current_term && current_tag.is_ref()) {
	      return false;
	    }
	    workstack.push_front(std::pair<term, int>(deref_term, 0));
	 }
	  break;
	}
	default: {
	  path.erase(current_term.raw_value());
	  break;
	}
	}
      }
      return false;
    }

    bool builtins::acyclic_term_1(interpreter_base &interp, size_t arity, common::term args[]) {
      bool cyclic_result = cyclic_term_1(interp, arity, args);
      return !cyclic_result;
    }

    bool builtins::is_list_1(interpreter_base &interp, size_t arity, common::term args[])
    {
	term t = args[0];
	while (interp.is_dotted_pair(t)) {
	    t = interp.arg(t, 1);
	}
	return interp.is_empty_list(t);
    }

    //
    // Arithmetics
    //

    bool builtins::is_2(interpreter_base &interp, size_t arity, common::term args[])
    {
	term lhs = args[0];
	term rhs = args[1];
	term result = interp.arith().eval(rhs, "is/2");
	bool ok = interp.unify(lhs, result);
	return ok;
    }	

    //
    // Analyzing & constructing terms
    //

    void builtins::store_p_on_heap_if_wam(interpreter_base &interp) {
	if (interp.p().has_wam_code()) {
	    wam_interpreter &wami = reinterpret_cast<wam_interpreter &>(interp);
	    auto ic = int_cell(wami.to_code_addr(interp.p().wam_code()));
	    interp.new_cell0(ic);
	} else {
	    interp.new_cell0(interpreter_base::EMPTY_LIST);
	}
    }

    void builtins::restore_p_from_heap_if_wam(interpreter_base &interp) {
	auto c = interp.get_heap()[interp.heap_size()-1];
	if (c.tag() == tag_t::INT) {
	    wam_interpreter &wami = reinterpret_cast<wam_interpreter &>(interp);
	    auto ic = static_cast<int_cell &>(c).value();
	    auto wam_code = wami.to_code(ic);
	    interp.p().set_wam_code(wam_code);
	}
    }

    bool builtins::arg_3_cp(interpreter_base &interp, size_t arity, common::term args[])
    {
	auto arg_index_term = interp.get_heap()[interp.heap_size()-2];
	auto arg_index = static_cast<int_cell &>(arg_index_term).value();
	term t = args[1];
	size_t n = interp.functor(t).arity();
	arg_index++;
	if (arg_index == n) {
	    interp.b()->bp = code_point::fail();
	    return false;
	}
	interp.get_heap()[interp.heap_size()-2] = int_cell(arg_index);
	restore_p_from_heap_if_wam(interp);
	interp.unify(args[0], int_cell(arg_index+1));
	term val = interp.arg(t, arg_index);
        return interp.unify(args[2], val);
    }

    bool builtins::arg_3(interpreter_base &interp, size_t arity, common::term args[]) {
	term t = args[1];
	if (t.tag() != tag_t::STR) {
	    std::string msg =
	      "arg/3: Second argument must be a functor with arguments; was " +
	      interp.to_string(t);
	    interp.abort(interpreter_exception_wrong_arg_type(msg));
	}

        term arg_index_term = args[0];

	if (arg_index_term.tag().is_ref()) {
	    static const common::con_cell ARG3("arg", 3);
	    // Store current index
	    interp.new_cell0(int_cell(0));
	    // Store P so we can restore program pointer
	    store_p_on_heap_if_wam(interp);
	    // Allocate choice point
	    interp.allocate_choice_point(code_point(interpreter_base::EMPTY_LIST,arg_3_cp, false));
	    interp.unify(arg_index_term, int_cell(1));
	    arg_index_term = int_cell(1);
	}

	if (arg_index_term.tag() != tag_t::INT) {
	    std::string msg =
	      "arg/3: First argument was not an integer or variable; was "
	      + interp.to_string(arg_index_term);
	    interp.abort(interpreter_exception_wrong_arg_type(msg));
	}

	auto arg_index = static_cast<int_cell &>(arg_index_term).value();
	
	size_t n = interp.functor(t).arity();
	if ((arg_index < 1) || (arg_index > n)) {
	    std::stringstream msg;
	    msg << "arg/3: Argument index is out of range. "
	        << "It needs to be within 1 and " << n << "; was "
		<< arg_index;
	    interp.abort(interpreter_exception_wrong_arg_type(msg.str()));
	}
	term val = interp.arg(t, arg_index-1);
        return interp.unify(args[2], val);
    }

    bool builtins::functor_3(interpreter_base &interp, size_t arity, common::term args[]) {
	term t = args[0];
	if (t.tag().is_ref()) {
	    // Create functor
	    term f = args[1];
	    term a = args[2];
	    if (f.tag().is_ref()) {
	        std::string msg =
	          "functor/3: Second argument must be a ground if first argument is a variable; was " + interp.to_string(f);
	        interp.abort(interpreter_exception_not_sufficiently_instantiated(msg));
	    }
	    if (a.tag().is_ref()) {
	        std::string msg =
	          "functor/3: Third argument must be a ground if first argument is a variable; was " + interp.to_string(a);
	        interp.abort(interpreter_exception_not_sufficiently_instantiated(msg));
	    }
	    if (a.tag() != tag_t::INT) {
	        std::string msg =
	          "functor/3: Expected third argument to be an integer; was " + interp.to_string(a);
	        interp.abort(interpreter_exception_wrong_arg_type(msg));
	    }
	    size_t arity = static_cast<int_cell &>(a).value();
	    if (arity < 0 || arity > con_cell::MAX_ARITY) {
	        std::stringstream msg;
		msg << "functor/3: Arity must be 0 or maximum " << con_cell::MAX_ARITY << "; was " << interp.to_string(arity);
	        interp.abort(interpreter_exception_wrong_arg_type(msg.str()));
	    }
	    switch (f.tag()) {
	    case tag_t::CON: break;
	    case tag_t::INT: case tag_t::BIG:
	        return interp.unify(t, f) && interp.unify(a, int_cell(0));
	    case tag_t::STR:
	        return false;
	    }

	    auto c = static_cast<con_cell &>(f);
	    con_cell newfun = interp.to_functor(c, arity);
	    return interp.unify(t, interp.new_term(newfun));
	} else {
	    // Extract functor
 	    if (t.tag() == tag_t::INT || t.tag() == tag_t::BIG) {
	        return interp.unify(args[1], t) &&
		       interp.unify(args[2], int_cell(0));
	    }
	  
	    if (t.tag() != tag_t::STR && t.tag() != tag_t::CON) {
	        std::string msg =
	          "functor/3: First argument must be a functor (or a variable); was " + interp.to_string(t);
	        interp.abort(interpreter_exception_wrong_arg_type(msg));
	    }

	    con_cell fun = interp.functor(t);
	    con_cell atom = interp.to_atom(fun);
	    size_t arity = fun.arity();

	    return interp.unify(args[1], atom) &&
	           interp.unify(args[2], int_cell(arity));
	}
    }
    
    bool builtins::copy_term_2(interpreter_base &interp, size_t arity, common::term args[])
    {
	term arg1 = args[0];
	term arg2 = args[1];
	term copy_arg1 = interp.copy(arg1);
	bool ok = interp.unify(arg2, copy_arg1);
	return ok;
    }

    bool builtins::same_term_2(interpreter_base &interp, size_t arity, common::term args[])
    {
	term t1 = args[0];
	term t2 = args[1];

	return t1 == t2;
    }

    term builtins::deconstruct_write_list(interpreter_base &interp,
					 term &t, size_t index)
    {
        term empty_lst = interpreter_base::EMPTY_LIST;
	term lst = empty_lst;
	con_cell f = interp.functor(t);
	size_t n = f.arity();
	term tail = lst;
	while (index < n) {
	    term arg = interp.arg(t, index);
	    term elem = interp.new_dotted_pair(arg, empty_lst);
	    if (interp.is_empty_list(tail)) {
		lst = elem;
		tail = elem;
	    } else {
		interp.set_arg(tail, 1, elem);
		tail = elem;
	    }
	    index++;
	}
	return lst;
    }

    bool builtins::deconstruct_read_list(interpreter_base &interp,
					 term lst,
					 term &t, size_t index)
    {
	con_cell f = interp.functor(t);
	size_t n = f.arity();
	while (index < n) {
	    if (lst.tag().is_ref()) {
		term tail = deconstruct_write_list(interp, t, index);
		return interp.unify(lst, tail);
	    }
	    if (!interp.is_dotted_pair(lst)) {
		return false;
	    }
	    term elem = interp.arg(lst, 0);
	    term arg = interp.arg(t, index);
	    if (!interp.unify(elem, arg)) {
		return false;
	    }
	    lst = interp.arg(lst, 1);
	    index++;
	}
	return true;
    }

    bool builtins::operator_deconstruct(interpreter_base &interp, size_t arity, common::term args[])
    {
	auto lhs = args[0];
	auto rhs = args[1];

	// To make deconstruction more efficient, let's handle the
	// common scenarios first.

	if (lhs.tag().is_ref() && rhs.tag().is_ref()) {
		interp.abort(interpreter_exception_not_sufficiently_instantiated("=../2: Arguments are not sufficiently instantiated"));
	}

	if (lhs.tag().is_ref()) {
	    if (!interp.is_list(rhs)) {
		interp.abort(interpreter_exception_not_list("=../2: Second argument is not a list; found " + interp.to_string(rhs)));
	    }
	    size_t lst_len = interp.list_length(rhs);
	    if (lst_len == 0) {
		interp.abort(interpreter_exception_not_list("=../2: Second argument must be non-empty; found " + interp.to_string(rhs)));
	    }
	    term first_elem = interp.arg(rhs,0);
	    if (first_elem.tag().is_ref()) {
		interp.abort(interpreter_exception_not_sufficiently_instantiated("=../2: Arguments are not sufficiently instantiated"));
	    }
	    if (first_elem.tag() == tag_t::INT && lst_len == 1) {
		return interp.unify(lhs, first_elem);
	    }
	    if (!interp.is_functor(first_elem)) {
		return false;
	    }
	    con_cell f = interp.functor(first_elem);
	    size_t num_args = lst_len - 1;
	    term t = interp.new_term(interp.to_functor(f, num_args));
	    term lst = interp.arg(rhs, 1);
	    for (size_t i = 0; i < num_args; i++) {
	        if (lst.tag().is_ref()) {
		    term tail = deconstruct_write_list(interp, lhs, i);
		    return interp.unify(lst, tail);
		}
		term arg = interp.arg(lst, 0);
		interp.set_arg(t, i, arg);

		lst = interp.arg(lst, 1);
	    }
	    return interp.unify(lhs, t);
	}
	if (lhs.tag() == tag_t::INT) {
  	    term empty = interpreter_base::EMPTY_LIST;
	    term lst = interp.new_dotted_pair(lhs,empty);
	    return interp.unify(rhs, lst);
	}
	if (lhs.tag() != tag_t::STR && lhs.tag() != tag_t::CON) {
	    return false;
	}
	con_cell f = interp.to_atom(interp.functor(lhs));
	term elem = f;
	term lst = deconstruct_write_list(interp, lhs, 0);
	lst = interp.new_dotted_pair(elem, lst);
	return interp.unify(rhs, lst);
    }

    bool builtins::sort_2(interpreter_base &interp, size_t arity, term args[])
    {
        term arg0 = args[0];
	term arg1 = args[1];

	if (arg0.tag().is_ref()) {
            interp.abort(interpreter_exception_not_sufficiently_instantiated("sort/2: Arguments are not sufficiently instantiated"));
	}

	if (!interp.is_list(arg0)) {
            interp.abort(interpreter_exception_not_sufficiently_instantiated("sort/2: First argument is not a list; found " + interp.to_string(arg0)));
	}

	interp.add_accumulated_cost(interp.cost(arg0));

	size_t n = interp.list_length(arg0);

	std::vector<term> vec(n);

	for (size_t i = 0; i < n; i++) {
	    term el = interp.arg(arg0, 0);
	    vec[i] = el;
	    arg0 = interp.arg(arg0, 1);
	}

	std::stable_sort(vec.begin(), vec.end(),
			 [&](const term &t1, const term &t2)
			 { return interp.standard_order(t1,t2) < 0; } );

	vec.erase( std::unique(vec.begin(), vec.end()), vec.end());
	n = vec.size();

	// Build list from vector

	term r = interpreter_base::EMPTY_LIST;
	
	for (size_t i = 0; i < n; i++) {
	    term el = vec[n-i-1];
	    r = interp.new_dotted_pair(el, r);
	}

	bool ok = interp.unify(arg1, r);
	return ok;
    }

    //
    // Meta
    //

    bool builtins::call_n(interpreter_base &interp, size_t arity, common::term args[]) {
	// Get functor of args[0]
	term c = args[0];
	if (c.tag() != tag_t::STR && c.tag() != tag_t::CON) {
	    std::stringstream msg;
	    msg << "call/" << arity << ": First argument must be something callable; was " << interp.to_string(c);
	    interp.abort(interpreter_exception_wrong_arg_type(msg.str()));
	}
	con_cell f = interp.functor(args[0]);

	// Get number of extra arguments (this arity - 1)
	size_t num_extra_args = arity - 1;
	size_t f_arity = f.arity();
	size_t new_f_arity = f_arity + num_extra_args;
	con_cell new_f = f;
	
	if (num_extra_args > 0) {
	    new_f = interp.to_functor(interp.to_atom(f), new_f_arity);
	}

	term new_call = interp.new_term(new_f);

	// Copy existing arguments from 'f'
	for (size_t i = 0; i < f_arity; i++) {
	    interp.set_arg(new_call, i, interp.arg(c, i));
	}

	// Append extra arguments
	for (size_t i = 0; i < num_extra_args; i++) {
	    interp.set_arg(new_call, f_arity + i, args[i+1]);
	}

	// Setup new environment and where to continue
        interp.allocate_environment<ENV_NAIVE>();
	
	interp.set_p(new_call);
	interp.set_cp(interpreter_base::EMPTY_LIST);

	return true;
    }
    
    bool builtins::operator_disprove(interpreter_base &interp, size_t arity, common::term args[])
    {
	term arg = args[0];
	interp.new_meta_context<meta_context>(&operator_disprove_meta);

	interp.set_top_e();
	interp.allocate_choice_point(code_point::fail());
	interp.set_top_b(interp.b());
	interp.set_p(code_point(arg));
	interp.set_cp(interpreter_base::EMPTY_LIST);

	return true;
    }

    bool builtins::operator_disprove_meta(interpreter_base &interp,
					  const meta_reason_t &reason)
    {
	if (reason == interp::meta_reason_t::META_DELETE) {
	    interp.unwind_to_top_choice_point();
	    interp.release_last_meta_context();
	    return true;
	} 

        bool failed = interp.is_top_fail();

	interp.unwind_to_top_choice_point();
	interp.release_last_meta_context();

	//	if (interp.e0() != interp.top_e()) {
	//	    interp.deallocate_environment();
	//	}
	interp.set_p(interp.cp());
	interp.set_cp(interpreter_base::EMPTY_LIST);

	interp.set_top_fail(false);
	interp.set_complete(false);

	// Note that this is "disprove," so its success is the reverse of
	// the underlying expression to succeed.
	return failed;
    }

    struct meta_context_findall : public meta_context {
	inline meta_context_findall(interpreter_base &interp, meta_fn fn)
	    : meta_context(interp, fn) { }
	size_t secondary_hb_;
        term template_;
	term result_;
	term interim_;
	term tail_;
    };


    bool builtins::findall_3(interpreter_base &interp, size_t arity, common::term args[])
    {
	term qr = args[1];
	auto *context = interp.new_meta_context<meta_context_findall>(&findall_3_meta);
	context->template_ = args[0];
	context->result_ = args[2];
	context->interim_ = interpreter_base::EMPTY_LIST;
	context->tail_ = interpreter_base::EMPTY_LIST;
	context->secondary_hb_ = interp.secondary_env().get_register_hb();
	interp.secondary_env().set_register_hb(interp.secondary_env().heap_size());
	interp.set_top_e();
	interp.allocate_choice_point(code_point::fail());
	interp.set_top_b(interp.b());
	interp.set_p(code_point(qr));
	interp.set_cp(interpreter_base::EMPTY_LIST);
	
	return true;
    }

    bool builtins::findall_3_meta(interpreter_base &interp, const meta_reason_t &reason)
    {
	bool failed = interp.is_top_fail();

	auto *context = interp.get_current_meta_context<meta_context_findall>();

	if (reason == interp::meta_reason_t::META_DELETE) {
	    interp.secondary_env().trim_heap(interp.secondary_env().get_register_hb());
	    interp.secondary_env().set_register_hb(context->secondary_hb_);
	    interp.release_last_meta_context();
	    return true;
	} 

	interp.set_complete(false);

	if (failed) {
	    interp.unwind_to_top_choice_point();
	    term result = interp.copy(context->interim_, interp.secondary_env());
	    term output = context->result_;
	    interp.secondary_env().trim_heap(interp.secondary_env().get_register_hb());
	    interp.secondary_env().set_register_hb(context->secondary_hb_);
	    interp.release_last_meta_context();
	    if (interp.e0() != interp.top_e()) {
		interp.deallocate_environment();
	    }
	    interp.set_p(interp.cp());
	    interp.set_cp(interpreter_base::EMPTY_LIST);

	    interp.set_top_fail(false);
	    interp.set_complete(false);

	    return interp.unify(result, output);
	}

	uint64_t cost = 0;
	auto elem = interp.secondary_env().copy(context->template_, interp, cost);
        interp.add_accumulated_cost(cost);
	auto newtail = interp.secondary_env().new_dotted_pair(elem, interpreter_base::EMPTY_LIST);
	if (interp.secondary_env().is_empty_list(context->interim_)) {
	    context->interim_ = newtail;
	    context->tail_ = context->interim_;
	} else {
	    interp.secondary_env().set_arg(context->tail_, 1, newtail);
	    context->tail_ = newtail;
	}

	interp.set_p(common::con_cell("fail",0));
	interp.set_cp(interpreter_base::EMPTY_LIST);

	return true;
    }

    bool builtins::freeze_2(interpreter_base &interp, size_t arity, common::term args[])
    {
        term v = args[0];
	if (!v.tag().is_ref()) {
	    interp.set_p(args[1]);
	    interp.set_cp(interpreter_base::EMPTY_LIST);
	    return true;
	}

	args[1] = interp.rewrite_freeze_body(args[0], args[1]);

	// At this point args[1] should be '$freeze':<id>( .... )

	interp.set_frozen_closure(static_cast<ref_cell &>(args[0]).index(), args[1]);

	interp.set_p(interp.cp());
	interp.set_cp(interpreter_base::EMPTY_LIST);
        
        return true;
    }

    bool builtins::use_module_1(interpreter_base &interp, size_t arity, common::term args[] ) {
	static const con_cell LIB("library", 1);

	if (args[0].tag() != tag_t::STR || interp.functor(args[0]) != LIB) {
	    std::string msg = "use_module/1: "
	      "Only use_module(library(LibraryName)) is supported; was "
		+ interp.to_string(args[0]);
	    interp.abort(interpreter_exception_wrong_arg_type(msg));
	}

	auto name_term = interp.arg(args[0], 0);

	if (name_term.tag() != tag_t::CON) {
	    std::stringstream msg;
	    msg << "use_module/1: Name must be a proper atom; was " << interp.to_string(name_term);
	    interp.abort(interpreter_exception_wrong_arg_type(msg.str()));
	}

	auto name = reinterpret_cast<con_cell &>(name_term);

	auto &qnames = interp.get_module(name);
	if (qnames.empty()) {
	    std::stringstream msg;
	    msg << "use_module/1: Library '" << interp.to_string(name) << "' does not exist";
	    interp.abort(interpreter_exception_wrong_arg_type(msg.str()));
	}

	for (auto &qn : qnames) {
	    auto &cp = interp.get_code(qn);
	    qname imported_qn(interpreter_base::EMPTY_LIST, qn.second);
	    interp.set_code(imported_qn, cp);
	}

	return true;
    }

    bool builtins::frozen_2(interpreter_base &interp, size_t arity, common::term args[] ) {

        term addr_term = args[0];
	if (addr_term.tag() != common::tag_t::INT) {
	    std::string msg = "frozen/2: "
	      "First argument was not an integer representing a heap address; was "
	      + interp.to_string(addr_term);
	    interp.abort(interpreter_exception_wrong_arg_type(msg));
	}

	auto addr = static_cast<int_cell &>(addr_term).value();
	if (addr < 0) {
	    std::string msg = "frozen/2: "
	      "Integer must be a positive integer; was "
	      + interp.to_string(addr_term);
	    interp.abort(interpreter_exception_wrong_arg_type(msg));
	}
	
        auto *closure = interp.frozen_closures.find(addr);
	if (closure == nullptr) {
	    return false;
	}

	return interp.unify(args[1], *closure);
    }

    bool builtins::frozenk_2(interpreter_base &interp, size_t arity, common::term args[] ) {

        term k_term = args[0];
	if (k_term.tag() != common::tag_t::INT) {
	    std::string msg = "frozenk/2: "
	      "First argument was not an integer; was "
	      + interp.to_string(k_term);
	    interp.abort(interpreter_exception_wrong_arg_type(msg));
	}

	auto k = static_cast<int_cell &>(k_term).value();
	if (k < 0 || k > 255) {
	    std::string msg = "frozenk/2: "
	      "Integer was out of range. Must be within 0 and 255; was "
	      + interp.to_string(k_term);
	    interp.abort(interpreter_exception_wrong_arg_type(msg));
	}

	// Extract the K last heap positions for frozen closures

	term lst = interpreter_base::EMPTY_LIST;
	
	auto at_end = interp.frozen_closures.end();
	auto it = at_end - 1;
	while (k > 0 && it != at_end) {
	    auto heap_address = static_cast<int64_t>(it->key());
  	    lst = interp.new_dotted_pair(int_cell(heap_address), lst);
	    --it;
	}

	return interp.unify(args[1], lst);
    }

    bool builtins::defrost_3(interpreter_base &interp, size_t arity, common::term args[] ) {

        term addr_term = args[0];
	if (addr_term.tag() != common::tag_t::INT) {
	    std::string msg = "defrost/3: "
	      "First argument was not an integer representing a heap address; was "
	      + interp.to_string(addr_term);
	    interp.abort(interpreter_exception_wrong_arg_type(msg));
	}

	auto addr = static_cast<int_cell &>(addr_term).value();
	if (addr < 0) {
	    std::string msg = "defrost/3: "
	      "Integer must be a positive integer; was "
	      + interp.to_string(addr_term);
	    interp.abort(interpreter_exception_wrong_arg_type(msg));
	}
	
        auto *closure = interp.frozen_closures.find(addr);
	if (closure == nullptr) {
	    return false;
	}

	// Third argument must be a list of values where the first value
	// is ground (the var freeze is frozen on.)

        term values_term = args[2];
	if (!interp.is_list(values_term) || !interp.is_dotted_pair(values_term)) {
	    std::string msg = "defrost/3: "
	      "Third argument must be a proper non-empty list of values; was "
	      + interp.to_string(values_term);
	    interp.abort(interpreter_exception_wrong_arg_type(msg));	  
	}

	term first_arg = interp.arg(values_term, 0);
	if (!interp.is_ground(first_arg)) {
	    std::string msg = "defrost/3: "
	      "First element in third argument must be ground; was "
	      + interp.to_string(first_arg);
	    interp.abort(interpreter_exception_wrong_arg_type(msg));
	}

	// Note that a closure is ':'('$freeze', TermWithVars)
	// Where TermWithVars is the actual closure.

	term closure_term = interp.arg(*closure, 1);

	if (!interp.unify(args[1], closure_term)) {
	    return false;
	}
	
	auto context = interp.save_term_state();
 
	size_t i = 0;
	size_t closure_arity = interp.functor(closure_term).arity();

	while (values_term != interpreter_base::EMPTY_LIST && i < closure_arity) {
	    term next_arg = interp.arg(values_term, 0);
	    term closure_arg = interp.arg(closure_term, i);

	    if (!interp.is_ground(closure_arg)) {
		if (!interp.unify(next_arg, closure_arg)) {
		    interp.restore_term_state(context);
		    return false;
		}
		values_term = interp.arg(values_term, 1);
	    }
	    i++;
	}

	return true;
    }
}}
