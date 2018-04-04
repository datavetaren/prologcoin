#include <algorithm>
#include <ctype.h>
#include "../common/term_serializer.hpp"
#include "../node/self_node.hpp"
#include "terminal.hpp"

namespace prologcoin { namespace main {

using namespace prologcoin::common;
using namespace prologcoin::node;

terminal::terminal(unsigned short port)
  : prompt_("?- "),
    stopped_(false),
    connected_(false),
    ctrl_c_(false),
    ioservice_(),
    endpoint_(boost::asio::ip::address::from_string("127.0.0.1"),port),
    socket_(ioservice_),
    buffer_(self_node::MAX_BUFFER_SIZE, ' '),
    buffer_len_(sizeof(cell)),
    in_query_(false)
{
    readline_.set_tick(true);
    readline_.set_accept_ctrl_c(true);
    readline_.set_callback([this](readline &, int ch){
	    return key_callback(ch);
	});
}

terminal::~terminal()
{
}

bool terminal::key_callback(int ch)
{
    if (ch == 3) {
	ctrl_c_ = true;
	readline_.end_read();
    }

    if (ch == readline::TIMEOUT) {
	boost::unique_lock<boost::mutex> guard(text_output_queue_mutex_);
	if (!text_output_queue_.empty()) {
	    readline_.clear_line();
	    std::cout << std::string(prompt_.size(), '\b')
		      << std::string(prompt_.size(), ' ')
	   	      << std::string(prompt_.size(), '\b');
	    while (!text_output_queue_.empty()) {
		auto line = text_output_queue_.front();
		std::cout << line;
		text_output_queue_.pop();
	    }
	    std::cout << prompt_;
	    std::cout.flush();
	    readline_.clear_render();
	    readline_.render();
	}
	return false;
    }

    bool r = readline_.has_standard_handling(ch);

    if (r && in_query_) {
	// Single keystrokes if in query mode.
	readline_.end_read();
    }

    return r;
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

void terminal::lock_text_output_and( std::function<void()> fn )
{
    boost::lock_guard<boost::mutex> guard(text_output_queue_mutex_);
    fn();
}

void terminal::add_error(const std::string &str)
{
    auto err_str = "[ERROR]: " + str;
    add_text_output(err_str);
    in_query_ = false;
}

void terminal::add_text_output(const std::string &str)
{
    text_output_queue_.push(str + "\n");
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

    if (ic.value() > static_cast<int>(self_node::MAX_BUFFER_SIZE)) {
	std::stringstream ss;
	ss << "Length of reply too big (" << ic.value() << " > " << self_node::MAX_BUFFER_SIZE;
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

    auto session_id_term = e.arg(reply, 0);
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
    in_query_ = false;
    if (state_term == con_cell("more",0)) {
	in_query_ = true;
    }
    return true;
}

void terminal::halt()
{
    stopped_ = true;
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

bool terminal::process_query_reply()
{
    auto reply = read_reply();
    if (reply == term()) {
	return false;
    }
    auto &e = env_;
    if (e.functor(reply) == con_cell("error",1)) {
	add_error(e.to_string(e.arg(reply,0)));
	return false;
    }

    if (e.functor(reply) == con_cell("ok",1)) {
	auto context = e.capture_state();
	auto result_term = e.arg(reply,0);
	if (e.functor(result_term) != con_cell("result",3)) {
	    add_error("Unexpected result. Expected result/3 inside ok/1, but got: " + e.to_string(result_term));
	    return false;
	}
	auto in_query_state = e.arg(result_term,2);
	in_query_ = in_query_state == con_cell("more",0);
	
	auto touched = e.prettify_var_names(result_term);
	auto vars = e.arg(result_term,1);
	while (vars != e.empty_list()) {
	    if (!e.is_dotted_pair(vars)) {
		add_error("Unexpected result. Second argument of result/3 was not a proper list. " + e.to_string(e.arg(result_term,1)));
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
	    auto var_value_str = e.to_string(var_value);
	    if (var_name != var_value_str) {
		std::string binding_str = var_name + " = " + var_value_str;
		if (e.is_dotted_pair(vars)) {
		    binding_str += ",";
		    add_text_output(binding_str);
		} else {
		    if (in_query_) {
			add_text_output_no_nl(binding_str + " ");
		    } else {
			add_text_output(binding_str);
		    }
		}
	    }
	}
	e.restore_state(context);
	for (auto touched_term : touched) {
	    e.clear_name(touched_term);
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
	in_query_ = false;
    } else {
	add_error("Unknown command. Only ';' or ENTER is allowed.");
	in_query_ = true; // Error does normally clear "in query" flag
    }
    return true;
}

void terminal::run()
{
    while (!stopped_) {
	// Set prompt
	if (in_query_) {
	    prompt_ = "";
	} else {
	    prompt_ = "?- ";
	}

	std::cout << prompt_;
	std::cout.flush();

	ctrl_c_ = false;
	std::string cmd = readline_.read();
	std::cout << "\n";

	if (ctrl_c_) {
	    in_query_ = false;
	    std::cout << "Enter 'halt.' to exit terminal." << std::endl;
	    continue;
	}

	if (stopped_) {
	    continue;
	}
	readline_.add_history(cmd);
	try {
	    if (in_query_) {
		execute_in_query(cmd);
		continue;
	    }

	    term t = env_.parse(cmd);
	    if (t == con_cell("halt",0)) {
		break;
	    }

	    execute_query(t);
	} catch (token_exception &ex) {
	    error(cmd, ex.column(), &ex, nullptr);
	} catch (term_parse_exception &ex) {
	    error(cmd, ex.column(), nullptr, &ex);
	} catch (std::runtime_error &ex) {
	    std::cout << "Error: " << ex.what() << std::endl;
	} catch (...) {
	    std::cout << "Unknown error" << std::endl;
 	}
    }
}

}}
