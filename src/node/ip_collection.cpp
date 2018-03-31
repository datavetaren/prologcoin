#include "../common/random.hpp"
#include "ip_collection.hpp"

namespace prologcoin { namespace node {

using namespace prologcoin::common;

int ip_collection::random_id()
{
    return random::next_int(1000000000);
}

int ip_collection::new_id(group_prop &gprop)
{
    int r = random_id();
    while (gprop.id_to_ip_.find(r) != gprop.id_to_ip_.end()) {
	r++;
    }
    return r;
}

int ip_collection::new_gid()
{
    int r = random_id();
    while (gid_to_group_prop_.find(r) != gid_to_group_prop_.end()) {
	r++;
    }
    return r;
}

void ip_collection::add(const ip_service &ip, int score)
{
    auto it = group_to_gid_.find(ip.group());
    if (it == group_to_gid_.end()) {
	auto gid = new_gid();
	gid_to_group_prop_[gid] = group_prop{gid, ip.group(), 0};
	group_to_gid_[ip.group()] = gid;
    }

    auto gid = group_to_gid_[ip.group()];
    auto &gprop = gid_to_group_prop_[gid];

    if (gprop.ip_to_id_.find(ip) != gprop.ip_to_id_.end()) {
	// IP service already exists. Exit.
	return;
    }

    auto id = new_id(gprop);
    gprop.ip_to_id_[ip] = id;
    gprop.id_to_ip_[id] = ip;
    gprop.score_to_id_[std::make_pair(score, id)] = id;
    gprop.count_++;

    scores_.insert(std::make_pair(score,ip));

    count_++;
}

ip_service ip_collection::spill(uint64_t group)
{
    auto gid = group_to_gid_[group];
    auto &gprop = gid_to_group_prop_[gid];

    // Pick the IP with lowest score
    auto it = gprop.score_to_id_.begin();
    if (it == gprop.score_to_id_.end()) {
	return ip_service();
    }
    auto id = it->second;
    auto score = it->first.first;
    auto ip = gprop.id_to_ip_[id];
    remove(ip, score);
    return ip;
}

ip_service ip_collection::spill()
{
    if (scores_.empty()){
	return ip_service();
    }
    auto it = scores_.begin();
    auto score = it->first;
    auto ip = it->second;
    remove(ip, score);

    return ip;
}

void ip_collection::remove(const ip_service &ip, int score)
{
    scores_.erase(std::make_pair(score, ip));

    auto it = group_to_gid_.find(ip.group());
    if (it == group_to_gid_.end()) {
	// Not found
	return;
    }
    auto gid = it->second;
    auto &gprop = gid_to_group_prop_[gid];

    auto it2 = gprop.ip_to_id_.find(ip);
    if (it2 == gprop.ip_to_id_.end()) {
	// Not found
	return;
    }

    auto id = it2->second;
    gprop.ip_to_id_.erase(ip);
    gprop.id_to_ip_.erase(id);
    gprop.score_to_id_.erase(std::make_pair(score, id));
    gprop.count_--;
    count_--;
}

bool ip_collection::exists(const ip_service &ip)
{
    auto it = group_to_gid_.find(ip.group());
    if (it == group_to_gid_.end()) {
	return false;
    }
    auto gid = it->second;
    auto &gprop = gid_to_group_prop_[gid];
    auto it2 = gprop.ip_to_id_.find(ip);
    if (it2 == gprop.ip_to_id_.end()) {
	return false;
    }

    return gprop.ip_to_id_.find(ip) != gprop.ip_to_id_.end();
}

// Select N services from collection. Prefer from different groups.
std::vector<ip_service> ip_collection::select(size_t n)
{
    std::vector<ip_service> result;
    
    std::unordered_set<int> selected;
    size_t fail_count = 0;
    for (size_t i = 0; i < n && fail_count < MAX_FAIL_COUNT;) {

        // First select a random group

	auto gid = random_id();
	auto it = gid_to_group_prop_.lower_bound(gid);
	if (it == gid_to_group_prop_.end()) {
	    fail_count++;
	    continue;
	}
	auto &gprop = it->second;

	// Then select a random IP in this group

	auto id = random_id();
	auto it2 = gprop.id_to_ip_.lower_bound(id);
	if (it2 == gprop.id_to_ip_.end()) {
	    it2 = gprop.id_to_ip_.begin();
	}
	if (it2 == gprop.id_to_ip_.end()) {
	    fail_count++;
	    continue;
	}
	
	id = it2->first;
	auto &ip = it2->second;
	
	// Fail if it has already been selected
	
	if (selected.find(id) != selected.end()) {
	    fail_count++;
	    continue;
	}
	
	selected.insert(id);
	result.push_back(ip);
	i++;
    }

    return result;
}

void ip_collection::integrity_check()
{
    for (auto p : group_to_gid_) {
	auto it = gid_to_group_prop_.find(p.second);
	assert(it != gid_to_group_prop_.end());
	auto &gprop = gid_to_group_prop_[p.second];
	assert(gprop.count_ == gprop.id_to_ip_.size());
    }
}

}}

