#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "../common/blake2.hpp"
#include "../common/checked_cast.hpp"
#include "blockchain.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace global {

blockchain::blockchain(const std::string &data_dir) :
    data_dir_(data_dir),
    db_meta_dir_((boost::filesystem::path(data_dir_) / "db" / "meta").string()),
    db_goal_blocks_dir_((boost::filesystem::path(data_dir_) / "db" / "goals").string()),
    db_heap_dir_((boost::filesystem::path(data_dir_) / "db" / "heap").string()),
    db_closures_dir_((boost::filesystem::path(data_dir_) / "db" / "closures").string()),
    db_symbols_dir_((boost::filesystem::path(data_dir_) / "db" / "symbols").string()),
    db_program_dir_((boost::filesystem::path(data_dir_) / "db" / "program").string()) {
   init();
}

void blockchain::update_meta_id()
{
    blake2b_state s;
    blake2b_init(&s, BLAKE2B_OUTBYTES);
    
    // Hash all root hashes from state databases
    auto &t = tip_;
    auto &h1 = heap_db().get_root_hash(t.get_root_id_heap());
    blake2b_update(&s, h1.hash(), h1.hash_size());
    auto &h2 = closures_db().get_root_hash(t.get_root_id_closures());
    blake2b_update(&s, h2.hash(), h2.hash_size());
    auto &h3 = symbols_db().get_root_hash(t.get_root_id_symbols());
    blake2b_update(&s, h3.hash(), h3.hash_size());
    auto &h4 = program_db().get_root_hash(t.get_root_id_program());
    blake2b_update(&s, h4.hash(), h4.hash_size());

    // Add previous id
    blake2b_update(&s, tip_.get_previous_id().hash(), meta_id::HASH_SIZE);

    // Reuse this data buffer for everything. pow-proof is the biggest.
    uint8_t data[64];

    // Hash all number of entries for each daatbase
    db::write_uint64(data, checked_cast<uint64_t>(heap_db().num_entries(t.get_root_id_heap())));
    blake2b_update(&s, data, sizeof(uint64_t));
    db::write_uint64(data, checked_cast<uint64_t>(closures_db().num_entries(t.get_root_id_closures())));
    blake2b_update(&s, data, sizeof(uint64_t));
    db::write_uint64(data, checked_cast<uint64_t>(symbols_db().num_entries(t.get_root_id_symbols())));
    blake2b_update(&s, data, sizeof(uint64_t));
    db::write_uint64(data, checked_cast<uint64_t>(program_db().num_entries(t.get_root_id_program())));
    blake2b_update(&s, data, sizeof(uint64_t));

    // Add version
    db::write_uint64(data, tip_.get_version());
    blake2b_update(&s, data, sizeof(uint32_t));

    // Add nonce
    db::write_uint64(data, tip_.get_nonce());
    blake2b_update(&s, data, sizeof(uint64_t));

    // Add timestamp
    auto &ts = tip_.get_timestamp();
    ts.write(data);
    blake2b_update(&s, data, ts.serialization_size());

    // Add pow difficulty
    tip_.get_pow_difficulty().write(data);
    blake2b_update(&s, data, tip_.get_pow_difficulty().serialization_size());

    memset(data, 0, sizeof(data));
    blake2b_final(&s, data, BLAKE2B_OUTBYTES);
    
    tip_.set_id(meta_id(data));
}

void blockchain::init()
{
    db_meta_ = nullptr;
    db_goal_blocks_ = nullptr;
    db_heap_ = nullptr;
    db_closures_ = nullptr;
    db_symbols_ = nullptr;
    db_program_ = nullptr;
    
    tip_ = meta_entry();
    at_height_.clear();
    chains_.clear();
    
    // We let the tip be a simple txt file to make it easy to edit and check

    db_root_id tip_id;

    // Read in meta records
    meta_entry best;
    for (size_t height = 0;;height++) {
	auto &ids = meta_db().find_roots(height);
	if (ids.empty()) {
	    break;
	}
	for (auto &id : ids) {
	    auto stored = meta_db().find(id, height);
	    if (stored == nullptr) {
		continue;
	    }
	    auto custom_data = stored->custom_data();
	    auto custom_data_size = stored->custom_data_size();
	    meta_entry entry;
	    assert(custom_data_size == entry.serialization_size());
	    entry.read(custom_data);
	    entry.set_height(height);
	    entry.set_root_id_meta(id);
	    if (!entry.get_id().is_zero()) {
		chains_.insert(std::make_pair(entry.get_id(), entry));
		at_height_[entry.get_height()].insert(entry.get_id());
		if (!entry.is_partial()) {
		    best = entry;
		}
	    }
	}
    }

    tip_ = best;
    
    auto tip_path = boost::filesystem::path(data_dir_) / "db" / "tip.txt";
    if (boost::filesystem::exists(tip_path)) {
	std::string line;
	std::ifstream fin = std::ifstream(tip_path.string());
	std::getline(fin, line);
	uint8_t id[meta_id::HASH_SIZE];
	if (line.length() != meta_id::HASH_SIZE) {
	    return;
	}
	hex::from_string(line, id, meta_id::HASH_SIZE);
	std::getline(fin, line);
	size_t height;
	std::istringstream iss(line);
	iss >> height;

	meta_id search_for(id);
	auto it = chains_.find(search_for);
	if (it != chains_.end()) {
	    tip_ = it->second;
	}
    }

    version_ = VERSION;
    nonce_ = 0;
    time_ = utime();
}

static bool is_match(const meta_id &id, const uint8_t *prefix, size_t prefix_len) {
    if (prefix_len > id.hash_size()) {
	return false;
    }
    return memcmp(id.hash(), prefix, prefix_len) == 0;
}

std::set<meta_id> blockchain::find_entries(const uint8_t *prefix, size_t prefix_len) {
    uint8_t search[meta_id::HASH_SIZE];
    assert(prefix_len <= meta_id::HASH_SIZE);
    memset(&search[0], 0, meta_id::HASH_SIZE);
    memcpy(&search[0], prefix, prefix_len);
    meta_id search_id(search);
    std::set<meta_id> found;
    auto it = chains_.lower_bound(search_id);
    for (; it != chains_.end(); ++it) {
	auto &id = it->first;
	if (!is_match(id, prefix, prefix_len)) {
	    break;
	}
	found.insert(id);
    }
    return found;
}
	
std::set<meta_id> blockchain::find_entries(size_t height, const uint8_t *prefix, size_t prefix_len)
{
    std::set<meta_id> matched;
    size_t NONE = std::numeric_limits<size_t>::max();
    size_t h = height;
    if (h == NONE) {
	h = 0;
    }
    for (; !at_height_[h].empty(); h++) {
	auto &ids = at_height_[h];
	for (auto &id : ids) {
	    if (is_match(id, prefix, prefix_len)) {
		matched.insert(id);
	    }
	}
	if (height != NONE) {
	    break;
	}
    }
    return matched;
}

void blockchain::flush_db() {
    goal_blocks_db().flush();
    heap_db().flush();
    closures_db().flush();
    symbols_db().flush();
    program_db().flush();
    meta_db().flush();
}

void blockchain::advance() {
    flush_db();

    db::root_id next_meta;
    db::root_id next_goal_blocks;
    db::root_id next_heap;
    db::root_id next_closures;
    db::root_id next_symbols;
    db::root_id next_program;

    bool genesis = false;
    
    if (tip().get_root_id_meta().is_zero()) {
	next_meta = meta_db().new_root();
	next_goal_blocks = goal_blocks_db().new_root();
	next_heap = heap_db().new_root();
	next_closures = closures_db().new_root();
	next_symbols = symbols_db().new_root();
	next_program = program_db().new_root();
	genesis = true;
    } else {
	next_meta = meta_db().new_root(tip().get_root_id_meta());
	next_goal_blocks = goal_blocks_db().new_root(tip().get_root_id_goal_blocks());
	next_heap = heap_db().new_root(tip().get_root_id_heap());
	next_closures = closures_db().new_root(tip().get_root_id_closures());
	next_symbols = symbols_db().new_root(tip().get_root_id_symbols());
	next_program = program_db().new_root(tip().get_root_id_program());
    }

    size_t new_height = genesis ? 0 : tip().get_height() + 1;

    meta_entry new_tip(tip());
    meta_id new_id;
    new_tip.set_id(new_id); // Will be recomputed, just setting it to zero
                            // to avoid confusion.
    new_tip.set_previous_id(tip().get_id());
    new_tip.set_version(version_);
    new_tip.set_timestamp(time_);
    new_tip.set_nonce(nonce_);
    new_tip.set_height(new_height);
    new_tip.set_root_id_meta(next_meta);
    new_tip.set_root_id_goal_blocks(next_goal_blocks);
    new_tip.set_root_id_heap(next_heap);
    new_tip.set_root_id_closures(next_closures);
    new_tip.set_root_id_symbols(next_symbols);
    new_tip.set_root_id_program(next_program);

    tip_ = new_tip;
}

void blockchain::add_meta_entry(meta_entry &e)
{
    if (e.get_previous_id().is_zero()) {
	e.set_root_id_meta(meta_db().new_root());
    } else {
	auto *prev_entry = get_meta_entry(e.get_previous_id());
	assert(prev_entry != nullptr);
	e.set_root_id_meta(meta_db().new_root(prev_entry->get_root_id_meta()));
    }
	
    const size_t data_size = meta_entry::serialization_size();
    uint8_t data[ data_size ];
    e.write(data);
    meta_db().update(e.get_root_id_meta(),
		     e.get_height(),
		     data, data_size);
    chains_.insert(std::make_pair(e.get_id(), e));
    at_height_[e.get_height()].insert(e.get_id());
}
	
void blockchain::update_tip() {
    update_meta_id();

    auto find_it = chains_.find(tip().get_id());
    if (find_it != chains_.end()) {
	tip_ = find_it->second;
	return;
    }

    add_meta_entry(tip());
}

}}
