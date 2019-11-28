#include "builtins.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace global {

void builtins::load(interpreter_base &interp, con_cell *module0)
{
    const con_cell M0("g", 0);
    const con_cell M = (module0 == nullptr) ? M0 : *module0;

    interp.load_builtin(M, con_cell("commit", 1), &builtins::commit_1);
}
    
bool builtins::commit_1(interpreter_base &interp, size_t arity, term args[] ) {
    return false;    
}

}}
