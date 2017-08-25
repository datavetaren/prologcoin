#pragma once

#ifndef _interp_arithmetics_hpp
#define _interp_arithmetics_hpp

#include "../common/term.hpp"

namespace prologcoin { namespace interp {
    class interpreter;

    class arithmetics_fn {
    public:
	static common::term plus_2(interpreter &interp, common::term *args);
	static common::term minus_2(interpreter &interp, common::term *args);
	static common::term times_2(interpreter &interp, common::term *args);
    private:
	static common::int_cell get_int(const common::term &t);
    };


    class arithmetics {
        typedef std::function<common::term (interpreter &interp,
				            common::term *args)> fn;

    public:
        arithmetics(interpreter &interp) : interp_(interp), debug_(false)
 	   { }

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

	interpreter &interp_;
	std::vector<common::term> args_;

	std::unordered_map<common::con_cell, fn> fn_map_;

	inline bool is_debug() const { return debug_; }

	bool debug_;

    };


}}

#endif
