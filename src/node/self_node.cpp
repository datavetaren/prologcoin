#include "self_node.hpp"
#include "../common/term_serializer.hpp"
#include <boost/asio/placeholders.hpp>

namespace prologcoin { namespace node {

self_node::self_node()
    : ioservice_(),
      endpoint_(self_node::tcp::v4(), self_node::DEFAULT_PORT),
      acceptor_(ioservice_, endpoint_),
      socket_(ioservice_)
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
    ioservice_.run();
}

void self_node::start_accept()
{
    using namespace boost::asio;
    using namespace boost::system;

    connections_.push_back(new connection(*this));
    acceptor_.async_accept(connections_.back()->get_socket(),
			   [&](const error_code &){
			       auto *conn = connections_.back();
			       conn->start();
			       start_accept();
			   });
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

connection::connection(self_node &self)
    : self_node_(self),
      strand_(self.get_io_service()),
      socket_(self.get_io_service()),
      state_(connection::READ_QUERY_LENGTH),
      read_bytes_(0),
      query_length_(0),
      write_bytes_(0),
      reply_length_(0),
      buffer_(self_node::MAX_BUFFER_SIZE, ' ')
{
    std::cout << "connection::connection()\n";
}

connection::~connection()
{
    std::cout << "connection::~connection()\n";
}

void connection::start()
{
    run();
}

void connection::read_query_length()
{
    using namespace prologcoin::common;

    auto &i = interpreter_;

    cell c = term_serializer::read_cell(buffer_, 0,
	"node/connection::run(): READ_QUERY_SIZE");
    if (c.tag() != tag_t::INT) {
	reply_error(i.functor("error_query_length_was_not_integer",0));
    } else {
	auto &ic=reinterpret_cast<const int_cell &>(c);
	size_t max = self_node::MAX_BUFFER_SIZE-sizeof(cell);
	if (ic.value() > max) {
	    reply_error(i.new_term(
			   i.functor("error_query_length_exceeds_max",1),
			   {i.new_term(i.functor(">",2),
				       {ic, int_cell(max)})}));
	} else if (ic.value() < sizeof(cell)) {
	    reply_error(i.new_term(i.functor("error_query_length_too_small",1),
			   {i.new_term(i.functor("<",2),
				       {ic, int_cell(sizeof(cell))})}));
	} else {
	    query_length_ = ic.value();
	    state_ = READ_QUERY;
	    read_bytes_ = 0;
	    buffer_.resize(query_length_);
	}
    }
}

void connection::process_query()
{
    using namespace prologcoin::common;

    auto &i = interpreter_;
    term_serializer ser(i);
    try {
	auto t = ser.read(buffer_, query_length_);
	std::cout << "Successful: " << i.to_string(t) << "\n";
	reply_ok(t);
    } catch (serializer_exception &ex) {
	reply_error(i.new_term(i.functor("serializer_exception",1),
			       {i.functor(ex.what(),0)}));
    }
}

void connection::reply_error(const common::term t)
{
    reply_answer(interpreter_.new_term(interpreter_.functor("error",1),{t}));
}

void connection::reply_ok(const common::term t)
{
    reply_answer(interpreter_.new_term(interpreter_.functor("ok",1),{t}));
}

void connection::reply_answer(const common::term t)
{
    using namespace prologcoin::common;

    term_serializer ser(interpreter_);
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
    using namespace prologcoin::common;

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
		         }
		  }));
	break;
    }
}

}}
