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

extern "C" void DebugBreak();

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
    for (size_t i = 0; i < n; i++) {
        if (i >= 9990) {
	    db.set_debug(true);
	}
        auto *leaf = db.find(n, entries[i]);
	if (leaf == nullptr) {
	    std::cout << "Could not find entry #" << i << ": key=" << entries[i] << std::endl;
	    assert(leaf != nullptr && leaf->key() == entries[i]);
	}
    }

    db.set_debug(false);
    
    std::cout << "Compare with expected baseline..." << std::endl;
	
    size_t i = 0;
	
    triedb_iterator it = db.begin(n);
    triedb_iterator it_end = db.end(n);
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
    
    it = db.begin(n/2 - 1);
    it_end = db.end(n/2 - 1);
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
        triedb db(params, test_dir);

	if (!db.is_empty()) {
	    std::cout << "Removing existing test database." << std::endl;
	    db.erase_all();
	}
			  
	// Set block sizes to be smaller to create more files...
	
	std::cout << "Create " << TEST_NUM_ENTRIES << " entries..." << std::endl;

	std::cout << "Insert entries..." << std::endl;
	
        for (size_t i = 0; i < TEST_NUM_ENTRIES; i++) {
	    db.insert(i, entries[i], nullptr, 0);
	}

	test_basic_check(db, entries, TEST_NUM_ENTRIES);
	
    } catch (triedb_exception &ex) {
        std::cout << "Exception: " << ex.what() << std::endl;
	throw ex;
    }

    try { 
        triedb db(params, test_dir);
	// DebugBreak();
	test_basic_check(db, entries, TEST_NUM_ENTRIES);
    } catch (triedb_exception &ex) {
        std::cout << "Exception: " << ex.what() << std::endl;
	throw ex;      
    }

    delete [] entries;
}

int main(int argc, char *argv[])
{
    home_dir = find_home_dir(argv[0]);
    test_dir = home_dir;
    test_dir = (boost::filesystem::path(test_dir) / "bin" / "test" / "db" / "triedb").string();

    random::set_for_testing(true);
     
    test_basic();

    return 0;
}
