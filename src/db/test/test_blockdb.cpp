#include <common/test/test_home_dir.hpp>
#include <common/utime.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <boost/filesystem.hpp>
#include <db/blockdb.hpp>

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

static void test_basic()
{
    header("test_basic");
    
    std::cout << "Test directory: " << test_dir << std::endl;

    try {
        blockdb db(test_dir);

	if (!db.is_empty()) {
	    std::cout << "Removing existing test database." << std::endl;
	    db.erase_all();
	}
			  
	// Set block sizes to be smaller to create more files...

	const size_t TEST_NUM_BLOCKS = 1000;
	
	const size_t BLOCK_SIZE = 1024;
	db.set_block_size(BLOCK_SIZE);
	db.set_bucket_size(16);
	
	std::cout << "Create blocks up to height " << TEST_NUM_BLOCKS << std::endl;
	uint8_t data[BLOCK_SIZE];
	for (size_t height = 0; height < TEST_NUM_BLOCKS; height++) {
	    for (size_t i = 0; i < BLOCK_SIZE; i++) {
	        data[i] = static_cast<uint8_t>(height);
	    }
	    db.new_block(data, BLOCK_SIZE, height, height);
	}
	db.flush();

	std::cout << "Check blocks..." << std::endl;
	
	// Let's check the integrity of those blocks
	for (size_t block_no = 0; block_no < TEST_NUM_BLOCKS; block_no++) {
	    auto *blk = db.find_block(block_no, block_no);
	    auto *data = reinterpret_cast<const uint8_t *>(blk->data());
	    for (size_t i = 0; i < BLOCK_SIZE; i++) {
	        if (data[i] != static_cast<uint8_t>(block_no)) {
		    std::cout << "Unexpected data. Expecting " << block_no << "; was " << data[i] << "(at i=" << i << ")" << std::endl;
	        }
		
	        assert(data[i] == static_cast<uint8_t>(block_no));
	    }
	}
	
    } catch (blockdb_exception &ex) {
        std::cout << "Exception: " << ex.what() << std::endl;
	throw ex;
    }
}

int main(int argc, char *argv[])
{
    home_dir = find_home_dir(argv[0]);
    test_dir = home_dir;
    test_dir = (boost::filesystem::path(test_dir) / "bin" / "test" / "db" / "blockdb").string();
     
    test_basic();

    return 0;
}

