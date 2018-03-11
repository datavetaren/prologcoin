#include "self_node.hpp"
#include "../common/term_serializer.hpp"
#include "../common/random.hpp"
#include <boost/asio/placeholders.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace prologcoin { namespace node {

using namespace prologcoin::common;

self_node::self_node()
    : ioservice_(),
      endpoint_(self_node::tcp::v4(), self_node::DEFAULT_PORT),
      acceptor_(ioservice_, endpoint_),
      socket_(ioservice_),
      strand_(ioservice_),
      timer_(ioservice_, boost::posix_time::seconds(TIMER_INTERVAL_SECONDS)),
      recent_connection_(nullptr)
{
}

void self_node::start()
{
    stopped_ = false;

    acceptor_.set_option(acceptor::reuse_address(true));
    acceptor_.set_option(socket_base::enable_connection_aborted(true));
    acceptor_.listen();

    thread_ = boost::thread([&](){ run(); });
}

void self_node::stop()
{
    stopped_ = true;
    ioservice_.stop();
}

void self_node::run()
{
    start_accept();
    start_prune_dead_connections();
    ioservice_.run();
}

void self_node::close(connection *conn)
{
    boost::lock_guard<boost::mutex> guard(lock_);    
    closed_.push_back(conn);
}

session_state * self_node::new_session(connection *conn)
{
    auto *ss = new session_state(conn);

    boost::lock_guard<boost::mutex> guard(lock_);
    states_[ss->id()] = ss;
    return ss;
}

session_state * self_node::find_session(const std::string &id)
{
    boost::lock_guard<boost::mutex> guard(lock_);
    
    auto it = states_.find(id);
    if (it == states_.end()) {
	return nullptr;
    }

    return it->second;
}

void self_node::session_connect(session_state *sess, connection *conn)
{
    boost::lock_guard<boost::mutex> guard(lock_);

    connection *old_conn = sess->get_connection();

    if (old_conn == conn) {
	return;
    }

    disconnect(old_conn);
    sess->set_connection(conn);
}

void self_node::kill_session(session_state *sess)
{
    boost::lock_guard<boost::mutex> guard(lock_);

    states_.erase(sess->id());
    delete sess;
}

void self_node::start_accept()
{
    using namespace boost::asio;
    using namespace boost::system;

    connection *conn = new connection(*this);
    connections_.insert(conn);
    recent_connection_ = conn;
    acceptor_.async_accept(conn->get_socket(),
		   strand_.wrap(
			   [this](const error_code &){
			       recent_connection_->start();
			       start_accept();
			   }));
}

void self_node::start_prune_dead_connections()
{
    using namespace boost::asio;
    using namespace boost::system;

    timer_.async_wait(
	      strand_.wrap(
			   [this](const error_code &) {
			       prune_dead_connections();
			       timer_.expires_from_now(
					 boost::posix_time::seconds(
					    TIMER_INTERVAL_SECONDS));
			       start_prune_dead_connections();
			   }));
}

void self_node::prune_dead_connections()
{
    boost::lock_guard<boost::mutex> guard(lock_);
    while (!closed_.empty()) {
	auto *c = closed_.back();
	auto *s = c->get_session();
	if (s != nullptr) {
	    s->reset_connection();
	}
	disconnect(c);
	closed_.pop_back();
    }
}

void self_node::join()
{
    thread_.join();
}

void self_node::disconnect(connection *conn)
{
    auto it = std::find(connections_.begin(), connections_.end(), conn);
    if (it != connections_.end()) {
	connections_.erase(it);
    }
    delete conn;
}

session_state::session_state(connection *conn)
  : connection_(conn),
    interp_initialized_(false),
    in_query_(false)
{
    id_ = "s" + random::next();
}

bool session_state::execute(const term query)
{
    if (!interp_initialized_) {
	interp_initialized_ = true;
	interp_.setup_standard_lib();
    }
    query_ = query;
    in_query_ = true;
    bool r = interp_.execute(query);
    if (!r) {
	in_query_ = false;
    }
    return r;
}

connection::connection(self_node &self)
    : self_node_(self),
      strand_(self.get_io_service()),
      socket_(self.get_io_service()),
      state_(connection::READ_QUERY_LENGTH),
      read_bytes_(0),
      query_length_(0),
      write_bytes_(0),
      reply_length_(0),
      buffer_(self_node::MAX_BUFFER_SIZE, ' '),
      session_(nullptr)
{
    setup_commands();
    std::cout << "connection::connection()\n";
}

connection::~connection()
{
    std::cout << "connection::~connection()\n";
}

void connection::setup_commands()
{
    commands_[con_cell("new",0)] = [this](const term cmd){ command_new(cmd); };
    commands_[con_cell("connect",1)] = [this](const term cmd){ command_connect(cmd); };
    commands_[con_cell("kill",1)] = [this](const term cmd){ command_kill(cmd); };
    commands_[con_cell("next",0)] = [this](const term cmd){ command_next(cmd); };
}

void connection::start()
{
    run();
}

void connection::close()
{
    node().close(this);
}

void connection::read_query_length()
{
    auto &e = env_;

    cell c = term_serializer::read_cell(buffer_, 0,
	"node/connection::run(): READ_QUERY_SIZE");
    if (c.tag() != tag_t::INT) {
	reply_error(e.functor("error_query_length_was_not_integer",0));
    } else {
	auto &ic=reinterpret_cast<const int_cell &>(c);
	size_t max = self_node::MAX_BUFFER_SIZE-sizeof(cell);
	if (ic.value() > max) {
	    reply_error(e.new_term(
			   e.functor("error_query_length_exceeds_max",1),
			   {e.new_term(e.functor(">",2),
				       {ic, int_cell(max)})}));
	} else if (ic.value() < sizeof(cell)) {
	    reply_error(e.new_term(e.functor("error_query_length_too_small",1),
			   {e.new_term(e.functor("<",2),
				       {ic, int_cell(sizeof(cell))})}));
	} else {
	    query_length_ = ic.value();
	    state_ = READ_QUERY;
	    read_bytes_ = 0;
	    buffer_.resize(query_length_);
	}
    }
}

void connection::command_new(const term)
{
    auto *ss = node().new_session(this);
    reply_ok(env_.functor(ss->id(),0));
}

session_state * connection::get_session(const term id_term)
{
    auto &e = env_;
    if (!e.is_atom(id_term)) {
	reply_error(e.new_term(e.functor("erroneous_session_id",1),{id_term}));
	return nullptr;
    }
    std::string id = e.atom_name(id_term);
    session_state *s = node().find_session(id);
    if (s == nullptr) {
	reply_error(e.new_term(e.functor("session_not_found",1),{id_term}));
	return nullptr;
    }
    return s;
}

con_cell connection::get_state_atom()
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

void connection::command_connect(const term cmd)
{
    auto &e = env_;
    term id_term = e.arg(cmd,0);

    session_state *s = get_session(id_term);
    if (s == nullptr) {
	return;
    }
    node().session_connect(s, this);
    session_ = s;
    reply_ok(e.new_term(e.functor("session_resumed",2),
			{id_term, get_state_atom()}));
}

void connection::command_kill(const term cmd)
{
    auto &e = env_;
    term id_term = e.arg(cmd,0);

    session_state *s = get_session(id_term);
    if (s == nullptr) {
	return;
    }
    node().kill_session(s);
    session_ = nullptr;
    reply_ok(e.new_term(e.functor("session_killed",1),{id_term}));
}

void connection::command_next(const term cmd)
{
    process_execution(cmd, true);
}

void connection::process_command(const term cmd)
{
    auto &e = env_;

    auto it = commands_.find(e.functor(cmd));
    if (it == commands_.end()) {
	reply_error(e.new_term(e.functor("unrecognized_commmand",1),{cmd}));
	return;
    }
    (it->second)(cmd);
}

void connection::process_query()
{
    auto &e = env_;
    term_serializer ser(e);
    try {
	auto t = ser.read(buffer_, query_length_);
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
    } catch (serializer_exception &ex) {
	reply_error(e.new_term(e.functor("serializer_exception",1),
			       {e.functor(ex.what(),0)}));
    }
}

void connection::process_execution(const term cmd, bool in_query)
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

void connection::reply_error(const term t)
{
    reply_answer(env_.new_term(env_.functor("error",1),{t}));
}

void connection::reply_ok(const term t)
{
    reply_answer(env_.new_term(env_.functor("ok",1),{t}));
}

void connection::reply_answer(const term t)
{
    term_serializer ser(env_);
    buffer_.clear();
    ser.write(buffer_, t);
    buffer_len_.resize(sizeof(cell));
    ser.write_cell(buffer_len_, 0, int_cell(buffer_.size()));
    write_bytes_ = 0;
    reply_length_ = buffer_.size();
    state_ = REPLY_WITH_ANSWER_LENGTH;
}

void connection::run()
{
    using namespace boost::asio;
    using namespace boost::system;

    switch (state_) {
    case READ_QUERY_LENGTH:
        buffer_.resize(sizeof(cell));
	get_socket().async_read_some(buffer(&buffer_[read_bytes_],
					    sizeof(cell) - read_bytes_),
	     strand_.wrap(
		  [this](const error_code &ec, size_t n) {
			 if (!ec) {
			     read_bytes_ += n;
			     if (read_bytes_ >= sizeof(cell)) {
				 read_query_length();
			     }
			     run();
			 } else {
			     close();
			 }
		  }));
	break;
    case READ_QUERY:
	get_socket().async_read_some(buffer(&buffer_[read_bytes_],
					    query_length_ - read_bytes_),
	     strand_.wrap(
		  [this](const error_code &ec, size_t n) {
			 if (!ec) {
			     read_bytes_ += n;
			     if (read_bytes_ >= query_length_) {
				 process_query();
			     }
			     run();
			 } else {
			     close();
			 }
		  }));
	break;
    case REPLY_WITH_ANSWER_LENGTH:
	get_socket().async_write_some(buffer(&buffer_len_[write_bytes_],
					     sizeof(cell) - write_bytes_),
             strand_.wrap(
		  [this](const error_code &ec, size_t n) {
		      if (!ec) {
			  write_bytes_ += n;
			  if (write_bytes_ >= sizeof(cell)) {
			      state_ = REPLY_WITH_ANSWER;
			      write_bytes_ = 0;
			  }
			  run();
		      } else {
			  close();
		      }
		  }));
	break;
    case REPLY_WITH_ANSWER:
	get_socket().async_write_some(buffer(&buffer_[write_bytes_],
					     reply_length_ - write_bytes_),
	     strand_.wrap(
		  [this](const error_code &ec, size_t n) {
		         if (!ec) {
			     write_bytes_ += n;
			     if (write_bytes_ >= buffer_.size()) {
				 state_ = READ_QUERY_LENGTH;
				 read_bytes_ = 0;
				 write_bytes_ = 0;
			     }
			     run();
		         } else {
			     close();
			 }
		  }));
	break;
    }
}

}}
