#include "global_interpreter.hpp"
#include "builtins.hpp"

namespace prologcoin { namespace global {

global_interpreter::global_interpreter() {
    builtins::load(*this);
}

}}
