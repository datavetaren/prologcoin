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

static void test_basic()
{
    header("test_basic");
    
    std::cout << "Test directory: " << test_dir << std::endl;

    try {
        triedb db(test_dir);

	if (!db.is_empty()) {
	    std::cout << "Removing existing test database." << std::endl;
	    db.erase_all();
	}
			  
	// Set block sizes to be smaller to create more files...

	db.set_bucket_size(65536);
	db.set_cache_num_streams(4);
	db.set_cache_num_nodes(1024);
	
	std::cout << "Create " << TEST_NUM_ENTRIES << " entries..." << std::endl;

	uint64_t *entries = new uint64_t[TEST_NUM_ENTRIES];
	for (size_t i = 0; i < TEST_NUM_ENTRIES; i++) {
	    entries[i] = random::next_int(static_cast<uint64_t>(100000000));
	}

        uint64_t *sorted_entries = new uint64_t[TEST_NUM_ENTRIES];
        std::copy(entries, entries + TEST_NUM_ENTRIES, sorted_entries);
        std::sort(sorted_entries, sorted_entries + TEST_NUM_ENTRIES);

	std::cout << "Insert entries..." << std::endl;
	
        for (size_t i = 0; i < TEST_NUM_ENTRIES; i++) {
	    db.insert(i, entries[i], nullptr, 0);
	}

	std::cout << "Compare with expected baseline..." << std::endl;
	
	size_t i = 0;
	
	triedb_iterator it = db.begin(TEST_NUM_ENTRIES);
	triedb_iterator it_end = db.end(TEST_NUM_ENTRIES);
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

	delete [] entries;
	delete [] sorted_entries;
	
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

    return 0;
}
