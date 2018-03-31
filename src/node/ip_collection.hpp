#pragma once

#ifndef _node_ip_collection_hpp
#define _node_ip_collection_hpp

#include "ip_service.hpp"
#include <map>
#include <set>

namespace prologcoin { namespace node {

//
// This class represents a collection of IP addresses.
// We keep track of the group property to enable quick randomly
// uniform distribution over that set. The design is that
// it should be at most O(log N) to add/remove elements of
// the collection while maintaining the group property.
//

class ip_collection : public boost::noncopyable {
public:
    inline ip_collection() : count_(0) { }

    inline ip_collection(ip_collection &&other) :
     count_(std::move(other.count_)),
     gid_to_group_prop_(std::move(other.gid_to_group_prop_)),
     group_to_gid_(std::move(other.group_to_gid_)),
     scores_(std::move(other.scores_)) 
       {  }

   
    void add( const ip_service &ip, int score );
    void remove( const ip_service &ip, int score );
    bool exists( const ip_service &ip );

    inline size_t size() const { return count_; }
    inline size_t size(uint64_t group)
       { auto it = group_to_gid_.find(group);
	 if (it == group_to_gid_.end()) {
	     return 0;
	 }
	 auto gid = it->second;
	 auto it2 = gid_to_group_prop_.find(gid);
	 if (it2 == gid_to_group_prop_.end()) {
	     return 0;
	 }
	 return it2->second.count_;
       }

    inline size_t num_groups() const
        { return gid_to_group_prop_.size(); }

    std::vector<ip_service> select(size_t n);
    ip_service spill(uint64_t group);
    ip_service spill();

    void integrity_check();

private:
    static const size_t MAX_FAIL_COUNT = 1000;

    struct group_prop {
	int id_;
	uint64_t group_;
	size_t count_;

	std::map<int, ip_service> id_to_ip_;
	std::map<ip_service, int> ip_to_id_;
	std::map<std::pair<int,int>, int> score_to_id_;
    };

    size_t count_;

    int random_id();
    int new_id(group_prop &prop);
    int new_gid();

    std::map<int, group_prop> gid_to_group_prop_;
    std::map<uint64_t, int> group_to_gid_;
    std::set<std::pair<int,ip_service> > scores_;
};

}}

#endif

