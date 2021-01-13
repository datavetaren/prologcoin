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
std::string test_dir2;

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
    uint32_t full_mask = static_cast<uint32_t>(-1);
    uint8_t full_mask_buffer[sizeof(uint32_t)];
    write_uint32(full_mask_buffer, full_mask);
    while (hashes.size() > 1) {
        std::vector<hash_t> next_hashes;
	size_t i = 0;
	blake2b_state s;
	blake2b_init(&s, sizeof(hash_t));
	blake2b_update(&s, full_mask_buffer, sizeof(full_mask_buffer));
	for (auto &h : hashes) {
	    blake2b_update(&s, &h.hash[0], sizeof(hash_t));
	    i++;
	    if (i == 32) {
	        hash_t next_hash;
	        blake2b_final(&s, &next_hash.hash[0], sizeof(hash_t));
	        next_hashes.push_back(next_hash);
		blake2b_init(&s, sizeof(hash_t));
		blake2b_update(&s, full_mask_buffer, sizeof(full_mask_buffer));
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

static void test_merkle()
{
    header("test_merkle");
    
    std::cout << "Test directory  : " << test_dir << std::endl;
    std::cout << "Test directory 2: " << test_dir2 << std::endl;

    triedb::erase_all(test_dir);
    triedb::erase_all(test_dir2);

    triedb db(test_dir);

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
    // at_root = db.find_root(TEST_NUM_ENTRIES-1);

    // Extract 1/3rd of keys
    size_t n = TEST_NUM_ENTRIES / 3;
    merkle_root mr;
    auto it = db.begin(at_root, 0);
    for (size_t i = 0; i < n; i++, ++it) {
	it.add_current(mr, true);
    }

    std::cout << "Size of sub-tree: " << mr.total_size() << std::endl;
    bool r = mr.validate(db);
    std::cout << "Validated sub-tree: " << r << std::endl;

    // Let's create a new database and insert the sub-tree

    root_id at_root2;
    {
	triedb db2(test_dir2);
	at_root2 = db2.new_root();
	db2.update(at_root2, mr);
    }

    // Let's see if we can enumerate 1/3rd of the keys.
    triedb db2(test_dir2);
    triedb_iterator it2 = db2.begin(at_root2);
    triedb_iterator it2_end = db2.end(at_root2);
    size_t j;
    for (j = 0; it2 != it2_end; ++it2, ++j) {
	auto &leaf = *it2;
	assert(leaf.key() == j);
    }

    auto M = triedb_params::MAX_BRANCH;
    
    assert(j == ((n+M-1)/M)*M);

    // Get the remaining keys, and add them to DB2
    merkle_root mr2;
    auto it_end = db.end(at_root);
    for (; it != it_end; ++it) {
	it.add_current(mr2, true);
    }
    db2.update(at_root2, mr2);

    // Check that ALL keys are now available
    it2 = db2.begin(at_root2);
    it2_end = db2.end(at_root2);
    for (j = 0; it2 != it2_end; ++it2, ++j) {
	auto &leaf = *it2;
	assert(leaf.key() == j);
    }
    assert(j == TEST_NUM_ENTRIES);
}

int main(int argc, char *argv[])
{
    home_dir = find_home_dir(argv[0]);
    test_dir = home_dir;
    test_dir = (boost::filesystem::path(test_dir) / "bin" / "test" / "db" / "triedb_data").string();
    test_dir2 = home_dir;
    test_dir2 = (boost::filesystem::path(test_dir2) / "bin" / "test" / "db" / "triedb_data2").string();
    
    random::set_for_testing(true);
     
    test_basic();
    test_merkle();    

    return 0;
}
