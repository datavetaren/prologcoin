#include "../common/term_serializer.hpp"
#include "../common/utime.hpp"
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
      timer_(self.get_io_service()),
      state_(IDLE),
      received_bytes_(0),
      receive_length_(0),
      sent_bytes_(0),
      send_length_(0),
      auto_send_(false),
      stopped_(false)
{
}

connection::~connection()
{
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
}

void connection::start()
{
    run();
}

void connection::stop()
{
    stopped_ = true;
    boost::system::error_code ec;
    socket_.cancel(ec);
}

inline boost::asio::io_service & connection::get_io_service()
{
    return self().get_io_service();
}

void connection::delete_connection(connection *conn)
{
    switch (conn->type()) {
    case CONNECTION_IN: delete reinterpret_cast<in_connection *>(conn); break;
    case CONNECTION_OUT: delete reinterpret_cast<out_connection *>(conn); break;
    default: break;
    }
}

void connection::close()
{
    self().close(this);
    set_state(CLOSED);
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

bool connection::received_length()
{
    auto &e = env_;

    cell c = term_serializer::read_cell(buffer_len_, 0,
	"node::connection::received_length");
    if (c.tag() != tag_t::INT) {
	if (auto_send()) {
	    send_error(e.functor("error_query_length_was_not_integer",0));
	}
	return false;
    } else {
	auto &ic=reinterpret_cast<const int_cell &>(c);
	size_t max = self_node::MAX_BUFFER_SIZE-sizeof(cell);
	if (ic.value() > static_cast<int>(max)) {
	    if (auto_send()) {
		send_error(e.new_term(
			      e.functor("error_query_length_exceeds_max",1),
			   {e.new_term(e.functor(">",2),
				       {ic, int_cell(max)})}));
	    }
	    return false;
	} else if (ic.value() < static_cast<int>(sizeof(cell))) {
	    if (auto_send()) {
		send_error(e.new_term(
		      e.functor("error_query_length_too_small",1),
		               {e.new_term(e.functor("<",2),
		                           {ic, int_cell(sizeof(cell))})}));
	    }
	    return false;
	} else {
	    receive_length_ = ic.value();
	    state_ = RECEIVE;
	    received_bytes_ = 0;
	    buffer_.resize(receive_length_);
	    return true;
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
	if (auto_send()) {
	    send_error(e.new_term(e.functor("serializer_exception",1),
				  {e.functor(ex.what(),0)}));
	}
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
				 bool r = received_length();
				 if (!r && !auto_send()) {
				     set_state(IDLE);
				 }
			     }
			     run();
			 } else {
			     set_state(ERROR);
			     dispatch();
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
			     set_state(ERROR);
			     dispatch();
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
			  set_state(ERROR);
			  dispatch();
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
			     set_state(ERROR);
			     dispatch();
			     close();
			 }
		  }));
	break;
    case RECEIVED: // These are never hit, because the state machine
    case SENT:     // has switched to another state already (if RECEIVED
    case CLOSED:   // or SENT.) Otherwise the connection is closed.
	break;
    case KILLED:
	dispatch();
	close();
	break;
    case ERROR:
	close();
	break;
    case IDLE:
	timer_.expires_from_now(boost::posix_time::microseconds(
			self().get_fast_timer_interval_microseconds()));
	timer_.async_wait(
	     strand_.wrap(
		     [this](const error_code &ec) {
			 if (!ec) {
			     dispatch();
			     run();
			 } else {
			     set_state(ERROR);
			     dispatch();
			     close();
			 }
		     }));
	break;
    }
}

in_connection::in_connection(self_node &self)
    : connection(self, CONNECTION_IN, env_),
      session_(nullptr)
{
    setup_commands();
    prepare_receive();
    set_auto_send(true);
    set_dispatcher( [this]() { this->on_state(); } );
    // std::cout << "in_connection::in_connection()" << std::endl;
}

in_connection::~in_connection()
{
    // std::cout << "in_connection::~in_connection()" << std::endl;
}

void in_connection::setup_commands()
{
    commands_[con_cell("new",0)] = [this](const term cmd){ command_new(cmd); };
    commands_[con_cell("connect",1)] = [this](const term cmd){ command_connect(cmd); };
    commands_[con_cell("kill",1)] = [this](const term cmd){ command_kill(cmd); };
    commands_[con_cell("next",0)] = [this](const term cmd){ command_next(cmd); };
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
    auto *ss = self().new_in_session(this);
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
    auto *s = self().find_in_session(id);
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
    self().in_session_connect(s, this);
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
    self().kill_in_session(s);
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
	if (session_ == nullptr) {
	    reply_error(e.functor("no_running_session",0));
	} else {
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
		    uint64_t cost = 0;
		    reply_error(e.new_term(e.functor("unrecognized_command",1)
				   ,{e.copy(cmd, session_->env(),cost)}));
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

//
// ----- out_connection
//

out_connection::out_connection(self_node &self, out_connection::out_type_t t, const ip_service &ip)
    :  connection(self, CONNECTION_OUT, env_), out_type_(t), ip_(ip), init_in_progress_(false), use_heartbeat_(true), connected_(false)
{
    using namespace boost::system;

    set_dispatcher( [this]() { this->on_state(); } );

    boost::asio::ip::tcp::endpoint endpoint(ip.to_addr(), ip.port());
    get_socket().async_connect(endpoint,
         strand().wrap(
		[this](const error_code &ec) {
		    if (!ec) {
			this->run();
		    } else {
			error(reason_t::ERROR_CANNOT_CONNECT,
			      "Could not connect: " + ec.message());
			this->close();
		    }
		}));
}

out_connection::~out_connection()
{
}

out_task out_connection::create_heartbeat_task()
{
    return out_task("heartbeat", *this, &out_connection::handle_heartbeat_task_fn);
}

void out_connection::handle_heartbeat_task_fn(out_task &task)
{
    task.connection().handle_heartbeat_task(task);
}

void out_connection::handle_heartbeat_task(out_task &task)
{
    static const con_cell colon(":", 2);
    static const con_cell me("me",0);

    switch (task.get_state()) {
    case out_task::IDLE:
	break;
    case out_task::RECEIVED:
	// Update address entry with most recent time
	task.self().book()().update_time(task.ip());
	reschedule(task, utime::us(self().get_timer_interval_microseconds()));
	break;
    case out_task::SEND:
	task.set_term(
	      env_.new_term(con_cell("query",1),
		    {env_.new_term(colon,
				   {me, env_.functor("heartbeat",0)})}));
	break;
    }
}

out_task out_connection::create_init_connection_task()
{
    return out_task("init_connection", *this, &out_connection::handle_init_connection_task_fn);
}

void out_connection::handle_init_connection_task_fn(out_task &task)
{
    task.connection().handle_init_connection_task(task);
}

void out_connection::handle_init_connection_task(out_task &task)
{
    static const con_cell ok("ok", 1);

    switch (task.get_state()) {
    case out_task::IDLE:
	break;
    case out_task::RECEIVED: {
	term t = task.get_term();
	if (t.tag() != tag_t::STR) {
	    error(reason_t::ERROR_FAIL_CONNECT,
		 "Unexpected response for init connection: "
		 + env_.to_string(t));
	    break;
	}
	auto f = env_.functor(t);
	if (f != ok) {
	    error(reason_t::ERROR_FAIL_CONNECT,
		  "Unexpected response for init connection: "
		  + env_.to_string(t));
	    break;
	}
	if (id_.empty()) {
	    auto id_term = env_.arg(t, 0);
	    if (!env_.is_atom(id_term)) {
		error(reason_t::ERROR_FAIL_CONNECT,
		      "Unexpected session id for init connection. "
		      "Expecting atom, was " + env_.to_string(t));
		break;
	    }
	    id_ = env_.atom_name(id_term);
	    schedule(task);
	} else {
	    connected_ = true;
	    if (use_heartbeat_) {
		auto hbtask = create_heartbeat_task();
		schedule(hbtask);
	    }
	}
	break;
        }
    case out_task::SEND:
	if (id_.empty()) {
	    task.set_term(
		  env_.new_term(con_cell("command",1), {con_cell("new",0)}));
	} else if (!connected_) {
	    task.set_term(
		  env_.new_term(con_cell("command",1),
				{env_.new_term(con_cell("connect",1),
					       {env_.functor(id_,0)})}));
	}
	break;
    }
}

void out_connection::idle_state()
{
    set_state(IDLE);
    // If 'id' is empty, then we don't have a session, so we need to
    // issue a command to create one.
    if (id_.empty() && !init_in_progress_) {
	init_in_progress_ = true;
	auto task = create_init_connection_task();
	schedule(task);
    }
}

void out_connection::error(const reason_t &reason,
			   const std::string &msg)
{
    //
    // Find address entry
    //
    
    address_entry entry;
    if (!self().book()().exists(ip(), &entry)) {
	// If we cannot find the entry in the book (for whatever reason)
	// we'll just end the session and quit. We won't reconnect anyway
	// because it's not in the address book.
	stop();
	return;
    }


    //
    // This is the base delta score we'll use to update the entry with.
    //
    int d_score = 0;

    switch (reason) {
    case reason_t::ERROR_CANNOT_CONNECT: d_score = -40; break;
    case reason_t::ERROR_FAIL_CONNECT: d_score = -80; break;
    case reason_t::ERROR_UNRECOGNIZED: d_score = -80; break;
    case reason_t::ERROR_SELF: d_score = 0; break; // Will be removed anyway
    case reason_t::ERROR_VERSION: d_score = -80; break;
    default: d_score = -10; break;
    }

    //
    // Next we'll "punish" w.r.t. the time distance when we last succeeded.
    // The longer the time period is, the more unlikely it is we'll ever
    // be connecting again. 24hrs = 100%. Otherwise linear scaling up to
    // 500% (max.)
    //

    auto d_time = utime::now() - entry.time();
    auto hours = static_cast<int>(d_time.in_hh());

    if (hours >= 24*5) {
	hours = 24*5;
    }
    //
    // In testing mode
    //
    if (self().is_testing_mode()) {
	if (hours < 1) hours = 1;
    }

    d_score = d_score * hours / 24;

    if (d_score != 0) {
	// std::cout << "Change score for entry: " << d_score << std::endl;
	self().book()().add_score(entry, d_score);
    }

    // std::cout << reason.str() << " " << msg << std::endl;
    stop();
}

void out_connection::send_next_task()
{
    // Don't pop work queue on send operations,
    // as send operations expect an answer. Once
    // the answer is processed, the task is done
    // and can be popped.
    if (work_.empty()) {
	idle_state();
	return;
    }
    auto next_task = work_.top();
    if (!next_task.expiring()) {
	idle_state();
	return;
    }

    next_task.set_state(out_task::SEND);
    next_task.set_term(term());
    next_task.run();
    if (next_task.get_term() == term()) {
	set_state(IDLE);
    } else {
	send(next_task.get_term());
    }
    if (get_state() == IDLE) {
	idle_state();
    }
}

void out_connection::on_state()
{
    switch (get_state()) {
    case IDLE: if (!is_stopped()) send_next_task(); break;
    case RECEIVED: {
	auto task = work_.top();
	work_.pop();
	auto r = received();
	task.set_state(out_task::RECEIVED);
	task.set_term(r);
	task.run();
	if (!is_stopped()) {
	    send_next_task();
	}
	break;
        }
    case SENT:
	prepare_receive();
	break;
    case ERROR:
	error(reason_t::ERROR_UNRECOGNIZED, "Probable node shutdown.");
	break;
    default:
	break;
    }

    if (is_stopped()) {
	set_state(KILLED);
    }
}

void out_connection::print_task_queue() const
{
    auto temp = work_;
    while (!temp.empty()) {
	auto task = temp.top();
	std::cout << task.get_when().str() << ": " << task.description() << std::endl;
	temp.pop();
    }
}

}}
