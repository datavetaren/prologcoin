#include <unordered_map>
#include <list>
#include <boost/optional.hpp>

namespace prologcoin { namespace common {

template<typename K, typename V> struct lru_cache_callback_nop {
    inline void evicted(const K &key, const V &value) { }
    inline void accessed(const K &key, const V &value) { }
};
    
template<typename K, typename V, typename C = lru_cache_callback_nop<K,V> > class lru_cache {
public:
    typedef K key_type;
    typedef V value_type;
  
    inline lru_cache(size_t capacity) : capacity_(capacity), callback_() { }
    inline lru_cache(size_t capacity, const C &callback) : capacity_(capacity), callback_(callback) { }
    inline ~lru_cache() { clear(); }

    inline void set_callback(C &callback) { callback_ = callback; }

    inline void insert(const K &key, const V &value) {
        auto it = map_.find(key);
	if (it == map_.end()) {
	    if (access_.size() >= capacity_) {
	        auto &removed_key = access_.back();
		erase(removed_key);
	    }
	    access_.push_front(key);
	    map_[key] = std::make_pair(value, access_.begin());
	}
    }

    inline boost::optional<V> find(const K &key)
    {
        auto it = map_.find(key);
	if (it == map_.end()) {
	    return boost::none;
	}
	auto &v = it->second.first;
	auto &access_it = it->second.second;
	access_.erase(access_it);
	access_.push_front(key);
	it->second.second = access_.begin();
	
	return v;
    }

    inline void erase(const K &key) {
	auto removed_it = map_.find(key);
	if (removed_it == map_.end()) {
	    return;
	}
	auto removed_value = removed_it->second.first;
        auto access_it = removed_it->second.second;
	access_.erase(access_it);
	map_.erase(removed_it);
	callback_.evicted(key, removed_value); 
    }

    inline void clear() {
        while (!access_.empty()) {
	    erase(access_.back());
        }
    }

    inline void foreach(const std::function<void(const K &key, V &value)> &apply) {
        for (auto &e : map_) {
	    apply(e.first, e.second.first);
        }
    }

private:
    typedef typename std::list<K>::iterator access_iterator_type;
    size_t capacity_;
    std::unordered_map<K, std::pair<V , access_iterator_type> > map_;
    std::list<K> access_;
    C callback_;

};

}}

