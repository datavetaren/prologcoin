#pragma once

#ifndef _global_meta_entry_hpp
#define _global_meta_entry_hpp

#include "../common/term_env.hpp"
#include "../common/utime.hpp"
#include "../pow/pow_verifier.hpp"
#include "../db/util.hpp"
#include "../db/triedb.hpp"

namespace prologcoin { namespace global {

using db_root_id = db::root_id;

class meta_id {
public:
    static const size_t HASH_SIZE = 32;

    inline meta_id() { memset(hash_, 0, HASH_SIZE); }
    inline meta_id(const uint8_t h[HASH_SIZE]) { memcpy(hash_, h, HASH_SIZE); }

    inline bool is_zero() const {
	static const uint8_t ZERO[HASH_SIZE]
	    = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	return memcmp(hash_, ZERO, HASH_SIZE) == 0;
    }

    inline const uint8_t * hash() const { return hash_; }
    inline size_t hash_size() const { return HASH_SIZE; }

    inline bool operator == (const meta_id &other) const {
	return memcmp(hash_, other.hash_, HASH_SIZE) == 0;
    }

    inline bool operator != (const meta_id &other) const {
	return ! (operator == (other));
    }

    inline bool operator < (const meta_id &other) const {
	return memcmp(hash_, other.hash_, HASH_SIZE) < 0;
    }

    void write(uint8_t *data) const {
	memcpy(data, hash_, HASH_SIZE);
    }

    void read(const uint8_t *data) {
	memcpy(hash_, data, HASH_SIZE);
    }

    bool from_term(common::term_env &src, common::term t);
    common::term to_term(common::term_env &dst) const;

private:
    uint8_t hash_[HASH_SIZE];
};

}}

namespace std {
    template<> struct hash<prologcoin::global::meta_id> {
	size_t operator () (const prologcoin::global::meta_id &key) const {
	    return static_cast<size_t>(prologcoin::db::read_uint64(key.hash()));
	}
    };
}

namespace prologcoin { namespace global {

//
// This is the main blockchain entry. This would correspond to
// bitcoin block header. We store in a separate database so it's easier
// to reason about these meta entries without having to load other
// data.
// 
// We have:
//      (These are shared across all nodes in the network)
//      id         : 32 byte identifier (computed)
//      version    : the current version
//      height     : so we don't have to compute it
//      timestamp  : When block was created as UNIX time
//      nonce 
//      pow_difficulty : the proof-of-work difficulty
//      pow_proof  : the proof-of-work on the total state hash
//
//      (Local data, that is not shared across all nodes in the network)
//      root_id_blocks: local db root id for blocks
//      root_id_heap: local db root id for heap
//      root_id_closures : local db root id for closures
//      root_id_symbols : local db root id for symbols
//      root_id_program : local db root id for program
// 
class meta_entry {
public:
    static const size_t DEFAULT_SUPER_DIFFICULTY = 8;

    meta_entry(const meta_entry &other) = default;

    meta_entry()
        : id_(),
	  previous_id_(),
	  version_(1),
	  height_(0),
	  nonce_(0),
	  timestamp_(),
	  pow_proof_(),
	  pow_difficulty_(pow::flt1648(1)),
	  root_id_meta_(),
	  root_id_blocks_(),
	  root_id_heap_(),
	  root_id_closure_(),
	  root_id_symbols_(),
	  root_id_program_() {
    }

    // The identifier is computed as a hash from a collection of things
    // The computation itself is in blockchain.cpp
    const meta_id & get_id() const {
	return id_;
    }

    void set_id(const meta_id &id) {
	id_ = id;
    }

    bool is_zero() const {
	return id_.is_zero();
    }

    const meta_id & get_previous_id() const {
	return previous_id_;
    }

    void set_previous_id(const meta_id &id) {
	previous_id_ = id;
    }

    uint64_t get_version() const {
	return version_;
    }
    void set_version(uint64_t v) {
	version_ = v;
    }

    uint32_t get_height() const {
	return height_;
    }

    void set_height(uint32_t h) {
	height_ = h;
    }

    uint64_t get_nonce() const {
	return nonce_;
    }

    void set_nonce(uint64_t n) {
	nonce_ = n;
    }

    const common::utime & get_timestamp() const {
	return timestamp_;
    }

    void set_timestamp(const common::utime &t) {
	timestamp_ = t;
    }

    const pow::pow_proof & get_pow_proof() const {
	return pow_proof_;
    }

    void set_pow_proof(const pow::pow_proof &pow) {
	pow_proof_ = pow;
    }

    const pow::pow_difficulty & get_pow_difficulty() const {
	return pow_difficulty_;
    }

    void set_pow_difficulty(const pow::pow_difficulty &dif) {
	pow_difficulty_ = dif;
    }

    bool validate_pow() const;

    bool is_partial() const {
	return get_root_id_heap().is_zero() ||
	       get_root_id_closure().is_zero() ||
	       get_root_id_symbols().is_zero() ||
	       get_root_id_program().is_zero();
    }

    const db_root_id get_root_id_meta() const {
	return root_id_meta_;
    }
    void set_root_id_meta(const db_root_id id) {
	root_id_meta_ = id;
    }

    const db_root_id get_root_id_blocks() const {
	return root_id_blocks_;
    }
    void set_root_id_blocks(const db_root_id id) {
	root_id_blocks_ = id;
    }

    const db_root_id get_root_id_heap() const {
	return root_id_heap_;
    }
    void set_root_id_heap(const db_root_id id) {
	root_id_heap_ = id;
    }

    const db_root_id get_root_id_closure() const {
	return root_id_closure_;
    }
    void set_root_id_closure(const db_root_id id) {
	root_id_closure_ = id;
    }

    const db_root_id get_root_id_symbols() const {
	return root_id_symbols_;
    }
    void set_root_id_symbols(const db_root_id id) {
	root_id_symbols_ = id;
    }

    const db_root_id get_root_id_program() const {
	return root_id_program_;
    }
    void set_root_id_program(const db_root_id id) {
	root_id_program_ = id;
    }

    static constexpr size_t serialization_size() {
	return meta_id::HASH_SIZE + // 32 bytes
	       sizeof(uint64_t) + // version 8 bytes
	       meta_id::HASH_SIZE + // previous id (32 bytes)
	       sizeof(uint64_t) + // nonce, 8 bytes (uint64_t)
	       sizeof(common::utime) + // timestamp 8 bytes (uint64_t)
	       pow::pow_proof::TOTAL_SIZE_BYTES + // currently 576 bytes
  	       8 + // pow difficulty, flt1648 is serialized in 8 bytes
	       5*sizeof(db_root_id) + // 5*8 bytes (5*uint64_t) = 40
	       4*sizeof(uint64_t); // number of entries for each
	                           // database (heap, symbol, program, closure)
	                           // 4*8 bytes (4*uint64_t) = 32
    }

    void read(const uint8_t *data) {
	auto p = data;
	id_.read(p); p += meta_id::HASH_SIZE;
	version_ = db::read_uint64(p); p += sizeof(uint64_t);
	previous_id_.read(p); p += meta_id::HASH_SIZE;
	nonce_ = db::read_uint64(p); p += sizeof(uint64_t);
	timestamp_.read(p); p += timestamp_.serialization_size();
	pow_difficulty_.read(p); p += pow_difficulty_.serialization_size();
	pow_proof_.read(p); p += pow::pow_proof::TOTAL_SIZE_BYTES;
	root_id_blocks_ = db_root_id(db::read_uint64(p)); p += sizeof(uint64_t);
	root_id_heap_ = db_root_id(db::read_uint64(p)); p += sizeof(uint64_t);
	root_id_closure_ = db_root_id(db::read_uint64(p)); p += sizeof(uint64_t);
	root_id_symbols_ = db_root_id(db::read_uint64(p)); p += sizeof(uint64_t);
	root_id_program_ = db_root_id(db::read_uint64(p)); p += sizeof(uint64_t);
    }

    void write(uint8_t *data) const {
	uint8_t *p = data;
	id_.write(p); p += meta_id::HASH_SIZE;
	db::write_uint64(p, version_); p += sizeof(uint64_t);
	previous_id_.write(p); p += meta_id::HASH_SIZE;
	db::write_uint64(p, nonce_); p += sizeof(uint64_t);
	timestamp_.write(p); p += timestamp_.serialization_size();
	pow_difficulty_.write(p); p += pow_difficulty_.serialization_size();
	pow_proof_.write(p); p += pow::pow_proof::TOTAL_SIZE_BYTES;
	db::write_uint64(p, root_id_blocks_.value()); p += sizeof(uint64_t);
	db::write_uint64(p, root_id_heap_.value()); p += sizeof(uint64_t);
	db::write_uint64(p, root_id_closure_.value()); p += sizeof(uint64_t);
	db::write_uint64(p, root_id_symbols_.value()); p += sizeof(uint64_t);
	db::write_uint64(p, root_id_program_.value()); p += sizeof(uint64_t);
    }

    bool operator == (const meta_entry &other) const {
	return get_id() == other.get_id() &&
	       get_previous_id() == other.get_previous_id() &&
	       get_version() == other.get_version() &&
	       get_height() == other.get_height() &&
	       get_nonce() == other.get_nonce() &&
	       get_timestamp() == other.get_timestamp() &&
	       get_pow_difficulty() == other.get_pow_difficulty();
    }

    bool operator != (const meta_entry &other) const {
	return ! operator == (other);
    }

private:
    meta_id id_;
    meta_id previous_id_;
    uint64_t version_;
    uint32_t height_;
    uint64_t nonce_;
    common::utime timestamp_;
    pow::pow_proof pow_proof_;
    pow::pow_difficulty pow_difficulty_;
    db_root_id root_id_meta_; // Local id for where this entry is stored
    db_root_id root_id_blocks_;
    db_root_id root_id_heap_;
    db_root_id root_id_closure_;
    db_root_id root_id_symbols_;
    db_root_id root_id_program_;
};

}}

#endif
