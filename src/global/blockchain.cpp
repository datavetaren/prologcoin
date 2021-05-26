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
    db_blocks_dir_((boost::filesystem::path(data_dir_) / "db" / "blocks").string()),
    db_heap_dir_((boost::filesystem::path(data_dir_) / "db" / "heap").string()),
    db_closure_dir_((boost::filesystem::path(data_dir_) / "db" / "closure").string()),
    db_symbols_dir_((boost::filesystem::path(data_dir_) / "db" / "symbols").string()),
    db_program_dir_((boost::filesystem::path(data_dir_) / "db" / "program").string()) {
   init();
}

void blockchain::update_meta_id()
{
    blake2b_state s;
    blake2b_init(&s, BLAKE2B_OUTBYTES);

    // Add previous id
    blake2b_update(&s, tip_.get_previous_id().hash(), meta_id::HASH_SIZE);

    auto &t = tip_;

    // Add the hash of the block that got us here
    // (If we add this, then we have the ability to falsify the whole
    //  meta-entry and mark it as invalid - otherwise we may attempt
    //  to infinitely look in vain for the proper version of its meta entry.)
    if (tip_.get_height() != 0) {
	auto *block = blocks_db().find(t.get_root_id_blocks(), tip_.get_height());
	assert(block != nullptr);
	blake2b_update(&s, block->hash(), block->hash_size());
    }	
    
    // Hash all root hashes from state databases
    auto &h1 = heap_db().get_root_hash(t.get_root_id_heap());
    blake2b_update(&s, h1.hash(), h1.hash_size());
    auto &h2 = closure_db().get_root_hash(t.get_root_id_closure());
    blake2b_update(&s, h2.hash(), h2.hash_size());
    auto &h3 = symbols_db().get_root_hash(t.get_root_id_symbols());
    blake2b_update(&s, h3.hash(), h3.hash_size());
    auto &h4 = program_db().get_root_hash(t.get_root_id_program());
    blake2b_update(&s, h4.hash(), h4.hash_size());

    // Reuse this data buffer for everything.
    uint8_t data[1024];

    // Add number of entries in heap, closures, symbols and program.
    db::write_uint64(data, heap_db().num_entries(t.get_root_id_heap()));
    blake2b_update(&s, data, sizeof(uint64_t));
    db::write_uint64(data, closure_db().num_entries(t.get_root_id_closure()));
    blake2b_update(&s, data, sizeof(uint64_t));
    db::write_uint64(data, symbols_db().num_entries(t.get_root_id_symbols()));
    blake2b_update(&s, data, sizeof(uint64_t));
    db::write_uint64(data, program_db().num_entries(t.get_root_id_program()));
    blake2b_update(&s, data, sizeof(uint64_t));

    // Add version
    db::write_uint64(data, tip_.get_version());
    blake2b_update(&s, data, sizeof(uint64_t));

    // Add height
    db::write_uint32(data, tip_.get_height());
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
    
    // The above hash can then be used as seed (and key) for
    // verifying the pow proof. Thus the pow proof itself is not part
    // of the hash.
    
    tip_.set_id(meta_id(data));
}

void blockchain::init()
{
    db_meta_ = nullptr;
    db_blocks_ = nullptr;
    db_heap_ = nullptr;
    db_closure_ = nullptr;
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
		    // Check if DB is really available (integrity check)
		    if (dbs_available(entry)) {
			best = entry;
		    }
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
    blocks_db().flush();
    heap_db().flush();
    closure_db().flush();
    symbols_db().flush();
    program_db().flush();
    meta_db().flush();
}

void blockchain::advance() {
    flush_db();

    db::root_id next_meta;
    db::root_id next_blocks;
    db::root_id next_heap;
    db::root_id next_closure;
    db::root_id next_symbols;
    db::root_id next_program;

    bool genesis = false;
    
    if (tip().get_root_id_meta().is_zero()) {
	next_meta = meta_db().new_root();
	next_blocks = blocks_db().new_root();
	next_heap = heap_db().new_root();
	next_closure = closure_db().new_root();
	next_symbols = symbols_db().new_root();
	next_program = program_db().new_root();
	genesis = true;
    } else {
	next_meta = meta_db().new_root(tip().get_root_id_meta());
	next_blocks = blocks_db().new_root(tip().get_root_id_blocks());
	next_heap = heap_db().new_root(tip().get_root_id_heap());
	next_closure = closure_db().new_root(tip().get_root_id_closure());
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
    new_tip.set_root_id_blocks(next_blocks);
    new_tip.set_root_id_heap(next_heap);
    new_tip.set_root_id_closure(next_closure);
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
    update_meta_entry(e);
}

void blockchain::update_meta_entry(const meta_entry &e)
{	
    const size_t data_size = meta_entry::serialization_size();
    uint8_t data[ data_size ];
    e.write(data);
    meta_db().update(e.get_root_id_meta(),
		     e.get_height(),
		     data, data_size);
    chains_[e.get_id()] = e;
    at_height_[e.get_height()].insert(e.get_id());
}

void blockchain::update_tip() {
    update_meta_id();

    auto find_it = chains_.find(tip().get_id());
    if (find_it != chains_.end()) {
	auto &existing_tip = find_it->second;
	if (tip_.get_id() == existing_tip.get_id()) {
	    existing_tip.set_root_id_heap(tip_.get_root_id_heap());
	    existing_tip.set_root_id_closure(tip_.get_root_id_closure());
	    existing_tip.set_root_id_symbols(tip_.get_root_id_symbols());
	    existing_tip.set_root_id_program(tip_.get_root_id_program());
	    existing_tip.set_root_id_blocks(tip_.get_root_id_blocks());
	}
	return;
    }

    add_meta_entry(tip());
}

size_t blockchain::depth(const meta_id &from_id, size_t max_depth)
{
    if (max_depth == 0) {
	return 0;
    }
    size_t d = 0;
    auto next_ids = follows(from_id);
    for (auto &next_id : next_ids) {
	auto next_depth = depth(next_id, max_depth - 1) + 1;
	if (next_depth > d) {
	    d = next_depth;
	}
    }
    return d;
}

// TODO: We should look at accumulated difficulty
meta_id blockchain::next(const meta_id &from_id, size_t max_lookahead) {
    if (max_lookahead == 0) {
	return from_id;
    }
    auto next_ids = follows(from_id);
    size_t best_depth = 0;
    meta_id best_id;
    for (auto &next_id : next_ids) {
	size_t d = depth(next_id, max_lookahead - 1);
	if (d > best_depth) {
	    best_depth = d;
	    best_id = next_id;
	}
    }
    return best_id;
}

meta_id blockchain::sync_point(int64_t height)
{
    if (at_height_.empty()) {
	return meta_id();
    }
    size_t abs_height = 0;
    if (height < 0) {
	size_t total_height = at_height_.rbegin()->first;
	if (static_cast<size_t>(-height) > total_height) {
	    abs_height = 0;
	} else {
	    abs_height = static_cast<size_t>(total_height - height);
	}
    } else {
	abs_height = static_cast<size_t>(height);
    }
    // If the chain at height abs_height is ambiguous, then go to
    // previous height until it isn't.
    while (abs_height > 0 && at_height_[abs_height].size() != 1) {
	abs_height--;
    }
    meta_id from_id = *(at_height_[abs_height].begin());
    // From this id we go forward and find the first branchpoint
    // that has two chains for which both are longer than 6.
    for (;;) {
	auto next_ids = follows(from_id);
	if (next_ids.size() == 0) {
	    break;
	}
	if (next_ids.size() == 1) {
	    from_id = *(next_ids.begin());
	    continue;
	}
	size_t found = 0;
	for (auto &next_id : next_ids) {
	    size_t d = depth(next_id, BRANCH_DEPTH);
	    if (d >= BRANCH_DEPTH) {
		found++;
	    }
	}
	if (found > 1) {
	    break;
	}
	from_id = next(from_id, BRANCH_DEPTH);
    }
    
    
    return from_id;
}
	
}}
