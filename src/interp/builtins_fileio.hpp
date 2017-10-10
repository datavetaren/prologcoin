#pragma once

#ifndef _interp_builtins_fileio_hpp
#define _interp_builtins_fileio_hpp

#include "../common/term.hpp"

namespace prologcoin { namespace interp {
    class interpreter;

    class builtins_fileio {
    public:
        static bool open_3(interpreter &interp, size_t arity, common::term args[]);
        static bool read_2(interpreter &interp, size_t arity, common::term args[]);
        static bool close_1(interpreter &interp, size_t arity, common::term args[]);
        static bool at_end_of_stream_1(interpreter &interp, size_t arity, common::term args[]);
	static bool write_1(interpreter &interp, size_t arity, common::term args[]);
	static bool nl_0(interpreter &interp, size_t arity, common::term args[]);
    private:
        static size_t get_stream_id(interpreter &interp, common::term &stream,
		  		    const std::string &from_fun);
    };

}}

#endif


