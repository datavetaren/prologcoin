#include "builtins_fileio.hpp"
#include "interpreter_base.hpp"
#include <boost/filesystem.hpp>
#include <fstream>

namespace prologcoin { namespace interp {

    using namespace prologcoin::common;

    static std::unordered_map<size_t, std::istream> file_map_;

    bool builtins_fileio::open_3(interpreter_base &interp, size_t arity, term args[])
    {
        term filename0 = args[0];
	term mode0 = args[1];
	term stream = args[2];

	if (!interp.is_atom(filename0)) {
	    interp.abort(interpreter_exception_wrong_arg_type(
		      "open/3: Filename must be an atom; was: "
		      + interp.to_string(filename0)));
	}

	if (!interp.is_atom(mode0)) {
	    interp.abort(interpreter_exception_wrong_arg_type(
		      "open/3: Mode must be an atom; was: "
		      + interp.to_string(mode0)));
	}

	std::string full_path = interp.get_full_path(interp.atom_name(filename0));
	std::string mode = interp.atom_name(mode0);

	if (!boost::filesystem::exists(full_path)) {
	    interp.abort(interpreter_exception_file_not_found(
		    "open/3: File '" + full_path + "' not found"));
	}

	if (mode != "read" && mode != "write") {
	    interp.abort(interpreter_exception_wrong_arg_type(
		    "open/3: Mode must be 'read' or 'write'; was: " + mode));
	}

	if (mode == "read") {
	    file_stream &fs = interp.new_file_stream(full_path);
	    fs.open(file_stream::READ);
	    size_t id = fs.get_id();
	    con_cell f = interp.functor("$stream", 1);
	    term newstream = interp.new_term(f, {int_cell(id)} );
	    return interp.unify(stream, newstream);
	}

	// TODO: Mode write...

	return false;
    }

    size_t builtins_fileio::get_stream_id(interpreter_base &interp, term &stream,
					  const std::string &from_fun)
    {
	if (!interp.is_functor(stream, con_cell("$stream", 1))) {
	    interp.abort(interpreter_exception_wrong_arg_type(
		      from_fun + ": Expected stream argument; was: "
		      + interp.to_string(stream)));
	}
	term stream_id = interp.arg(stream, 0);
	if (stream_id.tag() != tag_t::INT) {
	    interp.abort(interpreter_exception_wrong_arg_type(
		      from_fun + ": Unrecognized stream identifier: "
		      + interp.to_string(stream_id)));
	}

	cell sid = stream_id;
	int_cell &intid = static_cast<int_cell &>(sid);
	size_t id = intid.value();
	if (!interp.is_file_id(id)) {
	    interp.abort(interpreter_exception_file_not_found(
		      from_fun + ": Identifier is not an open file: " +
		      boost::lexical_cast<std::string>(id)));
	}

	return id;
    }

    bool builtins_fileio::close_1(interpreter_base &interp, size_t arity, term args[])
    {
	term stream = args[0];
	size_t id = get_stream_id(interp, stream, "close/1");
	interp.close_file_stream(id);
	return true;
    }

    bool builtins_fileio::read_2(interpreter_base &interp, size_t arity, term args[])
    {
	term stream = args[0];

	size_t id = get_stream_id(interp, stream, "read/2");
	file_stream &fs = interp.get_file_stream(id);
	term t;
	if (fs.is_eof()) {
	    t = interp.functor("end_of_file",0);
	} else {
	    t = fs.read_term();
	}
	term result = args[1];
	bool r = interp.unify(t, result);

	return r;
    }

    bool builtins_fileio::at_end_of_stream_1(interpreter_base &interp, size_t arity, term args[])
    {
	term stream = args[0];
	size_t id = get_stream_id(interp, stream, "at_end_of_stream/1");
	file_stream &fs = interp.get_file_stream(id);
	return fs.is_eof();
    }

    bool builtins_fileio::write_1(interpreter_base &interp, size_t arity, term args[])
    {
	term arg = args[0];
	interp.standard_output().write_term(arg);
	return true;
    }
    
    bool builtins_fileio::nl_0(interpreter_base &interp, size_t arity, term args[])
    {
	interp.standard_output().nl();
	return true;
    }

    bool builtins_fileio::tell_1(interpreter_base &interp, size_t arity, term args[])
    {
        term arg = args[0];

	if (!interp.is_atom(arg)) {
	    interp.abort(interpreter_exception_wrong_arg_type(
		      "tell/1: Argument must be an atom; was: "
		      + interp.to_string(arg)));
	}

	std::string full_path = interp.get_full_path(interp.atom_name(arg));
	file_stream &fs = interp.new_file_stream(full_path);
	fs.open(file_stream::WRITE);
	interp.tell_standard_output(fs);

	return true;
    }

    bool builtins_fileio::told_0(interpreter_base &interp, size_t arity, term args[])
    {
	if (!interp.has_told_standard_outputs()) {
	    interp.abort(interpreter_exception_nothing_told(
	        "told/0: Missing 'tell' for this 'told' operation"));
	}
	int id = interp.standard_output().get_id();
	interp.told_standard_output();
	interp.close_file_stream(id);

	return true;
    }
}}
