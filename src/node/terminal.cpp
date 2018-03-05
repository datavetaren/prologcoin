#include <algorithm>
#include <ctype.h>
#include "../common/term_serializer.hpp"
#include "terminal.hpp"
#include "self_node.hpp"

namespace prologcoin { namespace node {

using namespace prologcoin::common;

terminal::terminal() : prompt_("?- "),
		       stopped_(false),
		       ioservice_(),
		       endpoint_(boost::asio::ip::tcp::v4(),
				 self_node::DEFAULT_PORT),
		       socket_(ioservice_),
		       strand_(ioservice_),
		       connected_(false),
		       connect_count_(-1),
		       buffer_(self_node::MAX_BUFFER_SIZE, ' ')
{
    readline_.set_tick(true);
    readline_.set_callback([this](readline &, int ch){
	    return key_callback(ch);
	});
}

bool terminal::key_callback(int ch)
{
    if (ch == 3) {
	stopped_ = true;
    }

    if (ch == readline::TIMEOUT) {
	boost::unique_lock<boost::mutex> guard(text_output_queue_mutex_);
	if (!text_output_queue_.empty()) {
	    std::cout << std::endl;
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

void terminal::start_connector()
{
    connector_thread_ = boost::thread([this]() {
	while (!stopped_) {
	    connect_count_++;
	    if (!connected_) {
		bool doit = (connect_count_ < 10) ||
		    ((connect_count_ < 60) && (connect_count_ % 5) == 0) ||
	   	    ((connect_count_ % 10) == 0);

		if (doit) {
		    boost::system::error_code ec;
		    socket_ = socket(ioservice_);
		    socket_.open(boost::asio::ip::tcp::v4());
		    socket_.connect(endpoint_, ec);
		    if (!ec) {
			connected_ = true;

			lock_text_output_and( [this]() {
				add_text_output("[CONNECTED]");
		        });
		    }
		}
	    }
	    if (!connected_) {
		switch (connect_count_) {
		case 0: case 5: case 15:
		    lock_text_output_and( [this]() {
			    add_text_output("[DISCONNECTED]");
	  	    });
		    break;
		case 45:
		    lock_text_output_and( [this]() {
			    add_text_output("[DISCONNECTED (will now silence this message until connected)]");
	  	    });
		    break;		    
		}
	    }
	    sleep(1);
	}
	});
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

void terminal::send_buffer(term_serializer::buffer_t &buf, size_t n)
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
	    connect_count_ = -1;
	    return;
	}
    }
}

void terminal::send_length(size_t n)
{
    term_serializer::write_cell(buffer_len_, 0, int_cell(n));
    send_buffer(buffer_len_, sizeof(cell));
}

void terminal::send_query(const term t)
{
    term_serializer ser(env_);
    buffer_.clear();
    ser.write(buffer_, t);
    send_length(buffer_.size());
    send_buffer(buffer_, buffer_.size());
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
	    std::cout << "[REPLY]: Erroneous reply." << std::endl;
	    return term();
	}
	off += r;
    }

    auto c = ser.read_cell(buffer_len_, 0, "");
    if (c.tag() != tag_t::INT) {
	std::cout << "[REPLY]: Erreoneous encoding of reply length." << std::endl;
	return term();
    }

    auto &ic = reinterpret_cast<int_cell &>(c);
    if (ic.value() < sizeof(cell)) {
	std::cout << "[REPLY]: Length of reply too small (" << ic.value() << " < " << sizeof(cell) << std::endl;
	return term();
    }

    if (ic.value() > self_node::MAX_BUFFER_SIZE) {
	std::cout << "[REPLY]: Length of reply too big (" << ic.value() << " > " << self_node::MAX_BUFFER_SIZE << std::endl;
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
	    std::cout << "[REPLY]: Erroneous reply." << std::endl;
	    return term();
	}
	off += r;
    }

    term t = ser.read(buffer_);
    return t;
}

void terminal::run()
{
    start_connector();

    while (!stopped_) {
	std::cout << prompt_;
	std::cout.flush();
	std::string cmd = readline_.read();
	std::cout << "\n";
	if (stopped_) {
	    continue;
	}
	readline_.add_history(cmd);
	try {
	    term t = env_.parse(cmd);
	    send_query(t);

	    term r = read_reply();
	    if (r != term()) {
		std::cout << env_.to_string(r) << std::endl;
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
