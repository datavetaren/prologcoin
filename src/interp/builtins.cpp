#include "builtins.hpp"
#include "interpreter_base.hpp"
#include "wam_interpreter.hpp"
#include <stdarg.h>
#include <boost/algorithm/string.hpp>

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
	interp.allocate_environment(false);
	interp.set_cp(code_point(args[1]));
	interp.allocate_environment(false);
	interp.set_p(code_point(args[0]));
	interp.set_cp(interp.empty_list());

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
	interp.allocate_environment(false);
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
	interp.allocate_environment(false);
	interp.set_cp(code_point(arg1));
	interp.allocate_environment(false);
	interp.set_cp(code_point(cut_if));
	interp.allocate_environment(false);
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
	interp.allocate_environment(false);
	interp.set_cp(code_point(if_true));
	interp.allocate_environment(false);
	interp.set_cp(code_point(cut_if));
	interp.allocate_environment(false);
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
	if (c < 0) {
	    term lt = con_cell("<",0);
	    return interp.unify(order, lt);
	} else if (c > 0) {
	    term gt = con_cell(">",0);
	    return interp.unify(order, gt);
	} else {
	    term eq = con_cell("=",0);
	    return interp.unify(order, eq);
	}
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
	return args[0].tag() == tag_t::REF;
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
       return interp.unify(to, r);
   }

    // TODO: cyclic_term/1 and acyclic_term/1

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
	return interp.unify(lhs, result);
    }	

    //
    // Analyzing & constructing terms
    //

    bool builtins::copy_term_2(interpreter_base &interp, size_t arity, common::term args[])
    {
	term arg1 = args[0];
	term arg2 = args[1];
	term copy_arg1 = interp.copy(arg1);
	return interp.unify(arg2, copy_arg1);
    }

    bool builtins::functor_3(interpreter_base &interp, size_t arity, common::term args[])
    {
	term t = args[0];
	term f = args[1];
	term a = args[2];

	switch (t.tag()) {
  	  case tag_t::REF:
            interp.abort(interpreter_exception_not_sufficiently_instantiated("functor/3: Arguments are not sufficiently instantiated"));
	    return false;
	  case tag_t::INT:
 	  case tag_t::BIG: {
	    term zero = int_cell(0);
	    return interp.unify(f, t) && interp.unify(a, zero);
	    }
	  case tag_t::STR:
  	  case tag_t::CON: {
	      con_cell tf = interp.functor(t);
	      term fun = interp.to_atom(tf);
	      term arity = int_cell(tf.arity());
	      return interp.unify(f, fun) && interp.unify(a, arity);
	    }
	}

        return false;
    }

   term builtins::deconstruct_write_list(interpreter_base &interp,
					 term &t, size_t index)
    {
	term empty_lst = interp.empty_list();
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
	    if (lst.tag() == tag_t::REF) {
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

	if (lhs.tag() == tag_t::REF && rhs.tag() == tag_t::REF) {
		interp.abort(interpreter_exception_not_sufficiently_instantiated("=../2: Arguments are not sufficiently instantiated"));
	}

	if (lhs.tag() == tag_t::REF) {
	    if (!interp.is_list(rhs)) {
		interp.abort(interpreter_exception_not_list("=../2: Second argument is not a list; found " + interp.to_string(rhs)));
	    }
	    size_t lst_len = interp.list_length(rhs);
	    if (lst_len == 0) {
		interp.abort(interpreter_exception_not_list("=../2: Second argument must be non-empty; found " + interp.to_string(rhs)));
	    }
	    term first_elem = interp.arg(rhs,0);
	    if (first_elem.tag() == tag_t::REF) {
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
		if (lst.tag() == tag_t::REF) {
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
	    term empty = interp.empty_list();
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

    //
    // Meta
    //

    bool builtins::operator_disprove(interpreter_base &interp, size_t arity, common::term args[])
    {
	term arg = args[0];
	interp.new_meta_context<meta_context>(&operator_disprove_post);

	interp.set_top_e();
	interp.allocate_choice_point(code_point::fail());
	interp.set_top_b(interp.b());
	interp.set_p(code_point(arg));
	interp.set_cp(interp.empty_list());

	return true;
    }

    bool builtins::operator_disprove_post(interpreter_base &interp,
					  meta_context *context)
    {
        bool failed = interp.is_top_fail();

	interp.unwind_to_top_choice_point();
	interp.release_last_meta_context();

	if (interp.e0() != interp.top_e()) {
	    interp.deallocate_environment();
	}
	interp.set_p(interp.cp());
	interp.set_cp(interp.empty_list());

	interp.set_top_fail(false);
	interp.set_complete(false);

	// Note that this is "disprove," so its success is the reverse of
	// the underlying expression to succeed.
	return failed;
    }

    struct meta_context_findall : public meta_context {
	size_t secondary_hb_;
        term template_;
	term result_;
	term interim_;
	term tail_;
    };


    bool builtins::findall_3(interpreter_base &interp, size_t arity, common::term args[])
    {
	term qr = args[1];
	auto *mc = interp.new_meta_context<meta_context_findall>(&findall_3_post);
	mc->template_ = args[0];
	mc->result_ = args[2];
	mc->interim_ = interp.secondary_env().empty_list();
	mc->tail_ = interp.secondary_env().empty_list();
	mc->secondary_hb_ = interp.secondary_env().get_register_hb();
	interp.secondary_env().set_register_hb(interp.secondary_env().heap_size());
	interp.set_top_e();
	interp.allocate_choice_point(code_point::fail());
	interp.set_top_b(interp.b());
	interp.set_p(code_point(qr));
	interp.set_cp(interp.empty_list());
	
	return true;
    }

    bool builtins::findall_3_post(interpreter_base &interp, meta_context *context)
    {
	bool failed = interp.is_top_fail();

	auto *mc = interp.get_last_meta_context<meta_context_findall>();

	interp.set_complete(false);

	if (failed) {
	    interp.unwind_to_top_choice_point();
	    term result = interp.copy(mc->interim_, interp.secondary_env());
	    term output = mc->result_;
	    interp.secondary_env().trim_heap(interp.secondary_env().get_register_hb());
	    interp.secondary_env().set_register_hb(mc->secondary_hb_);
	    interp.release_last_meta_context();
	    if (interp.e0() != interp.top_e()) {
		interp.deallocate_environment();
	    }
	    interp.set_p(interp.cp());
	    interp.set_cp(interp.empty_list());

	    interp.set_top_fail(false);
	    interp.set_complete(false);

	    return interp.unify(result, output);
	}

	auto elem = interp.secondary_env().copy(mc->template_, interp);
	auto newtail = interp.secondary_env().new_dotted_pair(
			      elem, interp.secondary_env().empty_list());
	if (interp.secondary_env().is_empty_list(mc->interim_)) {
	    mc->interim_ = newtail;
	    mc->tail_ = mc->interim_;
	} else {
	    interp.secondary_env().set_arg(mc->tail_, 1, newtail);
	    mc->tail_ = newtail;
	}

	interp.set_p(common::con_cell("fail",0));
	interp.set_cp(interp.empty_list());

	return true;
    }

}}
