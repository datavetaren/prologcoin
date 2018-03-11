#include <algorithm>
#include <ctype.h>
#include "../common/term_serializer.hpp"
#include "terminal.hpp"
#include "self_node.hpp"

namespace prologcoin { namespace node {

using namespace prologcoin::common;

session::session(terminal &term,
		 session::io_service &ios,
		 const session::endpoint &ep,
		 session::socket &sock)
    : terminal_(term)
     ,ioservice_(ios)
     ,endpoint_(ep)
     ,socket_(std::move(sock))
     ,strand_(ios)
     ,connected_(false)
     ,buffer_(self_node::MAX_BUFFER_SIZE, ' ')
     ,buffer_len_(sizeof(cell))
{
}

session::~session()
{
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    socket_.close(ec);
}

void session::send_buffer(term_serializer::buffer_t &buf, size_t n)
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
	}
    }
}

void session::send_length(size_t n)
{
    term_serializer::write_cell(buffer_len_, 0, int_cell(n));
    send_buffer(buffer_len_, sizeof(cell));
}

void session::send_query(const term t)
{
    send_query(t, env_);
}

void session::send_query(const term t, term_env &src)
{
    term_serializer ser(src);
    buffer_.clear();
    ser.write(buffer_, t);
    send_length(buffer_.size());
    send_buffer(buffer_, buffer_.size());
}

term session::read_reply()
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
	    connected_ = false;
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

void session::add_error(const std::string &msg)
{
    errors_.push(msg);
}

terminal::terminal() : prompt_(">> "),
		       stopped_(false),
		       ctrl_c_(false),
		       ioservice_(),
		       current_session_(nullptr),
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
    commands_.clear();
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

void terminal::register_session(session *s)
{
    sessions_[s->id()] = s;
}

void terminal::unregister_session(session *s)
{
    sessions_.erase(s->id());
    if (current_session_ == s) {
	current_session_ = nullptr;
    }
    delete s;
}

void terminal::command_help(const term)
{
    add_text_output("help.                         This information.");
    add_text_output("list.                         List current sessions.");
    add_text_output("new(Host:Port).               Create a new session at host (with port)");
    add_text_output("connect(Session).             Connect to a session.");
    add_text_output("connect(Session,[Host:Port]). Connect to a session at specific host:port.");
    add_text_output("close(Session).               Disconnect session. (But keep session at node.)");
    add_text_output("kill(Session,[Host:Port]).    Kill session (and at node too.)");
    add_text_output("quit.                         End terminal.");
}

void terminal::command_list(const term)
{
    add_text_output("");
    if (sessions_.empty()) {
	add_text_output(" --- no sessions --- ");
    }
    for (auto it : sessions_) {
	auto *session = it.second;
	std::stringstream ss;
	ss << std::setw(26) << session->id() << " : " << session->get_endpoint();
	add_text_output(ss.str());
    }
    add_text_output("");
}

session * terminal::new_session(const term host_port)
{
    using namespace boost::asio;
    using namespace boost::asio::ip;

    term arg = host_port;
    int port = self_node::DEFAULT_PORT;
    std::string host = "";

    term host_term;

    if (env_.functor(arg) == con_cell(":",2)) {
	host_term = env_.arg(arg, 0);
	auto port_term  = env_.arg(arg, 1);
	if (port_term.tag() != tag_t::INT) {
	    add_error("Expecting host port as an integer; was "
		      + env_.to_string(port_term));
	    return nullptr;
	}
	port = reinterpret_cast<int_cell &>(port_term).value();
    } else {
	host_term = arg;
    }
    if (!env_.is_atom(host_term)) {
	add_error("Expecting host name as an atom; was "
		  + env_.to_string(host_term));
	return nullptr;
    }
    host = env_.atom_name(host_term);

    tcp::resolver resolver(ioservice_);
    tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
    
    boost::system::error_code ec = boost::asio::error::host_not_found;
    auto iterator = resolver.resolve(query, ec);

    if (ec) {
	std::stringstream ss;
	ss << "Error while resolving host " << host << ":"
	   << port << ": " << ec.message();
	add_error(ss.str());
	return nullptr;
    }

    tcp::socket sock(ioservice_);
    tcp::resolver::iterator end;

    ec = boost::asio::error::host_not_found;
    while (ec && iterator != end) {
	sock.close();
	sock.connect(*iterator, ec);
	if (ec) ++iterator;
    }
    if (ec) {
	std::stringstream ss;
	ss << "Could not connect to " << host << ":" << port;
	add_error(ss.str());
	return nullptr;
    }

    session *s = new session(*this, ioservice_, iterator->endpoint(), sock);
    return s;
}

bool terminal::process_errors(session *s)
{
    if (s->has_errors()) {
	while (s->has_errors()) {
	    auto line = s->next_error();
	    add_error(line);
	}
	return true;
    } else {
	return false;
    }
}

void terminal::command_new(const term cmd)
{
    using namespace boost::asio;
    using namespace boost::asio::ip;

    session *s = new_session(env_.arg(cmd,0));
    if (s == nullptr) {
	return;
    }
    auto &e = s->env();
    s->send_query(e.new_term(con_cell("command",1), {con_cell("new",0)}));
    if (process_errors(s)) {
	unregister_session(s);
	return;
    }
    auto reply = s->read_reply();
    if (process_errors(s)) {
	unregister_session(s);
	return;
    }

    if (e.functor(reply) != con_cell("ok",1)) {
	add_error("Node replied with failure: " + env_.to_string(reply));
	return;
    }

    auto session_id_term = e.arg(reply, 0);
    if (!e.is_atom(session_id_term)) {
	add_error("Node did not reply with session id: " + e.to_string(session_id_term));
    }

    std::string session_id = e.atom_name(session_id_term);
    s->set_id(session_id);
    register_session(s);

    add_text_output(session_id);
}

session * terminal::get_session(const term cmd)
{
    auto session_id_term = env_.arg(cmd,0);
    if (!env_.is_atom(session_id_term)) {
	add_error("Erroneous session id: " + env_.to_string(session_id_term));
	return nullptr;
    }
    auto session_id = env_.atom_name(session_id_term);
    session *s = nullptr;
    if (sessions_.find(session_id) != sessions_.end()) {
	s = sessions_[session_id];
	return s;
    } else {
	// Didn't find session id, but do we have a host:port?
	if (env_.functor(cmd).arity() < 2) {
	    std::stringstream ss;
	    ss << "Session '" << session_id
	       << "' was not found, and no host:port was provided.";
	    add_error(ss.str());
	    return nullptr;
	}
	s = new_session(env_.arg(cmd,1));
	s->set_id(session_id);
    }
    return s;
}

void terminal::command_connect(const term cmd)
{
    session *s = get_session(cmd);
    if (s == nullptr) {
	return;
    }

    auto &e = s->env();
    // We have a session. Send a connect command with identifier.
    s->send_query(e.new_term(con_cell("command",1),
			     {e.new_term(con_cell("connect",1),
					 {e.functor(s->id(),0)})}));

    if (process_errors(s)) {
	unregister_session(s);
	return;
    }

    auto reply = s->read_reply();
    if (process_errors(s)) {
	unregister_session(s);
	return;
    }

    if (e.functor(reply) != con_cell("ok",1)) {
	add_error("Unexpected reply from node: " + e.to_string(reply));
	unregister_session(s);
	return;
    }

    auto inner = e.arg(reply, 0);
    auto inner_f = e.functor(inner);
    auto inner_name = e.atom_name(inner_f);
    if (inner_name != "session_resumed" || inner_f.arity() != 2) {
	add_error("Unexpected reply from node: " + e.to_string(reply));
	unregister_session(s);
	return;
    }
    
    auto id_term = e.arg(inner,0);
    auto state_term = e.arg(inner,1);
    if (!e.is_atom(id_term) || !e.is_atom(state_term)) {
	delete s;
	add_error("Unexpected session id from node: " + e.to_string(id_term));
	return;
    }
    auto id = e.atom_name(id_term);
    s->set_id(id);
    register_session(s);
    current_session_ = s;

    in_query_ = false;
    if (state_term == con_cell("more",0)) {
	in_query_ = true;
    }

    std::stringstream ss;
    ss << "Connecting to session '" << s->id() << "'";
    add_text_output(ss.str());
}

void terminal::command_kill(const term t)
{
    session *s = get_session(t);
    if (s == nullptr) {
	return;
    }

    auto &e = s->env();
    // We have a session. Send a connect command with identifier.
    s->send_query(e.new_term(con_cell("command",1),
			     {e.new_term(con_cell("kill",1),
					 {e.functor(s->id(),0)})}));
    if (process_errors(s)) {
	unregister_session(s);
	return;
    }
    auto reply = s->read_reply();
    if (process_errors(s)) {
	unregister_session(s);
	return;
    }

    if (e.functor(reply) != con_cell("ok",1)) {
	add_error("Node replied with failure: " + e.to_string(reply));
	return;
    }

    std::stringstream ss;
    ss << "Session '" << s->id() << "' killed.";
    add_text_output(ss.str());
    unregister_session(s);
}

void terminal::command_close(const term cmd)
{
    auto session_id_term = env_.arg(cmd,0);
    if (!env_.is_atom(session_id_term)) {
	add_error("Erroneous session id: " + env_.to_string(session_id_term));
	return;
    }
    auto session_id = env_.atom_name(session_id_term);
    session *s = nullptr;
    if (sessions_.find(session_id) == sessions_.end()) {
	std::stringstream ss;
	ss << "Session '" << session_id << "' was not found.";
	add_error(ss.str());
	return;
    }
    s = sessions_[session_id];
    unregister_session(s);
}

void terminal::command_quit(const term t)
{
    stopped_ = true;
}

void terminal::setup_commands()
{
    commands_[con_cell("help",0)] = [this](const term cmd){ command_help(cmd); };
    commands_[con_cell("quit",0)] = [this](const term cmd){ command_quit(cmd); };
    commands_[con_cell("new",1)] = [this](const term cmd){ command_new(cmd); };
    commands_[con_cell("connect",1)] = [this](const term cmd){ command_connect(cmd); };
    commands_[con_cell("connect",2)] = [this](const term cmd){ command_connect(cmd); };
    commands_[con_cell("list",0)] = [this](const term cmd){ command_list(cmd); };
    commands_[con_cell("kill",1)] = [this](const term cmd){ command_kill(cmd); };
    commands_[con_cell("kill",2)] = [this](const term cmd){ command_kill(cmd); };
    commands_[con_cell("close",1)] = [this](const term cmd){ command_close(cmd); };
}

void terminal::execute_command(const term cmd)
{
    auto it = commands_.find(env_.functor(cmd));
    if (it == commands_.end()) {
	add_error("Unknown command. Type 'help.' for information.");
	return;
    }
    (it->second)(cmd);
}

void terminal::execute_query(const term query)
{
    session *s = current_session_;
    if (s == nullptr) {
	add_error("No current session.");
	return;
    }
    uint64_t cost = 0;
    auto &e = s->env();
    term s_query = e.copy(query, env_, cost);
    auto q = e.new_term(e.functor("query",1), {s_query});
    s->send_query(q);
    if (process_errors(s)) {
	unregister_session(s);
	return;
    }
    process_query_reply();
}

void terminal::process_query_reply()
{
    session *s = current_session_;
    auto reply = s->read_reply();
    if (process_errors(s)) {
	unregister_session(s);
	return;
    }
    auto &e = s->env();
    if (e.functor(reply) == con_cell("error",1)) {
	add_error(e.to_string(e.arg(reply,0)));
	return;
    }

    if (e.functor(reply) == con_cell("ok",1)) {
	auto context = e.capture_state();
	auto result_term = e.arg(reply,0);
	if (e.functor(result_term) != con_cell("result",3)) {
	    add_error("Unexpected result. Expected result/3 inside ok/1, but got: " + e.to_string(result_term));
	    return;
	}
	auto in_query_state = e.arg(result_term,2);
	in_query_ = in_query_state == con_cell("more",0);
	
	auto touched = e.prettify_var_names(result_term);
	auto vars = e.arg(result_term,1);
	while (vars != e.empty_list()) {
	    if (!e.is_dotted_pair(vars)) {
		add_error("Unexpected result. Second argument of result/3 was not a proper list. " + e.to_string(e.arg(result_term,1)));
		return;
	    }
	    auto var_binding = e.arg(vars,0);
	    if (e.functor(var_binding) != con_cell("=",2)) {
		add_error("Unexpected result. Unexpected name binding: " + e.to_string(var_binding));
		return;
	    }
	    auto var_name_term = e.arg(var_binding, 0);
	    if (var_name_term.tag() != tag_t::CON) {
		add_error("Unexpected result. Variable name was not an atom: " + e.to_string(var_binding));
		return;
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
	return;
    }
    add_error("Unexpected reply from node: " + e.to_string(reply));
}

void terminal::execute_in_query(const std::string &cmd)
{
    session *s = current_session_;
    if (s == nullptr) {
	add_error("No current session.");
	in_query_ = false;
	return;
    }
    if (cmd == ";") {
	auto q = env_.new_term(env_.functor("command",1),{env_.functor("next",0)});
	s->send_query(q, env_);
	if (process_errors(s)) {
	    unregister_session(s);
	    return;
	}
	process_query_reply();
    } else if (cmd == "") {
	in_query_ = false;
    } else {
	add_error("Unknown command. Only ';' or ENTER is allowed.");
	in_query_ = true; // Error does normally clear "in query" flag
    }
}

void terminal::run()
{
    setup_commands();

    while (!stopped_) {
	// Set prompt
	if (current_session_ == nullptr) {
	    prompt_ = ">> ";
	} else {
	    if (in_query_) {
		prompt_ = "";
	    } else {
		prompt_ = "?- ";
	    }
	}

	std::cout << prompt_;
	std::cout.flush();

	ctrl_c_ = false;
	std::string cmd = readline_.read();
	std::cout << "\n";

	if (ctrl_c_) {
	    in_query_ = false;
	    if (current_session_ == nullptr) {
		std::cout << "Enter quit. to exit terminal." << std::endl;
		continue;
	    } else {
		current_session_ = nullptr;
		std::cout << "Leaving session. Going to top level." << std::endl;
		continue;
	    }
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

	    if (current_session_ == nullptr) {
		execute_command(t);
	    } else {
		execute_query(t);
	    }

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
