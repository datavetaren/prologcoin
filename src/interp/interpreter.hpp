#pragma once

#ifndef _interp_interpreter_hpp
#define _interp_interpreter_hpp

#include "wam_interpreter.hpp"

namespace prologcoin { namespace interp {

class interpreter : public wam_interpreter
{
public:
  interpreter();
  ~interpreter();
};

}}

#endif

