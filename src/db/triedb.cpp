#include <bitset>
#include "triedb.hpp"
#include "../common/checked_cast.hpp"
#include "../common/hex.hpp"
#include "../common/blake2.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

using namespace prologcoin::common;

namespace prologcoin { namespace db {

void triedb_root::read(const uint8_t *buffer) {
    const uint8_t *p = buffer;

    // Total size
    assert(((p - buffer) + sizeof(uint32_t)) < MAX_SIZE_IN_BYTES);
    size_t sz = read_uint32(p); p += sizeof(uint32_t);
    assert(sz == serialization_size());

    assert(((p - buffer) + sizeof(uint32_t)) < MAX_SIZE_IN_BYTES);
    height_ = read_uint32(p); p += sizeof(uint32_t);

    assert(((p - buffer) + sizeof(uint64_t)) < MAX_SIZE_IN_BYTES);
    num_entries_ = read_uint64(p); p += sizeof(uint64_t);

    assert(((p - buffer) + sizeof(uint64_t)) < MAX_SIZE_IN_BYTES);
    previous_id_ = root_id(read_uint64(p)); p += sizeof(uint64_t);

    assert(((p - buffer) + sizeof(uint64_t)) < MAX_SIZE_IN_BYTES);
    ptr_ = read_uint64(p); p += sizeof(uint64_t);
}

void triedb_root::write(uint8_t *buffer) const {
    uint8_t *p = buffer;
    size_t sz = serialization_size();
    assert(sz < MAX_SIZE_IN_BYTES);
    write_uint32(p, checked_cast<uint32_t>(sz)); p += sizeof(uint32_t);
    write_uint32(p, checked_cast<uint32_t>(height_)); p += sizeof(uint32_t);
    write_uint64(p, checked_cast<uint64_t>(num_entries_)); p += sizeof(uint64_t);
    write_uint64(p, previous_id_.value()); p += sizeof(uint64_t);
    write_uint64(p, ptr_); p += sizeof(uint64_t);
}

struct version_buffer {
    uint8_t buffer[triedb_params::VERSION_SZ];
};

struct meta_buffer {
    static const size_t META_SZ = sizeof(uint32_t);
    uint8_t buffer[META_SZ];
};
    
static void triedb_version_check(fstream *f, const std::string &file_path) {
    version_buffer buffer;
    memset(&buffer, 'x', sizeof(version_buffer));
    f->seekg(0, fstream::beg);
    f->read(reinterpret_cast<char *>(&buffer), triedb_params::VERSION_SZ);
    if (f->fail()) {
        try {
	    f->exceptions(f->failbit);
        } catch (const fstream::failure &err) {
  	    throw triedb_version_exception("Unable to read version information from " + file_path + "; " + err.what());
        }
    }
    if (memcmp(&buffer, triedb_params::VERSION, triedb_params::VERSION_SZ) != 0) {
        buffer.buffer[15] = '\0';
	std::string msg = "Unrecognized version while reading '";
	msg += file_path;
	msg += "'";
	msg += reinterpret_cast<char *>(buffer.buffer);
	msg += "; expected: '";
	msg += reinterpret_cast<const char *>(&triedb_params::VERSION[0]);
	msg += "'";
	throw triedb_version_exception(msg);
    }
}

//
// triedb_leaf
//

void triedb_leaf::read(const uint8_t *buffer) {
    const uint8_t *p = buffer;

    // First read total size
    assert(((p - buffer) + sizeof(uint32_t)) < MAX_SIZE_IN_BYTES);
    size_t sz = read_uint32(p); p += sizeof(uint32_t);

    // Read hash size (1 byte)
    assert(((p - buffer) + sizeof(uint8_t)) < MAX_SIZE_IN_BYTES);	
    hash_size_ = *p; p++;

    // Read key
    assert(((p - buffer) + sizeof(uint64_t)) < MAX_SIZE_IN_BYTES);	
    key_ = read_uint64(p); p += sizeof(uint64_t);

    // Write hash
    assert(((p - buffer) + hash_size_) < MAX_SIZE_IN_BYTES);
    if (hash_ != nullptr) delete [] hash_;
    hash_ = new uint8_t[hash_size_];
    memcpy(hash_, p, hash_size_); p += hash_size_;

    // Remaining is custom data
    custom_data_size_ = sz - (p - buffer);
    assert(((p - buffer) + custom_data_size_) < MAX_SIZE_IN_BYTES);

    if (custom_data_ != nullptr) delete [] custom_data_;
    custom_data_ = new uint8_t [custom_data_size_];
    memcpy(custom_data_, p, custom_data_size_);
}

void triedb_leaf::write(uint8_t *buffer) const {
    uint8_t *p = buffer;
    // First write total size
    size_t sz = serialization_size();
    assert(sz < MAX_SIZE_IN_BYTES);
    write_uint32(p, checked_cast<uint32_t>(sz));
    p += sizeof(uint32_t);

    // Write hash size
    *p = hash_size_; p++;

    // Write key
    write_uint64(p, key_); p += sizeof(uint64_t);

    // Write hash (if any)
    if (hash_size_ > 0) {
	memcpy(p, hash_, hash_size_);
	p += hash_size_;
    }

    // Write custom data (if any)
    if (custom_data_ != nullptr) {
        memcpy(p, custom_data_, custom_data_size_);
    }
}
    
//
// triedb_branch
//

void triedb_branch::read(const uint8_t *buffer)
{
    const uint8_t *p = buffer;

    // First read total size of this node

    size_t sz = read_uint32(p); assert(sz >= 4 && sz < MAX_SIZE_IN_BYTES);
    p += sizeof(uint32_t);
    assert(((p - buffer) + sizeof(uint32_t)) <= sz);

    // Read max key bits (1 byte)
    max_key_bits_ = *p; p++;
    // Read hash size (1 byte)
    assert(((p - buffer) + sizeof(uint32_t)) <= sz);
    hash_size_ = *p; p++;
    // Read mask
    assert(((p - buffer) + sizeof(uint32_t)) <= sz);
    mask_ = read_uint32(p); p += sizeof(uint32_t);
    // Read leaf bits
    assert(((p - buffer) + sizeof(uint32_t)) <= sz);
    leaf_ = read_uint32(p); p += sizeof(uint32_t);
    // Compute number of children (based on mask)
    size_t n = num_children();
    // Allocate space for children
    if (ptr_ != nullptr) delete [] ptr_;
    ptr_ = new uint64_t[n];
    // For each active child, read child pointer (64 bit)
    for (size_t i = 0; i < n; i++) {
        assert(((p - buffer) + sizeof(uint64_t)) <= sz);
        ptr_[i] = read_uint64(p); p += sizeof(uint64_t);
    }
    assert((p - buffer) <= sz);
    // Remaining data is hash
    assert(hash_size_ == sz - (p - buffer));
    if (hash_size_ > 0) {
	if (hash_ != nullptr) delete [] hash_;
        hash_ = new uint8_t[hash_size_];
        memcpy(hash_, p, hash_size_);
    }
}

void triedb_branch::write(uint8_t *buffer) const
{
    uint8_t *p = buffer;
    size_t sz = serialization_size();
    // First write total size of this node
    write_uint32(p, common::checked_cast<uint32_t>(sz)); p += sizeof(uint32_t);

    // Write max key bits (1 byte)
    *p = max_key_bits_; p++;
    // Write hash size (1 byte)
    *p = hash_size_; p++;
    // Write mask (32 bits)
    write_uint32(p, mask_); p += sizeof(uint32_t);
    // Write leaf bits (32 bits)
    write_uint32(p, leaf_); p += sizeof(uint32_t);
    // Compute number of children (based on mask)
    size_t n = num_children();
    // Write each child pointer (64 bits)
    for (size_t i = 0; i < n; i++) {
        write_uint64(p, ptr_[i]); p += sizeof(uint64_t);
    }
    // Write hash (if any)
    if (hash_size_) {
        memcpy(p, hash_, hash_size_);
    }
}

//
// triedb
//

void triedb::branch_hasher(triedb_branch *branch) {
    if (!use_hashing()) {
	branch->set_hash(nullptr, 0);
	return;
    }
    uint8_t final_hash[32];

    blake2b_state s;
    blake2b_init(&s, sizeof(final_hash));
    for (size_t i = 0; i < triedb_params::MAX_BRANCH; i++) {
	if (branch->is_branch(i)) {
	    auto *sub_branch = get_branch(branch, i);
	    blake2b_update(&s, sub_branch->hash(), sub_branch->hash_size());
	} else if (branch->is_leaf(i)) {
	    auto *sub_leaf = get_leaf(branch, i);
	    blake2b_update(&s, sub_leaf->hash(), sub_leaf->hash_size());
	}
    }
    blake2b_final(&s, &final_hash[0], sizeof(final_hash));
    branch->set_hash(final_hash, sizeof(final_hash));
}

void triedb::leaf_hasher(triedb_leaf *leaf) {
    if (!use_hashing()) {
	leaf->set_hash(nullptr, 0);
	return;
    }

    uint8_t final_hash[32];

    blake2b_state s;
    blake2b_init(&s, sizeof(final_hash));

    uint8_t key_serialized[sizeof(uint64_t)];
    write_uint64(key_serialized, leaf->key());

    blake2b_update(&s, key_serialized, sizeof(uint64_t));

    if (leaf->custom_data_size() > 0) {
	blake2b_update(&s, leaf->custom_data(), leaf->custom_data_size());
    }
    blake2b_final(&s, &final_hash[0], sizeof(final_hash));
    leaf->set_hash(final_hash, sizeof(final_hash));
}
    
triedb::triedb(const std::string &dir_path) : triedb(triedb_params(), dir_path) {
}
    
triedb::triedb(const triedb_params &params, const std::string &dir_path)
  : triedb_params(params),
    dir_path_(dir_path),
    stream_flusher_(),
    stream_cache_(triedb_params::cache_num_streams(), stream_flusher_),
    leaf_flusher_(),
    leaf_cache_(triedb_params::cache_num_nodes(), leaf_flusher_),
    branch_flusher_(),
    branch_cache_(triedb_params::cache_num_nodes(), branch_flusher_),
    roots_stream_(nullptr),
    cache_shutdown_(false)
{
    read_roots();
    last_offset_ = scan_last_offset();
}

triedb::~triedb()
{
    cache_shutdown_ = true;
    leaf_cache_.clear();
    branch_cache_.clear();
    flush();
    if (roots_stream_) delete roots_stream_;
}

void triedb::erase_all()
{
    branch_cache_.clear();
    leaf_cache_.clear();
    stream_cache_.clear();
    roots_stream_->close();
    delete roots_stream_;
    roots_stream_ = nullptr;
    boost::system::error_code ec;
    boost::filesystem::remove_all(dir_path_, ec);
    if (ec) {
      throw triedb_write_exception( "Failed while attempting to erase all; " + ec.message());
    }
    last_offset_ = 0;
    roots_.clear();
}

void triedb::erase_all(const std::string &dir_path)
{
    boost::system::error_code ec;
    boost::filesystem::remove_all(dir_path, ec);
    if (ec) {
      throw triedb_write_exception( "Failed while attempting to erase all; " + ec.message());
    }
}

void triedb::flush()
{
    if (roots_stream_) roots_stream_->flush();

    stream_cache_.foreach( [](size_t, fstream *f) { f->flush(); } );
}

root_id triedb::new_root() {
    auto *new_branch = new triedb_branch();
    new_branch->set_max_key_bits(5);
    branch_hasher(new_branch);
    auto ptr = append_branch_node(new_branch);

    triedb_root root;
    root.set_height(0);
    auto s = get_roots_stream();
    size_t id = s->tellg();
    root_id rid = root_id(id);
    root.set_id(rid);
    root.set_ptr(ptr);
    uint8_t buffer[triedb_root::MAX_SIZE_IN_BYTES];
    root.write(buffer);
    size_t n = root.serialization_size();
    s->seekg(0, fstream::end);
    s->write(reinterpret_cast<char *>(buffer), n);
    roots_[rid] = root;
    roots_at_height_[0].insert(rid);

    return rid;
}

root_id triedb::new_root(const root_id &parent_id) {
    const triedb_root &parent = get_root(parent_id);
    triedb_root root;
    size_t height = parent.height()+1;
    root.set_height(height);
    auto s = get_roots_stream();
    s->seekg(0, fstream::end);
    auto rid = root_id(s->tellg());
    root.set_id(rid);
    root.set_previous_id(parent.id());
    root.set_ptr(parent.ptr());
    root.set_num_entries(parent.num_entries());
    uint8_t buffer[triedb_root::MAX_SIZE_IN_BYTES];
    root.write(buffer);
    size_t n = root.serialization_size();
    s->write(reinterpret_cast<char *>(buffer), n);
    roots_[rid] = root;
    roots_at_height_[height].insert(rid);
    return rid;
}

const std::set<root_id> & triedb::find_roots(size_t height) const {
    static std::set<root_id> empty_set;

    auto it = roots_at_height_.find(height);
    if (it == roots_at_height_.end()) {
	return empty_set;
    }
    return it->second;
}

root_id triedb::find_root(size_t height) const {
    auto roots = find_roots(height);
    assert(roots.size() == 1);
    return *roots.begin();
}

const triedb_root & triedb::get_root(const root_id &id) const {
    auto it = roots_.find(id);
    assert(it != roots_.end());
    return it->second;
}

void triedb::insert(const root_id &at_root, uint64_t key, const uint8_t *data, size_t data_size) {
    return insert_or_update(at_root, key, data, data_size, true);
}

void triedb::update(const root_id &at_root, uint64_t key, const uint8_t *data, size_t data_size) {
    return insert_or_update(at_root, key, data, data_size, false);
}

void triedb::insert_or_update(const root_id &at_root, uint64_t key, const uint8_t *data, size_t data_size, bool do_insert)
{
    auto some_root = roots_.find(at_root);
    if (some_root == roots_.end()) {
        // Grow with at most one height at a time
	if (roots_.empty() == 0) {
	     auto *new_branch = new triedb_branch();
	     new_branch->set_max_key_bits(5);
	     branch_hasher(new_branch);
	     auto ptr = append_branch_node(new_branch);
	     set_root(at_root, ptr);
	     some_root = roots_.find(at_root);
	}
    }
    uint64_t current_root_ptr = some_root->second.ptr();
    triedb_branch *current_root = get_branch(current_root_ptr);
    triedb_branch *new_branch = nullptr;
    uint64_t new_branch_ptr = 0;
    size_t current_key_bits = current_root->max_key_bits();
    size_t key_bits = triedb_branch::compute_max_key_bits(key);
    while (key_bits > current_key_bits) {
        // Increase height of tree until current_key_bits == key_bits
	current_key_bits += triedb_params::MAX_BRANCH_BITS;
        auto *new_root = new triedb_branch();
	new_root->set_max_key_bits(current_key_bits);
	new_root->set_child_pointer(0, current_root_ptr);
        auto new_root_ptr = append_branch_node(new_root);
	current_root = new_root;
	current_root_ptr = new_root_ptr;
    }

    bool new_entry = false;
    std::tie(new_branch, new_branch_ptr)
       = update_part(current_root, key, data, data_size, do_insert, new_entry);
    if (new_entry) increment_num_entries(at_root);
    set_root(at_root, new_branch_ptr);
}
    
std::pair<triedb_branch *, uint64_t> triedb::update_part(triedb_branch *node,
							 uint64_t key,
							 const uint8_t *data,
							 size_t data_size,
							 bool do_insert,
							 bool &new_entry)
{
    size_t max_key_bits = node->max_key_bits();

    size_t sub_index = (key >> (max_key_bits - MAX_BRANCH_BITS))
                        & (MAX_BRANCH-1);

    // If the child is empty, then create a new node at that position
    if (node->is_empty(sub_index)) {
        auto *new_leaf = new triedb_leaf(key, data, data_size);
	leaf_hasher_fn_(*this, new_leaf);
	auto leaf_ptr = append_leaf_node(*new_leaf);
	leaf_cache_.insert(leaf_ptr, new_leaf);
	auto *new_branch = new triedb_branch(*node);
	new_branch->set_child_pointer(sub_index, leaf_ptr);
	new_branch->set_leaf(sub_index);
	branch_hasher(new_branch);
	auto ptr = append_branch_node(new_branch);
	new_entry = true;
	return std::make_pair(new_branch, ptr);
    } else if (node->is_leaf(sub_index)) {
	// If the child already has a child with the same key, then
	// substitute with the new data.
        auto *leaf = get_leaf(node, sub_index);
	if (leaf->key() == key) {
	    if (do_insert) {
	        throw triedb_key_already_exists_exception(
		    "There's already a key '"
		    + boost::lexical_cast<std::string>(key)
		    + "' in the database.");
	    }
	    auto *new_leaf = new triedb_leaf(key, data, data_size);
	    leaf_hasher_fn_(*this, new_leaf);
	    auto leaf_ptr = append_leaf_node(*new_leaf);
	    leaf_cache_.insert(leaf_ptr, new_leaf);
	    auto *new_branch = new triedb_branch(*node);
	    new_branch->set_child_pointer(sub_index, leaf_ptr);
	    new_branch->set_leaf(sub_index);	    
	    branch_hasher(new_branch);
	    auto ptr = append_branch_node(new_branch);
	    return std::make_pair(new_branch, ptr);
	} else {
	    // We need to create a branch node at sub_index, reorg the current
	    // leaf at that position by pushing it down one level. However,
	    // this new node will be directly recursed into and thus will
	    // not needed to be stored on disk.
  	    size_t sub_key_bits = max_key_bits - MAX_BRANCH_BITS;
  	    size_t sub_sub_index = (leaf->key() >> (sub_key_bits - MAX_BRANCH_BITS)) & (MAX_BRANCH-1);
	    triedb_branch tmp_branch;
	    tmp_branch.set_max_key_bits(sub_key_bits);
            tmp_branch.set_child_pointer(sub_sub_index, node->get_child_pointer(sub_index));
	    tmp_branch.set_leaf(sub_sub_index);
	    triedb_branch *new_child = nullptr;
            uint64_t new_child_ptr = 0;
            std::tie(new_child, new_child_ptr) = update_part(&tmp_branch, key, data, data_size, do_insert, new_entry);
            auto *new_branch = new triedb_branch(*node);
            new_branch->set_child_pointer(sub_index, new_child_ptr);
            new_branch->set_branch(sub_index);
	    branch_hasher(new_branch);
            auto ptr = append_branch_node(new_branch);
            return std::make_pair(new_branch, ptr);
	}
    }

    auto *child = get_branch(node, sub_index);
    triedb_branch *new_child = nullptr;
    uint64_t new_child_ptr = 0;
    std::tie(new_child, new_child_ptr) = update_part(child, key, data, data_size, do_insert, new_entry);
    auto *new_branch = new triedb_branch(*node);
    new_branch->set_child_pointer(sub_index, new_child_ptr);
    branch_hasher(new_branch);
    auto new_branch_ptr = append_branch_node(new_branch);
    return std::make_pair(new_branch, new_branch_ptr);
}

void triedb::remove(const root_id &at_root, uint64_t key) {
    auto found_root = roots_.find(at_root);
    if (found_root == roots_.end()) {
	std::stringstream msg;
	msg << "Key '" << key << "' not found at root " << at_root.value();
	throw triedb_key_not_found_exception(msg.str());
    }
    uint64_t current_root_ptr = found_root->second.ptr();
    triedb_branch *current_root = get_branch(current_root_ptr);
    triedb_branch *new_branch = nullptr;
    uint64_t new_branch_ptr = 0;
    size_t current_key_bits = current_root->max_key_bits();
    size_t key_bits = triedb_branch::compute_max_key_bits(key);
    if (key_bits > current_key_bits) {
        std::stringstream msg;
	msg << "Key '" << key << "' not found at root " << at_root.value();
        throw triedb_key_not_found_exception(msg.str());
    }
      
    std::tie(new_branch, new_branch_ptr) = remove_part(at_root, current_root, key);
    decrement_num_entries(at_root);
    set_root(at_root, new_branch_ptr);
}

std::pair<triedb_branch *, uint64_t> triedb::remove_part(const root_id &at_root,
							 triedb_branch *node,
							 uint64_t key)
{
    size_t max_key_bits = node->max_key_bits();

    size_t sub_index = (key >> (max_key_bits - MAX_BRANCH_BITS))
                        & (MAX_BRANCH-1);

    // If the child is empty, then create a new node at that position
    if (node->is_empty(sub_index)) {
        std::stringstream msg;
	msg << "Key '" << key << "' not found at root " << at_root.value();
        throw triedb_key_not_found_exception(msg.str());
    }
    if (node->is_leaf(sub_index)) {
	// Remove key
        auto *leaf = get_leaf(node, sub_index);
	if (leaf->key() != key) {
	    std::stringstream msg;
	    msg << "Key '" << key << "' not found at root " << at_root.value();
	    throw triedb_key_already_exists_exception(msg.str());
	}
	auto *new_branch = new triedb_branch(*node);
	new_branch->set_empty(sub_index);
	branch_hasher(new_branch);
	auto ptr = append_branch_node(new_branch);
	return std::make_pair(new_branch, ptr);
    }

    auto *child = get_branch(node, sub_index);
    triedb_branch *new_child = nullptr;
    uint64_t new_child_ptr = 0;
    std::tie(new_child, new_child_ptr) = remove_part(at_root, child, key);
    // Is the child completely empty?
    auto *new_branch = new triedb_branch(*node);
    if (new_child->mask() == 0) {
        new_branch->set_empty(sub_index);
    } else {
        new_branch->set_child_pointer(sub_index, new_child_ptr);
    }
    branch_hasher(new_branch);
    auto new_branch_ptr = append_branch_node(new_branch);
    return std::make_pair(new_branch, new_branch_ptr);
}

boost::filesystem::path triedb::roots_file_path() const {
    auto file_path = boost::filesystem::path(dir_path_) / "roots.bin";
    return file_path;
}

fstream * triedb::get_roots_stream() {
    if (roots_stream_) {
        return roots_stream_;
    }
    auto file_path = roots_file_path();
    bool exists = boost::filesystem::exists(file_path);
    uint8_t bucket_size_buffer[sizeof(uint32_t)];
    if (!exists) {
        auto parent_dir = file_path.parent_path();
	boost::filesystem::create_directories(parent_dir);
	fstream fs;
	fs.open(file_path.string(), fstream::out | fstream::app | fstream::binary);
	fs.write(triedb_params::VERSION, triedb_params::VERSION_SZ);
	write_uint32(&bucket_size_buffer[0],
		     common::checked_cast<uint32_t>(bucket_size()));
	fs.write(reinterpret_cast<char *>(&bucket_size_buffer[0]),
		 sizeof(bucket_size_buffer));
	fs.close();
    }
    roots_stream_ = new fstream();
    roots_stream_->open(file_path.string(), fstream::in | fstream::out | fstream::binary);
    triedb_version_check(roots_stream_, file_path.string());
    roots_stream_->read(reinterpret_cast<char *>(&bucket_size_buffer[0]),
			sizeof(uint32_t));
    size_t bucket_size = read_uint32(bucket_size_buffer);
    set_bucket_size(bucket_size);
    return roots_stream_;
}

void triedb::read_roots() {
    fstream *f = get_roots_stream();
    f->seekg(0, fstream::end);
    size_t file_size = f->tellg();
    size_t overhead_size = VERSION_SZ + sizeof(uint32_t);
    assert(file_size >= overhead_size);

    f->seekg(overhead_size, fstream::beg);
    uint8_t buffer[triedb_root::MAX_SIZE_IN_BYTES];
    triedb_root root;

    size_t file_offset = f->tellg();

    while (file_offset < file_size) {
        f->read(reinterpret_cast<char *>(&buffer[0]), sizeof(uint32_t));
	size_t n = read_uint32(buffer);

	assert(n == triedb_root::serialization_size());
	f->read(reinterpret_cast<char *>(&buffer[sizeof(uint32_t)]),
		n-sizeof(uint32_t));
	root.read(buffer);
	root.set_id(root_id(file_offset));
	roots_.insert(std::make_pair(root.id(), root));
	roots_at_height_[root.height()].insert(root.id());

	file_offset = f->tellg();
    }
}

void triedb::set_root(const root_id &id, uint64_t offset)
{
    auto root = get_root(id);
    root.set_ptr(offset);
    size_t file_offset = root.id().value();
    assert(file_offset >= VERSION_SZ);
    auto *f = get_roots_stream();
    f->seekg(file_offset, fstream::beg);
    uint8_t buffer[triedb_root::MAX_SIZE_IN_BYTES];
    root.write(buffer);
    f->write(reinterpret_cast<char *>(&buffer[0]), root.serialization_size());
    roots_[id] = root;
}

boost::filesystem::path triedb::bucket_dir_location(size_t bucket_index) const {
    size_t start_bucket = (bucket_index / 64) * 64;
    size_t end_bucket = (bucket_index / 64) * 64 + 63;
    std::stringstream ss;
    ss << "buckets_" << start_bucket << "_" << end_bucket;
    std::string name = ss.str();
    auto r = boost::filesystem::path(dir_path_);
    return r / name;
}

size_t triedb::scan_last_bucket() {
    size_t i;
    // First find right bucket directory
    for (i = 0;;i += 64) {
        auto dir = bucket_dir_location(i);
	if (!boost::filesystem::exists(dir)) {
	    break;
	}
    }
    if (i > 0) {
        i -= 64; // Go to previous existing bucket directory
    }

    bool something_found = false;
    for (;;i++) {
        auto file_path = bucket_file_path(i);
	if (!boost::filesystem::exists(file_path)) {
	    break;
	}
	something_found = true;
    }
    if (!something_found) {
        // Nothing found!
        return static_cast<size_t>(-1);
    }
    // Return previous bucket    
    return i - 1;
}

uint64_t triedb::scan_last_offset() {
    size_t bucket_index = scan_last_bucket();
    if (bucket_index == static_cast<size_t>(-1)) {
         return 0;
    }
    auto *f = get_bucket_stream(bucket_index);
    f->seekg(0, fstream::end);
    uint64_t last_offset = f->tellg();
    last_offset -= VERSION_SZ;
    last_offset += bucket_index * bucket_size();
    return last_offset;
}
    
boost::filesystem::path triedb::bucket_file_path(size_t bucket_index) const {
    auto dir = bucket_dir_location(bucket_index);
    std::string name = "bucket_" + boost::lexical_cast<std::string>(bucket_index) + ".data.bin";
    return dir / name;
}

fstream * triedb::get_bucket_stream(size_t bucket_index) {
    auto *f = stream_cache_.find(bucket_index);
    if (f != nullptr) {
        return *f;
    }
    auto file_path = bucket_file_path(bucket_index);
    bool exists = boost::filesystem::exists(file_path);
    if (!exists) {
        auto parent_dir = file_path.parent_path();
	boost::filesystem::create_directories(parent_dir);
	fstream fs;
	fs.open(file_path.string(), fstream::out | fstream::app | fstream::binary);
	fs.write(triedb_params::VERSION, triedb_params::VERSION_SZ);
	fs.close();
    }
    auto *ff = new fstream;
    ff->open(file_path.string(), fstream::in | fstream::out | fstream::binary);
    triedb_version_check(ff, file_path.string());
    ff->seekg(0, fstream::end);
    stream_cache_.insert(bucket_index, ff);
    return ff;
}

fstream * triedb::set_file_offset(uint64_t offset)
{
    size_t bucket_index = offset / bucket_size();
    auto *f = get_bucket_stream(bucket_index);
    size_t first_offset = bucket_index * bucket_size();
    size_t file_offset = offset - first_offset + VERSION_SZ;
    f->seekg(file_offset, fstream::beg);
    return f;
}

const triedb_root & triedb::get_root(const root_id &id)
{
    auto found = roots_.find(id);
    assert(found != roots_.end());
    return found->second;
}
    
void triedb::read_leaf_node(uint64_t offset, triedb_leaf &node)
{
    uint8_t buffer[triedb_leaf::MAX_SIZE_IN_BYTES];  
    auto *f = set_file_offset(offset);
    f->read(reinterpret_cast<char *>(&buffer[0]), sizeof(uint32_t));
    uint32_t size = read_uint32(buffer);
    assert(size >= 4 && size < triedb_leaf::MAX_SIZE_IN_BYTES);
    f->read(reinterpret_cast<char *>(&buffer[sizeof(uint32_t)]),
	    size-sizeof(uint32_t));
    node.read(buffer);
}

uint64_t triedb::append_leaf_node(const triedb_leaf &node)
{
    auto offset = last_offset_;
    size_t num_bytes = node.serialization_size();
    size_t bucket_index = offset / bucket_size();
    auto first_offset = bucket_index * bucket_size();
    // Are we crossing the bucket boundary? 
    if (offset - first_offset + num_bytes >= bucket_size()) {
        // Then switch to the next bucket
        bucket_index++;
        offset = bucket_index*bucket_size();
	last_offset_ = offset;
    }
    uint8_t buffer[triedb_leaf::MAX_SIZE_IN_BYTES];
    size_t n = node.serialization_size();
    node.write(buffer);
    auto *f = set_file_offset(offset);
    f->write(reinterpret_cast<char *>(&buffer[0]), n);
    last_offset_ += n;
    return offset;
}
    
void triedb::read_branch_node(uint64_t offset, triedb_branch &node)
{
    uint8_t buffer[triedb_branch::MAX_SIZE_IN_BYTES];
    auto *f = set_file_offset(offset);
    f->read(reinterpret_cast<char *>(&buffer[0]), sizeof(uint32_t));
    uint32_t size = read_uint32(buffer);
    assert(size >= 4 && size < triedb_branch::MAX_SIZE_IN_BYTES);
    f->read(reinterpret_cast<char *>(&buffer[sizeof(uint32_t)]),
	    size-sizeof(uint32_t));
    node.read(buffer);
}

uint64_t triedb::append_branch_node(triedb_branch *node)
{
    auto offset = last_offset_;
    size_t num_bytes = node->serialization_size();
    size_t bucket_index = offset / bucket_size();
    auto first_offset = bucket_index * bucket_size();
    // Are we crossing the bucket boundary? 
    if (offset - first_offset + num_bytes >= bucket_size()) {
        // Then switch to the next bucket
        bucket_index++;
        offset = bucket_index*bucket_size();
	last_offset_ = offset;	
    }
    uint8_t buffer[triedb_branch::MAX_SIZE_IN_BYTES];
    size_t n = node->serialization_size();
    node->write(buffer);
    auto *f = set_file_offset(offset);
    f->write(reinterpret_cast<char *>(&buffer[0]), n);
    last_offset_ += n;
    if (!cache_shutdown_) branch_cache_.insert(offset, node);
    return offset;
}

void triedb_iterator::leftmost() {
    if (spine_.empty()) {
	return;
    }
    auto parent_ptr = spine_.back().parent_ptr;
    auto *parent = db_.get_branch(parent_ptr);
    while (!spine_.empty() && parent->mask() == 0) {
	spine_.pop_back();
	if (!spine_.empty()) {
	    parent_ptr = spine_.back().parent_ptr;
	    parent = db_.get_branch(parent_ptr);
	}
    }
    if (spine_.empty()) {
	return;
    }
    auto sub_index = spine_.back().sub_index;
    bool found = true;
    while (parent->is_branch(sub_index)) {
	parent_ptr = parent->get_child_pointer(sub_index);
	parent = db_.get_branch(parent_ptr);
	if (parent->mask() == 0) {
	    found = false;
	    break;
	}
	sub_index = common::lsb(parent->mask());
	spine_.push_back(cursor(parent, parent_ptr, sub_index));
    }
    if (!found) {
        next();
    }
}

void triedb_iterator::rightmost() {
    if (spine_.empty()) {
	return;
    }
    auto parent_ptr = spine_.back().parent_ptr;
    auto *parent = db_.get_branch(parent_ptr);
    while (!spine_.empty() && parent->mask() == 0) {
	spine_.pop_back();
	if (!spine_.empty()) {
	    parent_ptr = spine_.back().parent_ptr;
	    parent = db_.get_branch(parent_ptr);
	}
    }
    if (spine_.empty()) {
	return;
    }
    auto sub_index = spine_.back().sub_index;
    bool found = true;
    while (parent->is_branch(sub_index)) {
	parent_ptr = parent->get_child_pointer(sub_index);
	parent = db_.get_branch(parent_ptr);
	if (parent->mask() == 0) {
	    found = false;
	    break;
	}
	sub_index = common::msb(parent->mask());
	spine_.push_back(cursor(parent, parent_ptr, sub_index));
    }
    if (!found) {
        previous();
    }
}

void triedb_iterator::start_from_key(uint64_t parent_ptr, uint64_t key) {
    auto *parent = db_.get_branch(parent_ptr);
    size_t sub_index = get_sub_index(parent, key);
    auto m = parent->mask();
    // Is the key bigger than what is represented by the root node,
    // then it's not present.
    if (common::msb(key) >= parent->max_key_bits()) {
        return;
    }
    if (parent->is_empty(sub_index)) {
	m &= (static_cast<uint32_t>(-1) << sub_index) << 1;
	sub_index = (m == 0) ? triedb_params::MAX_BRANCH : common::lsb(m);
	if (sub_index == triedb_params::MAX_BRANCH) {
	    return;
	}
	spine_.push_back(cursor(parent, parent_ptr, sub_index));
	leftmost();
	return;
    }
    
    while (!parent->is_empty(sub_index) && parent->is_branch(sub_index)) {
        spine_.push_back(cursor(parent, parent_ptr, sub_index));
	parent_ptr = parent->get_child_pointer(sub_index);
	parent = db_.get_branch(parent_ptr);
	sub_index = get_sub_index(parent, key);
    }
    
    if (parent->is_empty(sub_index)) {
	m = parent->mask();
	m &= (static_cast<uint32_t>(-1) << sub_index) << 1;
	sub_index = (m == 0) ? triedb_params::MAX_BRANCH : common::lsb(m);
	if (sub_index == triedb_params::MAX_BRANCH) {
	    sub_index = triedb_params::MAX_BRANCH - 1;
	    spine_.push_back(cursor(parent, parent_ptr, sub_index));
	    next();
	    return;
	}
	spine_.push_back(cursor(parent, parent_ptr, sub_index));
	leftmost();
	return;
    }
    
    spine_.push_back(cursor(parent, parent_ptr, sub_index));
    
    bool found = false;
    while (!spine_.empty() && !found) {
	auto parent_ptr = spine_.back().parent_ptr;
	auto sub_index = spine_.back().sub_index;
	auto parent = db_.get_branch(parent_ptr);
	auto *leaf = db_.get_leaf(parent, sub_index);
	found = leaf->key() >= key;
	if (!found) {
	        next();
	}
    }
}

void triedb_iterator::next() {
    auto parent_ptr = spine_.back().parent_ptr;
    auto sub_index = spine_.back().sub_index;
    auto *parent = db_.get_branch(parent_ptr);
    auto mask_next = parent->mask() & ((static_cast<uint32_t>(-1) << sub_index) << 1);
    while (mask_next == 0) {
	spine_.pop_back();
	if (spine_.empty()) {
	    return;
	}
	parent_ptr = spine_.back().parent_ptr;
	sub_index = spine_.back().sub_index;
	parent = db_.get_branch(parent_ptr);
	mask_next = parent->mask() & ((static_cast<uint32_t>(-1) << sub_index) << 1);
    }
    spine_.back().sub_index = common::lsb(mask_next);
    leftmost();
}

void triedb_iterator::previous() {
    if (at_end()) {
	if (db_.is_empty()) {
	    return;
	}
        auto parent_ptr = db_.get_root(root_).ptr();
	auto parent = db_.get_branch(parent_ptr);
	if (parent->mask() == 0) {
	    return;
	}
	spine_.push_back(cursor(parent, parent_ptr,
				common::msb(parent->mask())));
	rightmost();
	return;
    }
    auto parent_ptr = spine_.back().parent_ptr;
    auto sub_index = spine_.back().sub_index;
    auto parent = db_.get_branch(parent_ptr);
    auto mask_prev = parent->mask() &
	((static_cast<uint32_t>(-1) >> (31-sub_index)) >> 1);
    
    while (mask_prev == 0) {
	spine_.pop_back();
	if (spine_.empty()) {
	    return;
	}
	parent_ptr = spine_.back().parent_ptr;
	sub_index = spine_.back().sub_index;
	parent = db_.get_branch(parent_ptr);
	mask_prev = parent->mask() & 
	    ((static_cast<uint32_t>(-1) >> (31-sub_index)) >> 1);
    }

    spine_.back().sub_index = common::msb(mask_prev);
    rightmost();
}

}}
