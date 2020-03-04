#include <common/test/test_home_dir.hpp>
#include <common/utime.hpp>
#include <common/sha1.hpp>
#include <common/hex.hpp>
#include <common/random.hpp>
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
    uint8_t hash[20];
};

struct custom_data_branch {
    uint8_t hash[20];
};

class custom_branch : public triedb_branch {
public:
    const uint8_t * hash() const {
        return custom_data();
    }
    void recompute_hash(triedb &db) {
        sha1 hasher;
        for (size_t i = 0; i < triedb_params::MAX_BRANCH; i++) {
	    if (is_branch(i)) {
	        auto *sub_branch = db.get_branch(this, i);
		hasher.update(sub_branch->custom_data(),
			      sub_branch->custom_data_size());
	    } else if (is_leaf(i)) {
	        auto *sub_leaf = db.get_leaf(this, i);
		auto *data_leaf = reinterpret_cast<custom_data_leaf *>(sub_leaf->custom_data());
		hasher.update(data_leaf->hash, sizeof(data_leaf->hash));
	    }
        }
	if (custom_data() == nullptr) {
	    allocate_custom_data(sizeof(custom_data_branch));
	}
    
	hasher.finalize(custom_data());
    }
};

static void test_basic()
{
    header("test_basic");
    
    std::cout << "Test directory: " << test_dir << std::endl;

    triedb db(test_dir);
    db.erase_all();
    
    db.set_update_function(
	   [](triedb &db0, triedb_branch &branch) {
	       auto *br = reinterpret_cast<custom_branch *>(&branch);
	       br->recompute_hash(db0);
	   });

    std::vector<std::pair<triedb_branch *, size_t> > path;
    
    for (size_t i = 0; i < TEST_NUM_ENTRIES; i++) {
        custom_data_leaf data;
	for (size_t j = 0; j < 65536; j++) {
	    uint8_t b = (i + j) & 0xff;
	    data.heap_block[j] = b;
	}
	sha1 hasher;
	hasher.update(&data.heap_block[0], 65536);
	hasher.finalize( &data.hash[0] );
        db.insert(i, i, reinterpret_cast<uint8_t *>(&data), sizeof(data));	
	/*
	db.find(i, 0, &path);
	auto *root = reinterpret_cast<const custom_data_branch *>(path[0].first->custom_data());

	std::cout << "Insert " << i << ": ";
	std::cout << "Root Hash: " << hex::to_string(&root->hash[0], sizeof(root->hash));
	std::cout << std::endl;
	*/
    }

    // Check root hash
    db.find(TEST_NUM_ENTRIES, 0, &path);

    hash_t actual_root_hash;

    std::cout << "PATH: ";
    bool first = true;
    std::pair<triedb_branch *, size_t> *root_hash;    
    std::pair<triedb_branch *, size_t> *last_e;
    for (auto &e : path) {
        last_e = &e;
        auto *br = reinterpret_cast<custom_branch *>(e.first);
	if (first) {
	    memcpy(&actual_root_hash.hash[0], br->custom_data(), sizeof(hash_t));
	}
	if (!first) std::cout << " -> ";
	std::cout << e.second << "[" << hex::to_string(br->hash(), 20) << "]";
	first = false;
    }
    auto *lf = reinterpret_cast<custom_data_leaf *>(db.get_leaf(last_e->first,last_e->second)->custom_data());
    std::cout << " -> " << hex::to_string(lf->hash, 20);
    std::cout << std::endl;

    //
    // Compute the root hash a priori
    //
    std::cout << "Compute root hash using first principles..." << std::endl;
    std::vector<hash_t> hashes;
    for (size_t i = 0; i < TEST_NUM_ENTRIES; i++) {
      uint8_t data[65536];
      for (size_t j = 0; j < 65536; j++) {
	  data[j] = static_cast<uint8_t>((i+j) & 0xff);
      }
      sha1 hasher;
      hasher.update(data, 65536);
      hash_t hash;
      hasher.finalize(&hash.hash[0]);
      hashes.push_back(hash);
    }
    while (hashes.size() > 1) {
        std::vector<hash_t> next_hashes;
	size_t i = 0;
	sha1 hasher;
	for (auto &h : hashes) {
	    hasher.update(&h.hash[0], sizeof(hash_t));
	    i++;
	    if (i == 32) {
	        hash_t next_hash;
	        hasher.finalize(&next_hash.hash[0]);
	        next_hashes.push_back(next_hash);
		hasher.init();
		i = 0;
	    }
	}
	hashes = next_hashes;
    }
    hash_t &expect_root_hash = hashes[0];
    std::cout << "Actual root hash: " << hex::to_string(&actual_root_hash.hash[0], sizeof(hash_t)) << std::endl;
    std::cout << "Expect root hash: " << hex::to_string(&expect_root_hash.hash[0], sizeof(hash_t)) << std::endl;
    assert(expect_root_hash == actual_root_hash);
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
