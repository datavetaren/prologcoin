#include "builtins_fileio.hpp"
#include "interpreter.hpp"
#include <boost/filesystem.hpp>
#include <fstream>

namespace prologcoin { namespace interp {

    using namespace prologcoin::common;

    static std::unordered_map<size_t, std::istream> file_map_;

    bool builtins_fileio::open_3(interpreter &interp, term &caller)
    {
        term filename0 = interp.arg(caller, 0);
	term mode0 = interp.arg(caller, 1);
	term stream = interp.arg(caller, 2);

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

    size_t builtins_fileio::get_stream_id(interpreter &interp, term &stream,
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

    bool builtins_fileio::close_1(interpreter &interp, term &caller)
    {
	term stream = interp.arg(caller, 0);
	size_t id = get_stream_id(interp, stream, "close/1");
	interp.close_file_stream(id);
	return true;
    }

    bool builtins_fileio::read_2(interpreter &interp, term &caller)
    {
	term stream = interp.arg(caller, 0);
	size_t id = get_stream_id(interp, stream, "read/2");
	file_stream &fs = interp.get_file_stream(id);
	term t;
	if (fs.is_eof()) {
	    t = interp.functor("end_of_file",0);
	} else {
	    t = fs.read_term();
	}
	term result = interp.arg(caller, 1);
	bool r = interp.unify(t, result);

	return r;
    }

    bool builtins_fileio::at_end_of_stream_1(interpreter &interp, term &caller)
    {
	term stream = interp.arg(caller, 0);
	size_t id = get_stream_id(interp, stream, "at_end_of_stream/1");
	file_stream &fs = interp.get_file_stream(id);
	return fs.is_eof();
    }

    bool builtins_fileio::write_1(interpreter &interp, term &caller)
    {
	term arg = interp.arg(caller, 0);
	std::string str = interp.to_string(arg);
	std::cout << str << std::flush;
	return true;
    }

    bool builtins_fileio::nl_0(interpreter &interp, term &caller)
    {
	std::cout << std::endl << std::flush;
	return true;
    }

}}
