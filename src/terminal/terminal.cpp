#include <algorithm>
#include <ctype.h>
#include <boost/algorithm/string/predicate.hpp>
#include "../common/term_serializer.hpp"
#include "terminal.hpp"

namespace prologcoin { namespace terminal {

using namespace prologcoin::common;

terminal::terminal(unsigned short port, const std::string &ip_address)
  : connected_(false),
    ioservice_(),
    endpoint_(boost::asio::ip::address::from_string(ip_address),port),
    socket_(ioservice_),
    buffer_(MAX_BUFFER_SIZE, ' '),
    buffer_len_(sizeof(cell)),
    has_more_(false),
    at_end_(false),    
    result_to_text_(true),
    propagate_exceptions_(false)
{
}

terminal::~terminal()
{
}

term terminal::parse(const std::string &cmd)
{
    try {
	term t = env_.parse(cmd);
	return t;
    } catch (token_exception &ex) {
	error(cmd, ex.column(), &ex, nullptr);
    } catch (term_parse_exception &ex) {
	error(cmd, ex.column(), nullptr, &ex);
    } catch (std::runtime_error &ex) {
	std::cout << "Error: " << ex.what() << std::endl;
    } catch (...) {
	std::cout << "Unknown error" << std::endl;
    }
    return term();
}

bool terminal::reset()
{
    auto &e = env_;
    if (!send_query(e.new_term(con_cell("command",1),
			       {con_cell("reset",0)}))) {
	return false;
    }
    if (!process_query_reply()) {
	return false;
    }
    has_more_ = false;
    return true;
}

void terminal::error(const std::string &cmd,
		     int column,
		     token_exception *token_ex,
		     term_parse_exception *parse_ex)
{
    std::stringstream ss;
    ss << "While parsing at column ";
    ss << column;
    if (parse_ex != nullptr) {
        ss << " for state: ";
	add_error(ss.str());
	auto &desc = parse_ex->state_description();
	for (auto &line : desc) {
	    add_error(line);
	}
	ss.str(std::string());
	ss << "Expected: {";
	bool first = true;
	for (auto exp : env_.get_expected_simplified(*parse_ex)) {
	    if (!first) ss << ", ";
	    if (!isalnum(exp[0])) {
		exp = '\'' + exp + '\'';
	    }
	    ss << exp;
	    first = false;
	}
	ss << "}";
	add_error(ss.str());
    } if (token_ex != nullptr) {
	add_error(token_ex->what());
    } else {
    }

    int pos = column - 1;
    int start_excerpt = std::max(pos-20, 0);
    int end_excerpt = std::max(pos+10, static_cast<int>(cmd.size()));
    int excerpt_len = end_excerpt - start_excerpt;
    auto excerpt = cmd.substr(start_excerpt, excerpt_len);
    ss.str(std::string());
    if (start_excerpt > 0) {
	ss << "...";
    }
    ss << excerpt;
    if (excerpt_len < static_cast<int>(cmd.size())) {
	ss << "...";
    }
    add_error(ss.str());
    ss.str(std::string());
    if (start_excerpt > 0) {
	ss << "   ";
    }
    ss << std::string(pos-start_excerpt, ' ') << "^";
    add_error(ss.str());
}

void terminal::purge_text()
{
    boost::lock_guard<boost::mutex> guard(text_output_queue_mutex_);
    std::queue<std::string>().swap(text_output_queue_);
}

void terminal::print_text()
{
    boost::lock_guard<boost::mutex> guard(text_output_queue_mutex_);
    std::queue<std::string> copy = text_output_queue_;
    while (!copy.empty()) {
	std::cout << copy.front();
	copy.pop();
    }
}

std::string terminal::flush_text()
{
    boost::lock_guard<boost::mutex> guard(text_output_queue_mutex_);
    std::string s;
    while (!text_output_queue_.empty()) {
	s += text_output_queue_.front();
	text_output_queue_.pop();
    }
    return s;
}

void terminal::lock_text_output_and( std::function<void()> fn )
{
    boost::lock_guard<boost::mutex> guard(text_output_queue_mutex_);
    fn();
}

void terminal::add_error(const std::string &str)
{
    auto err_str = "[ERROR]: " + str;
    add_text_output(err_str);
    has_more_ = false;
}

void terminal::add_text_output(const std::string &str)
{
    if (!boost::ends_with(str, "\n")) {
	text_output_queue_.push(str + "\r\n");
    } else {
	text_output_queue_.push(str);
    }
}

void terminal::add_text_output_no_nl(const std::string &str)
{
    text_output_queue_.push(str);
}

bool terminal::send_buffer(term_serializer::buffer_t &buf, size_t n)
{
    size_t off = 0;
    while (n > 0) {
	boost::system::error_code ec;
	size_t r = socket_.write_some(boost::asio::buffer(&buf[off],n),ec);
	if (!ec) {
	    n -= r;
	    off += r;
	} else {
	    connected_ = false;
	    return false;
	}
    }
    return true;
}

bool terminal::send_length(size_t n)
{
    term_serializer::write_cell(buffer_len_, 0, int_cell(n));
    return send_buffer(buffer_len_, sizeof(cell));
}

bool terminal::send_query(const term t)
{
    return send_query(t, env_);
}

bool terminal::send_query(const term t, term_env &src)
{
    term_serializer ser(src);
    buffer_.clear();
    ser.write(buffer_, t);
    if (!send_length(buffer_.size())) {
	return false;
    }
    return send_buffer(buffer_, buffer_.size());
}

term terminal::read_reply()
{
    term_serializer ser(env_);
    size_t n = sizeof(cell);
    size_t off = 0;

    buffer_len_.resize(sizeof(cell));
    while (off < n) {
	boost::system::error_code ec;
	size_t r = socket_.read_some(boost::asio::buffer(&buffer_len_[off],
						 sizeof(cell)-off), ec);
	if (ec) {
	    std::stringstream ss;
	    ss << "Error while reading: " << ec.message();
	    add_error(ss.str());
	    return term();
	}
	off += r;
    }
    auto c = ser.read_cell(buffer_len_, 0, "");
    if (c.tag() != tag_t::INT) {
	add_error("Erreoneous encoding of reply length.");
	return term();
    }

    auto &ic = reinterpret_cast<int_cell &>(c);
    if (ic.value() < static_cast<int>(sizeof(cell))) {
	std::stringstream ss;
	ss << "Length of reply too small (" << ic.value() << " < "
	   << sizeof(cell) << std::endl;
	add_error(ss.str());
	return term();
    }

    if (ic.value() > static_cast<int>(MAX_BUFFER_SIZE)) {
	std::stringstream ss;
	ss << "Length of reply too big (" << ic.value() << " > " << MAX_BUFFER_SIZE;
	add_error(ss.str());
	return term();
    }

    n = ic.value();
    off = 0;

    buffer_.resize(n);

    while (off < n) {
	boost::system::error_code ec;
	size_t r = socket_.read_some(boost::asio::buffer(&buffer_[off],
						 n - off), ec);
	if (ec) {
	    std::stringstream ss;
	    ss << "Error while reading: " << ec.message();
	    add_error(ss.str());
	    return term();
	}
	off += r;
    }

    term t = ser.read(buffer_);

    return t;
}

bool terminal::connect()
{
    boost::system::error_code ec;
    socket_.connect(endpoint_, ec);
    if (ec) {
	return false;
    }

    auto &e = env_;
    if (!send_query(e.new_term(con_cell("command",1), {con_cell("new",0)}))) {
	return false;
    }
    auto reply = read_reply();
    if (reply == term()) {
	return false;
    }

    if (e.functor(reply) != con_cell("ok",1)) {
	add_error("Node replied with failure: " + env_.to_string(reply));
	return false;
    }

    static const con_cell session("session", 2);

    auto session_term = e.arg(reply, 0);
    if (e.functor(session_term) != session) {
	add_error("Node replied with unexpected session: " + env_.to_string(session_term));
	return false;
    }

    auto session_id_term = e.arg(session_term, 0);
    if (!e.is_atom(session_id_term)) {
	add_error("Node did not reply with session id: " + e.to_string(session_id_term));
	return false;
    }

    session_id_ = e.atom_name(session_id_term);

    // We have a session. Send a connect command with identifier.
    if (!send_query(e.new_term(con_cell("command",1),
			       {e.new_term(con_cell("connect",1),
					   {e.functor(session_id_,0)})}))) {
	return false;
    }

    reply = read_reply();
    if (reply == term()) {
	return false;
    }

    if (e.functor(reply) != con_cell("ok",1)) {
	add_error("Unexpected reply from node: " + e.to_string(reply));
	return false;
    }

    auto inner = e.arg(reply, 0);
    auto inner_f = e.functor(inner);
    auto inner_name = e.atom_name(inner_f);
    if (inner_name != "session_resumed" || inner_f.arity() != 2) {
	add_error("Unexpected reply from node: " + e.to_string(reply));
	return false;
    }
    
    auto id_term = e.arg(inner,0);
    auto state_term = e.arg(inner,1);
    if (!e.is_atom(id_term) || !e.is_atom(state_term)) {
	add_error("Unexpected session id from node: " + e.to_string(id_term));
	return false;
    }
    auto id = e.atom_name(id_term);
    has_more_ = false;
    if (state_term == con_cell("more",0)) {
	has_more_ = true;
    }
    if (state_term == con_cell("at_end",0)) {
        at_end_ = true;
    }
    return true;
}

bool terminal::delete_instance()
{
    auto &e = env_;
    if (!send_query(e.new_term(con_cell("command",1), {con_cell("delinst",0)}))) {
	return false;
    }
    auto reply = read_reply();
    if (reply == term()) {
	return false;
    }

    if (e.functor(reply) != con_cell("ok",1)) {
	add_error("Node replied with failure: " + env_.to_string(reply));
	return false;
    }
    return true;
}


void terminal::close()
{
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
}

void terminal::node_pulse()
{
    static con_cell COLON(":", 2);  
    static con_cell COMMA(",", 2);
    static con_cell ME("me",0);
  
    term_env &e = env();
    auto query_pulse = e.new_term(COMMA,
				      { e.new_term(
				    COLON,
				   {ME, e.functor("heartbeat",0)}),
		  	     e.new_term(
				    COLON,
				   {ME, e.functor("check_mail",0)})});
    bool old = is_result_to_text();
    set_result_to_text(false);
    execute(query_pulse);
    set_result_to_text(old);
}

bool terminal::execute_query(const term query)
{
    uint64_t cost = 0;
    auto &e = env_;
    term s_query = e.copy(query, env_, cost);
    auto q = e.new_term(e.functor("query",1), {s_query});
    if (!send_query(q)) {
	return false;
    }
    if (!process_query_reply()) {
	return false;
    }
    return true;
}

void terminal::handle_error(const std::string &msg)
{
    if (propagate_exceptions()) {
        throw interpreter_remote_exception(msg);
    }
  
    auto from = static_cast<size_t>(0);
    while (from != std::string::npos) {
	auto end = msg.find("\n", from);
	if (end == std::string::npos) {
	    add_error(msg.substr(from));
	    from = end;
	} else {
	    add_error(msg.substr(from,end-from+1));
	    from = end+1;
	}
    }
}

bool terminal::process_query_reply()
{
    auto reply = read_reply();
    if (reply == term()) {
	return false;
    }
    auto &e = env_;

    if (e.functor(reply) == con_cell("error",1)) {
	auto arg = e.arg(reply,0);
	if (e.functor(arg)  == e.functor("remote_exception",1)) {
	    arg = e.arg(arg,0);
	    handle_error(e.list_to_string(arg));
	} else {
	    handle_error(e.to_string(arg));
	}
	return false;
    }

    if (e.functor(reply) == con_cell("ok",1)) {
	auto context = e.save_state();
	auto result_term = e.arg(reply,0);

	if (e.functor(result_term) != con_cell("result",5)) {
	    add_error("Unexpected result. Expected result/5 inside ok/1, but got: " + e.to_string(result_term));
	    return false;
	}
	auto in_query_state = e.arg(result_term,2);
	has_more_ = in_query_state == con_cell("more",0);

	// Append any text output from fourth argument.
	// This never errors becaus list_to_string() never errors, it
	// just skips illegal items.
	auto text_out_term = e.arg(result_term,3);
	auto text_out = e.list_to_string(text_out_term);
	if (!text_out.empty()) {
	    add_text_output(text_out);
	}

	result_ = e.arg(result_term, 0);
	var_names_.clear();
	var_result_.clear();
	var_result_string_.clear();

	if (e.arg(result_term, 0) == con_cell("false",0)) {
	    if (result_to_text_) {
		add_text_output("false.");
	    }
	    return false;
	}
	auto vars = e.arg(result_term,1);

	if (vars == e.EMPTY_LIST) {
	    if (result_to_text_) {
		if (has_more_) {
		    add_text_output_no_nl("true ");
		} else {
		    add_text_output("true.");
		}
	    }
	    return true;
	}

	auto touched = e.prettify_var_names(vars);

	// Collect var values so that if we get
	std::vector<std::pair<std::string, term> > var_value_list;
	while (vars != e.EMPTY_LIST) {
	    if (!e.is_dotted_pair(vars)) {
		add_error("Unexpected result. Second argument of result/4 was not a proper list. " + e.to_string(e.arg(result_term,1)));
		return false;
	    }
	    auto var_binding = e.arg(vars,0);
	    if (e.functor(var_binding) != con_cell("=",2)) {
		add_error("Unexpected result. Unexpected name binding: " + e.to_string(var_binding));
		return false;
	    }
	    auto var_name_term = e.arg(var_binding, 0);
	    if (var_name_term.tag() != tag_t::CON) {
		add_error("Unexpected result. Variable name was not an atom: " + e.to_string(var_binding));
		return false;
	    }
	    auto var_name = e.atom_name(reinterpret_cast<con_cell &>(var_name_term));
	    auto var_value = e.arg(var_binding, 1);
	    vars = e.arg(vars, 1);
	    e.set_name(var_value, var_name);
	    var_value_list.push_back(std::pair<std::string, term>(var_name, var_value));
	    touched.push_back(var_value);
	}

	bool non_empty = false;

	for(auto &var_value_pair : var_value_list) {
	  auto var_name = var_value_pair.first;
	  auto var_value = var_value_pair.second;
	  emitter_options default_opt;
	  default_opt.set(emitter_option::EMIT_INTERACTIVE);
	  auto var_value_str = e.to_string(var_value, default_opt);
	  if (var_name != var_value_str) {
	    if (result_to_text_ && non_empty) {
	      add_text_output(",");
	    }
	    std::string binding_str = var_name + " = " + var_value_str;
	    if (result_to_text_) add_text_output_no_nl(binding_str);
	    non_empty = true;
	  }
	  var_names_.push_back(var_name);
	  var_result_[var_name] = var_value;
	  var_result_string_[var_name] = var_value_str;
	}

	e.restore_state(context);
	for (auto touched_term : touched) {
	    e.clear_name(touched_term);
	}
	if (result_to_text_) {
	    if (has_more_) {
		add_text_output_no_nl(" ");
	    } else {
		add_text_output(".");
	    }
	}

	return true;
    }
    add_error("Unexpected reply from node: " + e.to_string(reply));
    return false;
}

bool terminal::execute_in_query(const std::string &cmd)
{
    if (cmd == ";") {
	auto q = env_.new_term(env_.functor("command",1),{env_.functor("next",0)});
	if (!send_query(q, env_)) {
	    return false;
	}
	if (!process_query_reply()) {
	    return false;
	}
    } else if (cmd == "") {
	has_more_ = false;
    } else {
	add_error("Unknown command. Only ';' or ENTER is allowed.");
	has_more_ = true; // Error clears "has more" flag, so restore it
    }
    return true;
}

}}
