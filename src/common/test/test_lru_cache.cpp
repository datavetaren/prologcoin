#include <common/lru_cache.hpp>
#include <common/checked_cast.hpp>
#include <cassert>
#include <string>
#include <iostream>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_lru_cache_1()
{
    header("test_lru_cache_1");

    static const size_t CACHE_SIZE = 1024;
    static const size_t NUM_ELEMENTS = 10000;

    std::cout << "Cache size: " << CACHE_SIZE << std::endl;
    std::cout << "Number of elements inserted: " << NUM_ELEMENTS << std::endl;
    
    lru_cache<int, int> cache(CACHE_SIZE);

    for (size_t i = 0; i < NUM_ELEMENTS; i++) {
        cache.insert(i, i*10);
    }

    std::cout << "Check that the cache contains the proper " << CACHE_SIZE << " elements." << std::endl;
    
    for (size_t i = 0; i < NUM_ELEMENTS; i++) {
        auto f = cache.find(i);	
	if (i < NUM_ELEMENTS - CACHE_SIZE) {
	    assert(f == nullptr);
	} else {
	    assert(checked_cast<size_t>(*f) == i*10);
	}
    }
}

static void test_lru_cache_2()
{
    header("test_lru_cache_2");

    static const size_t CACHE_SIZE = 10;
    static const size_t NUM_ELEMENTS = 100;

    std::vector<std::pair<int, int> > evicted;
    
    struct my_callback_t : public lru_cache_callback_nop<int,int> {
        my_callback_t(const my_callback_t &other) : ev_(other.ev_) { }
        my_callback_t(std::vector<std::pair<int, int> > &ev) : ev_(ev) { }
        void evicted(int key, int value) {
	    ev_.push_back(std::make_pair(key,value));
        }
        std::vector<std::pair<int,int> > &ev_;
    } my_callback(evicted);

    lru_cache<int, int, my_callback_t> cache(CACHE_SIZE, my_callback);

    for (size_t i = 0; i < NUM_ELEMENTS; i++) {
        cache.insert(i, i*10);
    }

    std::cout << "Check evicted elements" << std::endl;
    std::cout << "Num evicted: " << evicted.size() << std::endl;

    assert(evicted.size() == NUM_ELEMENTS - CACHE_SIZE);
      
    size_t i = 0;
    for (auto &p : evicted) {
        assert(checked_cast<size_t>(p.first) == i);
        assert(checked_cast<size_t>(p.second) == i*10);
	i++;
    }

    // Clear cache, all exitsing elements will get evicted
    
    cache.clear();

    assert(evicted.size() == NUM_ELEMENTS);

    i = 0;
    for (auto &p : evicted) {
        assert(checked_cast<size_t>(p.first) == i);
        assert(checked_cast<size_t>(p.second) == i*10);
	i++;
    }
}

int main(int argc, char *argv[])
{
    test_lru_cache_1();
    test_lru_cache_2();    

    return 0;
}
