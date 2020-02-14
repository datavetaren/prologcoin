#include "../common/checked_cast.hpp"
#include <vector>
#include <algorithm>
#include <boost/optional.hpp>
#include <boost/functional/hash.hpp>
#include "util.hpp"

namespace prologcoin { namespace statedb {

class statedb;
class bucket;

class meta_key_entry {
public:
    meta_key_entry() = default;
    meta_key_entry(const meta_key_entry &other) = default;
    inline meta_key_entry(size_t index, size_t height) : index_(index), height_(height) { }

    inline size_t index() const { return static_cast<size_t>(index_); }
    inline size_t height() const { return static_cast<size_t>(height_); }

    inline bool operator == (const meta_key_entry &other) const {
        return index_ == other.index_ && height_ == other.height_;
    }

    inline size_t hash_value() const {
        size_t s = 0;
	boost::hash_combine(s, index());
	boost::hash_combine(s, height());
	return s;
    }
  
private:
    uint32_t index_;
    uint32_t height_;
};

}}

namespace std {
    template<> struct hash<::prologcoin::statedb::meta_key_entry> {
        inline size_t operator()(const ::prologcoin::statedb::meta_key_entry &e) const {
	    return e.hash_value();
        }
    };
}

namespace prologcoin { namespace statedb {
    
class meta_entry {
private:
    friend class statedb;
    friend class bucket;  
  
    inline meta_entry(uint32_t index, uint32_t height)
      : index_(index), height_(height), offset_(0), size_(0) { }
public:
    static const size_t SERIALIZATION_SIZE = sizeof(uint32_t)*4;
  
    inline meta_entry() : index_(0), height_(0), offset_(0), size_(0) { }
    inline meta_entry( uint32_t index, uint32_t height,
		       uint32_t offset, uint32_t sz )
      : index_(index), height_(height), offset_(offset), size_(sz) { }

    meta_entry(const meta_entry &other) = default;
  
    inline uint32_t index() const { return index_; }
    inline uint32_t height() const { return height_; }
    inline uint32_t offset() const { return offset_; }
    inline uint32_t size() const { return size_; }

    inline bool is_invalid() const { return offset_ == 0; }

    bool operator == (const meta_entry &other) const {
        return index() == other.index() && height() == other.height();
    }
    bool operator < (const meta_entry &other) const {
        if (index() < other.index()) {
	    return true;
        } else if (index() > other.index()) {
	    return false;
	}
	return height() < other.height();
    }

    void read(uint8_t *buffer, size_t &n)
    {
	uint8_t *p = &buffer[0];
	index_ = read_uint32(p); p += sizeof(uint32_t);
	height_ = read_uint32(p); p += sizeof(uint32_t);
	offset_ = read_uint32(p); p += sizeof(uint32_t);
	size_ = read_uint32(p);

	n = sizeof(uint32_t)*4;
    }
  
    void write(uint8_t *buffer, size_t &n)
    {
        uint8_t *p = &buffer[0];
        write_uint32(p, index_); p += sizeof(uint32_t);
	write_uint32(p, height_); p += sizeof(uint32_t);
	write_uint32(p, offset_); p += sizeof(uint32_t);
	write_uint32(p, size_);

	n = sizeof(uint32_t)*4;
    }

    void print(std::ostream &out) const;

private:
    uint32_t index_;   // Heap block number
    uint32_t height_;  // Introduced at height
    uint32_t offset_;  // Stored at offset in file
    uint32_t size_;    // Size of block
};

class bucket {
public:
    // Requires that provided height is higher than the existing one
    inline void add_entry(size_t index, size_t height, size_t offset, size_t sz) {
        add_entry(meta_entry(common::checked_cast<uint32_t>(index),
			     common::checked_cast<uint32_t>(height),
			     common::checked_cast<uint32_t>(offset),
			     common::checked_cast<uint32_t>(sz)));
    }

    inline void add_entry(const meta_entry &e) {
        size_t i = e.index() - first_index_;
	if (i >= entries_.size()) {
	    entries_.resize(i+1);
	}
        entries_[i].push_back(e);
    }

    inline boost::optional<const meta_entry &> find_entry(size_t index, size_t from_height) {
        size_t i = index - first_index_;
	if (i >= entries_.size()) {
	    return boost::none;
	}
	meta_entry key(index, from_height);
        auto it = std::lower_bound(entries_[i].begin(), entries_[i].end(),key);
	if (it == entries_[i].end()) {
	    return boost::none;
	}
	if (it->height() == from_height) {
	    return *it;
	} else {
   	    return *(it - 1);
	}
    }

    inline bool empty() const { return entries_.empty(); }
  
    void print(std::ostream &out) const;

private:
    size_t first_index_;
    // First sorted on index, then sorted on height.
    std::vector<std::vector<meta_entry> > entries_;
    meta_entry not_found_;
};

}}
