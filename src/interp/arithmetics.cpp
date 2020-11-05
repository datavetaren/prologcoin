#include "builtins.hpp"
#include "interpreter_base.hpp"
#include "arithmetics.hpp"

namespace prologcoin { namespace interp {

    using namespace prologcoin::common;

    int_cell arithmetics_fn::get_int(const term &t)
    {
	cell c = t;
	const int_cell &ic = static_cast<const int_cell &>(c);
	return ic;
    }

    term arithmetics_fn::plus_2(interpreter_base &interp, term *args)
    {
	return get_int(args[0]) + get_int(args[1]);
    }

    term arithmetics_fn::minus_2(interpreter_base &interp, term *args)
    {
	return get_int(args[0]) - get_int(args[1]);
    }

    term arithmetics_fn::times_2(interpreter_base &interp, term *args)
    {
	return get_int(args[0]) * get_int(args[1]);
    }

    term arithmetics_fn::div0_2(interpreter_base &interp, term *args)
    {
	auto a = get_int(args[0]);
	auto b = get_int(args[1]);
	if (b.is_zero()) {
	    throw interpreter_exception_division_by_zero(
			"// /2: attempt to divide by zero");
	}
	auto s = a.sign()*b.sign();
	auto v = a.abs()/b.abs();
	return int_cell(s*v);
    }

    term arithmetics_fn::div_2(interpreter_base &interp, term *args) {
	auto b = get_int(args[1]);
	if (b.is_zero()) {
	    throw interpreter_exception_division_by_zero(
			"div/2: attempt to divide by zero");
	}
	auto d = get_int(div0_2(interp, args));
	auto r = get_int(rem_2(interp, args));
	if (r.is_negative()) {
	    --d;
	}
	return d;
    }

    term arithmetics_fn::rem_2(interpreter_base &interp, term *args)
    {
	auto a = get_int(args[0]);
	auto b = get_int(args[1]);
	if (b.is_zero()) {
	    throw interpreter_exception_division_by_zero(
			"rem/2: attempt to divide by zero");
	}
	auto r = int_cell((a.abs().value() % b.abs().value()))*a.sign();
	return int_cell(r);
    }
	
    term arithmetics_fn::mod_2(interpreter_base &interp, term *args)
    {
	auto a = get_int(args[0]);
	auto b = get_int(args[1]);
	if (b.is_zero()) {
	    throw interpreter_exception_division_by_zero(
			"mod/2: attempt to divide by zero");
	}
	if (a.is_negative()) {
	    if (b.is_negative()) {
		return rem_2(interp, args);
	    } else {
		auto r = get_int(rem_2(interp, args));
		if (r.is_zero()) return r;
		return r + b;
	    }
	} else {
	    if (b.is_negative()) {
		auto r = get_int(rem_2(interp, args));
		if (r.is_zero()) return r;
		return r + b;
	    } else {
		return rem_2(interp, args);
	    }
	}
    }

    void arithmetics::total_reset() {
	fn_map_.clear();
	args_.clear();
	debug_ = false;
    }

    void arithmetics::load_fn(const std::string &name, size_t arity, arithmetics::fn fn)
    {
	con_cell f = interp_.functor(name, arity);
	fn_map_[f] = fn;
    }

    void arithmetics::load_fns()
    {
	if (fn_map_.size() != 0) {
	    return;
	}
        load_fn("+", 2, &arithmetics_fn::plus_2);
        load_fn("-", 2, &arithmetics_fn::minus_2);
        load_fn("*", 2, &arithmetics_fn::times_2);
	load_fn("mod", 2, &arithmetics_fn::mod_2);
	load_fn("rem", 2, &arithmetics_fn::rem_2);
	load_fn("div", 2, &arithmetics_fn::div_2);
	load_fn("//", 2, &arithmetics_fn::div0_2);
    }

    void arithmetics::unload()
    {
	fn_map_.clear();
	args_.clear();
    }

    term arithmetics::eval(term &expr,
			   const std::string &context)
    {
	load_fns();

	term result = expr;
	size_t stack_start = interp_.stack_size();
	interp_.push(expr);
	interp_.push(int_cell(0));
	while (stack_start < interp_.stack_size()) {
	    cell visited0 = interp_.pop();
	    term t = interp_.pop();
	    const int_cell &visited = static_cast<const int_cell &>(visited0);
            if (t.tag() == tag_t::INT || t.tag() == tag_t::BIG) {
		args_.push_back(t);
		continue;
	    }
	    if (visited.value() != 0) {
		cell c = t;
		const con_cell &f = static_cast<const con_cell &>(c);
		size_t arity = f.arity();
		auto fn_call = lookup(f);
		if (fn_call == nullptr) {
		    interp_.abort(interpreter_exception_undefined_function(
			   context + ": Undefined function: " +
			   interp_.atom_name(f) + "/" +
			   boost::lexical_cast<std::string>(arity) +
			   " in " + interp_.safe_to_string(expr)));
		}
		size_t end = args_.size();
		size_t off = end - arity;
		if (is_debug()) {
		    std::cout << "arithmetics::eval(): " << 
    		        interp_.atom_name(f) + "/" +
			boost::lexical_cast<std::string>(arity) + "(";
		    bool first = true;
		    for (size_t i = off; i < end; i++) {
			if (!first) std::cout << ", ";
			std::cout << interp_.to_string(args_[i]);
			first = false;
		    }
		    std::cout << ")\n";
		}
		result = fn_call(interp_, &args_[off]);
		args_.resize(off);
		interp_.push(result);
		interp_.push(int_cell(1));
		continue;
	    }
	    switch (t.tag()) {
	    case tag_t::CON:
	    case tag_t::STR: {
		con_cell f = interp_.functor(t);
		interp_.push(f);
		interp_.push(int_cell(1));
		size_t n = f.arity();
		for (size_t i = 0; i < n; i++) {
		    term arg = interp_.arg(t, n - i - 1);
		    interp_.push(arg);
		    interp_.push(int_cell(0));
		}
		break;
	    }
	    case tag_t::REF: case tag_t::RFW: {
		interp_.abort(interpreter_exception_not_sufficiently_instantiated(context + ": Arguments are not sufficiently instantiated"));
		break;
	    }
	    case tag_t::BIG: {
		interp_.abort(interpreter_exception_unsupported(context + ": Big integers are unsupported."));
		break;
	    }
	    case tag_t::INT: {
		assert(false); // Should not occur
		break;
	    }
	    }
	}
	args_.clear();
	return result;
    }

    //
    // Simple
    //

    int_cell arithmetics::get_int_arg_type(term &arg,
					   const std::string &context)
    {
	if (arg.tag() != tag_t::INT) {
	    interp_.abort(interpreter_exception_argument_not_number(
			      context + ": argument is not a number: " +
			      interp_.safe_to_string(arg)));
	}
	cell c = arg;
	int_cell &r = static_cast<int_cell &>(c);
	return r;
    }

}}
