#include <common/test/test_home_dir.hpp>
#include <common/utime.hpp>
#include <common/sha1.hpp>
#include <common/hex.hpp>
#include <common/random.hpp>
#include <common/blake2.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <boost/filesystem.hpp>
#include <db/triedb.hpp>

using namespace prologcoin::common;
using namespace prologcoin::db;

std::string home_dir;
std::string test_dir;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

const size_t TEST_NUM_ENTRIES = 1024; // Perfect full trie (32x32)

struct custom_data_leaf {
    uint8_t heap_block[65536];
};

static void test_basic()
{
    header("test_basic");
    
    std::cout << "Test directory: " << test_dir << std::endl;

    triedb::erase_all(test_dir);
    triedb db(test_dir);

    std::vector<std::pair<const triedb_branch *, size_t> > path;

    auto at_root = db.new_root();
    for (size_t i = 0; i < TEST_NUM_ENTRIES; i++) {
        custom_data_leaf data;
	for (size_t j = 0; j < 65536; j++) {
	    uint8_t b = (i + j) & 0xff;
	    data.heap_block[j] = b;
	}
        db.insert(at_root, i, reinterpret_cast<uint8_t *>(&data), sizeof(data));
	at_root = db.new_root(at_root);
    }

    // Check root hash
    at_root = db.find_root(TEST_NUM_ENTRIES-1);
    db.find(at_root, 0, &path);

    using hash_t = struct { uint8_t hash[32]; };

    hash_t actual_root_hash;

    std::cout << "PATH: ";
    bool first = true;
    std::pair<const triedb_branch *, size_t> *last_e;
    for (auto &e : path) {
        last_e = &e;
        auto *br = e.first;
	if (first) {
	    assert(br->hash_size() == sizeof(hash_t));
	    memcpy(&actual_root_hash.hash[0], br->hash(), br->hash_size());
	}
	if (!first) std::cout << " -> ";
	std::cout << e.second << "[" << hex::to_string(br->hash(), br->hash_size()) << "]";
	first = false;
    }
    auto *lf = db.get_leaf(last_e->first,last_e->second);
    std::cout << " -> " << hex::to_string(lf->hash(), lf->hash_size());
    std::cout << std::endl;

    //
    // Compute the root hash a priori
    //
    std::cout << "Compute root hash using first principles..." << std::endl;
    std::vector<hash_t> hashes;
    for (size_t i = 0; i < TEST_NUM_ENTRIES; i++) {
      uint8_t data[65536];
      for (size_t j = 0; j < sizeof(data); j++) {
	  data[j] = static_cast<uint8_t>((i+j) & 0xff);
      }
      blake2b_state s;
      blake2b_init(&s, sizeof(hash_t));
      uint64_t key = i;
      uint8_t key_serialized[sizeof(uint64_t)];
      write_uint64(key_serialized, key);
      blake2b_update(&s, key_serialized, sizeof(key_serialized));
      blake2b_update(&s, data, sizeof(data));
      hash_t hash;
      blake2b_final(&s, &hash.hash[0], sizeof(hash_t));
      hashes.push_back(hash);
      if (i == 0) {
	  std::cout << "First hash (bottom left): " << hex::to_string(&hash.hash[0], sizeof(hash)) << std::endl;
      }
    }
    while (hashes.size() > 1) {
        std::vector<hash_t> next_hashes;
	size_t i = 0;
	blake2b_state s;
	blake2b_init(&s, sizeof(hash_t));
	for (auto &h : hashes) {
	    blake2b_update(&s, &h.hash[0], sizeof(hash_t));
	    i++;
	    if (i == 32) {
	        hash_t next_hash;
	        blake2b_final(&s, &next_hash.hash[0], sizeof(hash_t));
	        next_hashes.push_back(next_hash);
		blake2b_init(&s, sizeof(hash_t));
		i = 0;
	    }
	}
	assert(i == 0); // We have a perfect trie
	hashes = next_hashes;
    }
    hash_t &expect_root_hash = hashes[0];
    std::cout << "Actual root hash: " << hex::to_string(&actual_root_hash.hash[0], sizeof(hash_t)) << std::endl;
    std::cout << "Expect root hash: " << hex::to_string(&expect_root_hash.hash[0], sizeof(hash_t)) << std::endl;
    assert(memcmp(expect_root_hash.hash, actual_root_hash.hash, sizeof(hash_t)) == 0);
}

int main(int argc, char *argv[])
{
    home_dir = find_home_dir(argv[0]);
    test_dir = home_dir;
    test_dir = (boost::filesystem::path(test_dir) / "bin" / "test" / "db" / "triedb_data").string();

    random::set_for_testing(true);
     
    test_basic();

    return 0;
}
