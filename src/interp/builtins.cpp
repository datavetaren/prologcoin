#include "builtins.hpp"
#include "interpreter_base.hpp"
#include "wam_interpreter.hpp"
#include "../common/checked_cast.hpp"
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

bool builtins::upcase_atom_2(interpreter_base &interp, size_t arity, common::term args[]) {

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
	case tag_t::CON: {
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
	    term deref_term = interp.deref(current_term);
	    if (deref_term == current_term && current_tag.is_ref()) {
	        return false;
	    }
	    workstack.push_front(std::pair<term, int>(deref_term, 0));
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

bool builtins::greater_than_equals_2(interpreter_base &interp, size_t, common::term args[])
{
    term lhs = args[0];
    term rhs = args[1];
    term result_lhs = interp.arith().eval(lhs, ">=/2");
    term result_rhs = interp.arith().eval(rhs, ">=/2");
    return interp.standard_order(result_lhs, result_rhs) >= 0;
}

bool builtins::less_than_equals_2(interpreter_base &interp, size_t, common::term args[])
{
    term lhs = args[0];
    term rhs = args[1];
    term result_lhs = interp.arith().eval(lhs, "=</2");
    term result_rhs = interp.arith().eval(rhs, "=</2");
    return interp.standard_order(result_lhs, result_rhs) <= 0;
}


bool builtins::greater_than_2(interpreter_base &interp, size_t, common::term args[])
{
    term lhs = args[0];
    term rhs = args[1];
    term result_lhs = interp.arith().eval(lhs, ">/2");
    term result_rhs = interp.arith().eval(rhs, ">/2");
    return interp.standard_order(result_lhs, result_rhs) > 0;
}

bool builtins::less_than_2(interpreter_base &interp, size_t, common::term args[])
{
    term lhs = args[0];
    term rhs = args[1];
    term result_lhs = interp.arith().eval(lhs, "</2");
    term result_rhs = interp.arith().eval(rhs, "</2");
    return interp.standard_order(result_lhs, result_rhs) < 0;
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
					 term &t, size_t index) {
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

bool builtins::operator_deconstruct(interpreter_base &interp, size_t arity, common::term args[]) {
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

    interp.use_module(name);

    return true;
}

bool builtins::module_1(interpreter_base &interp, size_t arity, common::term args[] ) {
    if (args[0].tag() != tag_t::STR && args[0].tag() != tag_t::CON) {
        std::string msg = "module/1: "
	  "Module name must be an atom; was " + interp.to_string(args[0]);
	interp.abort(interpreter_exception_wrong_arg_type(msg));
    }
    con_cell name = interp.functor(args[0]);
    interp.set_current_module(name);
    
    return true;
}

term builtins::get_frozen(interpreter_base &interp, common::term arg)
{
    if (arg.tag() != common::tag_t::INT) {
        std::string msg = "frozen/2: "
	  "Argument was not an integer representing a heap address; was "
	  + interp.to_string(arg);
	interp.abort(interpreter_exception_wrong_arg_type(msg));
    }

    auto addr = static_cast<int_cell &>(arg).value();
    if (addr < 0) {
        std::string msg = "frozen/2: "
	  "Integer must be a positive integer; was "
	  + interp.to_string(arg);
	interp.abort(interpreter_exception_wrong_arg_type(msg));
    }

    auto closure = interp.get_frozen_closure(addr);
    if (closure == interpreter_base::EMPTY_LIST) {
        std::stringstream msg;
	msg << "frozen/2: There was no frozen closure at " << interp.to_string(arg) << std::endl;
	interp.abort(interpreter_exception_wrong_arg_type(msg.str()));
    }
    return closure;
}

bool builtins::frozen_2(interpreter_base &interp, size_t arity, common::term args[] ) {

    term addr_term = args[0];
    if (addr_term.tag() != common::tag_t::INT && !interp.is_list(addr_term)) {
        std::string msg = "frozen/2: "
	  "First argument was not an integer (or a list of integers) "
	  "representing a heap address(es); was "
	      + interp.to_string(addr_term);
	interp.abort(interpreter_exception_wrong_arg_type(msg));
    }

    term result = interpreter_base::EMPTY_LIST;

    if (addr_term.tag() == common::tag_t::INT) {
        result = get_frozen(interp, addr_term);
    } else {
        auto lst = addr_term;
	auto tail_lst = result;
	while (interp.is_dotted_pair(lst)) {
            auto arg = interp.arg(lst, 0);
	    term closure = get_frozen(interp, arg);
	    auto next_lst = interp.new_dotted_pair(closure,
						   interpreter_base::EMPTY_LIST);
	    if (tail_lst == interpreter_base::EMPTY_LIST) {
	        tail_lst = next_lst;
		result = next_lst;
	    } else {
	        interp.set_arg(tail_lst, 1, next_lst);
		tail_lst = next_lst;
	    }
	    lst = interp.arg(lst, 1);
	}
    }

    return interp.unify(args[1], result);
}

bool builtins::frozenk_3(interpreter_base &interp, size_t arity, common::term args[] ) {

    term start_term = args[0];
    if (start_term.tag() != common::tag_t::INT) {
        std::string msg = "frozenk/3: "
	  "First argument, the starting heap address, was not an integer; was "
	  + interp.to_string(start_term);
	interp.abort(interpreter_exception_wrong_arg_type(msg));
    }
    int64_t start = static_cast<int_cell &>(start_term).value();
    if (start < -1) {
        std::string msg = "frozenk/3: "
	  "Starting address was out of range. "
	  "Must be positive number or -1 (for extracting from the end); "
	  + interp.to_string(start_term);
	interp.abort(interpreter_exception_wrong_arg_type(msg));
    }

    term k_term = args[1];
    if (k_term.tag() != common::tag_t::INT) {
        std::string msg = "frozenk/3: "
	  "Second argument, the number of frozen closures to extract, "
	  "is not an integer; was " + interp.to_string(k_term);
	interp.abort(interpreter_exception_wrong_arg_type(msg));
    }
    auto k = static_cast<int_cell &>(k_term).value();
    if (k < 0 || k > 255) {
        std::string msg = "frozenk/3: "
	  "Number of frozen closures was out of range. "
	  "Must be within 0 and 255; was "
	  + interp.to_string(k_term);
	interp.abort(interpreter_exception_wrong_arg_type(msg));
    }

    size_t start_heap_addr = start == -1 ? interp.heap_size() : checked_cast<size_t>(start);
    size_t end_heap_addr = interp.heap_size();
    
    interp.frozen_closure_fn_(interp, interpreter_base::LOAD_FROZEN_CLOSURE,
			      start_heap_addr,
			      end_heap_addr,
			      checked_cast<size_t>(k));

    // Extract the K heap positions frozen closures starting from address

    term lst = interpreter_base::EMPTY_LIST;

    if (start == -1) {
        auto it = interp.frozen_closures_.rbegin();
	auto at_end = interp.frozen_closures_.rend();
	while (k > 0 && it != at_end) {
	    auto heap_address = it->first;
	    lst = interp.new_dotted_pair(int_cell(heap_address), lst);
	    ++it;
	    k--;
	}
    } else {
        auto i_start = static_cast<size_t>(start);
        auto it = interp.frozen_closures_.lower_bound(i_start);
	auto at_end = interp.frozen_closures_.end();
	auto last = lst;
	while (k > 0 && it != at_end) {
  	    auto heap_address = it->first;
	    auto next_lst = interp.new_dotted_pair(int_cell(heap_address),
						   interpreter_base::EMPTY_LIST);
	    if (last == interpreter_base::EMPTY_LIST) {
	        lst = next_lst;
	        last = next_lst;
	    } else {
	        interp.set_arg(last, 1, next_lst);
		last = next_lst;
	    }
	    ++it;
	    k--;
	}
    }

    return interp.unify(args[2], lst);
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
	
    auto closure = interp.get_frozen_closure(addr);
    if (closure == interpreter_base::EMPTY_LIST) {
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

    term closure_term = interp.arg(closure, 1);

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

bool builtins::password_2(interpreter_base &interp, size_t arity, common::term args[])
{
    static con_cell SYSTEM("system",0);
    static con_cell PASSWD("$passwd",1);
    if (args[0].tag() == tag_t::REF) {
        auto &pred = interp.get_predicate(SYSTEM, PASSWD);
	if (pred.empty()) {
	    return false;
	}
	auto &clauses = pred.clauses();
	if (interp.functor(clauses[0].clause()) != PASSWD) {
	    return false;
	}
	auto head = interp.clause_head(clauses[0].clause());
	auto passwd = interp.arg(head,0);
	if (!interp.is_string(passwd)) {
	    return false;
	}
	if (arity > 1) {
	    bool is_pers = interp.is_persistent_password();
	    term opt = is_pers ? interp.functor("persistent",0) : interp.functor("temporary",0);
	    if (!interp.unify(args[1], opt)) {
	        return false;
	    }
	}
	return interp.unify(args[0], passwd);
    }

    bool is_pers = false;
    
    if (arity == 2) {
        term persistent_atom = interp.functor("persistent",0);
	term temporary_atom = interp.functor("temporary",0);
        if (args[1] == persistent_atom) {
	    is_pers = true;
        } else if (args[1] == temporary_atom) {
	    is_pers = false;
	} else {
	    std::stringstream msg;
	    msg << "password/" << arity << ": Second arugment must be the atom 'persistent' or 'temporary'; was " << interp.to_string(args[1]);
	    throw interpreter_exception_wrong_arg_type(msg.str());	  
	}
    }

    // First argument is bound; check that it is a string
    if (!interp.is_string(args[0])) {
        std::stringstream msg;
        msg << "password/" << arity << ": First argument must be a proper string; was " << interp.to_string(args[0]);
	throw interpreter_exception_wrong_arg_type(msg.str());
    }

    auto &pred = interp.get_predicate(interp, qname(SYSTEM, PASSWD));
    term clause = interp.new_term(PASSWD);
    interp.set_arg(clause, 0, args[0]);
    pred.clear();
    pred.add_clause(interp, clause);

    interp.set_persistent_password(is_pers);

    interp.new_roots();

    return true;
}

// Modifying program database

bool builtins::show_0(interpreter_base &interp, size_t arity, common::term args[]) {
    interp.print_db();
    return true;
}

bool builtins::asserta_1(interpreter_base &interp, size_t arity, common::term args[] ) {

    term clause = interp.copy(args[0]);
    interp.load_clause(clause, FIRST_CLAUSE);
    return true;
}

bool builtins::assertz_1(interpreter_base &interp, size_t arity, common::term args[] ) {

    term clause = interp.copy(args[0]);
    interp.load_clause(clause, LAST_CLAUSE);
    return true;
}

bool builtins::retract_1(interpreter_base &interp, size_t arity, common::term args[]) {
    return retract(interp, "retract/1", args[0], false);
}

bool builtins::retractall_1(interpreter_base &interp, size_t arity, common::term args[]) {
    return retract(interp, "retractall/1", args[0], true);
}

bool builtins::retract(interpreter_base &interp, const std::string &pname, term head, bool all) {
    static const common::con_cell COLON(":", 2);
    con_cell module = interp.current_module();
    if (head.tag() == tag_t::STR && interp.functor(head) == COLON) {
        auto module_term = interp.arg(head, 0);
	if (module_term.tag() != tag_t::CON) {
	    std::stringstream msg;
	    msg << pname << ": Expected an atom as module name; was " << interp.to_string(module_term);
	    throw interpreter_exception_wrong_arg_type(msg.str());
	}
	module = static_cast<con_cell &>(module_term);
        head = interp.arg(head, 1);
    }
    if (head.tag() != tag_t::STR && head.tag() != tag_t::CON) {
        std::stringstream msg;
	msg << pname << ": Expected a structure or an atom; was " << interp.to_string(head);
        throw interpreter_exception_wrong_arg_type(msg.str());
    }
    con_cell p = interp.functor(head);
    auto qn = qname{module,p};
    auto &pred = interp.get_predicate(interp, qn);
    bool r = pred.remove_clauses(interp, head, all);
    if (r) interp.add_updated_predicate(qn);
    return r;
}


qname builtins::check_predicate(interpreter_base &interp, const std::string &pname, term arg)
{
    static con_cell SLASH("/",2);

    con_cell module = interp.current_module();

    if (interp.is_functor(arg) &&
	interp.functor(arg) == interpreter_base::COLON) {
        // Extract module
        term module_term = interp.arg(arg, 0);
	if (!interp.is_atom(module_term)) {
	    throw interpreter_exception_wrong_arg_type(pname + ": Module name must be an atom; was " + interp.to_string(module_term));
	}
	module = reinterpret_cast<con_cell &>(module_term);
	arg = interp.arg(arg, 1);
    }
    
    if (!interp.is_functor(arg) || interp.functor(arg) != SLASH) {
        throw interpreter_exception_wrong_arg_type(pname + ": Only an instantiated term like f/a is supported.");
    }
    if (!interp.is_atom(interp.arg(arg, 0))) {
        throw interpreter_exception_wrong_arg_type(pname + ": Expected predicate name; was " + interp.to_string(interp.arg(arg,0)));
    }
    con_cell f = interp.functor(interp.arg(arg, 0));
    term arity_part = interp.arg(arg, 1);
    if (arity_part.tag() != tag_t::INT) {
        throw interpreter_exception_wrong_arg_type(pname + ": Expected arity as an integer of predicate; was " + interp.to_string(arity_part));
    }
    auto a = reinterpret_cast<int_cell &>(arity_part).value();
    if (a < 0 || a > con_cell::MAX_ARITY) {
        std::stringstream msg;
	msg << pname << ": Arity must be within 0 and " << con_cell::MAX_ARITY;
        throw interpreter_exception_wrong_arg_type(msg.str());
    }

    con_cell pred = interp.to_functor(f, a);

    qname qn(module, pred);
    return qn;
}

bool builtins::current_predicate_1(interpreter_base &interp, size_t arity, common::term args[])
{
    qname qn = check_predicate(interp, "current_predicate/1", args[0]);

    auto &cp = interp.get_code(qn);

    if (!cp.is_fail()) {
        auto module = cp.module();
	qn = qname(module, qn.second);
    }

    bool exists = !interp.get_predicate(qn).empty();

    return exists;
}

bool builtins::status_predicate_2(interpreter_base &interp, size_t arity, common::term args[])
{
    qname qn = check_predicate(interp, "status_predicate/2", args[0]);

    auto &cp = interp.get_code(qn);

    if (!cp.is_fail()) {
        auto module = cp.module();
	qn = qname(module, qn.second);
    }

    bool exists = !interp.get_predicate(qn).empty();    

    if (exists) {
        const predicate &p = interp.get_predicate(qn);
	if (p.empty()) return false;
	return interp.unify(args[1],int_cell(static_cast<int64_t>(p.performance_count())));
    }

    return false;
}


void builtins::load(interpreter_base &interp) {
    auto &i = interp;
  
    // Profiling
    i.load_builtin(con_cell("profile", 0), &builtins::profile_0);

    // Simple
    i.load_builtin(con_cell("true",0), &builtins::true_0);
    i.load_builtin(con_cell("fail",0), &builtins::fail_0);

    // Control flow
    i.load_builtin(con_cell(",",2), builtin(&builtins::operator_comma,true));
    i.load_builtin(con_cell("!",0), &builtins::operator_cut);
    i.load_builtin(con_cell("_!",0), &builtins::operator_cut_if);
    i.load_builtin(con_cell(";",2), builtin(&builtins::operator_disjunction,true));
    i.load_builtin(con_cell("->",2), builtin(&builtins::operator_if_then, true));

    // Standard order, equality and unification

    i.load_builtin(con_cell("@<",2), &builtins::operator_at_less_than);
    i.load_builtin(con_cell("@=<",2), &builtins::operator_at_equals_less_than);
    i.load_builtin(con_cell("@>",2), &builtins::operator_at_greater_than);
    i.load_builtin(con_cell("@>=",2), &builtins::operator_at_greater_than_equals);
    i.load_builtin(con_cell("==",2), &builtins::operator_equals);
    i.load_builtin(con_cell("\\==",2), &builtins::operator_not_equals);
    i.load_builtin(con_cell("compare",3), &builtins::compare_3);
    i.load_builtin(con_cell("=",2), &builtins::operator_unification);
    i.load_builtin(con_cell("\\=",2), &builtins::operator_cannot_unify);

    // Type tests
    i.load_builtin(con_cell("var",1), &builtins::var_1);
    i.load_builtin(con_cell("nonvar",1), &builtins::nonvar_1);
    i.load_builtin(con_cell("integer",1), &builtins::integer_1);
    i.load_builtin(con_cell("number",1), &builtins::number_1);
    i.load_builtin(con_cell("atom",1), &builtins::atom_1);
    i.load_builtin(con_cell("atomic",1), &builtins::atomic_1);
    i.load_builtin(i.functor("compound",1), &builtins::compound_1);
    i.load_builtin(i.functor("callable",1), &builtins::callable_1);
    i.load_builtin(con_cell("ground", 1), &builtins::ground_1);
    i.load_builtin(i.functor("cyclic_term", 1), &builtins::cyclic_term_1);
    i.load_builtin(i.functor("acyclic_term", 1), &builtins::acyclic_term_1);
    i.load_builtin(con_cell("is_list",1), &builtins::is_list_1);

    // Character properties
    i.load_builtin(i.functor("upcase_atom",2), &builtins::upcase_atom_2);
    i.load_builtin(i.functor("bytes_number",2), &builtins::bytes_number_2);

    // Arithmetics
    i.load_builtin(con_cell("is",2), &builtins::is_2);
    i.load_builtin(con_cell(">=",2), &builtins::greater_than_equals_2);
    i.load_builtin(con_cell("=<",2), &builtins::less_than_equals_2);
    i.load_builtin(con_cell(">",2), &builtins::greater_than_2);
    i.load_builtin(con_cell("<",2), &builtins::less_than_2);    

    // Analyzing & constructing terms
    i.load_builtin(con_cell("arg",3), &builtins::arg_3);
    i.load_builtin(i.functor("functor",3), &builtins::functor_3);
    i.load_builtin(i.functor("same_term", 2), &builtins::same_term_2);
    i.load_builtin(i.functor("copy_term",2), &builtins::copy_term_2);
    i.load_builtin(con_cell("=..", 2), &builtins::operator_deconstruct);
    i.load_builtin(con_cell("sort", 2), &builtins::sort_2);

    // Meta
    i.load_builtin(con_cell("\\+", 1), builtin(&builtins::operator_disprove,true));
    i.load_builtin(con_cell("findall",3), builtin(&builtins::findall_3,true));
    i.load_builtin(con_cell("freeze",2), builtin(&builtins::freeze_2,true));

    // call/n with n [1..11]
    for (size_t j = 1; j <= 11; j++) {
        i.load_builtin(con_cell("call", j), builtin(&builtins::call_n,true));
    }

    // System
    i.load_builtin(i.functor("module",1), builtin(&builtins::module_1,false));    
    i.load_builtin(i.functor("use_module",1), builtin(&builtins::use_module_1,false));

    // Non-standard
    i.load_builtin(con_cell("frozen",2), builtin(&builtins::frozen_2));
    i.load_builtin(con_cell("frozenk",3), builtin(&builtins::frozenk_3));
    i.load_builtin(con_cell("defrost",3), builtin(&builtins::defrost_3));
    i.load_builtin(i.functor("password",1), builtin(&builtins::password_2));
    i.load_builtin(i.functor("password",2), builtin(&builtins::password_2));

    // Program database
    i.load_builtin(con_cell("show",0), builtin(&builtins::show_0));
    i.load_builtin(con_cell("asserta",1), builtin(&builtins::asserta_1));
    i.load_builtin(con_cell("assertz",1), builtin(&builtins::assertz_1));
    i.load_builtin(con_cell("assert",1), builtin(&builtins::assertz_1));
    i.load_builtin(con_cell("retract",1), builtin(&builtins::retract_1));
    i.load_builtin(i.functor("retractall",1), builtin(&builtins::retractall_1));
    i.load_builtin(i.functor("current_predicate",1), builtin(&builtins::current_predicate_1));
    i.load_builtin(i.functor("status_predicate",2), builtin(&builtins::status_predicate_2));
    
}
 
}}
