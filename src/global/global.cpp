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
      interp_(nullptr) {
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
}

bool global::db_get_predicate(const qname &qn,
			      std::vector<term> &clauses) { 
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

size_t global::current_height() const {
    return blockchain_.tip().get_height();
}

void global::increment_height()
{
    get_blockchain().advance();
    interp().commit_heap();
    interp().commit_closures();
    interp().commit_symbols();
    interp().commit_program();
    get_blockchain().update_tip();
}
    
}}

