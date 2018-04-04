#pragma once

#ifndef _node_address_book_hpp
#define _node_address_book_hpp

#include <iostream>
#include <map>
#include <unordered_set>
#include "ip_address.hpp"
#include "ip_service.hpp"
#include "ip_collection.hpp"
#include "../common/term_serializer.hpp"

namespace prologcoin { namespace node {

class address_book_load_exception : public std::runtime_error
{
public:
    address_book_load_exception(const std::string &msg, int line) :
	std::runtime_error("Error at line "
			   + boost::lexical_cast<std::string>(line) + ": "
			   + msg) { }
};

class address_book;

//
// The score of an address entry is something that is determined by
// the client only (it is not propagated to others, as that could be
// used as an attack vector.) If we succeed connecting to the address
// multiple times over a long period, then its score is increased.
// Likewise, if we fail to connect, its score is decreased.
// 
//
class address_entry : public ip_service {
public:
    static const int32_t VERIFIED_INITIAL_SCORE = 100;
    
    using buffer_t = prologcoin::common::term_serializer::buffer_t;
    using utime = prologcoin::common::utime;

    address_entry();
    address_entry(const address_entry &other);
    address_entry(const ip_address &addr, unsigned short port,
		  const ip_address &src_addr, unsigned short src_port);
    address_entry(const ip_address &addr, unsigned short port);
    address_entry(const ip_service &ip);

    inline size_t id() const { return id_; }
    inline const ip_service & source() const { return source_; }
    inline int32_t score() const { return score_; }
    inline utime time() const { return time_; }
    inline int32_t version_major() const { return version_major_; }
    inline int32_t version_minor() const { return version_minor_; }
    inline const buffer_t & comment() const { return comment_; }
    std::string comment_str() const;

    inline void set_source(const ip_address &addr, unsigned short port)
    { source_ = ip_service(addr, port); }
    inline void set_source(const ip_service &src) { source_ = src; }
    inline void set_score(int32_t score) { score_ = score; }
    inline void set_time(const utime time) { time_ = time; }
    inline void set_version(int32_t major, int32_t minor)
    { version_major_ = major; version_minor_ = minor; }

    void set_comment(const common::term t, common::term_env &src);
    inline void set_comment(buffer_t comment) { comment_ = comment; }

    // Very expensive! But good for debugging/testing.
    // (It invokes the parser and then the serializer...)
    void set_comment(const std::string &str);
    void set_comment(common::term_env &env, const std::string &str);

    inline bool operator == (const address_entry &other) const {
	return addr() == other.addr() && port() == other.port();
    }
    inline bool operator != (const address_entry &other) const {
	return ! operator == (other);
    }

    inline bool deep_equal(const address_entry &other) const {
	return id() == other.id() &&
	       addr() == other.addr() &&
	       source() == other.source() &&
	       port() == other.port() &&
	       score() == other.score() &&
	       time() == other.time() &&
  	       version_major() == other.version_major() &&
  	       version_minor() == other.version_minor() &&
	       comment() == other.comment();
    }

    void read( common::term_env &env, common::term_parser &parser );
    void write( common::term_env &env, common::term_emitter &emitter ) const;

    common::term to_term(common::term_env &env) const;
    bool from_term(common::term_env &env, const common::term t);

    std::string str() const;

private:
    inline void set_id(size_t id) const { id_ = id; }

    mutable size_t id_;       // Identifier
    ip_service source_;       // Where the IP service came from
    int32_t score_;           // Current score
    utime time_;              // Last time we accessed it
    int32_t version_major_;   // Major version
    int32_t version_minor_;   // Minor version
    buffer_t comment_;        // Extra information

    friend class address_book;
};

}}

namespace std {
    template<> struct hash<prologcoin::node::address_entry> {
        size_t operator()(const prologcoin::node::address_entry &e) const {
	    prologcoin::common::fast_hash h;
	    h.update(e.addr().to_bytes(), e.addr().bytes_size());
	    h << e.port();
	    return static_cast<uint32_t>(h);
	}
    };
}

namespace prologcoin { namespace node {

class test_address_book;

class address_book {
public:
    address_book();

    inline void add( const std::string &addr, unsigned short port )
    { add(address_entry(addr,port)); }

    void add( const address_entry &entry );
    void remove( size_t id );
    void remove( const ip_service &ip );
    void add_score(address_entry &entry, int change );
    size_t size() const;
    bool exists( const ip_service &ip, address_entry *entry = nullptr );

    std::vector<address_entry> get_all_true(std::function<bool (const address_entry &e)> fn);

    std::vector<address_entry> get_all();
    std::vector<address_entry> get_all_verified();
    std::vector<address_entry> get_all_unverified();

    std::vector<address_entry> get_from_top_10_pt(size_t n);
    std::vector<address_entry> get_from_bottom_90_pt(size_t n);
    std::vector<address_entry> get_randomly_from_top_10_pt(size_t n);
    std::vector<address_entry> get_randomly_from_bottom_90_pt(size_t n);
    std::vector<address_entry> get_randomly_from_unverified(size_t n);

    void load( const std::string &path );
    void save( const std::string &path );

    // Compare if two address books are identical in contents
    bool operator == (const address_book &other) const;
    inline bool operator != (const address_book &other) const
    { return ! operator == (other); }

    void print(std::ostream &out, const std::vector<address_entry> &entries);
    void print(std::ostream &out);
    void print(std::ostream &out, size_t n);

    std::string stat() const;

    inline size_t num_spilled() const { return num_spilled_; }
    inline size_t num_groups() const {
	return top_10_collection_.num_groups() +
	    bottom_90_collection_.num_groups() +
	    unverified_gid_to_group_.size();
    }
    inline size_t num_unverified() const
        { return unverified_id_to_gid_.size(); }

    inline bool is_unverified(const address_entry &e)
    { return !is_verified(e); }

    inline bool is_verified(const address_entry &e)
    { return e.source().is_zero(); }

    void update_time(const ip_service &ip);

    void integrity_check();

    void for_each_address_entry(const std::function<void (const address_entry &entry)> &fn);

private:
    inline bool is_spill_enabled() const { return spill_enabled_; }
    inline void set_spill_enabled(bool e) { spill_enabled_ = e; }

    enum spill_area { SPILL_IN_10, SPILL_IN_90, SPILL_IN_UNVERIFIED };
    void spill_check(const address_entry &e, spill_area area);

    void calibrate();

    // Create a lexicographic order on score & id.
    // This way we can quickly access the best 10% or worst 90% from
    // the map.
    struct score_entry {
	inline score_entry() : score_(0), id_(0) { }
	inline score_entry(const score_entry &other)
            : score_(other.score_), id_(other.id_) { }
	inline score_entry(int32_t score, size_t id) :
	    score_(score), id_(id) { }
	inline int32_t score() const { return score_; }
	inline size_t id() const { return id_; }

	inline std::string str() const
	    { return "{score=" + boost::lexical_cast<std::string>(score())
		    + ", id=" + boost::lexical_cast<std::string>(id()) + "}";
	    }

	inline bool operator < (const score_entry &other) const {
	    if (score_ > other.score_) {
		return true;
	    } else if (score_ < other.score_) {
	        return false;
	    } else {
		return id_ < other.id_;
	    }
	}

	inline bool operator == (const score_entry &other) const {
	    return score_ == other.score_ && id_ == other.id_;
	}
	inline bool operator != (const score_entry &other) const {
	    return ! operator == (other);
	}

    private:
	int32_t score_;
	size_t id_;
    };

    size_t id_count_;
    size_t num_spilled_;
    bool spill_enabled_;

    std::map<size_t, address_entry> id_to_entry_;
    std::map<ip_service, size_t> ip_to_id_;

    std::map<score_entry, size_t> top_10_;
    ip_collection top_10_collection_;
    std::map<score_entry, size_t> bottom_90_;
    ip_collection bottom_90_collection_;

    // The key is the source group
    std::map<uint64_t, ip_collection> unverified_;
    std::map<int, uint64_t> unverified_gid_to_group_;
    std::map<uint64_t, int> unverified_group_to_gid_;
    std::map<size_t, uint64_t> unverified_id_to_gid_;

    static const int MAX_GID = 1000000000;
    static const size_t MAX_FAIL_COUNT = 1000;

    // Not more than 100 addesses per group. We'll spill
    // (the worst) one before adding.
    static const size_t MAX_GROUP_SIZE = 100;
    static const size_t MAX_SOURCE_SIZE = 400;

    int random_gid();
    int new_gid();

    friend class test_address_book;
};

}}

#endif

