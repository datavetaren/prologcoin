#include "../common/term_serializer.hpp"
#include "connection.hpp"
#include "session.hpp"
#include "self_node.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace node {

connection::connection(self_node &self,
		       connection::connection_type type,
		       term_env &env)
    : self_node_(self),
      type_(type),
      env_(env),
      strand_(self.get_io_service()),
      socket_(self.get_io_service()),
      state_(IDLE),
      received_bytes_(0),
      receive_length_(0),
      sent_bytes_(0),
      send_length_(0)
{
}


inline boost::asio::io_service & connection::get_io_service()
{
    return node().get_io_service();
}

void connection::delete_connection(connection *conn)
{
    switch (conn->type()) {
    case IN: delete reinterpret_cast<in_connection *>(conn); break;
    default: break;
    }
}

void connection::close()
{
    node().close(this);
}

void connection::send_error(const term t)
{
    send(env_.new_term(env_.functor("error",1),{t}));
}

void connection::send_ok(const term t)
{
    send(env_.new_term(env_.functor("ok",1),{t}));
}

void connection::send(const term t)
{
    term_serializer ser(env_);
    buffer_.clear();
    ser.write(buffer_, t);
    buffer_len_.resize(sizeof(cell));
    ser.write_cell(buffer_len_, 0, int_cell(buffer_.size()));
    sent_bytes_ = 0;
    send_length_ = buffer_.size();
    state_ = SEND_LENGTH;
}

void connection::received_length()
{
    auto &e = env_;

    cell c = term_serializer::read_cell(buffer_len_, 0,
	"node::connection::received_length");
    if (c.tag() != tag_t::INT) {
	send_error(e.functor("error_query_length_was_not_integer",0));
    } else {
	auto &ic=reinterpret_cast<const int_cell &>(c);
	size_t max = self_node::MAX_BUFFER_SIZE-sizeof(cell);
	if (ic.value() > static_cast<int>(max)) {
	    send_error(e.new_term(
			   e.functor("error_query_length_exceeds_max",1),
			   {e.new_term(e.functor(">",2),
				       {ic, int_cell(max)})}));
	} else if (ic.value() < static_cast<int>(sizeof(cell))) {
	    send_error(e.new_term(e.functor("error_query_length_too_small",1),
			   {e.new_term(e.functor("<",2),
				       {ic, int_cell(sizeof(cell))})}));
	} else {
	    receive_length_ = ic.value();
	    state_ = RECEIVE;
	    received_bytes_ = 0;
	    buffer_.resize(receive_length_);
	}
    }
}

term connection::received()
{
    auto &e = env_;
    term_serializer ser(e);
    try {
	auto t = ser.read(buffer_, receive_length_);
	return t;
    } catch (serializer_exception &ex) {
	send_error(e.new_term(e.functor("serializer_exception",1),
			       {e.functor(ex.what(),0)}));
	return term();
    }
}

void connection::run()
{
    using namespace boost::asio;
    using namespace boost::system;

    switch (state_) {
    case RECEIVE_LENGTH:
        buffer_len_.resize(sizeof(cell));
	get_socket().async_read_some(buffer(&buffer_len_[received_bytes_],
					    sizeof(cell) - received_bytes_),
	     strand_.wrap(
		  [this](const error_code &ec, size_t n) {
			 if (!ec) {
			     received_bytes_ += n;
			     if (received_bytes_ >= sizeof(cell)) {
				 received_length();
			     }
			     run();
			 } else {
			     close();
			 }
		  }));
	break;
    case RECEIVE: {
	size_t to_read = buffer_.size() - received_bytes_;
	if (to_read > receive_length_ - received_bytes_) {
	    to_read = receive_length_ - received_bytes_;
	}
	get_socket().async_read_some(buffer(&buffer_[received_bytes_],to_read),
	     strand_.wrap(
		  [this](const error_code &ec, size_t n) {
			 if (!ec) {
			     received_bytes_ += n;
			     if (received_bytes_ >= receive_length_) {
				 state_ = RECEIVED;
			     }
			     dispatch();
			     run();
			 } else {
			     close();
			 }
		  }));
	break;
        }
    case SEND_LENGTH:
	get_socket().async_write_some(buffer(&buffer_len_[sent_bytes_],
					     sizeof(cell) - sent_bytes_),
             strand_.wrap(
		  [this](const error_code &ec, size_t n) {
		      if (!ec) {
			  sent_bytes_ += n;
			  if (sent_bytes_ >= sizeof(cell)) {
			      state_ = SEND;
			      sent_bytes_ = 0;
			  }
			  run();
		      } else {
			  close();
		      }
		  }));
	break;
    case SEND:
	get_socket().async_write_some(buffer(&buffer_[sent_bytes_],
					     send_length_ - sent_bytes_),
	     strand_.wrap(
		  [this](const error_code &ec, size_t n) {
		         if (!ec) {
			     sent_bytes_ += n;
			     if (sent_bytes_ >= buffer_.size()) {
				 state_ = SENT;
				 received_bytes_ = 0;
				 sent_bytes_ = 0;
				 dispatch();
			     }
			     run();
		         } else {
			     close();
			 }
		  }));
	break;
    case RECEIVED:
    case SENT:
    case IDLE:
	break;
    }
}

in_connection::in_connection(self_node &self)
    : connection(self, IN, env_),
      session_(nullptr)
{
    setup_commands();
    prepare_receive();
    set_dispatcher( [this]() { this->on_state(); } );
    std::cout << "in_connection::in_connection()\n";
}

in_connection::~in_connection()
{
    std::cout << "in_connection::~in_connection()\n";
}

void in_connection::setup_commands()
{
    commands_[con_cell("new",0)] = [this](const term cmd){ command_new(cmd); };
    commands_[con_cell("connect",1)] = [this](const term cmd){ command_connect(cmd); };
    commands_[con_cell("kill",1)] = [this](const term cmd){ command_kill(cmd); };
    commands_[con_cell("next",0)] = [this](const term cmd){ command_next(cmd); };
}

void in_connection::start()
{
    run();
}

void in_connection::on_state()
{
    switch (get_state()) {
    case RECEIVED: process_query(); break;
    case SENT: prepare_receive(); break;
    default: break;
    }
}

void in_connection::reply_ok(const term t)
{
    send_ok(t);
}

void in_connection::reply_error(const term t)
{
    send_error(t);
}

void in_connection::command_new(const term)
{
    auto *ss = node().new_in_session(this);
    reply_ok(env_.functor(ss->id(),0));
}

in_session_state * in_connection::get_session(const term id_term)
{
    auto &e = env_;
    if (!e.is_atom(id_term)) {
	reply_error(e.new_term(e.functor("erroneous_session_id",1),{id_term}));
	return nullptr;
    }
    std::string id = e.atom_name(id_term);
    auto *s = node().find_in_session(id);
    if (s == nullptr) {
	reply_error(e.new_term(e.functor("session_not_found",1),{id_term}));
	return nullptr;
    }
    return s;
}

con_cell in_connection::get_state_atom()
{
    if (session_ == nullptr) {
	return con_cell("void",0);
    } else if (!session_->in_query()) {
	return con_cell("ask",0);
    } else if (session_->has_more()) {
	return con_cell("more",0);
    } else {
	return con_cell("unknown",0);
    }
}

void in_connection::command_connect(const term cmd)
{
    auto &e = env_;
    term id_term = e.arg(cmd,0);

    auto *s = get_session(id_term);
    if (s == nullptr) {
	return;
    }
    node().in_session_connect(s, this);
    session_ = s;
    reply_ok(e.new_term(e.functor("session_resumed",2),
			{id_term, get_state_atom()}));
}

void in_connection::command_kill(const term cmd)
{
    auto &e = env_;
    term id_term = e.arg(cmd,0);

    auto *s = get_session(id_term);
    if (s == nullptr) {
	return;
    }
    node().kill_in_session(s);
    session_ = nullptr;
    reply_ok(e.new_term(e.functor("session_killed",1),{id_term}));
}

void in_connection::command_next(const term cmd)
{
    process_execution(cmd, true);
}

void in_connection::process_command(const term cmd)
{
    auto &e = env_;

    auto it = commands_.find(e.functor(cmd));
    if (it == commands_.end()) {
	reply_error(e.new_term(e.functor("unrecognized_commmand",1),{cmd}));
	return;
    }
    (it->second)(cmd);
}

void in_connection::process_query()
{
    auto &e = env_;
    auto t = received();
    if (t == term()) {
	return;
    }
    if (t.tag() != tag_t::STR) {
	reply_error(e.new_term(e.functor("unrecognized_command",1),{t}));
	return;
    }
    auto f = e.functor(t);
    if (f == con_cell("command",1)) {
	process_command(e.arg(t,0));
    } else if (f == con_cell("query",1)) {
	term qr;
	try {
	    uint64_t cost = 0;
	    qr = session_->env().copy(e.arg(t,0), e, cost);
	    session_->set_query(qr);
	    process_execution(qr, false);
	} catch (std::exception &ex) {
	    reply_error(e.new_term(e.functor("remote_exception",1),
				   {e.functor(ex.what(),0)}));
	}
    } else {
	reply_error(e.new_term(e.functor("unrecognized_command",1),{t}));
    }
}

void in_connection::process_execution(const term cmd, bool in_query)
{
    auto &e = env_;
    try {
	if (session_ == nullptr) {
	    reply_error(e.functor("no_running_session",0));
	} else {
	    uint64_t cost = 0;
	    if (in_query) {
		if (cmd != con_cell("next",0)) {
		    reply_error(e.new_term(e.functor("unrecognized_command",1)
					   ,{cmd}));
		    return;
		}
	    }
	    auto qr = session_->query();
	    bool r = in_query ? session_->next() : session_->execute(qr);
	    if (!r) {
		reply_ok(e.new_term(e.functor("result",3),
				    {e.functor("false",0),
				     e.empty_list(),
					    get_state_atom()}));
	    } else {
		term closure = session_->env().new_dotted_pair(
				      session_->get_result(),
				      session_->query_vars());
		term closure_copy = e.copy(closure, session_->env(), cost);
		term result = e.arg(closure_copy, 0);
		term vars_term = e.arg(closure_copy, 1);

		result = e.new_term(e.functor("result",3),
				    {result, vars_term, get_state_atom()});

		reply_ok(result);
	    }
	}
    } catch (std::exception &ex) {
	reply_error(e.new_term(e.functor("remote_exception",1),
			       {e.functor(ex.what(),0)}));
    }
}

}}
