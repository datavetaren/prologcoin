#pragma once

#ifndef _interp_builtins_opt_hpp
#define _interp_builtins_opt_hpp

#include "../common/term.hpp"
#include <boost/logic/tribool.hpp>

namespace prologcoin { namespace interp {
    class interpreter_base;

    using namespace boost::logic;

    typedef std::function<tribool (interpreter_base &interp, size_t arity, common::term caller[])> builtin_opt;

    class builtins_opt {
    public:

        static tribool sort_2(interpreter_base &interp, size_t arity, common::term caller[]);
    };

}}

#endif


