#pragma once

#ifndef _interp_builtins_hpp
#define _interp_builtins_hpp

#include "../common/term.hpp"

namespace prologcoin { namespace interp {
    class interpreter;

    typedef std::function<bool (interpreter &interp, common::term &caller)> builtin;

    class builtins {
    public:
        // operator @<
        static bool operator_at_less_than(interpreter &interp, common::term &caller);
        // operator @=<
        static bool operator_at_equals_less_than(interpreter &interp, common::term &caller);
        // operator @>
        static bool operator_at_greater_than(interpreter &interp, common::term &caller);
        // operator @>=
        static bool operator_at_greater_than_equals(interpreter &interp, common::term &caller);
        // operator ==
        static bool operator_equals(interpreter &interp, common::term &caller);
	// operator =
	static bool operator_unification(interpreter &interp, common::term &caller);
    };

}}

#endif


