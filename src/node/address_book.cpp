#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "address_book.hpp"
#include "ip_address.hpp"
#include "../common/random.hpp"

namespace prologcoin { namespace node {

address_entry::address_entry() : ip_service()
{
    id_ = 0;
    score_ = 0;
    time_ = 0;
}

address_entry::address_entry(const address_entry &other) : ip_service(other)
{
    id_ = other.id_;
    source_ = other.source_;
    score_ = other.score_;
    time_ = other.time_;
    comment_ = other.comment_;
}

address_entry::address_entry(const ip_address &addr, const ip_address &src,
			     unsigned short port) : ip_service(addr, port)
{
    id_ = 0;
    source_ = src;
    score_ = 0;
    time_ = 0;
}

address_entry::address_entry(const ip_address &addr, unsigned short port)
    : ip_service(addr, port)
{
    id_ = 0;
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

common::term address_entry::to_term(common::term_env &env) const
{
    using namespace prologcoin::common;

    term term_addr = env.functor(addr().str(), 0);
    term term_src = env.functor(source().str(), 0);
    term term_port = int_cell(port());
    term term_score = int_cell(static_cast<int64_t>(score()));
    term term_time = env.functor(time().str(), 0);

    term_serializer ser(env);
    term term_comment = (comment().size() > 0) ? ser.read(comment()) : env.empty_list();

    
    term term_entry = env.new_term(env.functor("entry",6),
	   {term_addr, term_src, term_port,
	    term_score, term_time, term_comment});
    return term_entry;
}

void address_entry::write(common::term_env &env, common::term_emitter &emitter) const
{
    using namespace prologcoin::common;

    //Create a term of everything.

    term term_entry = to_term(env);

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
	throw address_book_load_exception( "Expected functor entry/6, but got: " + env.to_string(t), line);
    }
    
    if (env.functor(t) != env.functor("entry",6)) {
	throw address_book_load_exception( "Expected functor entry/6, but got: " + env.to_string(env.functor(t)), line);
    }

    term term_addr = env.arg(t, 0);
    line = parser.line(parser.position_arg(parser.positions(),0));
    if (term_addr.tag() != tag_t::CON) {
	throw address_book_load_exception( "First argument of entry/6 should be an atom to represent an IP-address, but was " + env.to_string(term_addr), line);
    }
    std::string addr = env.atom_name(term_addr);
    try {
	boost::asio::ip::make_address(addr);
    } catch (std::runtime_error &ex) {
	throw address_book_load_exception( "Couldn't parse IP address: " + addr, line);
    }


    term term_src = env.arg(t, 1);
    line = parser.line(parser.position_arg(parser.positions(),1));
    if (term_addr.tag() != tag_t::CON) {
	throw address_book_load_exception( "Second argument of entry/6 should be an atom to represent an IP-address, but was " + env.to_string(term_src), line);
    }
    std::string src = env.atom_name(term_src);
    try {
	boost::asio::ip::make_address(src);
    } catch (std::runtime_error &ex) {
	throw address_book_load_exception( "Couldn't parse IP address: " + src, line);
    }

    term term_port = env.arg(t, 2);
    line = parser.line(parser.position_arg(parser.positions(),2));
    if (term_port.tag() != tag_t::INT) {
	throw address_book_load_exception( "Third argument of entry/6 should be an integer to represent a port number, but was " + env.to_string(term_port), line);
    }
    int64_t port_value = reinterpret_cast<const int_cell &>(term_port).value();
    if (!(port_value >= 0 && port_value <= 65535)) {
	throw address_book_load_exception( "Third argument of entry/6 is port number but was not in valid range 0..65535: " + env.to_string(term_port),line);
    }
    unsigned short port = static_cast<unsigned short>(term_port.value());
    
    term term_score = env.arg(t, 3);
    line = parser.line(parser.position_arg(parser.positions(),3));
    if (term_score.tag() != tag_t::INT) {
	throw address_book_load_exception( "Fourth argument of entry/6 should be an integer to represent a score, but was " + env.to_string(term_score), line);
    }

    int64_t score_value = reinterpret_cast<const int_cell &>(term_score).value();
    if (score_value < -1000000 || score_value > 1000000) {
	throw address_book_load_exception( "Fourth argument of entry/6 represents score but was not in the valid range -1000000..1000000: " + env.to_string(term_score), line);

    }
    int32_t score = static_cast<int32_t>(score_value);

    term term_time = env.arg(t, 4);
    line = parser.line(parser.position_arg(parser.positions(),4));
    if (term_time.tag() != tag_t::CON) {
	throw address_book_load_exception( "Fifth argument of entry/6 should be an atom to represent time, but was " + env.to_string(term_time), line);
    }
    std::string time_str = env.to_string(term_time);
    if (time_str[0] == '\'' && time_str[time_str.size()-1] == '\'') {
	// Remove quotes
	time_str = time_str.substr(1,time_str.size()-2);
    }
    utime ut;
    if (!ut.parse(time_str)) {
	throw address_book_load_exception( "Fifth argument of entry/6 should represent time, but failed to parse: " + time_str, line);
    }

    term term_comment = env.arg(t, 5);
    line = parser.line(parser.position_arg(parser.positions(),5));
    term_serializer::buffer_t buf;

    if (!env.is_empty_list(term_comment)) {
	term_serializer ser(env);
	ser.write(buf, term_comment);
    }

    set_addr(addr);
    set_source(src);
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
    emitter.set_option_nl(false);
    write(env, emitter);
    return ss.str();
}

// --- address_book ---

address_book::address_book() : id_count_(0), num_spilled_(0), spill_enabled_(true)
{
}

void address_book::print(std::ostream &out, const std::vector<address_entry> &entries)
{
    using namespace prologcoin::common;

    out << std::setw(16) << "Address" << " " << std::setw(16) << "Source" << " Port" << std::setw(7) << " Score"  << std::setw(21) << " Time" << " Comment" << std::endl;

    for (auto &e : entries) {
	out << std::setw(16) << e.addr().str(16) << " "
	    << std::setw(16) << e.source().str(16) << " "
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

void address_book::print(std::ostream &out, size_t n)
{
    std::vector<address_entry> entries;
    size_t i = 0;
    for (auto e : id_to_entry_) {
	if (i >= n) {
	    break;
	}
	entries.push_back(e.second);
	i++;
    }
    print(out, entries);
}

void address_book::print(std::ostream &out)
{
    print(out, id_to_entry_.size());
}

std::string address_book::stat() const
{
    std::stringstream ss;
    ss << "{";
    ss << "total=" << size() << ", "
       << "num_groups=" << num_groups() << ", "
       << "unverified=" << num_unverified() << ", "
       << "spilled=" << num_spilled();
    ss << "}";
    return ss.str();
}

void address_book::spill_check(const address_entry &e, address_book::spill_area area)
{
    if (!is_spill_enabled()) {
	return;
    }
    auto group = e.group();
    switch (area) {
    case SPILL_IN_10:
	if (top_10_collection_.size(group) >= MAX_GROUP_SIZE) {
	    auto spilled = top_10_collection_.spill(group);
	    auto spill_id = ip_to_id_[spilled];
	    num_spilled_++;
	    remove(spill_id);
	}
	break;
    case SPILL_IN_90:
	if (bottom_90_collection_.size(group) >= MAX_GROUP_SIZE) {
	    auto spilled = bottom_90_collection_.spill(group);
	    auto spill_id = ip_to_id_[spilled];
	    num_spilled_++;
	    remove(spill_id);
	}
	break;
    case SPILL_IN_UNVERIFIED: {
	auto key = e.source().group();
	ip_collection &coll = unverified_[key];
	if (coll.size() >= MAX_SOURCE_SIZE) {
	    remove(coll.spill());
	    num_spilled_++;
	}
	if (coll.size(e.group()) >= MAX_GROUP_SIZE) {
	    remove(coll.spill(e.group()));
	    num_spilled_++;
	}
	break;
        }
    }
}

void address_book::add(const address_entry &e)
{
    if (ip_to_id_.find(e) != ip_to_id_.end()) {
	std::cout << "Already exists...\n";
	// Already exists. Exit.
	return;
    }

    size_t new_id = ++id_count_;
    e.set_id(new_id);
    id_to_entry_.insert(std::make_pair(new_id, e));
    ip_to_id_.insert(std::make_pair(e, new_id));

    if (e.source().is_zero()) {
	score_entry sce = score_entry(e.score(),new_id);
	if (top_10_.empty() || sce < top_10_.rbegin()->first) {
	    spill_check(e, SPILL_IN_10);
	    top_10_.insert(std::make_pair(sce, new_id));
	    top_10_collection_.add(e, e.score());
	} else {
	    spill_check(e, SPILL_IN_90);
	    bottom_90_.insert(std::make_pair(sce, new_id));
	    bottom_90_collection_.add(e, e.score());
	}
    } else {
	auto key = e.source().group();
	auto it = unverified_group_to_gid_.find(key);
	if (it == unverified_group_to_gid_.end()) {
	    auto gid = new_gid();
	    unverified_group_to_gid_[key] = gid;
	    unverified_gid_to_group_[gid] = key;
	}
	spill_check(e, SPILL_IN_UNVERIFIED);
	unverified_[key].add(e, 0);
	unverified_id_to_gid_[e.id()] = key;
    }

    calibrate();
}

void address_book::calibrate()
{
    // Recalibrate top 10% and bottom 90&
    size_t tot = top_10_.size() + bottom_90_.size();
    size_t top_10_threshold = 10 * tot / 100;
    if (tot > 0 && top_10_threshold == 0) top_10_threshold = 1;
    if (top_10_.size() > top_10_threshold) {
	auto it = top_10_.rbegin();
        auto last_pair = *it;
	auto score_entry = last_pair.first;
	auto id = last_pair.second;
	auto entry = id_to_entry_[id];
	top_10_.erase(score_entry);
	top_10_collection_.remove(entry, entry.score());
	spill_check(entry, SPILL_IN_90);
	bottom_90_.insert(last_pair);
	bottom_90_collection_.add(entry, entry.score());
    } else if (top_10_.size() < top_10_threshold && bottom_90_.size() > 0) {
	auto it = bottom_90_.begin();
	auto first_pair = *it;
	auto score_entry = first_pair.first;
	auto id = first_pair.second;
	auto entry = id_to_entry_[id];
	bottom_90_.erase(score_entry);
	bottom_90_collection_.remove(entry, entry.score());
	spill_check(entry, SPILL_IN_10);
	top_10_.insert(first_pair);
	top_10_collection_.add(entry, entry.score());
    }
}

void address_book::remove(const ip_service &ip)
{
    auto it = ip_to_id_.find(ip);
    if (it == ip_to_id_.end()) {
	// Not found
	return;
    }
    remove(it->second);
}

void address_book::remove(size_t id)
{
    auto it = id_to_entry_.find(id);
    if (it == id_to_entry_.end()) {
	assert(false);
	// Not found
	return;
    }
    ip_service ip = it->second;
    int score = it->second.score();
    id_to_entry_.erase(it);
    ip_to_id_.erase(ip);
    top_10_.erase(score_entry(score, id));
    top_10_collection_.remove(ip, score);
    bottom_90_collection_.remove(ip, score);
    bottom_90_.erase(score_entry(score, id));

    auto it2 = unverified_id_to_gid_.find(id);
    if (it2 != unverified_id_to_gid_.end()) {
	auto key = it2->second;
	unverified_[key].remove(ip, 0);
	unverified_id_to_gid_.erase(it2);
    }
}

void address_book::add_score(address_entry &e, int change)
{
}

size_t address_book::size() const
{
    return id_to_entry_.size();
}

std::vector<address_entry> address_book::get_from_top_10_pt(size_t n)
{
    assert(n < id_to_entry_.size());

    std::vector<address_entry> result;
    size_t cnt = 0;
    for (auto ips = top_10_.begin(); ips != top_10_.end(); ++ips) {
	auto p = *ips;
	if (++cnt > n) {
	    break;
	}
	result.push_back(id_to_entry_[p.second]);
    }

    return result;
}


// Select N entries from top 10%. Preferably addresses from different groups.
std::vector<address_entry> address_book::get_randomly_from_top_10_pt(size_t n)
{
    std::vector<address_entry> result;
    auto ips = top_10_collection_.select(n);
    for (auto &ip : ips) {
	result.push_back(id_to_entry_[ip_to_id_[ip]]);
    }

    return result;
}

std::vector<address_entry> address_book::get_randomly_from_bottom_90_pt(size_t n)
{
    std::vector<address_entry> result;
    auto ips = bottom_90_collection_.select(n);
    for (auto &ip : ips) {
	result.push_back(id_to_entry_[ip_to_id_[ip]]);
    }

    return result;
}

std::vector<address_entry> address_book::get_randomly_from_unverified(size_t n)
{
    std::vector<address_entry> result;
    
    std::unordered_set<int> selected;
    size_t fail_count = 0;
    for (size_t i = 0; i < n && fail_count < MAX_FAIL_COUNT; i++) {

        // First select a random group
	auto gid = random_gid();
	auto it = unverified_gid_to_group_.lower_bound(gid);
	if (it == unverified_gid_to_group_.end()) {
	    fail_count++;
	    continue;
	}
	auto &group = it->second;
	auto tmp = unverified_[group].select(1);
	for (auto e : tmp) {
	    auto id = ip_to_id_[e];
	    if (selected.find(id) != selected.end()) {
		fail_count++;
		continue;
    	    }
	    selected.insert(id);
	    result.push_back(id_to_entry_[id]);
	}
    }

    return result;
}

void address_book::save(const std::string &path)
{
    using namespace prologcoin::common;

    std::ofstream out(path);
    term_env env;
    term_emitter emitter(out, env);
    emitter.set_option_nl(false);
    for (auto &p : id_to_entry_) {
	auto &e = p.second;
	e.write(env, emitter);
    }
}

void address_book::load(const std::string &path)
{
    using namespace prologcoin::common;

    set_spill_enabled(false);

    term_env env;
    std::ifstream in(path);
    term_tokenizer tok(in);
    term_parser parser(tok, env);

    while (!parser.is_eof()) {
	address_entry e;
	e.read(env, parser);
	add(e);
    }

    set_spill_enabled(true);
}

bool address_book::operator == (const address_book &other) const
{
    auto it1 = id_to_entry_.begin();
    auto it2 = other.id_to_entry_.begin();
    
    size_t index = 0;

    while (it1 != id_to_entry_.end()) {
	if (it2 == other.id_to_entry_.end()) {
	    return false;
	}

	auto &e1 = it1->second;
	auto &e2 = it2->second;

	if (e1 != e2) {
	    return false;
	}

	++it1;
	++it2;
	++index;
    }

    return it2 == other.id_to_entry_.end();
}

void address_book::integrity_check()
{
    // Build score_to_id map
    std::map<score_entry, size_t> score_to_id;
    for (auto p : id_to_entry_) {
	auto e = p.second;
	if (is_unverified(e)) {
	    continue;
	}
	score_to_id.insert(std::make_pair(score_entry(e.score(), e.id()), e.id()));
    }

    // Check that top 10 is correct.
    size_t num_top_10 = 10 * score_to_id.size() / 100;
    if (score_to_id.size() > 0 && num_top_10 == 0) {
	num_top_10 = 1;
    }

    auto it0 = score_to_id.begin();
    auto it1 = top_10_.begin();
    size_t i = 0;
    for (i = 0; i < num_top_10; i++, ++it0, ++it1) {
	auto e0 = it0->first;
	auto e1 = it1->first;
	if (e0 != e1) {
	    std::cout << "Failed at entry index " << i << " (on top 10%)" << std::endl;
	    std::cout << "e0=" << e0.str() << std::endl;
	    std::cout << "e1=" << e1.str() << std::endl;
	    
	    if (top_10_.find(e0) == top_10_.end()) {
		std::cout << "e0 did not exist in top 10%!" << std::endl;
		if (bottom_90_.find(e0) != bottom_90_.end()) {
		    std::cout << "  but it was found in bottom 90%!" << std::endl;
		} else {
		    std::cout << "  and it wasn't found in bottom 90% either!" << std::endl;
		}
		
	    } else {
		std::cout << "e0 did exist in top 10%, but just not here" << std::endl;
	    }
	}
	assert(e0 == e1);
	auto id = it0->second;
	auto eit = id_to_entry_.find(id);
	assert(eit != id_to_entry_.end());
	auto &entry = eit->second;
	if (top_10_collection_.size(entry.group()) > MAX_GROUP_SIZE) {
	    std::cout << "While processing index " << i << std::endl;
	    std::cout << "Group size for " << entry.str() << " exceeded maximum " << MAX_GROUP_SIZE << "; was " << top_10_collection_.size(entry.group()) << std::endl;
	}
	assert(top_10_collection_.size(entry.group()) <= MAX_GROUP_SIZE);
    }
    assert(it1 == top_10_.end());
    it1 = bottom_90_.begin();
    for (; i < score_to_id.size(); i++, ++it0, ++it1) {
	auto e0 = it0->first;
	auto e1 = it1->first;
	if (e0 != e1) {
	    std::cout << "Failed at entry index " << i << " (on bottom 90%)" << std::endl;
	    std::cout << "e0=" << e0.str() << std::endl;
	    std::cout << "e1=" << e1.str() << std::endl;
	}
	assert(e0 == e1);
	auto id = it0->second;
	auto eit = id_to_entry_.find(id);
	assert(eit != id_to_entry_.end());
	auto &entry = eit->second;

	if (bottom_90_collection_.size(entry.group()) > MAX_GROUP_SIZE) {
	    std::cout << "While processing index " << i << std::endl;
	    std::cout << "Group size for " << entry.str() << " exceeded maximum " << MAX_GROUP_SIZE << "; was " << bottom_90_collection_.size(entry.group()) << std::endl;
	}

	assert(bottom_90_collection_.size(entry.group()) <= MAX_GROUP_SIZE);
    }
    assert(it1 == bottom_90_.end());
    assert(it0 == score_to_id.end());

    top_10_collection_.integrity_check();
    bottom_90_collection_.integrity_check();
}

int address_book::random_gid()
{
    return prologcoin::common::random::next_int(MAX_GID);
}

int address_book::new_gid()
{
    int gid = random_gid();
    while (unverified_gid_to_group_.find(gid) != unverified_gid_to_group_.end()) {
	gid++;
	if (gid == MAX_GID) {
	    gid = 0;
	}
    }
    return gid;
}

}}
