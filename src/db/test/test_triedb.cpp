#include <common/test/test_home_dir.hpp>
#include <common/utime.hpp>
#include <common/merkle_trie.hpp>
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

const size_t TEST_NUM_ENTRIES = 10000;

static void test_basic_check(triedb &db, uint64_t entries[], size_t n)
{
    uint64_t *sorted_entries(new uint64_t[n]);
    uint64_t *half_sorted_entries(new uint64_t[n/2]);
    std::copy(entries, entries + n, sorted_entries);
    std::copy(entries, entries + n/2, half_sorted_entries);	
    std::sort(&sorted_entries[0], &sorted_entries[0] + n);
    std::sort(&half_sorted_entries[0], &half_sorted_entries[0] + n/2);
  
    std::cout << "Check if entries can be found through direct query..." << std::endl;
    auto n_root = db.find_root(n-1);
    for (size_t i = 0; i < n; i++) {
        if (i >= 9990) {
	    db.set_debug(true);
	}
        auto *leaf = db.find(n_root, entries[i]);
	if (leaf == nullptr) {
	    std::cout << "Could not find entry #" << i << ": key=" << entries[i] << std::endl;
	    assert(leaf != nullptr && leaf->key() == entries[i]);
	}
    }

    db.set_debug(false);
    
    std::cout << "Compare with expected baseline..." << std::endl;
    
    size_t i = 0;

    triedb_iterator it = db.begin(n_root);
    triedb_iterator it_end = db.end(n_root);
    for (; it != it_end; ++it, ++i) {
        auto &leaf = *it;
	uint64_t expect_key = sorted_entries[i];
	if (expect_key != leaf.key()) {
	    std::cout << "Error at index " << i << std::endl;
	    std::cout << "Actual key: " << leaf.key() << std::endl;
	    std::cout << "Expect key: " << expect_key << std::endl;
	    assert(expect_key == leaf.key());
	}
    }

    std::cout << "Compare with expected baseline at half the height..." << std::endl;
	
    // Check how the triedb looks at half height
    // (only half of the entries are inserted)
    
    auto n2_root = db.find_root(n/2 -1);
    it = db.begin(n2_root);
    it_end = db.end(n2_root);
    i = 0;
    for (; it != it_end; ++it, ++i) {
        auto &leaf = *it;
	uint64_t expect_key = half_sorted_entries[i];
	if (expect_key != leaf.key()) {
	    std::cout << "Error at index " << i << std::endl;
	    std::cout << "Actual key: " << leaf.key() << std::endl;
	    std::cout << "Expect key: " << expect_key << std::endl;
	    assert(expect_key == leaf.key());
	}
    }

    //
    // Reverse iteration
    //
    std::cout << "Reverse iteration check..." << std::endl;
    auto n1_root = db.find_root(n-1);
    it_end = db.end(n1_root);
    it = it_end -1 ;
    i = n - 1;
    for (; it != it_end; --it, --i) {
        auto &leaf = *it;
	uint64_t expect_key = sorted_entries[i];
	if (expect_key != leaf.key()) {
	    std::cout << "Error at index " << i << std::endl;
	    std::cout << "Actual key: " << leaf.key() << std::endl;
	    std::cout << "Expect key: " << expect_key << std::endl;
	    assert(expect_key == leaf.key());
	}
    }

    //
    // Reverse iteration
    //
    std::cout << "Reverse iteration check from half height..." << std::endl;
    n2_root = db.find_root(n/2-1);
    it_end = db.end(n2_root);
    it = it_end - 1;
    i = n/2 - 1;
    for (; it != it_end; --it, --i) {
        auto &leaf = *it;
	uint64_t expect_key = half_sorted_entries[i];
	if (expect_key != leaf.key()) {
	    std::cout << "Error at index " << i << std::endl;
	    std::cout << "Actual key: " << leaf.key() << std::endl;
	    std::cout << "Expect key: " << expect_key << std::endl;
	    assert(expect_key == leaf.key());
	}
    }

    std::cout << "Nearby searching..." << std::endl;
    //
    // Start iteration from key - 10000
    // check 10 elements (or end of iteration)
    //
    n1_root = db.find_root(n-1);
    it_end = db.end(n1_root);
    for (i = 0; i < n; i++) {
       uint64_t from_key = sorted_entries[i] - 10000;
       it = db.begin(n1_root, from_key);
       auto it_expect = std::lower_bound(&sorted_entries[0], &sorted_entries[n], from_key);
       for (size_t j = 0; j < 10; j++, ++it, ++it_expect) {
	   if (it == it_end) {
	       break;
  	   }
	   if (it->key() != *it_expect) {
	       std::cout << "Error at index " << i << "," << j << std::endl;
	       std::cout << "Actual key: " << it->key() << std::endl;
	       std::cout << "Expect key: " << *it_expect << std::endl;
	       assert(it->key() == *it_expect);
	   }
	}
    }
    
    delete [] sorted_entries;
    delete [] half_sorted_entries;
}

static void test_basic()
{
    header("test_basic");
    
    std::cout << "Test directory: " << test_dir << std::endl;

    uint64_t *entries(new uint64_t[TEST_NUM_ENTRIES]);
    for (size_t i = 0; i < TEST_NUM_ENTRIES; i++) {
        entries[i] = random::next_int(static_cast<uint64_t>(100000000));
    }

    triedb_params params;
    params.set_bucket_size(65536);
    params.set_cache_num_streams(4);
    params.set_cache_num_nodes(1024);
    
    try {
	triedb::erase_all(test_dir);
			  
        triedb db(params, test_dir);

	// Create genesis
	root_id root = db.new_root();

	// Set block sizes to be smaller to create more files...
	
	std::cout << "Create " << TEST_NUM_ENTRIES << " entries..." << std::endl;

	std::cout << "Insert entries..." << std::endl;
	
        for (size_t i = 0; i < TEST_NUM_ENTRIES; i++) {
	    db.insert(root, entries[i], nullptr, 0);
	    root = db.new_root(root);
	}

	test_basic_check(db, entries, TEST_NUM_ENTRIES);
	
    } catch (triedb_exception &ex) {
        std::cout << "Exception: " << ex.what() << std::endl;
	throw ex;
    }

    try { 
        triedb db(test_dir);
	db.set_cache_num_streams(4);
	db.set_cache_num_nodes(1024);	
	test_basic_check(db, entries, TEST_NUM_ENTRIES);
    } catch (triedb_exception &ex) {
        std::cout << "Exception: " << ex.what() << std::endl;
	throw ex;      
    }

    delete [] entries;
}

static void test_increasing_check(triedb &db, size_t n)
{
    std::cout << "Testing finding keys directly..." << std::endl;
    for (size_t i = 0; i < n; i++) {
	auto i_root = db.find_root(i);
        auto *leaf = db.find(i_root, i);
	assert(leaf->key() == i);
	auto *not_found = db.find(i_root, i+1);
	assert(not_found == nullptr);
    }

    std::cout << "Iteration check..." << std::endl;
    auto n_root = db.find_root(n);
    auto it = db.begin(n_root, 0);
    auto it_end = db.end(n_root);
    size_t i = 0;
    for (; it != it_end; ++it, ++i) {
        auto &leaf = *it;
	if (leaf.key() != i) {
	    std::cout << "Error at index " << i << std::endl;
	    std::cout << "Actual key: " << leaf.key() << std::endl;
	    std::cout << "Expect key: " << i << std::endl;
	}
	assert(leaf.key() == i);
    }

    std::cout << "Reverse iteration check..." << std::endl;
    it_end = db.end(n_root);
    it = it_end - 1;
    i = 9999;
    for (; it != it_end; --it, --i) {
        auto &leaf = *it;
	if (leaf.key() != i) {
	    std::cout << "Error at index " << i << std::endl;
	    std::cout << "Actual key: " << leaf.key() << std::endl;
	    std::cout << "Expect key: " << i << std::endl;
	}
	assert(leaf.key() == i);        
    }
}

static void test_increasing()
{
    header("test_increasing");
    
    std::cout << "Test directory: " << test_dir << std::endl;
    std::cout << "Remove any exisint files..." << std::endl;
    triedb::erase_all(test_dir);

    triedb_params params;
    params.set_bucket_size(65536);
    params.set_cache_num_streams(4);
    params.set_cache_num_nodes(1024);
    
    try {
        triedb db(params, test_dir);
	std::cout << "Create " << TEST_NUM_ENTRIES << " entries as 0, 1, 2, 3, ..." << std::endl;

	std::cout << "Insert entries..." << std::endl;
	
	auto at_root = db.new_root();

        for (size_t i = 0; i < TEST_NUM_ENTRIES; i++) {
	    db.insert(at_root, i, nullptr, 0);
	    at_root = db.new_root(at_root);

	    // Zero presence check...
	
	    auto *zero_check = db.find(at_root, 0);
	    assert(zero_check->key() == 0);
	}

	test_increasing_check(db, TEST_NUM_ENTRIES);
	
    } catch (triedb_exception &ex) {
        std::cout << "Exception: " << ex.what() << std::endl;
	throw ex;
    }

    // Open the database again
    
    try { 
        triedb db(test_dir);
	db.set_cache_num_streams(4);
	db.set_cache_num_nodes(1024);	
	test_increasing_check(db, TEST_NUM_ENTRIES);
    } catch (triedb_exception &ex) {
        std::cout << "Exception: " << ex.what() << std::endl;
	throw ex;      
    }

    // Open the database again and remove every odd number

    try {
        size_t at_height = TEST_NUM_ENTRIES-1;
        triedb db(test_dir);
	auto at_root = db.find_root(at_height);
	std::cout << "Remove 1, 3, 5, ... up to 9999" << std::endl;
	for (size_t i = 1; i < TEST_NUM_ENTRIES; i += 2) {
	    db.remove(at_root, i);
	}
	std::cout << "Check database integrity..." << std::endl;

	for (size_t i = 0; i < TEST_NUM_ENTRIES; i++) {
	    auto *leaf = db.find(at_root, i);
	    if (i % 2 == 0) {
	        assert(leaf != nullptr && leaf->key() == i);
  	    } else {
	        assert(leaf == nullptr);
	    }
	}

	std::cout << "Remove a range of values..." << std::endl;

	for (size_t i = 1000; i < 2000; i += 2) {
	    db.remove(at_root, i);
	}

	std::cout << "Check database integrity again..." << std::endl;

	for (size_t i = 0; i < TEST_NUM_ENTRIES; i++) {
	    auto *leaf = db.find(at_root, i);
	    if (i % 2 == 0) {
	        if (i < 1000 || i >= 2000) {
		    assert(leaf != nullptr && leaf->key() == i);
	        } else {
		    assert(leaf == nullptr);
		}
  	    } else {
	        assert(leaf == nullptr);	      
	    }
	}

	std::cout << "Check database integrity through iteration..." << std::endl;
	auto it = db.begin(at_root, 0);
	auto it_end = db.end(at_root);

	size_t i = 0;
	for (; it != it_end; ++it) {
	    assert(it->key() == i);
	    ++i;
	    if (i % 2 == 1) ++i;
	    while (i >= 1000 && i < 2000) ++i;
	}
	assert(i == TEST_NUM_ENTRIES);

    } catch (triedb_exception &ex) {
        std::cout << "Exception: " << ex.what() << std::endl;
	throw ex;      
    }
    
}

int main(int argc, char *argv[])
{
    home_dir = find_home_dir(argv[0]);
    test_dir = home_dir;
    test_dir = (boost::filesystem::path(test_dir) / "bin" / "test" / "db" / "triedb").string();

    random::set_for_testing(true);
     
    test_basic();
    test_increasing();

    return 0;
}
