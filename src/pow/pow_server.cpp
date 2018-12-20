#include "pow_server.hpp"
#include "dipper_detector.hpp"
#include "observatory.hpp"
#include "star.hpp"
#include "pow_verifier.hpp"

#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/asio/write.hpp>
#include <boost/filesystem/path.hpp>
#include <unordered_map>

namespace prologcoin { namespace pow {

connections::connections(class pow_server &pow) : pow_server_(pow) { 
}

void connections::start(connection_ptr c)
{
    connections_.insert(c);
    c->start();
}

void connections::stop(connection_ptr c)
{
    connections_.erase(c);
    c->stop();
}

void connections::stop_all()
{
    for (auto c : connections_) {
	c->stop();
    }
    connections_.clear();
}

connection::connection(boost::asio::ip::tcp::socket socket,
		       connections &conns)
    : socket_(std::move(socket)),
      connections_(conns),
      parsed_index_(0),
      parsed_new_line_(std::string::npos),
      state_(connection::PARSE_HTTP),
      keep_alive_(false)
{
}

connection::~connection()
{
}

const std::string & connection::read_file(const std::string &path) const
{
    return connections_.pow_server().read_file(path);
}

void connection::start()
{
    do_read();
}

void connection::stop()
{
    socket_.close();
}

void connection::do_read()
{
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(buffer_),
        [this, self](boost::system::error_code ec, std::size_t bytes_num) {
	    if (!ec) {
		parse(buffer_.data(), bytes_num);
		if (state_ == PARSE_DONE) {
		    do_action();
		    do_write();
		    return;
		}
		do_read();
	    } else if (ec != boost::asio::error::operation_aborted) {
		if (!keep_alive_) {
		    connections_.stop(shared_from_this());
	        }
	    }
    });
}

void connection::do_write()
{
    auto self(shared_from_this());

    boost::asio::async_write(socket_, boost::asio::buffer(reply_),
     [this, self](boost::system::error_code ec, std::size_t num) {
	 if (!ec) {
	     if (!keep_alive_) {
		 boost::system::error_code ignored_ec;
		 socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
				  ignored_ec);
	     }
	 }
	 if (ec != boost::asio::error::operation_aborted) {
	     state_ = PARSE_HTTP;
	     if (!keep_alive_) {
		 connections_.stop(shared_from_this());
	     } else {
		 do_read();
	     }
	 }
     });
}

void connection::parse(char *buffer, size_t n)
{
    auto to_copy = n;
    if (parsed_index_ + n > parsed_.size()) {
	to_copy = parsed_.size() - parsed_index_;
    }
    size_t i = 0;

    while (parsed_new_line_ == std::string::npos) {
	for (; i < to_copy; i++) {
	    char ch = buffer[i];
	    if (ch == '\r') {
		continue;
	    }
	    parsed_[parsed_index_] = ch;
	    if (ch == '\n' && parsed_new_line_ == std::string::npos) {
		parsed_new_line_ = parsed_index_;
		parsed_index_++;
		i++;
		break;
	    }
	    parsed_index_++;
	}

	if (parsed_new_line_ == std::string::npos) {
	    // We've consumed the entire buffer, but no newline...
	    return;
	}

	std::string str(parsed_.data(), parsed_.data()+parsed_new_line_);

	switch (state_) {
	case PARSE_HTTP: parse_http(); break;
	case PARSE_HEADER: parse_header(); break;
	case PARSE_CONTENT: parse_content(); break;
	default:
	    do_error();
	    break;
	}
    }
}

void connection::parse_consume(size_t n)
{
    size_t remaining = parsed_index_ - n;
    std::copy_n(parsed_.data() + n, remaining, parsed_.data());
    parsed_index_ = remaining;
}

void connection::parse_consume()
{
    parse_consume(parsed_new_line_+1);
    parsed_new_line_ = std::string::npos;
}

void connection::parse_http()
{
    std::string str(parsed_.data(), parsed_.data()+parsed_new_line_);
    std::vector<std::string> tokens;
    boost::split(tokens, str, boost::is_any_of(" "));

    parse_consume();

    if (tokens.size() < 3) {
	do_error();
	return;
    }

    method_ = METHOD_UNKNOWN;
    if (tokens[0] == "GET") {
	method_ = METHOD_GET;
    }
    uri_ = tokens[1];
    if (boost::starts_with(tokens[2], "HTTP/")) {
	ver_ = tokens[2].substr(5);
    }
    state_ = PARSE_HEADER;
    header_.clear();
}

void connection::parse_header()
{
    std::string str(parsed_.data(), parsed_.data()+parsed_new_line_);
    parse_consume();
    auto colon_pos = str.find(':');
    if (colon_pos == std::string::npos) {
	if (str.empty()) {
	    if (content_length_ != 0) {
		state_ = PARSE_CONTENT;
	    } else {
		state_ = PARSE_DONE;
	    }
	    return;
	}
	do_error();
	return;
    }
    std::string key = str.substr(0, colon_pos);
    boost::trim(key);
    std::string value = str.substr(colon_pos+1);
    boost::trim(value);
    if (key == "Content-Length") {
	content_length_ = lexical_cast_no_error<size_t>(value);
    }
    if (key == "Connection" && value == "keep-alive") {
	keep_alive_ = true;
    }
    header_.push_back(std::make_pair(key, value));
    header_map_[key] = value;
}

void connection::parse_content()
{
    std::string str(parsed_.data(), parsed_.data()+parsed_new_line_);
    parse_consume();
    if (str.empty()) {
	state_ = PARSE_DONE;
    }
}

void connection::do_action()
{
    arith_t x(0), y(0), z(0);
    arith_t r = arith_t(1)/10, th = arith_t(3141)/1000/2, phi(0);
    size_t proof_num = 0, nonce = 0;

    // std::cout << "URI: " << uri_ << std::endl;

    std::string page = uri_;
    size_t query_index = uri_.find("?");
    if (query_index != std::string::npos) {
	page = uri_.substr(0,query_index);
	std::string params = uri_.substr(query_index+1);
	std::vector<std::string> tokens;
	boost::split(tokens, params, boost::is_any_of("&"));
	for (auto &tok : tokens) {
	    size_t eq_index = tok.find("=");
	    if (eq_index == std::string::npos) {
		continue;
	    }
	    std::string key = tok.substr(0, eq_index);
	    std::string value = tok.substr(eq_index+1);
	    double v = lexical_cast_no_error<double>(value);
	    if (key == "x") {
		x = v;
	    } else if (key == "y") {
		y = v;
	    } else if (key == "z") {
		z = v;
	    } else if (key == "r") {
		r = v;
	    } else if (key == "th") {
		th = v;
	    } else if (key == "phi") {
		phi = v;
	    } else if (key == "nonce") {
	        nonce = static_cast<size_t>(v);
	    } else if (key == "proof") {
		proof_num = static_cast<size_t>(v);
	    } else {
		// Unrecognized. Ignore.
	    }
	}
    }

    pow_server &server = connections_.pow_server();
    if (nonce != 0) {
	server.set_target(proof_num, nonce);
	auto target = server.get_target();
	r = target.r();
	th = target.theta();
	phi = target.phi();
    } else {
	// std::cout << "SETTING TARGET: " << r << " " << th << " " << phi << std::endl;
	server.set_target(r, th, phi);
    }

    // std::cout << "URI: " << page << std::endl;

    if (page == "/render.html") {
	std::string str = read_file("render.html");
	boost::replace_all(str, "[[radius]]", boost::lexical_cast<std::string>(r));
	boost::replace_all(str, "[[theta]]", boost::lexical_cast<std::string>(th));
	boost::replace_all(str, "[[phi]]", boost::lexical_cast<std::string>(phi));
	do_reply_ok( str );
    } else if (page == "/render") {
	std::vector<projected_star> stars;
	server.take_picture(stars);
	dipper_detector detector(stars);
	std::vector<std::vector<projected_star> > found;
	std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t> > found_ids;
	detector.search_n(found, 10);
	std::stringstream ss;
	ss << "[";
	bool first = true;
	for (auto s : stars) {
	    if (!first) ss << ",";
	    ss << "{i=" << s.id() << ",x=" <<  s.x() << ",y=" << s.y() << ",r=" << s.r() << "}";
	    first = false;
	}
        if (found.size() > 0) {
            for (auto f : found) {
		ss << ",s=[";
		bool s_first = true;
		for (auto s : f) {
		    if (!s_first) ss << ",";
		    ss << s.id(); 
		    s_first = false;
		}
		ss << "]";
            }
        }
	ss << "]";

	std::string str = ss.str();

	/*
	if (found.size() > 0) {
	    std::cout << "FOUND..." << std::endl;
	}
	*/

	do_reply_ok(str);
	
    } else if (boost::starts_with(uri_, "/scan")) {
	std::vector<projected_star> stars;
	pow_server &server = connections_.pow_server();
	size_t nonce = 0;
	server.scan(0, stars, nonce);
    } else {
	do_reply_ok("<html><body><h1>Hello World</h1></body></html>\r\n\r\n");
    }
}

void connection::do_reply_ok(const std::string &content)
{
    reply_.clear();
    reply_ += "HTTP/1.1 200 OK\r\n";
    std::stringstream ss;
    ss << "Content-Length: "<< content.size() << "\r\n";
    reply_ += ss.str();
    reply_ += "Content-Type: text/html\r\n";
    if (keep_alive_) {
	reply_ += "Connection: Keep-Alive\r\n";
    }
    reply_ += "\r\n";
    reply_ += content;
}

void connection::do_error()
{
    state_ = PARSE_ERROR;
    reply_.clear();
    reply_ += "HTTP/1.0 400 Bad Request\r\n";
    reply_ += "\r\n";
    do_write();
}

template<size_t N, typename T> static inline observatory<N,T> & o(void *v) {
    return *reinterpret_cast<observatory<N,T> *>(v);
}

pow_server::pow_server(const std::string &address, const std::string &port,
			const std::string &home_dir) :
    home_dir_(home_dir),
    io_service_(),
    signals_(io_service_),
    acceptor_(io_service_),
    socket_(io_service_),
    connections_(*this) {

    observatory_ = new observatory<8,arith_t>();
    o<8,arith_t>(observatory_).init("hello42", 7);

    std::cout << "Galaxy initialized" << std::endl;
    o<8,arith_t>(observatory_).status();
    std::cout << "Galaxy checked" << std::endl;

    setup_shutdown();
    setup_acceptor(address, port);

    do_accept();
}

void pow_server::take_picture(std::vector<projected_star> &stars) const
{
    o<8,arith_t>(observatory_).take_picture(stars);
}

void pow_server::set_target(arith_t r, arith_t th, arith_t phi)
{
    o<8,arith_t>(observatory_).set_target(vec3<arith_t>(vec3<arith_t>::SPHERICAL(), r, th, phi));
}

void pow_server::set_target(size_t proof_num, size_t nonce)
{
    o<8,arith_t>(observatory_).set_target(proof_num, nonce);
}

vec3<arith_t> pow_server::get_target() const
{
    return o<8,arith_t>(observatory_).get_target();
}

bool pow_server::scan(size_t proof_num, std::vector<projected_star> &stars, size_t &nonce)
{
    return o<8,arith_t>(observatory_).scan(proof_num, stars, nonce);
}

void pow_server::run()
{
    io_service_.run();
}

const std::string & pow_server::read_file(const std::string &path) const
{
    auto it = file_cache_.find(path);
    if (it != file_cache_.end()) {
	return it->second;
    }
    std::ifstream ifs((boost::filesystem::path(home_dir()) / path).string());
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string str = ss.str();
    file_cache_[path] = str;
    return file_cache_[path];
}

void pow_server::do_accept()
{
    acceptor_.async_accept(socket_,
			   [this](boost::system::error_code ec)
			   {
			       if (!acceptor_.is_open()) {
				   return;
			       }
			       if (!ec) {
				   connections_.start(std::make_shared<connection>(std::move(socket_), connections_));
			       }
			       do_accept();
			   });
}

void pow_server::setup_shutdown()
{
    // signals_.add(SIGINT);
    // signals_.add(SIGTERM);
    // signals_.add(SIGQUIT);

    signals_.async_wait(
	[this](boost::system::error_code /*ec*/, int /*signo*/)
	{
	    acceptor_.close();
	    connections_.stop_all();
	});
}

void pow_server::setup_acceptor(const std::string &address,
				const std::string &port)
{
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({address, port});
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
}

}}
