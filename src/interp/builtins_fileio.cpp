#include "builtins_fileio.hpp"
#include "interpreter_base.hpp"
#include "interpreter.hpp"
#include <boost/filesystem.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <ctype.h>

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

    bool builtins_fileio::format_helper(const std::string &name, interpreter_base &interp, term format_term, term format_args, std::string &out)
    {
	if (!interp.is_atom(format_term) && !interp.is_string(format_term)) {
	    interp.abort(interpreter_exception_wrong_arg_type(
		      name + ": Format identifier must be an atom or a string;"
		      " was " + interp.to_string(format_term)));
	}

	std::string format_str = interp.is_atom(format_term) ?
	    interp.atom_name(format_term) : interp.list_to_string(format_term);

	auto next_arg = [&name, &format_args, &interp](){
	    term r;
	    if (interp.is_empty_list(format_args)) {
		interp.abort(interpreter_exception_missing_arg(
			       name + ": Missing format argument."));
	    }
	    if (interp.is_dotted_pair(format_args)) {
		r = interp.arg(format_args, 0);
		format_args = interp.arg(format_args, 1);
	    } else {
		r = format_args;
		format_args = interp.empty_list();
	    }
	    return r;
	};

	auto it = format_str.begin();
	auto it_end = format_str.end();

	auto unexpected_end_check = [&](){
	    if (it == it_end) {
		interp.abort(interpreter_exception_wrong_arg_type(
		  name + ": Unexpected end while parsing format arg."));
	    }
	};

	std::vector<size_t> tab_stops;
	std::string str;

	auto format_tilde_val = [&](){
	    int32_t val = -1;
	    if (isdigit(*it)) {
		val = 0;
		while (isdigit(*it)) {
		    val = 10*val + ((*it) - '0');
		    ++it;
		    unexpected_end_check();
		}
	    } else if (*it == '`') {
		++it;
		unexpected_end_check();
		val = static_cast<int32_t>(*it);
	    } else if (*it == '*') {
		auto arg = next_arg();
		if (arg.tag() != tag_t::INT) {
		    interp.abort(interpreter_exception_wrong_arg_type(
		      name + ": Non-integer argument to format '*'"));
		}
		auto v = reinterpret_cast<const int_cell &>(arg).value();
		if (v < 0) {
		    interp.abort(interpreter_exception_wrong_arg_type(
		      name + ": Negative integer argument to format '*'"));
		}
		if (v > std::numeric_limits<int32_t>::max()) {
		    interp.abort(interpreter_exception_wrong_arg_type(
		      name + ": Integer argument to format '*' is "
		      "exceeded maximum ("
		      + boost::lexical_cast<std::string>(
			 std::numeric_limits<int32_t>::max()) + "), was "
		      + boost::lexical_cast<std::string>(v)));
		}
		val = static_cast<int32_t>(v);
		++it;
	    }
	    return val;
	};

	auto format_tilde_modifier = [&](){
	    unexpected_end_check();
	    if (*it == ':') {
		++it;
		return true;
	    } else {
		return false;
	    }
	};

	auto format_atom = [&](){
	    auto arg = next_arg();
	    if (interp.is_atom(arg)) {
		str += interp.atom_name(arg);
	    } else if (arg.tag() == tag_t::INT) {
		str += boost::lexical_cast<std::string>(reinterpret_cast<const int_cell &>(arg).value());
	    } else if (arg.tag() == tag_t::BIG) {
		str += interp.to_string(arg);
	    } else {
		interp.abort(interpreter_exception_wrong_arg_type(
		      name + ": Argument to ~a was not an atom; was "
		      + interp.to_string(arg)));
	    }
	};

	auto max_val_check = [&](int32_t value, const std::string &context) {
	    if (value > 65536) {
		interp.abort(interpreter_exception_wrong_arg_type(
		  name + ": Numeric to " + context + " ~c is too big "
		  "(maximum 65536 is allowed); was "
		  + boost::lexical_cast<std::string>(value)));
	    }
	};

	auto format_char = [&](int32_t value){
	    max_val_check(value, "~c");
	    // No numeric is interpreted as 1
	    if (value == -1) value = 1;
	    auto arg = next_arg();
	    if (arg.tag() != tag_t::INT) {
		interp.abort(interpreter_exception_wrong_arg_type(
		      name + ": Argument to ~c was not an integer; was "
		      + interp.to_string(arg)));
	    }
	    char ch = static_cast<char>(reinterpret_cast<const int_cell &>(arg).value());
	    str.append( value, ch );
	};

	auto int_to_string = [&](const std::string &context, term arg, int32_t base = 10) {
	    using namespace boost::multiprecision;

	    if (arg.tag() != tag_t::INT && arg.tag() != tag_t::BIG) {
		interp.abort(interpreter_exception_wrong_arg_type(
		      name + ": Argument to " + context + " was not an integer; was "
		      + interp.to_string(arg)));
	    }
	    std::string s;
	    if (base != 10) {
		if (arg.tag() == tag_t::INT) {
		    cpp_int ci(reinterpret_cast<const int_cell &>(arg).value());
		    s = interp.big_to_string(ci, base);
		} else {
		    s = interp.big_to_string(arg, base);
		}
	    } else {
		s = (arg.tag() == tag_t::INT) ? interp.to_string(arg)
		     : interp.big_to_string(arg, base);
	    }
	    return s;
	};

	auto to_int_group = [&](std::string &int_part,
				const std::string &sep,
				const std::vector<int> &grouping) {
	    // Group int part
	    size_t pos = int_part.size();
	    std::vector<std::string> parts;
	    for (auto g : grouping) {
		if (g > 0) {
		    if (g > pos) {
			// Nothing more to group
			break;
		    }
		    parts.push_back(int_part.substr(pos-g, g));
		    pos -= g;
		} else if (g < 0) {
		    // Repeat until done
	 	    g = -g;
		    while (g < pos) {
			parts.push_back(int_part.substr(pos-g, g));
			pos -= g;
		    }
		    break;
		}
	    }
	    parts.push_back(int_part.substr(0, pos));
	    int_part.clear();
	    for (auto &p : boost::adaptors::reverse(parts)) {
		if (!int_part.empty()) {
		    int_part += sep;
		}
		int_part += p;
	    }
	};

	auto to_group = [&](const std::string &s, int32_t val, bool mod, bool force_grouping) {
	    size_t slen = s.size();
	    std::string decp(".");
	    if (mod) {
		decp = interp.atom_name(interp.current_locale().decimal_point());
	    }
	    std::string sep(",");
	    if (mod) {
		sep = interp.atom_name(interp.current_locale().thousands_sep());
	    }
	    const std::vector<int> grouping = mod ? interp.current_locale().grouping() : force_grouping ? std::vector<int>{-3} : std::vector<int>();
	    
	    std::string int_part = s;
	    std::string dec_part;
	    if (val > 0) {
		if (val < slen) {
		    // Put decimal point in place
		    dec_part = s.substr(slen-val);
		    int_part = s.substr(0, slen-val);
		} else {
		    int_part = "0";
		    dec_part.append(val-slen, '0');
		    dec_part += s;
		}
	    }

	    to_int_group(int_part, sep, grouping);
	    if (!dec_part.empty()) {
		return int_part + decp + dec_part;
	    } else {
		return int_part;
	    }
	};

	auto format_decimal = [&](int32_t value, bool mod){
	    max_val_check(value, "~d");
	    // No numeric is interpreted as 1
	    if (value == -1) value = 0;
	    auto arg = next_arg();
	    arg = interp.arith().eval(arg, name + ": ~d");
	    auto s = int_to_string("~d", arg);
	    s = to_group(s, value, mod, false);
	    str += s;
	};

	auto format_capital_decimal = [&](int32_t value, bool mod) {
	    max_val_check(value, "~D");
	    if (value == -1) value = 0;
	    auto arg = next_arg();
	    arg = interp.arith().eval(arg, name + ": ~D");
	    auto s = int_to_string("~D", arg);
	    s = to_group(s, value, false, true);
	    str += s;
	};

	auto num_to_string = [&](const term arg, size_t precision, bool scientific, const std::string &context) {
	    using namespace boost::multiprecision;
	    if (arg.tag() != tag_t::INT && arg.tag() != tag_t::BIG) {
		interp.abort(interpreter_exception_wrong_arg_type(
		      name + ": Argument to " + context + " was not an integer; was "
		      + interp.to_string(arg)));
	    }
	    cpp_int i;
	    if (arg.tag() == tag_t::INT) {
		i = reinterpret_cast<const int_cell &>(arg).value();
	    } else {
		interp.get_big(arg, i);
	    }
	    double d = static_cast<double>(i);
	    std::stringstream ss;
	    ss.precision(precision);
	    if (scientific) {
		ss << std::scientific;
	    } else {
		ss << std::fixed;
	    }
	    ss << d;
	    return ss.str();
	};

        auto to_exponent = [&](const std::string &context, term arg, int32_t val, bool capital_e) {
	    if (val == -1) val = 6;
	    arg = interp.arith().eval(arg, name + ": " + context);
	    if (arg.tag() != tag_t::INT && arg.tag() != tag_t::BIG) {
		interp.abort(interpreter_exception_wrong_arg_type(
		      name + ": Argument to " + context + " was not an integer; was "
		      + interp.to_string(arg)));
	    }

	    std::string s = num_to_string(arg, val, true, context);
	    if (capital_e) {
		std::replace(s.begin(), s.end(), 'e', 'E');
	    }
	    return s;
	};

	auto format_exponent = [&](int32_t val) {
	    auto arg = next_arg();
	    str += to_exponent("~e", arg, val, false);
	};

	auto to_fixed = [&](const std::string &context, term arg, int32_t val, bool mod) {
	    if (val == -1) val = 6;
	    arg = interp.arith().eval(arg, name + ": " + context);
	    if (arg.tag() != tag_t::INT && arg.tag() != tag_t::BIG) {
		interp.abort(interpreter_exception_wrong_arg_type(
		      name + ": Argument to " + context + " was not an integer; was "
		      + interp.to_string(arg)));
	    }
	    std::string s = (arg.tag() == tag_t::INT) ? interp.to_string(arg)
	                            : interp.big_to_string(arg, 10);
	    // Because we don't have floating point, we add the 0s to the end
	    s.append(val, '0');
	    s = to_group(s, val, mod, false);
	    return s;
	};

        auto format_fixed = [&](int32_t val, bool mod){
	    auto arg = next_arg();
	    str += to_fixed("~f", arg, val, mod);
	};

	//
	// I think SWI-Prolog is wrong on this one.
	// It doesn't do what the doc says. We're following the strict
	// rule, that we compute '~e' and '~f' and pick the shortest one.
	// Also, when we apply modifier to '~g', it would make sense to
	// pass this one to '~f' to get the locale grouping correct.
	//
	auto format_g = [&](int32_t val, bool mod) {
	    if (val == -1) val = 6;
	    auto arg = next_arg();
	    std::string e_str = to_exponent("~g", arg, val, false);
	    std::string f_str = to_fixed("~g", arg, val, mod);
	    
	    if (e_str.size() < f_str.size()) {
		str += e_str;
	    } else {
		str += f_str;
	    }
	};

	auto format_capital_exponent = [&](int32_t val) {
	    auto arg = next_arg();
	    str += to_exponent("~E", arg, val, true);
	};

	auto format_capital_g = [&](int32_t val, bool mod) {
	    if (val == -1) val = 6;
	    auto arg = next_arg();
	    std::string e_str = to_exponent("~G", arg, val, true);
	    std::string f_str = to_fixed("~G", arg, val, mod);
	    if (e_str.size() < f_str.size()) {
		str += e_str;
	    } else {
		str += f_str;
	    }
	};

	auto format_ignore = [&]() {
	    next_arg();
	};

	auto format_integer_group = [&](int32_t val) {
	    if (val == -1) val = 3;
	    auto arg = next_arg();
	    auto s = int_to_string("~I", arg);
	    std::vector<int> grouping = {-val};
	    to_int_group(s, "_", grouping);
	    str += s;
	};

	auto format_canonical = [&]() {
	    auto arg = next_arg();
	    common::emitter_options opt;
	    opt.set(common::emitter_option::EMIT_CANONICAL);
	    auto s = interp.to_string(arg, opt);
	    str += s;
	};

	auto format_newline = [&](int32_t val) {
	    if (val == -1) val = 1;
	    str.append(val, '\n');
	};

	// We interpret ~3N as ~N~N~N, so the value has no effect
	auto format_newline_opt = [&]() {
	    if (str.empty() || str.back() != '\n') {
		str += '\n';
	    }
	};

	auto format_print = [&]() {
	    throw interpreter_exception_unsupported("~p is unsupported.");
	};

	auto format_writeq = [&]() {
	    auto arg = next_arg();
	    common::emitter_options opt;
	    opt.clear(emitter_option::EMIT_NEWLINE);
	    str += interp.to_string(arg, opt);
	};

	auto radix_helper = [&](const std::string &context, int32_t val, bool mod) {
	    max_val_check(val, context);
	    bool legal_val = (val >= 2 && val <= 36) || val == 58;
	    if (!legal_val) {
		if (val == -1) {
		    interp.abort(interpreter_exception_wrong_arg_type(
		      name + ": Missing numeric to " + context));
		}
		interp.abort(interpreter_exception_wrong_arg_type(
		  name + ": Numeric to " + context + " is not valid "
		  "(only 2..36 and 58 are allowed); was "
		  + boost::lexical_cast<std::string>(val)));
	    }
	    auto arg = next_arg();
	    arg = interp.arith().eval(arg, name + ": " + context);
	    auto s = int_to_string(context, arg, val);
	    if (context == "~R") {
		if (val > 10 && val <= 36) {
		    boost::to_upper(s);
		}
	    }
	    s = to_group(s, 0, mod, false);
	    str += s;
	};

	auto format_radix = [&](int32_t val, bool mod) {
	    radix_helper("~r", val, mod);
	};

	auto format_capital_radix = [&](int32_t val, bool mod) {
	    radix_helper("~R", val, mod);
	};

	auto format_string = [&]() {
	    auto arg = next_arg();
	    if (!interp.is_string(arg)) {
		interp.abort(interpreter_exception_wrong_arg_type(
		      name + ": ~s expecting a string, was: "
		      + interp.to_string(arg)));
	    }
	    str += interp.list_to_string(arg);
	};

	auto format_goal = [&]() {
	    term arg = next_arg();
	    term arg_cut = interp.new_term(con_cell(",",2), {arg, con_cell("!",0)});
	    auto &interp1 = reinterpret_cast<interpreter &>(interp);
	    interp1.new_instance();
	    auto hb = interp.get_register_hb();
	    bool r = interp1.execute(arg_cut);
	    if (r) {
		str += interp1.get_result();
	    }
	    interp1.delete_instance();
	    if (r) {
		interp.set_register_hb(hb);
	    }
	    return r;
	};

	auto format_write = [&]() {
	    term arg = next_arg();
	    str += interp.to_string(arg);
	};

	auto format_tilde = [&](){
	    ++it;
	    auto val = format_tilde_val();
	    auto mod = format_tilde_modifier();
	    unexpected_end_check();
	    switch (*it) {
	    case '~': str += '~'; break;
	    case 'a': format_atom(); break;
	    case 'c': format_char(val); break;
	    case 'd': format_decimal(val, mod); break;
	    case 'D': format_capital_decimal(val, mod); break;
	    case 'e': format_exponent(val); break;
	    case 'E': format_capital_exponent(val); break;
	    case 'f': format_fixed(val, mod); break;
	    case 'g': format_g(val, mod); break;
	    case 'G': format_capital_g(val, mod); break;
	    case 'i': format_ignore(); break;
	    case 'I': format_integer_group(val); break;
	    case 'k': format_canonical(); break;
	    case 'n': format_newline(val); break;
	    case 'N': format_newline_opt(); break;
	    case 'p': format_print(); break;
	    case 'q': format_writeq(); break;
	    case 'r': format_radix(val, mod); break;
	    case 'R': format_capital_radix(val, mod); break;
	    case 's': format_string(); break;
	    case '@': if (!format_goal()) return false; break;
	    // TOOD: ~t ~| ~+ (tabs and expand)
	    case 'w': format_write(); break;
	    default: 
	        throw interpreter_exception_unsupported(name + ": format ~" + *it + " does not exist.");
	    }
	    return true;
	};

	while (it != it_end) {
	    auto ch = *it;
	    if (ch== '~') {
		if (!format_tilde()) return false;
	    } else {
		str += ch;
	    }
	    ++it;
	}

	out = str;

	return true;
    }

    bool builtins_fileio::format_2(interpreter_base &interp, size_t arity, common::term args[])
    {
	term format_term = args[0];
	term format_args = args[1];

	std::string out_str;

	if (!format_helper("format/2", interp, format_term, format_args, out_str)) {
	    return false;
	}

	interp.standard_output().write(out_str);

	return true;
    }

    bool builtins_fileio::sformat_3(interpreter_base &interp, size_t arity, common::term args[])
    {
	term format_term = args[1];
	term format_args = args[2];

	std::string out_str;

	if (!format_helper("sformat/3", interp, format_term, format_args, out_str)) {
	    return false;
	}

	term out = interp.string_to_list(out_str);

	return interp.unify(out, args[0]);
    }
	
}}
