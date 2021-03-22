#pragma once

#ifndef _interp_arithmetics_hpp
#define _interp_arithmetics_hpp

#include "../common/term.hpp"
#include "interpreter_exception.hpp"

namespace prologcoin { namespace interp {

class arithmetic_exception : public interpreter_exception 
{
public:
    arithmetic_exception(const std::string &msg)
	  : interpreter_exception(msg) { }    
};

class interpreter_exception_division_by_zero : public arithmetic_exception
{
public:
    interpreter_exception_division_by_zero(const std::string &msg)
	: arithmetic_exception(msg) { }
};
	
    class interpreter_base;

    class arithmetics_fn {
    public:
	static common::term plus_2(interpreter_base &interp, common::term *args);
	static common::term minus_2(interpreter_base &interp, common::term *args);
	static common::term times_2(interpreter_base &interp, common::term *args);
	static common::term mod_2(interpreter_base &interp, common::term *args);
	static common::term rem_2(interpreter_base &interp, common::term *args);
	static common::term div0_2(interpreter_base &interp, common::term *args);
	static common::term div_2(interpreter_base &interp, common::term *args);
	static common::term max_2(interpreter_base &interp, common::term *args);
	static common::term min_2(interpreter_base &interp, common::term *args);	
    private:
	static common::int_cell get_int(const common::term &t);
    };


    class arithmetics {
        typedef std::function<common::term (interpreter_base &interp,
				            common::term *args)> fn;

    public:
        arithmetics(interpreter_base &interp) : interp_(interp), debug_(false)
 	   { }

	void total_reset();

	inline void set_debug(bool dbg) { debug_ = dbg; }

	void unload();

	common::term eval(common::term &expr, const std::string &context);

    private:
	void load_fn(const std::string &name, size_t arity, fn f);
	void load_fns();

	inline fn lookup(common::con_cell c) {
	    return fn_map_[c];
	}

	common::int_cell get_int_arg_type(common::term &arg,
					  const std::string &context);

	interpreter_base &interp_;
	std::vector<common::term> args_;

	std::unordered_map<common::con_cell, fn> fn_map_;

	inline bool is_debug() const { return debug_; }

	bool debug_;

    };


}}

#endif
