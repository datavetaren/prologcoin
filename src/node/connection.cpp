#include "../common/term_serializer.hpp"
#include "../common/utime.hpp"
#include "../common/term_match.hpp"
#include "../common/checked_cast.hpp"
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
    commands_[con_cell("name",1)] = [this](const term cmd){ command_name(cmd); };
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
	    term standard_out = e.empty_list();
	    if (!session_->get_text_out().empty()) {
		standard_out = e.string_to_list(session_->get_text_out());
	    }
	    if (!r) {
		reply_ok(e.new_term(e.functor("result",4),
				    {e.functor("false",0),
				     e.empty_list(),
		 		     get_state_atom(),
				     standard_out} ));
	    } else {
		term closure = session_->env().new_dotted_pair(
				      session_->get_result(),
				      session_->query_vars());
		term closure_copy = e.copy(closure, session_->env(), cost);
		term result = e.arg(closure_copy, 0);
		term vars_term = e.arg(closure_copy, 1);

		result = e.new_term(e.functor("result",4),
				    {result, vars_term, get_state_atom(),
				     standard_out} );
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
    :  connection(self, CONNECTION_OUT, env_), out_type_(t), ip_(ip), init_in_progress_(false), use_heartbeat_(true), connected_(false), sent_my_name_(false)
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

out_task out_connection::create_publish_task()
{
    return out_task("publish", *this, &out_connection::handle_publish_task_fn);
}

out_task out_connection::create_info_task()
{
    return out_task("info", *this, &out_connection::handle_info_task_fn);
}

void out_connection::handle_heartbeat_task_fn(out_task &task)
{
    task.connection().handle_heartbeat_task(task);
}

void out_connection::handle_publish_task_fn(out_task &task)
{
    task.connection().handle_publish_task(task);
}

void out_connection::handle_info_task_fn(out_task &task)
{
    task.connection().handle_info_task(task);
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

//
// Publish my own address
// (this will be optional in future, because a mobile wallet may not
//  have an official address for incoming connections.)
//
void out_connection::handle_publish_task(out_task &task)
{
    static const con_cell colon(":", 2);
    static const con_cell me("me",0);

    switch (task.get_state()) {
    case out_task::IDLE:
	break;
    case out_task::RECEIVED:
	// This is a one time event only.
	break;
    case out_task::SEND:
	task.set_term(
	      env_.new_term(con_cell("query",1),
		    {env_.new_term(colon, {me,
		     env_.new_term(env_.functor("add_address",2),
				   {env_.empty_list(),
					   int_cell(task.self().port())})})
		    }
		    ));
	break;
    }
}


//
// Grab version and comment info. Update address book.
//
void out_connection::handle_info_task(out_task &task)
{
    using namespace prologcoin::common;

    static const con_cell colon(":", 2);
    static const con_cell comma(",", 2);
    static const con_cell me("me",0);
    static const con_cell version_1("version",1);
    static const con_cell comment_1("comment",1);
    static const con_cell ver_2("ver",2);

    switch (task.get_state()) {
    case out_task::IDLE:
	break;
    case out_task::RECEIVED: {
	// This is a one time event only.
	auto &e = env_;

	pattern p(e);
	int64_t major_ver0 = 0, minor_ver0 = 0;
	term comment;
	auto const pat = p.str(comma,
			       p.str(colon,
				     p.con(me),
				     p.str(version_1,
					   p.str(ver_2,
						 p.any_int(major_ver0),
						 p.any_int(minor_ver0)))),
			       p.str(colon,
				     p.con(me),
				     p.str(comment_1, p.any(comment))));
	if (!pat(e, task.get_result_goal())) {
	    error(reason_t::ERROR_UNRECOGNIZED, "");
	    return;
	}

	int32_t major_ver = 0, minor_ver = 0;
	try {
	    major_ver = checked_cast<int32_t>(major_ver0, 0, 1000);
	    minor_ver = checked_cast<int32_t>(minor_ver0, 0, 1000);
	} catch (checked_cast_exception &ex) {
	    error(reason_t::ERROR_UNRECOGNIZED, "");
	    return;
	}

	// Successful. So update address book etc.
	address_entry entry;
	if (self().book()().exists(ip(), &entry)) {
	    entry.set_version(major_ver, minor_ver);
	    entry.set_comment(comment, e);
	    self().book()().update(entry);
	}
	break;
        }
    case out_task::SEND:
	task.set_term(
          env_.new_term(con_cell("query",1),
	    {env_.new_term(comma, {
			   env_.new_term(colon,
				 {me, env_.new_term(version_1,{env_.new_ref()})
					 }),
			   env_.new_term(colon,
				 {me, env_.new_term(comment_1,
						    {env_.new_ref()})
					 })})
	    }));
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
    static const con_cell session("session",2);

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
	    auto session_term = env_.arg(t, 0);
	    if (session_term.tag() != tag_t::STR ||
		env_.functor(session_term) != session) {
		error(reason_t::ERROR_FAIL_CONNECT,
		      "Unexpected session term for init connection. "
		      "Expecting session/2, was " + env_.to_string(t));
		break;
	    }
	    auto id_term = env_.arg(session_term, 0);
	    if (!env_.is_atom(env_.arg(session_term,0))) {
		error(reason_t::ERROR_FAIL_CONNECT,
		      "Unexpected session id for init connection. "
		      "Expecting atom, was " + env_.to_string(id_term));
		break;
	    }
	    auto name_term = env_.arg(session_term, 1);
	    if (!env_.is_atom(env_.arg(session_term,1))) {
		error(reason_t::ERROR_FAIL_CONNECT,
		      "Unexpected session name for init connection. "
		      "Expecting atom, was " + env_.to_string(name_term));
	    }
	    id_ = env_.atom_name(id_term);
	    name_ = env_.atom_name(name_term);
	    schedule(task);
	} else {
	    connected_ = true;
	    self().successful_connection(ip());
	    if (use_heartbeat_) {
		auto infotask = create_info_task();
		schedule(infotask);
		auto hbtask = create_heartbeat_task();
		schedule(hbtask);
		auto pubtask = create_publish_task();
		schedule(pubtask);
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
	} else if (!sent_my_name_) {
	    sent_my_name_ = true;
	    task.set_term(
		  env_.new_term(con_cell("command",1),
			{env_.new_term(con_cell("name",1),
				       {env_.functor(self().name(),0)})}));
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
