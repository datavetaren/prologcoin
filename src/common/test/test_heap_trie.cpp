#include <iostream>
#include <iomanip>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <array>
#include <boost/lexical_cast.hpp>
#include <common/merkle_trie.hpp>
#include <common/random.hpp>
#include <common/hex.hpp>
#include <common/utime.hpp>
#include <common/blake2.hpp>
#include <common/sha1.hpp>
#include <common/ripemd160.hpp>

using namespace prologcoin::common;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

struct hash_t {
    uint8_t hash[sha1::HASH_SIZE];
};

static void compute_hash(const uint8_t *data, size_t block_size, hash_t &out)
{
    uint8_t tmp[sha1::HASH_SIZE];
    sha1 sha;
    sha.update(data, block_size);
    sha.finalize(tmp);
    sha.update(tmp, sizeof(tmp));
    sha.finalize(tmp);
}

static void test_heap_trie(size_t block_size)
{
    std::string h = "test_heap_trie(): block_size=" + boost::lexical_cast<std::string>(block_size);
    header( h );

    const size_t GRANULARITY = 256;
    
    // The idea with this test is to see how fast we can maintain
    // a very large heap set and compute a heap trie on it.
    // We need to figure out what are the acceptable size of heap
    // blocks.

    const size_t MB = 1000000;
    const size_t GB = 1000*MB;

    size_t mem_space = 100*GB;

    size_t num_blocks = mem_space / block_size;

    std::cout << "Total memory space: " << mem_space << " (" << (mem_space/GB) << " GB)" << std::endl;
    std::cout << "Num blocks        : " << num_blocks << std::endl;

    uint8_t *blocks[GRANULARITY];
    hash_t hash[GRANULARITY];

    // Create N blocks
    for (size_t i = 0; i < GRANULARITY; i++) {
        blocks[i] = new uint8_t[block_size];
        random::next_bytes(blocks[i], block_size);
    }

    auto start = utime::now();
    // Hash them
    for (size_t i = 0; i < GRANULARITY; i++) {
        compute_hash(blocks[i], block_size, hash[i]);
    }
    auto end = utime::now();
    auto time_x_blocks = (end - start).in_us();
    auto total_time_blocks = ((num_blocks / GRANULARITY) * time_x_blocks);

    auto start1 = utime::now();
    for (size_t i = 0; i < GRANULARITY; i++) {
        compute_hash(blocks[i], sizeof(hash[i]), hash[i]);
    }
    auto end1 = utime::now();
    auto time_x_nodes = (end1 - start1).in_us();
    
    std::cout << std::setw(4) << GRANULARITY << " blocks hash  : " << time_x_blocks << " microseconds" << std::endl;
    std::cout << "Estimated blocks  : " << total_time_blocks << " microseconds" << std::endl;
    std::cout << std::setw(4) << GRANULARITY << " nodes hash   : " << time_x_nodes << " microseconds" << std::endl;

    // Let's assume merkle trie has fanout of 32 (default) see how much hashing
    // it's involved

    size_t total_time = 0;
    for (size_t i = num_blocks; i > 0; i /= 32) {
        if (i == num_blocks) {
	    total_time += total_time_blocks;
        } else {
	    total_time += (i * time_x_nodes / GRANULARITY);
	}
    }
    std::cout << "Estimated total   : " << utime(total_time).in_ss() << " seconds" << std::endl;
}

int main( int argc, char *argv[] )
{
    random::set_for_testing(true);

    test_heap_trie(65536*4);    
    test_heap_trie(65536*2);
    test_heap_trie(65536);
    test_heap_trie(32768);

    return 0;
}

    
