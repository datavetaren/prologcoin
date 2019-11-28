#pragma once

#ifndef _global_global_hpp
#define _global_global_hpp

#include "../common/term_env.hpp"
#include "global_interpreter.hpp"

namespace prologcoin { namespace global {

//
// global. This class captures the global state that everybody shares
// in the network.
class global {
private:
    using term_env = prologcoin::common::term_env;
  
public:
    global();
    inline term_env & env() { return interp_; }

private:
    global_interpreter interp_;
};

}}

#endif
