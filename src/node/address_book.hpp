#pragma once

#ifndef _node_address_book_hpp
#define _node_address_book_hpp

#include <iostream>
#include <map>
#include <unordered_set>
#include "ip_address.hpp"

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
class address_entry {
public:
    using buffer_t = prologcoin::common::term_serializer::buffer_t;
    using utime = prologcoin::common::utime;

    address_entry();
    address_entry(const address_entry &other);
    address_entry(const ip_address &addr, unsigned short port);

    inline size_t index() const { return index_; }
    inline const ip_address & addr() const { return addr_; }
    inline unsigned short port() const { return port_; }
    inline int32_t score() const { return score_; }
    inline utime time() const { return time_; }
    inline const buffer_t & comment() const { return comment_; }

    inline void set_addr(const ip_address addr) { addr_ = addr; }
    inline void set_port(unsigned short port) { port_ = port; }
    inline void set_score(int32_t score) { score_ = score; }
    inline void set_time(const utime time) { time_ = time; }
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
	return index() == other.index() &&
	       addr() == other.addr() &&
	       port() == other.port() &&
	       score() == other.score() &&
	       time() == other.time() &&
	       comment() == other.comment();
    }

    void read( common::term_env &env, common::term_parser &parser );
    void write( common::term_env &ebv, common::term_emitter &emitter ) const;

    std::string str() const;
    
private:
    inline void set_index(size_t index) const { index_ = index; }

    mutable size_t index_;    // Index slot
    ip_address addr_;         // IP address
    unsigned short port_;     // Port number
    uint32_t score_;          // Current score
    utime time_;              // Last time we accessed it
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

class address_book {
public:
    address_book();

    void add( const address_entry &entry );

    void load( const std::string &path );
    void save( const std::string &path );

    // Compare if two address books are identical in contents
    bool operator == (const address_book &other) const;
    inline bool operator != (const address_book &other) const
    { return ! operator == (other); }

    void print(std::ostream &out);

private:
    std::unordered_set<address_entry> all_;     // O(log 1) to insert/remove
    std::map<size_t, address_entry> index_map_; // O(log N) to insert/remove
    std::map<uint32_t, size_t> score_map_;      // O(log N) to insert/remove
};

}}

#endif

