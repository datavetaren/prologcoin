#include "builtins.hpp"
#include "interpreter_base.hpp"
#include "wam_interpreter.hpp"
#include <stdarg.h>

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
    // Simple
    //

    bool builtins::true_0(interpreter_base &interp, size_t arity, common::term args[])
    {
	return true;
    }

    //
    // Control flow
    //

    // TODO: This needs to be modified to use callbacks.
    bool builtins::operator_comma(interpreter_base &interp, size_t arity, common::term args[])
    {
	interp.allocate_environment(false);
        interp.set_cp(code_point(args[1]));
	interp.set_p(code_point(args[0]));
	return true;
    }

    // TODO: This needs to be modified to use callbacks. Cut should be a primitive.
    bool builtins::operator_cut(interpreter_base &interp, size_t arity, common::term args[])
    {
        if (interp.has_late_choice_point()) {
	    interp.reset_choice_point();
	    interp.tidy_trail();
	}
        return true;
    }

    // TODO: This needs to be modified to use callbacks. Cut should be a primitive.
    bool builtins::operator_cut_if(interpreter_base &interp, size_t arity, common::term args[])
    {
        if (interp.has_late_choice_point()) {
	    interp.reset_choice_point();
	    interp.tidy_trail();
	}
	auto *ch = interp.get_last_choice_point();
	ch->bp = code_point::fail(); // Don't back track to false clause

        return true;
    }

    bool builtins::operator_deallocate_and_proceed(interpreter_base &interp, size_t arity, common::term args[])
    {
	interp.set_p(interp.cp());
        interp.deallocate_environment();

        return true;
    }

    // TODO: This needs to be modified to use callbacks. 
    bool builtins::operator_disjunction(interpreter_base &interp, size_t arity, common::term args[])
    {
	static con_cell arrow("->", 2);
	static con_cell deallocate_and_proceed_op("_#",0);

	// Check if this is an if-then-else
	term arg0 = args[0];
	if (interp.functor(arg0) == arrow) {
	    return operator_if_then_else(interp, arity, args);
	}

	term arg1 = args[1];
	interp.allocate_environment(false);
	interp.set_cp(interp.p());
	interp.allocate_environment(false);
	interp.set_cp(code_point(deallocate_and_proceed_op));
        interp.allocate_choice_point(code_point(arg1));
	interp.set_p(code_point(arg0));
	return true;
    }

    // TODO: This needs to be modified to use callbacks.
    bool builtins::operator_if_then(interpreter_base &interp, size_t arity, common::term args[])
    {
	static con_cell cut_op("!",0);

	interp.allocate_choice_point(code_point::fail());
	interp.move_cut_point_to_last_choice_point();
	term cut = cut_op;
	term arg0 = args[0];
	term arg1 = args[1];
	interp.allocate_environment(false);
	interp.set_cp(interp.p());
	interp.allocate_environment(false);
	interp.set_cp(code_point(arg1));
	interp.allocate_environment(false);
	interp.set_cp(code_point(cut));
	interp.allocate_environment(false);
	interp.set_p(code_point(arg0));
        return true;
    }

    // TODO: This needs to be modified to use callbacks.
    bool builtins::operator_if_then_else(interpreter_base &interp, size_t arity, common::term args[])
    {
	static con_cell cut_op_if("_!",0);
	static con_cell deallocate_and_proceed_op("_#",0);

	term lhs = args[0];

	term cond = interp.arg(lhs, 0);
	term if_true = interp.arg(lhs, 1);
	term if_false = args[1];
	term cut_if = cut_op_if;

	interp.allocate_environment(false);
	interp.set_cp(interp.p());
	interp.allocate_environment(false);
	interp.set_cp(code_point(deallocate_and_proceed_op));
	// Go to 'C' the false clause if ((A->B) ; C) fails
	interp.allocate_choice_point(code_point(if_false));
	interp.move_cut_point_to_last_choice_point();
	interp.allocate_environment(false);
	interp.set_cp(code_point(if_true));
	interp.allocate_environment(false);
	interp.set_cp(code_point(cut_if));
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

    // TODO: cyclic_term/1 and acyclic_term/1

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
	      term fun = tf;
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
	interp.allocate_environment(false);
	interp.set_qr(arg);
	interp.set_p(code_point(arg));
	interp.set_cp(interp.empty_list());

	return true;
    }

    void builtins::operator_disprove_post(interpreter_base &interp,
					  meta_context *context)
    {
        bool failed = interp.is_top_fail();

        interp.deallocate_environment();
	interp.unwind_to_top_choice_point();
	interp.release_last_meta_context();

	// Note that this is "disprove," so its success is the reverse of
	// the underlying expression to succeed.
	interp.set_top_fail(!failed);
	if (!failed) {
	    interp.set_p(code_point(interp.empty_list()));
        }
    }

}}
