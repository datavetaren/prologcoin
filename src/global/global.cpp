#include <boost/filesystem.hpp>
#include "global.hpp"
#include "meta_entry.hpp"
#include "global_interpreter.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

namespace prologcoin { namespace global {

global::global(const std::string &data_dir)
    : data_dir_(data_dir),
      blockchain_(data_dir),
      interp_(nullptr),
      commit_version_(blockchain::VERSION),
      commit_nonce_(0),
      commit_time_(),
      commit_goals_() {
    interp_ = std::unique_ptr<global_interpreter>(new global_interpreter(*this));
    interp_->init();
}

bool global::set_tip(const meta_id &id)
{
    auto entry = blockchain_.get_meta_entry(id);
    if (!entry) {
	return false;
    }
    blockchain_.set_tip(*entry);
    interp_ = nullptr;
    interp_ = std::unique_ptr<global_interpreter>(new global_interpreter(*this));
    interp_->init();
    return true;
}
	
void global::erase_db(const std::string &data_dir)
{
    auto dir_path = boost::filesystem::path(data_dir) / "db";
    boost::system::error_code ec;
    boost::filesystem::remove_all(dir_path, ec);
    if (ec) {
        throw global_db_exception( "Failed while attempting to erase all files at " + dir_path.string() + "; " + ec.message());
    }
}

void global::total_reset() {
    interp_ = nullptr;
    erase_db(data_dir_);
    blockchain_.init();
    interp_ = std::unique_ptr<global_interpreter>(new global_interpreter(*this));
    interp_->init();
}

bool global::db_get_predicate(const qname &qn,
			      std::vector<term> &clauses) {

    if (blockchain_.program_root().is_zero()) {
	return false;
    }
    
    // Max 10 collisions
    size_t i;
    const uint8_t *p = nullptr;
    size_t custom_data_size = 0;
    for (i = 0; i < 10; i++) {
	common::fast_hash h;
	h << qn.first.raw_value() << qn.second.raw_value() << i;
	auto key = h.finalize();
	auto leaf = blockchain_.program_db().find(
						  blockchain_.program_root(), key);
	if (leaf == nullptr) {
	    return false;
	}
	p = leaf->custom_data();
	custom_data_size = leaf->custom_data_size();
	common::untagged_cell qfirst = db::read_uint64(p); p += sizeof(uint64_t);
	common::untagged_cell qsecond = db::read_uint64(p); p += sizeof(uint64_t);
	common::con_cell &qfirst_con = reinterpret_cast<common::con_cell &>(qfirst);
	common::con_cell &qsecond_con = reinterpret_cast<common::con_cell &>(qsecond);
	interp::qname qn_entry(qfirst_con, qsecond_con);
	if (qn == qn_entry) {
	    break;
	}
    }
    if (i == 10) {
	return false;
    }
    size_t n_bytes = custom_data_size - 2*sizeof(uint64_t);
    size_t n_cells = n_bytes / sizeof(common::cell);
    for (i = 0; i < n_cells; i++) {
	common::cell c(db::read_uint64(p)); p += sizeof(uint64_t);
	clauses.push_back(c);
    }
    return true;
}
	
bool global::db_set_predicate(const qname &qn,
			      const std::vector<term> &clauses) { 
    static const size_t MAX_CLAUSES = 32;
    if (clauses.size() > MAX_CLAUSES) {
	return false;
    }   

    // Max 10 collisions
    size_t i;
    uint64_t key;
    for (i = 0; i < 10; i++) {
	common::fast_hash h;
	h << qn.first.raw_value() << qn.second.raw_value() << i;
	key = h.finalize();
	auto leaf = blockchain_.program_db().find(
						  blockchain_.program_root(), key);
	if (leaf == nullptr) {
	    break;
	}
	auto p = leaf->custom_data();
	common::untagged_cell qfirst = db::read_uint64(p); p += sizeof(uint64_t);
	common::untagged_cell qsecond = db::read_uint64(p); p += sizeof(uint64_t);
	common::con_cell &qfirst_con = reinterpret_cast<common::con_cell &>(qfirst);
	common::con_cell &qsecond_con = reinterpret_cast<common::con_cell &>(qsecond);
	interp::qname qn_entry(qfirst_con, qsecond_con);
	if (qn == qn_entry) {
	    break;
	}
    }
    if (i == 10) {
	return false;
    }
    uint8_t custom_data[2*sizeof(uint64_t)+MAX_CLAUSES*sizeof(common::cell)];
    uint8_t *p = &custom_data[0];
    db::write_uint64(p, qn.first.raw_value()); p += sizeof(uint64_t);
    db::write_uint64(p, qn.second.raw_value()); p += sizeof(uint64_t);
    for (auto &c : clauses) {
	db::write_uint64(p, c.raw_value()); p += sizeof(cell);
    }
    size_t custom_data_size = 2*sizeof(uint64_t)+clauses.size()*sizeof(cell);
    blockchain_.program_db().update(blockchain_.program_root(), key,
				    custom_data, custom_data_size);
    return true;
}

term global::db_get_goal_block(term_env &dst, const meta_id &root_id) {
    auto *e = blockchain_.get_meta_entry(root_id);
    if (e == nullptr) {
	return common::heap::EMPTY_LIST;
    }
    return db_get_goal_block(dst, *e);
}
	
term global::db_get_goal_block(term_env &dst, const meta_entry &e) {
    size_t height = e.get_height();
    auto leaf = blockchain_.goal_blocks_db().find(e.get_root_id_goal_blocks(), height);
    if (leaf == nullptr) {
	return common::heap::EMPTY_LIST;
    }
    term_serializer::buffer_t buf(&leaf->custom_data()[0],
				  &leaf->custom_data()[leaf->custom_data_size()]);
    
    term_serializer ser(dst);
    try {
        term blk = ser.read(buf);
	return blk;
    } catch (serializer_exception &ex) {
	if (&dst == &(*interp_)) reset();
	throw ex;
    }
}

void global::db_set_goal_block(size_t height, const term_serializer::buffer_t &buf)
{
    blockchain_.goal_blocks_db().update(blockchain_.goal_blocks_root(), height,
					&buf[0], buf.size());
}

static term to_number(term_env &dst, uint64_t v)
{
    static uint64_t limit = int_cell::max().value();
    if (v > limit) {
	boost::multiprecision::cpp_int i = v;
	term big = dst.new_big(64);
	dst.set_big(big, i);
	return big;
    } else {
	return int_cell(static_cast<int64_t>(v));
    }
}

term global::db_get_meta(term_env &dst, const meta_id &root_id) {
    auto *e = blockchain_.get_meta_entry(root_id);
    if (e == nullptr) {
	return common::heap::EMPTY_LIST;
    }
    term lst = common::heap::EMPTY_LIST;
    auto id = dst.new_term( con_cell("id",1), { root_id.to_term(dst) } );
    auto previd = dst.new_term(con_cell("previd",1), { e->get_previous_id().to_term(dst) });
    auto ver = dst.new_term( con_cell("version",1), { int_cell(static_cast<int64_t>(e->get_version())) });
    auto height = dst.new_term( con_cell("height",1), { int_cell(static_cast<int64_t>(e->get_height())) });
    auto nonce = dst.new_term( con_cell("nonce",1), { to_number(dst, e->get_nonce()) });
    auto time = dst.new_term( con_cell("time",1), { int_cell(static_cast<int64_t>(e->get_timestamp().in_ss())) });
    lst = dst.new_dotted_pair(time, lst);
    lst = dst.new_dotted_pair(nonce, lst);
    lst = dst.new_dotted_pair(height, lst);
    lst = dst.new_dotted_pair(ver, lst);
    lst = dst.new_dotted_pair(previd, lst);
    lst = dst.new_dotted_pair(id, lst);
    auto meta = dst.new_term(con_cell("meta",1), { lst } );
    return meta;
}

static bool get_meta_id(term_env &src, term term_id, meta_id &id) {
    return id.from_term(src, term_id);
}

bool global::db_parse_meta(term_env &src, term meta_term, meta_entry &entry)
{
    if (meta_term.tag() != tag_t::STR) {
	return false;
    }
    if (src.functor(meta_term) != con_cell("meta",1)) {
	return false;
    }
    term lst = src.arg(meta_term,0);

    while (src.is_dotted_pair(lst)) {
	term prop = src.arg(lst, 0);
	if (prop.tag() != tag_t::STR) {
	    throw global_db_exception("Unexpected property in meta: " + src.to_string(prop));
	}
	term f = src.functor(prop);
	if (f == con_cell("id",1)) {
	    meta_id id;
	    if (!get_meta_id(src, src.arg(prop,0), id)) {
		throw global_db_exception("Unexpected identifier in meta: " + src.to_string(prop));
	    }
	    entry.set_id(id);
	} else if (f == con_cell("previd",1)) {
	    meta_id previd;
	    if (!get_meta_id(src, src.arg(prop,0), previd)) {
		throw global_db_exception("Unexpected identifier in meta: " + src.to_string(prop));
	    }
	    entry.set_previous_id(previd);
	} else if (f == con_cell("version",1)) {
	    uint64_t ver;
	    term ver_term = src.arg(prop,0);
	    if (!src.get_number(ver_term, ver)) {
		throw global_db_exception("Expected non-negative integer version number; was " + src.to_string(prop));
	    }
	    if (ver != 1) {
		throw global_db_exception("Expected integer version 1; was " + src.to_string(prop));
	    }
	    entry.set_version(ver);
	} else if (f == con_cell("height",1)) {
	    term height_term  = src.arg(prop,0);
	    uint32_t height;
	    if (!src.get_number(height_term, height)) {
		throw global_db_exception("Expected non-negative integer for height: " + src.to_string(prop));
	    }
	    entry.set_height(height);
	} else if (f == con_cell("nonce", 1)) {
	    term nonce_term = src.arg(prop,0);
	    uint64_t nonce;
	    if (!src.get_number(nonce_term, nonce)) {
		throw global_db_exception("Nonce is not a valid number: " + src.to_string(prop));
	    }
	    entry.set_nonce(nonce);
	} else if (f == con_cell("time",1)) {
	    term time_term = src.arg(prop, 0);
	    uint64_t t;
	    if (!src.get_number(time_term, t)) {
		throw global_db_exception("Time is not a valid unsigned 64-bit number: " + src.to_string(prop));
	    }
	    entry.set_timestamp(utime(t));
	} else {
	    // Ignore unrecognized fields
	}
	lst = src.arg(lst, 1);
    }
    return true;
}

bool global::db_put_meta(term_env &src, term meta_term) {
    meta_entry entry;
    
    if (!db_parse_meta(src, meta_term, entry)) {
	return false;
    }
    
    if (check_pow()) {
	if (!entry.validate_pow()) {
	    std::string short_id = boost::lexical_cast<std::string>(entry.get_height()) + "(" + hex::to_string(entry.get_id().hash(), 2) + ")";
	    throw global_db_exception("PoW check failed for " + short_id);
	}
    }

    auto &previd = entry.get_previous_id();
    if (!previd.is_zero() &&
	blockchain_.get_meta_entry(previd) == nullptr) {
	auto previd_str = src.to_string(previd.to_term(src));
	throw global_db_exception("Couldn't find previoius identifier: " + previd_str);
    }    

    // We never want to corrupt the genesis state or existing entries
    if (entry.get_height() > 0) {
	if (!blockchain_.get_meta_entry(entry.get_id())) {
	    blockchain_.add_meta_entry(entry);
	}
    }

    return true;
}

size_t global::db_get_meta_length(const meta_id &root_id, size_t lookahead_n) {
    auto follows = blockchain_.follows(root_id);
    if (lookahead_n == 0 || follows.empty()) {
	return 1;
    }
    size_t max_len = 0;
    for (auto &child_id : follows) {
	size_t len = 1 + db_get_meta_length(child_id, lookahead_n-1);
	if (len > max_len) max_len = len;
    }
    return max_len;
}

term global::db_get_meta_roots(term_env &dst, const meta_id &root_id, size_t spacing, size_t n) {
    term lst = common::heap::EMPTY_LIST;

    if (n == 0) {
	return lst;
    }

    auto id = root_id;
    auto id_term = id.to_term(dst);
    term head  = dst.new_dotted_pair(id_term, common::heap::EMPTY_LIST);
    term tail = head;
    n--;
    
    while (n > 0) {
	bool found = false;
	for (size_t i = 0; i < spacing; i++) {
	    auto follows = blockchain_.follows(id);
	    if (follows.empty()) {
		break;
	    }
	    found = true;
	    // Pick the longest branch with 10 blocks as max length
	    // Note that this is not really an attack vector. Yes, idiot miners
	    // can mine spurious branches (that are long), but this only impacts
	    // the parallelization of downloading meta records.
	    size_t max_len = 0;
	    meta_id best_id;
	    if (follows.size() > 1) {
		for (auto &follow_id : follows) {
		    auto len = db_get_meta_length(follow_id, 10);
		    if (len > max_len) {
			max_len = len;
			best_id = follow_id;
		    }
		}
	    } else {
		best_id = *follows.begin();
	    }
	    id = best_id;
	}
	if (!found) {
	    break;
	}

	id_term = id.to_term(dst);
	
	term new_term = dst.new_dotted_pair(id_term, common::heap::EMPTY_LIST);
	dst.set_arg(tail, 1, new_term);
	tail = new_term;
	
	n--;
    }

    return head;
}

term global::db_get_metas(term_env &dst, const meta_id &root_id, size_t n)
{
    term lst = common::heap::EMPTY_LIST;

    std::vector<meta_id> branches;
    branches.push_back(root_id);

    term m = db_get_meta(dst, root_id);
    lst = dst.new_dotted_pair(m, lst);
    term head = lst, tail = lst;
    bool has_more = true;
    if (n > 0) n--;
    
    while (n > 0 && has_more) {
	has_more = false;
	size_t num_branches = branches.size();
	for (size_t i = 0; i < num_branches; i++) {
	    auto const &id = branches[i];
	    auto follows = blockchain_.follows(id);
	    if (follows.empty()) {
		branches.erase(branches.begin()+i);
		num_branches--;
		i--;
		continue;
	    }
	    bool first = true;
	    for (auto const &fid : follows) {
		has_more = true;
		term m = db_get_meta(dst, fid);
		term new_term = dst.new_dotted_pair(m, common::heap::EMPTY_LIST);
		dst.set_arg(tail, 1, new_term);
		tail = new_term;
		
		if (!first) branches.push_back(fid);
		if (first) {
		    branches[i] = fid;
		    first = false;
		}
		if (n > 0) n--;
	    }
	}
    }

    return head;
}

void global::db_put_metas(term_env &src, term lst)
{
    while (src.is_dotted_pair(lst)) {
	auto meta_term = src.arg(lst, 0);
	db_put_meta(src, meta_term);
	lst = src.arg(lst, 1);
    }
}

size_t global::current_height() const {
    return blockchain_.tip().get_height();
}

void global::increment_height()
{
    get_blockchain().set_version(commit_version_);
    get_blockchain().set_nonce(commit_nonce_);
    get_blockchain().set_time(commit_time_);
    get_blockchain().advance();
    interp().commit_heap();
    interp().commit_closures();
    interp().commit_symbols();
    interp().commit_program();
    if (!commit_goals_.empty()) {
	db_set_goal_block(current_height(), commit_goals_);
	commit_goals_.clear();
    }
    get_blockchain().update_tip();
}

void global::setup_commit(const term_serializer::buffer_t &buf) {
    term_env env;
    term_serializer ser(env);
    term m = ser.read(buf);
    if (m.tag() != tag_t::STR || env.functor(m) != con_cell("meta",1)) {
	throw global_db_exception( "expected meta/1; was " + env.to_string(m));
    }
    term lst = env.arg(m, 0);
    while (env.is_dotted_pair(lst)) {
	auto el = env.arg(lst, 0);
	if (env.functor(el) == con_cell("version",1)) {
	    auto val = env.arg(el, 0);
	    if (val.tag() != tag_t::INT) {
		throw global_db_exception( "version must be an integer; was " + env.to_string(val));
	    }
	    commit_version_ = static_cast<uint64_t>(reinterpret_cast<int_cell &>(val).value());
	} else if (env.functor(el) == con_cell("nonce",1)) {
	    auto val = env.arg(el, 0);
	    if (val.tag() != tag_t::INT &&
		val.tag() != tag_t::BIG) {
		throw global_db_exception( "nonce must be a number; was " + env.to_string(val));
	    }
	    if (val.tag() == tag_t::INT) {
		commit_nonce_ = static_cast<uint64_t>(reinterpret_cast<int_cell &>(val).value());
	    } else {
		assert(val.tag() == tag_t::BIG);
		boost::multiprecision::cpp_int i;
		size_t num_bits = 0;
		env.get_big(val, i, num_bits);
		if (num_bits > 64) {
		    throw global_db_exception( "Number too big, bigger than 64 bits; was " + env.to_string(val));
		}
		commit_nonce_ = static_cast<uint64_t>(i);
	    }
	} else if (env.functor(el) == con_cell("time",1)) {
	    auto val = env.arg(el, 0);
	    if (val.tag() != tag_t::INT) {
		throw global_db_exception("time must be an integer; was " + env.to_string(val));
	    }
	    auto timestamp = reinterpret_cast<int_cell &>(val).value();
	    commit_time_ = utime(static_cast<uint64_t>(timestamp));
	}
	lst = env.arg(lst, 1);
    }
}

bool global::execute_commit(const term_serializer::buffer_t &buf) {
    if (!execute_goal_silent(buf)) {
	discard();
	return false;
    }
    execute_cut();
    assert(is_clean());
    commit_goals_ = buf;
    advance();
    return true;
}
    
}}

