#include <iostream>
#include <iomanip>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <array>
#include <common/merkle_trie.hpp>
#include <common/random.hpp>
#include <common/hex.hpp>
#include <common/utime.hpp>

using namespace prologcoin::common;

#define PERFORMANCE_TEST 0

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_merkle_trie_order()
{
    header( "test_merkle_trie_order" );

    static const size_t N = 10000;

    std::cout << "Generate " << N << " random keys & values." << std::endl;
    
    uint64_t *keys = new uint64_t[N];
    uint64_t *values = new uint64_t[N];
    for (size_t i = 0; i < N; i++) {
        keys[i] = random::next_int(static_cast<uint64_t>(1000000000));
	values[i] = random::next_int(static_cast<uint64_t>(1000000000));
    }

    std::cout << "Insert them into trie." << std::endl;
    merkle_trie<uint64_t,60> mtrie;
    for (size_t i = 0; i < N; i++) {
        mtrie.insert(keys[i], values[i]);
    }

    std::cout << "Check against sorted values." << std::endl;
    std::sort(keys, keys+N);
    size_t cnt = 0;
    for (auto &it : mtrie) {
        auto i = it.index();
	assert(keys[cnt] == i);
	cnt++;
    }
    assert(cnt == N);

    delete [] keys;
    delete [] values;
    std::cout << "Everything is ok." << std::endl;
}

static void test_merkle_trie_hash()
{
    header( "test_merkle_trie" );

    static const size_t N = 10000;

    std::cout << "Generate " << N << " random keys & values." << std::endl;
    
    uint64_t *keys = new uint64_t[N];
    uint64_t *values = new uint64_t[N];
    for (size_t i = 0; i < N; i++) {
        keys[i] = random::next_int(static_cast<uint64_t>(1000000000));
	values[i] = random::next_int(static_cast<uint64_t>(1000000000));
    }
    
    merkle_trie<uint64_t,60> mtrie1;
    merkle_trie<uint64_t,60> mtrie2;
    merkle_trie<uint64_t,60> mtrie3; mtrie3.set_auto_rehash(false);

    std::cout << "Insert them into two tries, one in reversed order." << std::endl;
        
    auto time_start = utime::now();

    // Verify that hash is independent of insertion order
    for (size_t i = 0; i < N; i++) {
        mtrie1.insert(keys[i], values[i]);
	mtrie2.insert(keys[N-i-1], values[N-i-1]);
	mtrie3.insert(keys[i], values[i]);
    }
    auto hash1 = mtrie1.hash();
    auto hash2 = mtrie2.hash();
    mtrie3.rehash_all();
    auto hash3 = mtrie3.hash();

    auto time_end = utime::now();

    std::cout << "Hash1 is: " << hex::to_string(reinterpret_cast<uint8_t *>(hash1.data), 32) << std::endl;
    std::cout << "Hash2 is: " << hex::to_string(reinterpret_cast<uint8_t *>(hash2.data), 32) << std::endl;
    std::cout << "Hash3 is: " << hex::to_string(reinterpret_cast<uint8_t *>(hash3.data), 32) << std::endl;    

    assert(hash1 == hash2 && hash2 == hash3);
    
    std::cout << "Number of bytes 1: " << mtrie1.num_bytes() << std::endl;
    std::cout << "Number of bytes 2: " << mtrie2.num_bytes() << std::endl;
    std::cout << "Number of bytes 3: " << mtrie3.num_bytes() << std::endl;    

    delete [] keys;
    delete [] values;
    
    std::cout << "Time: " << (time_end - time_start).in_ms() << std::endl;
}

#if PERFORMANCE_TEST
static void test_merkle_trie_performance()
{
    header( "test_merkle_trie_performance" );

    static const size_t N = 1000000;

    static const uint64_t SPARSENESS = 100000000;

    std::cout << "Generate " << N << " random keys & values." << std::endl;
    uint64_t *keys = new uint64_t[N];
    uint64_t *values = new uint64_t[N];
    for (size_t i = 0; i < N; i++) {
        auto key = random::next_int(static_cast<uint64_t>(SPARSENESS));
        auto value = random::next_int(static_cast<uint64_t>(SPARSENESS));
	keys[i] = key;
	values[i] = value;
    }
    
    merkle_trie<uint64_t,60> mtrie;
    merkle_trie<uint64_t,60> mtrie2; mtrie2.set_auto_rehash(false);

    std::cout << "Insert " << N << " random keys & values." << std::endl;
    
    // Verify that hash is independent of insertion order
    auto time_start = utime::now();
    for (size_t i = 0; i < N; i++) {
        if (i % 100000 == 0) std::cout << "progress i=" << i << std::endl;
        mtrie.insert(keys[i], values[i]);
    }
    auto hash = mtrie.hash();
    auto time_end = utime::now();

    std::cout << "Insert again but without auto rehash" << std::endl;
    
    // The same but without auto rehash
    auto time_start2 = utime::now();
    for (size_t i = 0; i < N; i++) {
        if (i % 100000 == 0) std::cout << "progress i=" << i << std::endl;
        mtrie2.insert(keys[i], values[i]);        
    }
    mtrie2.rehash_all();
    auto hash2 = mtrie2.hash();
    auto time_end2 = utime::now();    
    
    std::cout << "Hash1 is: " << hex::to_string(reinterpret_cast<uint8_t *>(hash.data), 32) << std::endl;
    std::cout << "Hash2 is: " << hex::to_string(reinterpret_cast<uint8_t *>(hash2.data), 32) << std::endl;

    std::cout << "Time (auto rehash on): " << (time_end - time_start).in_ms() << std::endl;
    std::cout << "Time (auto rehash off):" << (time_end2 - time_start2).in_ms() << std::endl;

    assert(hash == hash2);

    delete [] keys;
    delete [] values;
}
#endif

int main(int argc, char *argv[])
{
    random::set_for_testing(true);

    test_merkle_trie_order();
    test_merkle_trie_hash();
#if PERFORMANCE_TEST
    test_merkle_trie_performance();
#endif
    return 0;
}