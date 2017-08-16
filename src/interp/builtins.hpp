#pragma once

#ifndef _interp_builtins_hpp
#define _interp_builtins_hpp

#include <istream>
#include <vector>
#include "../common/term_env.hpp"

namespace prologcoin { namespace interp {

class builtin {

private:
    term_env_ &env_;
};

}}

#endif


