#include <bitset>
#include "triedb.hpp"
#include "../common/checked_cast.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace prologcoin { namespace db {

struct version_buffer {
    uint8_t buffer[triedb_params::VERSION_SZ];
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
    assert(((p - buffer) + sizeof(uint32_t)) < MAX_SIZE_IN_BYTES);
    size_t sz = read_uint32(p); p += sizeof(uint32_t);
    assert(((p - buffer) + sizeof(uint64_t)) < MAX_SIZE_IN_BYTES);	
    key_ = read_uint64(p); p += sizeof(uint64_t);
    assert(((p - buffer) + sizeof(uint32_t)) < MAX_SIZE_IN_BYTES);		
    custom_data_size_ = sz - (p - buffer);
    assert(((p - buffer) + custom_data_size_) < MAX_SIZE_IN_BYTES);
    if (custom_data_ != nullptr) delete [] custom_data_;
    custom_data_ = new uint8_t [custom_data_size_];
    memcpy(custom_data_, p, custom_data_size_);
}

void triedb_leaf::write(uint8_t *buffer) const {
    uint8_t *p = buffer;
    size_t sz = serialization_size();
    assert(sz < MAX_SIZE_IN_BYTES);
    write_uint32(p, common::checked_cast<uint32_t>(sz));
    p += sizeof(uint32_t);
    write_uint64(p, key_); p += sizeof(uint64_t);
    write_uint32(p, custom_data_size_); p += sizeof(uint32_t);
    if (custom_data_ != nullptr) {
        memcpy(p, custom_data_, custom_data_size_);
    }
}
    
//
// triedb_branch
//

size_t triedb_branch::serialization_size() const {
    size_t sz = sizeof(uint32_t) +  // total size
                sizeof(uint32_t) +  // mask
                sizeof(uint32_t) +  // leaf
                sizeof(uint64_t)*num_children() + // ptrs
                custom_data_size_;
    assert(sz < MAX_SIZE_IN_BYTES);
    return sz;
}
    
void triedb_branch::read(const uint8_t *buffer)
{
    const uint8_t *p = buffer;
    size_t sz = read_uint32(p); assert(sz >= 4 && sz < MAX_SIZE_IN_BYTES);
    p += sizeof(uint32_t);
    assert(((p - buffer) + sizeof(uint32_t)) <= sz);
    mask_ = read_uint32(p); p += sizeof(uint32_t);
    assert(((p - buffer) + sizeof(uint32_t)) <= sz);
    leaf_ = read_uint32(p); p += sizeof(uint32_t);
    size_t n = num_children();
    if (ptr_ != nullptr) delete [] ptr_;
    ptr_ = new uint64_t[n];
    for (size_t i = 0; i < n; i++) {
        assert(((p - buffer) + sizeof(uint64_t)) <= sz);
        ptr_[i] = read_uint64(p); p += sizeof(uint64_t);
    }
    assert((p - buffer) <= sz);
    custom_data_size_ = sz - (p - buffer);
    if (custom_data_ != nullptr) delete [] custom_data_;
    custom_data_ = nullptr;
    if (custom_data_size_ > 0) {
        custom_data_ = new uint8_t[custom_data_size_];
        memcpy(custom_data_, p, custom_data_size_);
    }
}

void triedb_branch::write(uint8_t *buffer) const
{
    uint8_t *p = buffer;
    size_t sz = serialization_size();
    write_uint32(p, common::checked_cast<uint32_t>(sz)); p += sizeof(uint32_t);
    write_uint32(p, mask_); p += sizeof(uint32_t);
    write_uint32(p, leaf_); p += sizeof(uint32_t);
    size_t n = num_children();
    for (size_t i = 0; i < n; i++) {
        write_uint64(p, ptr_[i]); p += sizeof(uint64_t);
    }
    if (custom_data_ != nullptr) {
        memcpy(p, custom_data_, custom_data_size_);
    }
}

//
// triedb
//
    
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
    branch_update_fn_(nullptr) {
    last_offset_ = scan_last_offset();

    read_roots();
}

triedb::~triedb()
{
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

void triedb::flush()
{
    if (roots_stream_) roots_stream_->flush();

    stream_cache_.foreach( [](size_t, fstream *f) { f->flush(); } );
}

void triedb::insert(size_t at_height, uint64_t key, const uint8_t *data, size_t data_size)
{
    if (at_height >= roots_.size()) {
        // Grow with at most one height at a time
        assert(at_height == roots_.size() || at_height == roots_.size()+1);
	if (roots_.size() == 0) {
	     auto *new_branch = new triedb_branch();
	     if (branch_update_fn_) branch_update_fn_(*this, *new_branch);
	     auto ptr = append_branch_node(*new_branch);
	     branch_cache_.insert(ptr, new_branch);
	     set_root(at_height, ptr);
	}
    }
    triedb_branch *prev_root = nullptr;
    if (at_height >= roots_.size()) {
        prev_root = get_branch(roots_[at_height-1]);
    } else {
        prev_root = get_branch(roots_[at_height]);
    }
    triedb_branch *new_branch = nullptr;
    uint64_t new_branch_ptr = 0;
    std::tie(new_branch, new_branch_ptr) =
      insert_part(prev_root, key, data, data_size, 0);
    set_root(at_height, new_branch_ptr);
}
    
std::pair<triedb_branch *, uint64_t> triedb::insert_part(triedb_branch *node, uint64_t key, const uint8_t *data, size_t data_size, size_t part)
{
    size_t sub_index = (key >> (MAX_KEY_SIZE_BITS - MAX_BRANCH_BITS - part)) & (MAX_BRANCH-1);

    // If the child is empty, then create a new node at that position
    if (node->is_empty(sub_index)) {
        auto *new_leaf = new triedb_leaf(key, data, data_size);
	auto leaf_ptr = append_leaf_node(*new_leaf);
	leaf_cache_.insert(leaf_ptr, new_leaf);
	auto *new_branch = new triedb_branch(*node);
	new_branch->set_child_pointer(sub_index, leaf_ptr);
	new_branch->set_leaf(sub_index);
	auto ptr = append_branch_node(*new_branch);
	if (branch_update_fn_) branch_update_fn_(*this, *new_branch);
	return std::make_pair(new_branch, ptr);
    } else if (node->is_leaf(sub_index)) {
	// If the child already has a child with the same key, then
	// substitute with the new data.
        auto *leaf = get_leaf(node, sub_index);
	if (leaf->key() == key) {
	    auto *new_leaf = new triedb_leaf(key, data, data_size);
	    auto leaf_ptr = append_leaf_node(*new_leaf);
	    leaf_cache_.insert(leaf_ptr, new_leaf);
	    auto *new_branch = new triedb_branch(*node);
	    new_branch->set_child_pointer(sub_index, leaf_ptr);
	    new_branch->set_leaf(sub_index);	    
	    if (branch_update_fn_) branch_update_fn_(*this, *new_branch);
	    auto ptr = append_branch_node(*new_branch);
	    return std::make_pair(new_branch, ptr);
	} else {
	    // We need to create a branch node at sub_index, reorg the current
	    // leaf at that position by pushing it down one level. However,
	    // this new node will be directly recursed into and thus will
	    // not needed to be stored on disk.
	    size_t sub_sub_index = (leaf->key() >> (MAX_KEY_SIZE_BITS - 2*MAX_BRANCH_BITS - part)) & (MAX_BRANCH-1);
	    triedb_branch tmp_branch;
            tmp_branch.set_child_pointer(sub_sub_index, node->get_child_pointer(sub_index));
	    tmp_branch.set_leaf(sub_sub_index);
	    triedb_branch *new_child = nullptr;
            uint64_t new_child_ptr = 0;
            std::tie(new_child, new_child_ptr) = insert_part(&tmp_branch, key, data, data_size, part + MAX_BRANCH_BITS);
            auto *new_branch = new triedb_branch(*node);
            new_branch->set_child_pointer(sub_index, new_child_ptr);
            new_branch->set_branch(sub_index);
            auto ptr = append_branch_node(*new_branch);
            return std::make_pair(new_branch, ptr);
	}
    }

    auto *child = get_branch(node, sub_index);
    triedb_branch *new_child = nullptr;
    uint64_t new_child_ptr = 0;
    std::tie(new_child, new_child_ptr) = insert_part(child, key, data, data_size, part + MAX_BRANCH_BITS);
    auto *new_branch = new triedb_branch(*node);
    new_branch->set_child_pointer(sub_index, new_child_ptr);
    if (branch_update_fn_) branch_update_fn_(*this, *new_branch);
    auto new_branch_ptr = append_branch_node(*new_branch);
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
    if (!exists) {
        auto parent_dir = file_path.parent_path();
	boost::filesystem::create_directories(parent_dir);
	fstream fs;
	fs.open(file_path.string(), fstream::out | fstream::app | fstream::binary);
	fs.write(triedb_params::VERSION, triedb_params::VERSION_SZ);
	fs.close();
    }
    roots_stream_ = new fstream();
    roots_stream_->open(file_path.string(), fstream::in | fstream::out | fstream::binary);
    triedb_version_check(roots_stream_, file_path.string());
    return roots_stream_;
}

void triedb::read_roots() {
    fstream *f = get_roots_stream();
    f->seekg(0, fstream::end);
    size_t file_size = f->tellg();
    assert(file_size >= 16);
    size_t num_roots = (file_size - VERSION_SZ) / sizeof(uint64_t);
    roots_.resize(num_roots);
    f->seekg(VERSION_SZ, fstream::beg);
    uint8_t buffer[sizeof(uint64_t)];
    for (size_t i = 0; i < num_roots; i++) {
        f->read(reinterpret_cast<char *>(&buffer[0]), sizeof(uint64_t));
        roots_[i] = read_uint64(buffer);
    }

    num_heights_ = num_roots;
}

void triedb::set_root(size_t height, uint64_t offset)
{
    if (height >= roots_.size()) {
        assert(height == roots_.size());
	roots_.resize(height+1);
    }
    auto *f = get_roots_stream();
    f->seekg(VERSION_SZ + height*sizeof(uint64_t), fstream::beg);
    uint8_t buffer[sizeof(uint64_t)];
    write_uint64(buffer, offset);
    f->write(reinterpret_cast<char *>(&buffer[0]), sizeof(uint64_t));
    roots_[height] = offset;
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
	    return i;
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

uint64_t triedb::get_root(size_t at_height)
{
    assert(roots_.size() > 0);
    if (at_height >= roots_.size()) {
        at_height = roots_.size() - 1;
    }
    return roots_[at_height];
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

uint64_t triedb::append_branch_node(const triedb_branch &node)
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
    uint8_t buffer[triedb_branch::MAX_SIZE_IN_BYTES];
    size_t n = node.serialization_size();
    node.write(buffer);
    auto *f = set_file_offset(offset);
    f->write(reinterpret_cast<char *>(&buffer[0]), n);
    last_offset_ += n;    
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
    while (parent->is_branch(sub_index)) {
	parent_ptr = parent->get_child_pointer(sub_index);
	parent = db_.get_branch(parent_ptr);
	sub_index = common::lsb(parent->mask());
	spine_.push_back(cursor(parent_ptr, sub_index));
    }
    // At this point we've must found a leaf
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
    while (parent->is_branch(sub_index)) {
	parent_ptr = parent->get_child_pointer(sub_index);
	parent = db_.get_branch(parent_ptr);
	sub_index = common::msb(parent->mask());
	spine_.push_back(cursor(parent_ptr, sub_index));
    }
    // At this point we've must found a leaf
}

void triedb_iterator::start_from_key(uint64_t parent_ptr, uint64_t key) {
    auto *parent = db_.get_branch(parent_ptr);
    size_t part = 0;
    size_t sub_index = get_sub_index(key, part);
    auto m = parent->mask();
    if (parent->is_empty(sub_index)) {
	m &= (static_cast<uint32_t>(-1) << sub_index) << 1;
	sub_index = (m == 0) ? triedb_params::MAX_BRANCH : common::lsb(m);
	if (sub_index == triedb_params::MAX_BRANCH) {
	    return;
	}
	spine_.push_back(cursor(parent_ptr, sub_index));
	leftmost();
	return;
    }
    
    while (!parent->is_empty(sub_index) && parent->is_branch(sub_index)) {
	spine_.push_back(cursor(parent_ptr, sub_index));
	parent_ptr = parent->get_child_pointer(sub_index);
	parent = db_.get_branch(parent_ptr);
	part += triedb_params::MAX_BRANCH_BITS;
	sub_index = get_sub_index(key, part);
    }
    
    if (parent->is_empty(sub_index)) {
	m = parent->mask();
	m &= (static_cast<uint32_t>(-1) << sub_index) << 1;
	sub_index = (m == 0) ? triedb_params::MAX_BRANCH : common::lsb(m);
	if (sub_index == triedb_params::MAX_BRANCH) {
	    sub_index = triedb_params::MAX_BRANCH - 1;
	    spine_.push_back(cursor(parent_ptr, sub_index));
	    next();
	    return;
	}
	spine_.push_back(cursor(parent_ptr, sub_index));
	leftmost();
	return;
    }
    
    spine_.push_back(cursor(parent_ptr, sub_index));
    
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
	spine_.push_back(cursor(db_.get_root(height_),
				triedb_params::MAX_BRANCH-1));
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
