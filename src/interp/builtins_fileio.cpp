#include "builtins_fileio.hpp"
#include "interpreter.hpp"

namespace prologcoin { namespace interp {

    using namespace prologcoin::common;

    bool builtins_fileio::open_3(interpreter &interp, term &caller)
    {
        term filename = interp.env().arg(caller, 0);
	term mode = interp.env().arg(caller, 1);
	term stream = interp.env().arg(caller, 2);

	std::cout << "open/3: filename=" << interp.env().to_string(filename)
		  << " mode=" << interp.env().to_string(mode)
		  << " stream=" << interp.env().to_string(stream) << std::endl;

	return true;
    }

    // TODO: cyclic_term/1 and acyclic_term/1
}}
