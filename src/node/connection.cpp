#include "../common/term_serializer.hpp"
#include "../common/utime.hpp"
#include "../common/term_match.hpp"
#include "../common/checked_cast.hpp"
#include "connection.hpp"
#include "session.hpp"
#include "self_node.hpp"
#include "task_heartbeat.hpp"
#include "task_publish.hpp"
#include "task_info.hpp"
#include "task_init_connection.hpp"
#include "task_reset.hpp"

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
      state_(STATE_IDLE),
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
    // std::cout << "connection::~connection(): this=" << this << " port=" << self_node_.port() << std::endl;
    boost::system::error_code ec;
    socket_.cancel(ec);
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
    socket_.release(ec);
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
    set_state(STATE_CLOSED);
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
    state_ = STATE_SEND_LENGTH;
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
	    state_ = STATE_RECEIVE;
	    received_bytes_ = 0;
	    buffer_.resize(receive_length_);
	    return true;
	}
    }
}

term connection::received()
{
    return received(env_);
}

term connection::received(term_env &env)
{
    auto &e = env;
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
    case STATE_RECEIVE_LENGTH:
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
				     set_state(STATE_IDLE);
				 }
			     }
			     run();
			 } else {
			     std::stringstream msg;
			     msg << "While receiving length: n=" << n << "; error: " << ec.message();
			     set_state_error(msg.str());
			     dispatch();
			     close();
			 }
		  }));
	break;
    case STATE_RECEIVE: {
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
				 state_ = STATE_RECEIVED;
			     }
			     dispatch();
			     run();
			 } else {
			     std::stringstream msg;
			     msg << "While receiving data: n=" << n << "; error: " << ec.message();
			     set_state_error(msg.str());
			     dispatch();
			     close();
			 }
		  }));
	break;
        }
    case STATE_SEND_LENGTH:
	get_socket().async_write_some(buffer(&buffer_len_[sent_bytes_],
					     sizeof(cell) - sent_bytes_),
             strand_.wrap(
		  [this](const error_code &ec, size_t n) {
		      if (!ec) {
			  sent_bytes_ += n;
			  if (sent_bytes_ >= sizeof(cell)) {
			      state_ = STATE_SEND;
			      sent_bytes_ = 0;
			  }
			  run();
		      } else {
			  std::stringstream msg;
			  msg << "While sending len: n=" << n << "; error: " << ec.message();
			  set_state_error(msg.str());
			  dispatch();
			  close();
		      }
		  }));
	break;
    case STATE_SEND:
	get_socket().async_write_some(buffer(&buffer_[sent_bytes_],
					     send_length_ - sent_bytes_),
	     strand_.wrap(
		  [this](const error_code &ec, size_t n) {
		         if (!ec) {
			     sent_bytes_ += n;
			     if (sent_bytes_ >= buffer_.size()) {
				 state_ = STATE_SENT;
				 received_bytes_ = 0;
				 sent_bytes_ = 0;
				 dispatch();
			     }
			     run();
		         } else {
			     std::stringstream msg;
			     msg << "While sending data: n=" << n << "; error: " << ec.message();
			     set_state_error(msg.str());
			     dispatch();
			     close();
			 }
		  }));
	break;
    case STATE_RECEIVED: // These are never hit, because the state machine
    case STATE_SENT:     // has switched to another state already (if RECEIVED
    case STATE_CLOSED:   // or SENT.) Otherwise the connection is closed.
	break;
    case STATE_KILLED:
	dispatch();
	close();
	break;
    case STATE_ERROR:
	close();
	break;
    case STATE_IDLE:
	timer_.expires_from_now(boost::posix_time::microseconds(
			self().get_fast_timer_interval_microseconds()));
	timer_.async_wait(
		  strand_.wrap(
		     [this](const error_code &ec) {
			 if (!ec) {
			     dispatch();
			     run();
			 } else {
			     if (is_stopped()) {
				 set_state_error("Node stopped");
				 dispatch();
				 close();
			     } else {
				 // This timer got retriggered.
				 dispatch();
				 run();
			     }
			 }
		     })
			  );
	break;
    }
}

void connection::trigger_now()
{
    timer_.expires_from_now(boost::posix_time::microseconds(
			    self().get_fast_timer_interval_microseconds()));
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
    // std::cout << self().port() << ": in, this=" << this << std::endl;
    // std::cout << "in_connection::~in_connection()" << std::endl;
}

void in_connection::setup_commands()
{
    commands_[con_cell("new",0)] = [this](const term cmd){ command_new(cmd); };
    commands_[con_cell("connect",1)] = [this](const term cmd){ command_connect(cmd); };
    commands_[con_cell("kill",1)] = [this](const term cmd){ command_kill(cmd); };
    commands_[con_cell("next",0)] = [this](const term cmd){ command_next(cmd); };
    commands_[con_cell("delinst",0)] = [this](const term cmd){ command_delete_instance(cmd); };
    commands_[con_cell("reset",0)] = [this](const term cmd){ command_reset(cmd); };
    commands_[con_cell("lreset",0)] = [this](const term cmd){ command_local_reset(cmd); };
    commands_[con_cell("name",1)] = [this](const term cmd){ command_name(cmd); };
}

void in_connection::on_state()
{
    switch (get_state()) {
    case STATE_RECEIVED: process_query(); break;
    case STATE_SENT: prepare_receive(); break;
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

void in_connection::reply_exception(const std::string &msg)
{
    auto &e = env_;
    reply_error(e.new_term(e.functor("remote_exception",1),
    			   {e.string_to_list(msg)}));
}

void in_connection::command_new(const term)
{
    // Check if this is a local IP address (if yes, then allow root access)
    bool do_root = false;
    ip_address ip(get_socket().remote_endpoint().address());

    if (ip.is_local() && self().is_grant_root_for_local()) {
        do_root = true;
    }
    auto *ss = self().new_in_session(this, do_root);
    reply_ok(env_.new_term(con_cell("session",2),
			   {env_.functor(ss->id(),0),
			    env_.functor(self().name(),0)}));
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
    } else if (session_->at_end()) {
	return con_cell("at_end",0);
    } else if (session_->has_more()) {
	return con_cell("more",0);
    } else {
	return con_cell("ask",0);
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

void in_connection::command_name(const term cmd)
{
    auto &e = env_;
    term name_term = e.arg(cmd, 0);
    if (!e.is_atom(name_term)) {
	reply_error(e.new_term(e.functor("erroneous_name",1),{name_term}));
	return;
    }
    name_ = e.atom_name(name_term);
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

void in_connection::command_delete_instance(const term cmd)
{
    auto &e = env_;
    if (session_ == nullptr) {
	reply_error(e.functor("no_running_session",0));
    } else {
	try {
	    session_->delete_instance();
	    reply_ok(e.new_term(e.functor("result",5),
				{e.EMPTY_LIST,
				 e.EMPTY_LIST,
			 	 e.EMPTY_LIST,
			 	 e.EMPTY_LIST,
	 			 e.EMPTY_LIST
					} ));
	} catch (std::exception &ex) {
	    reply_exception(ex.what());
	}
    }
}

void in_connection::command_reset(const term cmd)
{
    auto &e = env_;
    if (session_ == nullptr) {
	reply_error(e.functor("no_running_session",0));
    } else {
	try {
	    session_->reset();
	    reply_ok(e.new_term(e.functor("result",5),
				{e.EMPTY_LIST,
				 e.EMPTY_LIST,
			 	 e.EMPTY_LIST,
			 	 e.EMPTY_LIST,
			 	 e.EMPTY_LIST
					} ));
	} catch (std::exception &ex) {
	    reply_exception(ex.what());
	}
    }
}

void in_connection::command_local_reset(const term cmd)
{
    auto &e = env_;
    if (session_ == nullptr) {
	reply_error(e.functor("no_running_session",0));
    } else {
	try {
	    session_->local_reset();
	    reply_ok(e.new_term(e.functor("result",5),
				{e.EMPTY_LIST,
				 e.EMPTY_LIST,
			 	 e.EMPTY_LIST,
			 	 e.EMPTY_LIST,
			 	 e.EMPTY_LIST
					} ));
	} catch (std::exception &ex) {
	    reply_exception(ex.what());
	}
    }
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
		process_execution(qr, false);
	    } catch (std::exception &ex) {
		reply_exception(ex.what());
	    }
	}
    } else {
	reply_error(e.new_term(e.functor("unrecognized_command",1),{t}));
    }
}

std::string in_connection::to_error_message(const std::vector<std::string> &msgs)
{
    std::stringstream ss;
    for (auto &msg : msgs) {
	ss << msg << "\n";
    }
    return ss.str();
}

std::string in_connection::to_error_message(const token_exception &ex)
{
    return to_error_message(env_.to_error_messages(ex));
}

std::string in_connection::to_error_message(const term_parse_exception &ex)
{
    return to_error_message(env_.to_error_messages(ex));
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
	    auto qr = cmd;
	    // std::cout << "QUERY: " << session_->env().to_string(qr) << std::endl;
	    bool r = in_query ? session_->next() : session_->execute(qr);
	    term standard_out = e.EMPTY_LIST;
	    if (!session_->get_text_out().empty()) {
		standard_out = e.string_to_list(session_->get_text_out());
		session_->reset_text_out();
	    }
	    auto last_cost = static_cast<int64_t>(session_->last_cost());
	    if (!r) {
		reply_ok(e.new_term(e.functor("result",5),
				    {e.functor("false",0),
				     e.EMPTY_LIST,
		 		     get_state_atom(),
			 	     standard_out,
				     int_cell(last_cost) } ));
	    } else {
		// We're just copying the result from the session to our
		// own lcoal heap. There's no danger in creating coins out of thin
		// air.
		auto disabled = e.disable_coin_security();

		term closure = session_->query_closure();
		term closure_copy = e.copy(closure, session_->env(), cost);
		term result = e.arg(closure_copy, 0);
		term vars_term = e.arg(closure_copy, 1);

		result = e.new_term(e.functor("result",5),
				    {result, vars_term, get_state_atom(),
				     standard_out,
				     int_cell(last_cost) } );
		reply_ok(result);
	    }
	}
    } catch (const token_exception &ex) {
	reply_exception(to_error_message(ex));
    } catch (const term_parse_exception &ex) {
	reply_exception(to_error_message(ex));
    } catch (const std::exception &ex) {
	reply_exception(ex.what());
    }
}

//
// ----- out_connection
//

out_connection::out_connection(self_node &self, out_connection::out_type_t t, const ip_service &ip)
    :  connection(self, CONNECTION_OUT, env_), out_type_(t), ip_(ip), init_in_progress_(false), use_heartbeat_(true), connected_(false), sent_my_name_(false), work_( &out_task::comparator )
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
    // std::cout << self().port() << ": out=" << ip().port() << " this=" << this << std::endl;

    boost::lock_guard<boost::recursive_mutex> guard(work_lock_);

    while (!work_.empty()) {
	out_task *t = work_.top();
        delete t;
	work_.pop();
    }
}

out_task * out_connection::create_heartbeat_task()
{
    return new task_heartbeat(*this);
}

out_task * out_connection::create_publish_task()
{
    return new task_publish(*this);
}

out_task * out_connection::create_info_task()
{
    return new task_info(*this);
}

out_task * out_connection::create_init_connection_task()
{
    return new task_init_connection(*this);
}

task_reset * out_connection::create_reset_task()
{
    return new task_reset(*this);
}

void out_connection::idle_state()
{
    set_state(STATE_IDLE);
    // If 'id' is empty, then we don't have a session, so we need to
    // issue a command to create one.
    if (id_.empty() && !init_in_progress_) {
	init_in_progress_ = true;
	auto task = create_init_connection_task();
	schedule(task);
    }
}

void out_connection::reschedule(out_task *task, utime t)
{
    {
    boost::lock_guard<boost::recursive_mutex> guard(work_lock_);

    // If task issues a reschedule on SEND, which normally it doesn't
    // as it doesn't pop the work queue, then we'll pop the work queue
    // to remove it first.
    if (task->get_state() == out_task::SEND) {
	work_.pop();
    }
    
    task->set_when(t);
    if (t > last_in_work_) {
	last_in_work_ = t;
    }
    work_.push(task);
    }

    trigger_now();
}

void out_connection::error(const reason_t &reason,
			   const std::string &msg)
{
    // std::cout << "out_connection: " << "self=" << self().port() << " to=" << ip().port() << " error=" << msg << " last_error=" << last_error() << std::endl;
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

    self().failed_connection(entry);

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
    if (!next_task->expiring()) {
	idle_state();
	return;
    }
    if (next_task->get_state() == out_task::KILLED) {
	delete next_task;
	work_.pop();
	return;
    }
    next_task->set_state(out_task::SEND);
    next_task->set_term(term());
    next_task->process();
    if (next_task->get_term() == term()) {
	set_state(STATE_IDLE);
    } else {
	send(next_task->get_term());
    }
    if (get_state() == STATE_IDLE) {
	idle_state();
    }
}

void out_connection::on_state()
{
    boost::lock_guard<boost::recursive_mutex> guard(work_lock_);

    switch (get_state()) {
    case STATE_IDLE: if (!is_stopped()) send_next_task(); break;
    case STATE_RECEIVED: {
	auto task = work_.top();
	work_.pop();
	auto r = received(task->env());
	task->set_state(out_task::RECEIVED);
	task->set_term(r);
	task->process();
	if (task->get_state() == out_task::KILLED) {
	    delete task;
	}
	if (!is_stopped()) {
	    send_next_task();
	}
	break;
        }
    case STATE_SENT:
	prepare_receive();
	break;
    case STATE_ERROR:
	error(reason_t::ERROR_UNRECOGNIZED, "Probable node shutdown.");
	break;
    default:
	break;
    }

    if (is_stopped()) {
	set_state(STATE_KILLED);
    }
}

void out_connection::print_task_queue() const
{
    auto temp = work_;
    while (!temp.empty()) {
	auto task = temp.top();
	std::cout << task->get_when().str() << ": " << task->description() << std::endl;
	temp.pop();
    }
}

}}
