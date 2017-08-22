#pragma once

#ifndef _interp_builtins_fileio_hpp
#define _interp_builtins_fileio_hpp

#include "../common/term.hpp"

namespace prologcoin { namespace interp {
    class interpreter;

    class builtins_fileio {
    public:
        static bool open_3(interpreter &interp, common::term &caller);
        static bool read_2(interpreter &interp, common::term &caller);
        static bool close_1(interpreter &interp, common::term &caller);
    };

}}

#endif


