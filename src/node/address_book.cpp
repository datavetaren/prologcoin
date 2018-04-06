#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "address_book.hpp"
#include "ip_address.hpp"
#include "../common/random.hpp"
#include "../common/checked_cast.hpp"
#include "../common/term_match.hpp"

namespace prologcoin { namespace node {

address_entry::address_entry()
    : ip_service(), id_(0), score_(0), time_(0), version_major_(0),
      version_minor_(0)
{
}

address_entry::address_entry(const address_entry &other)
    : ip_service(other), id_(other.id_), source_(other.source_),
      score_(other.score_), time_(other.time_),
      version_major_(other.version_major_),
      version_minor_(other.version_minor_),
      comment_(other.comment_)
{
}

address_entry::address_entry(const ip_address &addr, unsigned short port,
			     const ip_address &src_addr, unsigned short src_port)
    : ip_service(addr, port), id_(0), source_(src_addr, src_port),
      score_(0), time_(0), version_major_(0), version_minor_(0)
{
}

address_entry::address_entry(const ip_address &addr, unsigned short port)
    : ip_service(addr, port), id_(0), score_(0), time_(0),
      version_major_(0), version_minor_(0)
{
}

address_entry::address_entry(const ip_service &ip)
    : ip_service(ip), id_(0), score_(0), time_(0), version_major_(0),
      version_minor_(0)
{
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

void address_entry::set_comment(const common::term t, common::term_env &src)
{
    using namespace prologcoin::common;

    term_serializer ser(src);
    comment_.clear();
    ser.write(comment_, t);
}

std::string address_entry::comment_str() const
{
    using namespace prologcoin::common;

    if (comment_.empty()) {
	return "[]";
    }

    term_env env;
    term_serializer ser(env);
    term t = ser.read(comment_);
    return env.to_string(t);
}

common::term address_entry::to_term(common::term_env &env) const
{
    using namespace prologcoin::common;

    term term_addr = env.functor(addr().str(), 0);
    term term_port = int_cell(port());
    term term_src_addr = env.functor(source().addr().str(), 0);
    term term_src_port = int_cell(source().port());
    term term_score = int_cell(static_cast<int64_t>(score()));
    term term_time = env.functor(time().str(), 0);

    term term_version = env.new_term(env.functor("ver",2),
				     {int_cell(version_major()),
				      int_cell(version_minor())});
    term_serializer ser(env);
    term term_comment = (comment().size() > 0) ? ser.read(comment()) : env.empty_list();

    
    term term_entry = env.new_term(env.functor("entry",8),
	   {term_addr,term_port, term_src_addr, term_src_port,
	    term_score, term_time, term_version, term_comment});

    return term_entry;
}

bool address_entry::from_term(common::term_env &env, const common::term t) 
{
    using namespace prologcoin::common;
    static const con_cell entry_8("entry",8);

    pattern p(env);

    con_cell e_addr;
    int64_t e_port;
    con_cell e_src_addr;
    int64_t e_src_port;
    int64_t e_score;
    con_cell e_time;
    int64_t e_version_major;
    int64_t e_version_minor;
    term e_comment;
    auto pat = p.str(entry_8,
		     p.any_atom(e_addr),
		     p.any_int(e_port),
		     p.any_atom(e_src_addr),
		     p.any_int(e_src_port),
		     p.any_int(e_score),
		     p.any_atom(e_time),
		     p.str(con_cell("ver",2),
			   p.any_int(e_version_major),
			   p.any_int(e_version_minor)),
		     p.any(e_comment));
    if (!pat(env, t)) {
	return false;
    }

    try {
	auto addr = ip_address(env.atom_name(e_addr));
	auto port = checked_cast<unsigned short>(e_port);
	auto src_addr = ip_address(env.atom_name(e_src_addr));
	auto src_port = checked_cast<unsigned short>(e_src_port);
	auto score = checked_cast<int64_t>(e_score, -1000000, 1000000);
	utime time = utime::from_string(env.atom_name(e_time));
	auto version_major = checked_cast<int32_t>(e_version_major, 0, 1000);
	auto version_minor = checked_cast<int32_t>(e_version_minor, 0, 1000);
	term_serializer ser(env);
	term_serializer::buffer_t comment;
	ser.write(comment, e_comment);

	set_addr(addr);
	set_port(port);
	set_source(src_addr, src_port);
	set_score(score);
	set_time(time);
	set_version(version_major, version_minor);
	set_comment(comment);
    } catch (std::runtime_error &err) {
	return false;
    }

    return true;
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
	throw address_book_load_exception( "Expected functor entry/8, but got: " + env.to_string(t), line);
    }
    
    if (env.functor(t) != env.functor("entry",8)) {
	throw address_book_load_exception( "Expected functor entry/8, but got: " + env.to_string(env.functor(t)), line);
    }

    term term_addr = env.arg(t, 0);
    line = parser.line(parser.position_arg(parser.positions(),0));
    if (term_addr.tag() != tag_t::CON) {
	throw address_book_load_exception( "First argument of entry/8 should be an atom to represent an IP-address, but was " + env.to_string(term_addr), line);
    }
    std::string addr = env.atom_name(term_addr);
    try {
        boost::asio::ip::address::from_string(addr);
    } catch (std::runtime_error &ex) {
	throw address_book_load_exception( "Couldn't parse IP address: " + addr, line);
    }

    term term_port = env.arg(t, 1);
    line = parser.line(parser.position_arg(parser.positions(),1));
    if (term_port.tag() != tag_t::INT) {
	throw address_book_load_exception( "Second argument of entry/8 should be an integer to represent a port number, but was " + env.to_string(term_port), line);
    }
    int64_t port_value = reinterpret_cast<const int_cell &>(term_port).value();
    if (!(port_value >= 0 && port_value <= 65535)) {
	throw address_book_load_exception( "Second argument of entry/6 is port number but was not in valid range 0..65535: " + env.to_string(term_port),line);
    }
    unsigned short port = static_cast<unsigned short>(term_port.value());
    

    term term_src_addr = env.arg(t, 2);
    line = parser.line(parser.position_arg(parser.positions(),2));
    if (term_addr.tag() != tag_t::CON) {
	throw address_book_load_exception( "Third argument of entry/8 should be an atom to represent an IP-address, but was " + env.to_string(term_src_addr), line);
    }
    std::string src_addr = env.atom_name(term_src_addr);
    try {
        boost::asio::ip::address::from_string(src_addr);
    } catch (std::runtime_error &ex) {
	throw address_book_load_exception( "Couldn't parse IP address: " + src_addr, line);
    }

    term term_src_port = env.arg(t, 3);
    line = parser.line(parser.position_arg(parser.positions(),3));
    if (term_port.tag() != tag_t::INT) {
	throw address_book_load_exception( "Fourth argument of entry/8 should be an integer to represent a port number, but was " + env.to_string(term_port), line);
    }
    int64_t src_port_value = reinterpret_cast<const int_cell &>(term_src_port).value();
    if (!(src_port_value >= 0 && src_port_value <= 65535)) {
	throw address_book_load_exception( "Fourth argument of entry/8 is port number but was not in valid range 0..65535: " + env.to_string(term_src_port),line);
    }
    unsigned short src_port = static_cast<unsigned short>(term_src_port.value());


    term term_score = env.arg(t, 4);
    line = parser.line(parser.position_arg(parser.positions(),4));
    if (term_score.tag() != tag_t::INT) {
	throw address_book_load_exception( "Fifth argument of entry/8 should be an integer to represent a score, but was " + env.to_string(term_score), line);
    }

    int64_t score_value = reinterpret_cast<const int_cell &>(term_score).value();
    if (score_value < -1000000 || score_value > 1000000) {
	throw address_book_load_exception( "Fifth argument of entry/8 represents score but was not in the valid range -1000000..1000000: " + env.to_string(term_score), line);

    }
    int32_t score = static_cast<int32_t>(score_value);

    term term_time = env.arg(t, 5);
    line = parser.line(parser.position_arg(parser.positions(),5));
    if (term_time.tag() != tag_t::CON) {
	throw address_book_load_exception( "Sixth argument of entry/8 should be an atom to represent time, but was " + env.to_string(term_time), line);
    }
    std::string time_str = env.to_string(term_time);
    if (time_str[0] == '\'' && time_str[time_str.size()-1] == '\'') {
	// Remove quotes
	time_str = time_str.substr(1,time_str.size()-2);
    }
    utime ut;
    if (!ut.parse(time_str)) {
	throw address_book_load_exception( "Sixth argument of entry/8 should represent time, but failed to parse: " + time_str, line);
    }

    term term_ver = env.arg(t, 6);
    pattern p(env);
    int64_t version_major = 0, version_minor = 0;
    auto pat = p.str(con_cell("ver", 2), p.any_int(version_major), p.any_int(version_minor));
    if (!pat(env, term_ver)) {
	throw address_book_load_exception( "Seventh argument of entry/8 should represent a version ver(Major,Minor) but was: " + env.to_string(term_ver), line);
    }
    if (version_major < 0 || version_major > 1000) {
	throw address_book_load_exception( "Seventh argument of entry/8 should represents a version but major version number is outside the valid range 0..1000, it was: " + boost::lexical_cast<std::string>(version_major), line);
    }

    if (version_minor < 0 || version_minor > 1000) {
	throw address_book_load_exception( "Seventh argument of entry/8 should represents a version but minor version number is outside the valid range 0..1000, it was: " + boost::lexical_cast<std::string>(version_minor), line);
    }

    term term_comment = env.arg(t, 7);
    line = parser.line(parser.position_arg(parser.positions(),7));
    term_serializer::buffer_t buf;

    if (!env.is_empty_list(term_comment)) {
	term_serializer ser(env);
	ser.write(buf, term_comment);
    }

    set_addr(addr);
    set_port(port);
    set_source(src_addr, src_port);
    set_score(score);
    set_time(ut);
    set_version(static_cast<int32_t>(version_major),
		static_cast<int32_t>(version_minor));
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

    out << std::setw(14) << "Address" << " Port" << " " << std::setw(14) << "Source" << " Port" << std::setw(7) << " Score"  << "  Ver" << std::setw(20) << " Time" << " Comment" << std::endl;

    for (auto &e : entries) {
	std::string ver_str = boost::lexical_cast<std::string>(e.version_major()) + "." + boost::lexical_cast<std::string>(e.version_minor());
	out << std::setw(14) << e.addr().str(14) << " "
	    << std::setw(4) << e.port() << " "
	    << std::setw(14) << e.source().addr().str(14) << " "
	    << std::setw(4) << e.source().port() << " "
	    << std::setw(6) << e.score() << " "
	    << std::setw(4) << ver_str << " "
	    << std::setw(19) << e.time().str() << " ";
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
 	    auto to_spill = coll.spill();
	    remove(to_spill);
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

bool address_book::exists(const ip_service &ip, address_entry *entry)
{
    auto it = ip_to_id_.find(ip);
    if (it == ip_to_id_.end()) {
	return false;
    }
    const address_entry &e = id_to_entry_[it->second];
    if (entry != nullptr) {
	*entry = e;
    }
    if (top_10_collection_.exists(ip)) {
	return true;
    }
    if (bottom_90_collection_.exists(ip)) {
	return true;
    }
    auto id = it->second;
    auto it2 = unverified_id_to_gid_.find(id);
    if (it2 == unverified_id_to_gid_.end()) {
	return false;
    }
    auto key = it2->second;
    return unverified_[key].exists(ip);
}

void address_book::add_score(address_entry &e, int change)
{
    // Simplest is to remove and add it. It's all O(log n) operations.
    // Maybe not the most efficient way, but perhaps efficient enough.
    // And hopefully less errorprone.

    remove(e);
    auto new_score = e.score() + change;
    // Don't bother put it back if scores go under -10000.
    if (new_score >= -10000) { 
	e.set_score(new_score);
	add(e);
    }
}

void address_book::update(address_entry &e)
{
    address_entry ee = e;
    if (exists(e, &ee)) {
	remove(ee);
    }
    add(e);
}

size_t address_book::size() const
{
    return id_to_entry_.size();
}

std::vector<address_entry> address_book::get_all_true(std::function<bool (const address_entry &e)> predicate)
{
    std::vector<address_entry> result;
    
    for (auto e : id_to_entry_) {
	if (!predicate(e.second)) {
	    continue;
	}
	result.push_back(e.second);
    }
    return result;
}

std::vector<address_entry> address_book::get_all()
{
    return get_all_true([](const address_entry &) {return true; });
}

std::vector<address_entry> address_book::get_all_verified()
{
    return get_all_true([this](const address_entry &e) {return !this->is_unverified(e); });
}

std::vector<address_entry> address_book::get_all_unverified()
{
    return get_all_true([this](const address_entry &e) {return this->is_unverified(e); });
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

std::vector<address_entry> address_book::get_from_bottom_90_pt(size_t n)
{
    assert(n < id_to_entry_.size());

    std::vector<address_entry> result;
    size_t cnt = 0;
    for (auto ips = bottom_90_.rbegin(); ips != bottom_90_.rend(); ++ips) {
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
	    it = unverified_gid_to_group_.begin();
	    if (it == unverified_gid_to_group_.end()) {
		fail_count++;
		continue;
	    }
	}
	auto &group = it->second;
	auto tmp = unverified_[group].select(1);
	if (tmp.empty()) {
	    fail_count++;
	    continue;
	}
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

void address_book::update_time(const ip_service &ip)
{
    using namespace prologcoin::common;

    auto it = ip_to_id_.find(ip);
    if (it == ip_to_id_.end()) {
	return;
    }
    auto it2 = id_to_entry_.find(it->second);
    if (it2 == id_to_entry_.end()) {
	return;
    }
    auto &entry = it2->second;
    entry.set_time(utime::now_seconds());
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

void address_book::for_each_address_entry(const std::function<void (const address_entry &e)> &fn) {
    for (auto p : id_to_entry_) {
	auto const &entry = p.second;
	fn(entry);
    }
}

}}
