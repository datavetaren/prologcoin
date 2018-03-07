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
    term_serializer ser(env_);
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
    if (ic.value() < sizeof(cell)) {
	std::stringstream ss;
	ss << "Length of reply too small (" << ic.value() << " < "
	   << sizeof(cell) << std::endl;
	add_error(ss.str());
	return term();
    }

    if (ic.value() > self_node::MAX_BUFFER_SIZE) {
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

terminal::terminal() : prompt_(">> "),
		       stopped_(false),
		       ctrl_c_(false),
		       ioservice_(),
		       current_session_(nullptr)
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
	    while (!text_output_queue_.empty()) {
		auto line = text_output_queue_.front();
		std::cout << line << std::endl;
		text_output_queue_.pop();
	    }
	    std::cout << prompt_;
	    std::cout.flush();
	    readline_.clear_render();
	    readline_.render();
	}
	return false;
    }

    return readline_.has_standard_handling(ch);
}

void terminal::error(const std::string &cmd,
		     int column,
		     token_exception *token_ex,
		     term_parse_exception *parse_ex)
{
    std::cout << "[ERROR]: While parsing at column " << column;
    if (parse_ex != nullptr) {
        std::cout << " for parse state: " << std::endl;
	auto &desc = parse_ex->state_description();
	for (auto &line : desc) {
	    std::cout << "[ERROR]:   " << line << std::endl;
	}
	std::cout << "[ERROR]: Expected: {";
	bool first = true;
	for (auto exp : env_.get_expected_simplified(*parse_ex)) {
	    if (!first) std::cout << ", ";
	    if (!isalnum(exp[0])) {
		exp = '\'' + exp + '\'';
	    }
	    std::cout << exp;
	    first = false;
	}
	std::cout << "}" << std::endl;
    } if (token_ex != nullptr) {
	std::cout << ": " << std::endl;
	std::cout << "[ERROR]:   " << token_ex->what() << std::endl;
    } else {
	std::cout << ": " << std::endl;
    }

    int pos = column - 1;
    int start_excerpt = std::max(pos-20, 0);
    int end_excerpt = std::max(pos+10, static_cast<int>(cmd.size()));
    int excerpt_len = end_excerpt - start_excerpt;
    auto excerpt = cmd.substr(start_excerpt, excerpt_len);
    std::cout << "[ERROR]: ";
    if (start_excerpt > 0) {
	std::cout << "...";
    }
    std::cout << excerpt;
    if (excerpt_len < cmd.size()) {
	std::cout << "...";
    }
    std::cout << std::endl;
    std::cout << "[ERROR]: ";
    if (start_excerpt > 0) {
	std::cout << "   ";
    }
    std::cout << std::string(pos-start_excerpt, ' ') << "^" << std::endl;
}

void terminal::lock_text_output_and( std::function<void()> fn )
{
    boost::lock_guard<boost::mutex> guard(text_output_queue_mutex_);
    fn();
}

void terminal::add_text_output(const std::string &str)
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
    std::cout << R"TEXT(
  help.                         This information.
  list.                         List current sessions.
  new(Host:Port).               Create a new session at host (with port)
  connect(Session).             Connect to a session.
  connect(Session,[Host:Port]). Connect to a session at specific host:port.
  close(Session).               Disconnect session. (But keep session at node.)
  kill(Session,[Host:Port]).    Kill session (and at node too.)
  quit.                         End terminal.

)TEXT";

}

void terminal::command_list(const term)
{
    std::cout << std::endl;
    if (sessions_.empty()) {
	std::cout << " --- no sessions --- " << std::endl;
    }
    for (auto it : sessions_) {
	auto *session = it.second;
	std::cout << std::setw(26) << session->id() << " : " << session->get_endpoint() << std::endl;
    }
    std::cout << std::endl;
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
	    std::cout << "new: Expecting host port as an integer; was "
		      << env_.to_string(port_term) << std::endl;
	    return nullptr;
	}
	port = reinterpret_cast<int_cell &>(port_term).value();
    } else {
	host_term = arg;
    }
    if (!env_.is_atom(host_term)) {
	std::cout << "new: Expecting host name as an atom; was "
		  << env_.to_string(host_term) << std::endl;
	return nullptr;
    }
    host = env_.atom_name(host_term);

    tcp::resolver resolver(ioservice_);
    tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
    
    boost::system::error_code ec = boost::asio::error::host_not_found;
    auto iterator = resolver.resolve(query, ec);

    if (ec) {
	std::cout << "Error while resolving host " << host << ":" << port << ": " << ec.message() << "\n";
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
	std::cout << "Could not connect to " << host << ":" << port << "\n";
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
	    std::cout << line << std::endl;
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
	std::cout << "Node replied with failure: " << env_.to_string(reply) << std::endl;
	return;
    }

    auto session_id_term = e.arg(reply, 0);
    if (!e.is_atom(session_id_term)) {
	std::cout << "Node did not reply with session id: " << e.to_string(session_id_term) << std::endl;
    }

    std::string session_id = e.atom_name(session_id_term);
    s->set_id(session_id);
    register_session(s);

    std::cout << session_id << std::endl;
}

session * terminal::get_session(const term cmd)
{
    auto session_id_term = env_.arg(cmd,0);
    if (!env_.is_atom(session_id_term)) {
	std::cout << "Erroneous session id: " << env_.to_string(session_id_term) << std::endl;
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
	    std::cout << "Session '" << session_id << "' was not found, and no host:port was provided." << std::endl;
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
	std::cout << "Unexpected reply from node: " << e.to_string(reply) << std::endl;
	unregister_session(s);
	return;
    }

    auto inner = e.arg(reply, 0);
    auto inner_f = e.functor(inner);
    auto inner_name = e.atom_name(inner_f);
    if (inner_name != "session_resumed" || inner_f.arity() != 1) {
	std::cout << "Unexpected reply from node: " << e.to_string(reply) << std::endl;
	unregister_session(s);
	return;
    }
    
    auto id_term = e.arg(inner,0);
    if (!e.is_atom(id_term)) {
	delete s;
	std::cout << "Unexpected session id from node: " << e.to_string(id_term) << std::endl;
	return;
    }
    auto id = e.atom_name(id_term);
    s->set_id(id);
    register_session(s);
    current_session_ = s;

    std::cout << "Connecting to session '" << s->id() << "'" << std::endl;
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
	std::cout << "Node replied with failure: " << e.to_string(reply) << std::endl;
	return;
    }

    std::cout << "Session '" << s->id() << "' killed." << std::endl;
    unregister_session(s);
}

void terminal::command_close(const term cmd)
{
    auto session_id_term = env_.arg(cmd,0);
    if (!env_.is_atom(session_id_term)) {
	std::cout << "Erroneous session id: " << env_.to_string(session_id_term) << std::endl;
	return;
    }
    auto session_id = env_.atom_name(session_id_term);
    session *s = nullptr;
    if (sessions_.find(session_id) == sessions_.end()) {
	std::cout << "Session '" << session_id << "' was not found." << std::endl;
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
	std::cout << "Unknown command. Type 'help.' for information."
		  << std::endl;
	return;
    }
    (it->second)(cmd);
}

void terminal::execute_query(const term query)
{
    (void)query;
    std::cout << "execute_query" << std::endl;
}

void terminal::run()
{
    setup_commands();

    while (!stopped_) {
	// Set prompt
	if (current_session_ == nullptr) {
	    prompt_ = ">> ";
	} else {
	    prompt_ = "?- ";
	}

	std::cout << prompt_;
	std::cout.flush();

	ctrl_c_ = false;
	std::string cmd = readline_.read();
	std::cout << "\n";

	if (ctrl_c_) {
	    if (current_session_ == nullptr) {
		std::cout << "Enter quit. to exit terminal." << std::endl;
		continue;
	    } else {
		current_session_ = nullptr;
		std::cout << "Leaving session. Go to top level command." << std::endl;
		continue;
	    }
	}

	if (stopped_) {
	    continue;
	}
	readline_.add_history(cmd);
	try {
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
