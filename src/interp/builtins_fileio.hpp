#pragma once

#ifndef _interp_builtins_fileio_hpp
#define _interp_builtins_fileio_hpp

#include "../common/term.hpp"

namespace prologcoin { namespace interp {
    class interpreter_base;

    class builtins_fileio {
    public:
        static bool open_3(interpreter_base &interp, size_t arity, common::term args[]);
        static bool read_2(interpreter_base &interp, size_t arity, common::term args[]);
        static bool close_1(interpreter_base &interp, size_t arity, common::term args[]);
        static bool at_end_of_stream_1(interpreter_base &interp, size_t arity, common::term args[]);
	static bool write_1(interpreter_base &interp, size_t arity, common::term args[]);
	static bool write_2(interpreter_base &interp, size_t arity, common::term args[]);
	static bool nl_0(interpreter_base &interp, size_t arity, common::term args[]);
	static bool tell_1(interpreter_base &interp, size_t arity, common::term args[]);
	static bool told_0(interpreter_base &interp, size_t arity, common::term args[]);
	static bool format_2(interpreter_base &interp, size_t arity, common::term args[]);
	static bool sformat_3(interpreter_base &interp, size_t arity, common::term args[]);

    private:
        static size_t get_stream_id(interpreter_base &interp, common::term &stream,
		  		    const std::string &from_fun);

        static bool format_helper(const std::string &name, interpreter_base &interp, common::term format_term, common::term format_args, std::string &out);

    };

}}

#endif


