#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "address_book.hpp"
#include "ip_address.hpp"

namespace prologcoin { namespace node {

address_entry::address_entry()
{
    index_ = 0;
    port_ = 0;
    score_ = 0;
    time_ = 0;
}

address_entry::address_entry(const address_entry &other)
{
    index_ = other.index_;
    addr_ = other.addr_;
    port_ = other.port_;
    score_ = other.score_;
    time_ = other.time_;
    comment_ = other.comment_;
}

address_entry::address_entry(const ip_address &ip, unsigned short port)
{
    index_ = 0;
    addr_ = ip;
    port_ = port;
    score_ = 0;
    time_ = 0;
}

void address_entry::set_comment(const std::string &str)
{
    using namespace prologcoin::common;

    term_env env;
    set_comment(env, str);
}

void address_entry::set_comment(common::term_env &env, const std::string &str)
{
    using namespace prologcoin::common;

    term t = env.parse(str);
    term_serializer ser(env);
    term_serializer::buffer_t buf;
    ser.write(buf, t);
    set_comment(buf);
}

void address_entry::write(common::term_env &env, common::term_emitter &emitter) const
{
    using namespace prologcoin::common;

    //Create a term of everything.

    term term_addr = env.functor(addr().str(), 0);
    term term_port = int_cell(port());
    term term_score = int_cell(static_cast<int64_t>(score()));
    term term_time = env.functor(time().str(), 0);

    term_serializer ser(env);
    term term_comment = (comment().size() > 0) ? ser.read(comment()) : env.empty_list();

    
    term term_entry = env.new_term(env.functor("entry",5),
		 {term_addr, term_port, term_score, term_time, term_comment});
    emitter.print(term_entry);
    emitter.out() << ".";
    emitter.nl();
}

void address_entry::read(common::term_env &env, common::term_parser &parser)
{
    using namespace prologcoin::common;

    parser.set_track_positions(true);

    term t;
    try {
	t = parser.parse();
    } catch (const term_parse_exception &ex) {
	std::string msg = "While parsing " + std::string(ex.what());
	throw address_book_load_exception(msg, ex.line());
    } catch (const token_exception &ex) {
	std::string msg = "While parsing " + std::string(ex.what());
	throw address_book_load_exception(msg, ex.line());
    }

    int line = parser.line(parser.positions());

    if (t.tag() != tag_t::STR) {
	throw address_book_load_exception( "Expected functor entry/5, but got: " + env.to_string(t), line);
    }
    
    if (env.functor(t) != env.functor("entry",5)) {
	throw address_book_load_exception( "Expected functor entry/5, but got: " + env.to_string(env.functor(t)), line);
    }

    term term_addr = env.arg(t, 0);
    line = parser.line(parser.position_arg(parser.positions(),0));
    if (term_addr.tag() != tag_t::CON) {
	throw address_book_load_exception( "First argument of entry/5 should be an atom to represent an IP-address, but was " + env.to_string(term_addr), line);
    }
    std::string addr = env.atom_name(term_addr);
    try {
	boost::asio::ip::make_address(addr);
    } catch (std::runtime_error &ex) {
	throw address_book_load_exception( "Couldn't parse IP address: " + addr, line);
    }

    term term_port = env.arg(t, 1);
    line = parser.line(parser.position_arg(parser.positions(),1));
    if (term_port.tag() != tag_t::INT) {
	throw address_book_load_exception( "Second argument of entry/5 should be an integer to represent a port number, but was " + env.to_string(term_port), line);
    }
    int64_t port_value = reinterpret_cast<const int_cell &>(term_port).value();
    if (!(port_value >= 0 && port_value <= 65535)) {
	throw address_book_load_exception( "Second argument of entry/5 is port number but was not in valid range 0..65535: " + env.to_string(term_port),line);
    }
    unsigned short port = static_cast<unsigned short>(term_port.value());
    
    term term_score = env.arg(t, 2);
    line = parser.line(parser.position_arg(parser.positions(),2));
    if (term_score.tag() != tag_t::INT) {
	throw address_book_load_exception( "Third argument of entry/5 should be an integer to rerepsent a score, but was " + env.to_string(term_score), line);
    }

    int64_t score_value = reinterpret_cast<const int_cell &>(term_score).value();
    if (score_value < -1000000 || score_value > 1000000) {
	throw address_book_load_exception( "Third argument of entry/5 represents score but was not in valid range -1000000..1000000: " + env.to_string(term_score), line);

    }
    int32_t score = static_cast<int32_t>(score_value);

    term term_time = env.arg(t, 3);
    line = parser.line(parser.position_arg(parser.positions(),3));
    if (term_time.tag() != tag_t::CON) {
	throw address_book_load_exception( "Fourth argument of entry/5 should be an atom to represent time, but was " + env.to_string(term_time), line);
    }
    std::string time_str = env.to_string(term_time);
    if (time_str[0] == '\'' && time_str[time_str.size()-1] == '\'') {
	time_str = time_str.substr(1,time_str.size()-2);
    }
    utime ut;
    if (!ut.parse(time_str)) {
	throw address_book_load_exception( "Fourth argument of entry/5 should represent time, but failed to parse: " + time_str, line);
    }

    term term_comment = env.arg(t, 4);
    line = parser.line(parser.position_arg(parser.positions(),4));
    term_serializer::buffer_t buf;

    if (!env.is_empty_list(term_comment)) {
	term_serializer ser(env);
	ser.write(buf, term_comment);
    }

    set_addr(addr);
    set_port(port);
    set_score(score);
    set_time(ut);
    set_comment(buf);
}

std::string address_entry::str() const
{
    using namespace prologcoin::common;

    term_env env;
    std::stringstream ss;
    term_emitter emitter(ss, env);
    write(env, emitter);
    return ss.str();
}

// --- address_book ---

address_book::address_book()
{
}

void address_book::print(std::ostream &out)
{
    using namespace prologcoin::common;

    out << std::setw(22) << "Address" << " Port" << std::setw(7) << " Score"  << std::setw(21) << " Time" << " Comment" << std::endl;

    size_t n = index_map_.size();
    for (size_t i = 0; i < n; i++) {
	const address_entry &e = index_map_[i];
	out << std::setw(22) << e.addr().str(22) << " "
	    << std::setw(4) << e.port() << " "
	    << std::setw(6) << e.score() << " "
	    << std::setw(20) << e.time().str() << " ";
	if (e.comment().size() != 0) {
	    try {
		term_env env;
		term_serializer ser(env);
		term t = ser.read(e.comment());
		out << env.to_string(t);
	    } catch (serializer_exception &ex) {
		out << "?";
	    }
	}
	out << std::endl;
    }
}

void address_book::add(const address_entry &e)
{
    if (all_.find(e) != all_.end()) {
	// Already exists. Exit.
	return;
    }

    size_t new_index = index_map_.size();
    e.set_index(new_index);
    index_map_.insert(std::make_pair(new_index, e));
    score_map_.insert(std::make_pair(e.score(), e.index()));
    all_.insert(e);
}

void address_book::save(const std::string &path)
{
    using namespace prologcoin::common;

    std::ofstream out(path);
    term_env env;
    term_emitter emitter(out, env);
    for (auto &p : index_map_) {
	auto &e = p.second;
	e.write(env, emitter);
    }
}

void address_book::load(const std::string &path)
{
    using namespace prologcoin::common;

    term_env env;
    std::ifstream in(path);
    term_tokenizer tok(in);
    term_parser parser(tok, env);

    while (!parser.is_eof()) {
	address_entry e;
	e.read(env, parser);
	add(e);
    }
}

bool address_book::operator == (const address_book &other) const
{
    auto it1 = index_map_.begin();
    auto it2 = other.index_map_.begin();
    
    while (it1 != index_map_.end()) {
	if (it2 == other.index_map_.end()) {
	    return false;
	}

	auto &e1 = it1->second;
	auto &e2 = it2->second;

	if (e1 != e2) {
	    return false;
	}

	++it1;
	++it2;
    }

    return it2 == other.index_map_.end();
}

}}
