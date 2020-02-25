#include <common/test/test_home_dir.hpp>
#include <common/utime.hpp>
#include <common/merkle_trie.hpp>
#include <common/hex.hpp>
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

const size_t TEST_NUM_BLOCKS = 1000;
const size_t BLOCK_SIZE = 1024;

static void check_blocks_1(blockdb &db)
{
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
}

static void check_blocks_2(blockdb &db)
{
    // Let's check at what heights blocks are available at
    // Block 0 is available at all heights
    // Block 1 is available at all heights starting from height 1, etc
    for (size_t height = 0; height < TEST_NUM_BLOCKS; height++) {
        // Check maximum block index at height
	for (size_t block_no = 0; block_no < TEST_NUM_BLOCKS; block_no++) {
	    auto *blk = db.find_block(block_no, height);
	    if (block_no > height) {
	        // Should not exist
	        assert(blk == nullptr);
	    } else {
	        // Should exist
	        assert(blk != nullptr);
		// Index is introuced at the same height for this example.
		assert(blk->height() == blk->block_index());
	    }
	}
    }
}

static void check_blocks_3(blockdb &db)
{
    // Verify nothing has changed at height TEST_NUM_BLOCKS-1
    for (size_t block_no = 0; block_no < TEST_NUM_BLOCKS; block_no++) {
        auto *blk = db.find_block(block_no, TEST_NUM_BLOCKS-1);
	assert(blk->height() == blk->block_index());
    }
}

static void check_blocks_4(blockdb &db)
{
    // Verify that only block 10 has changed at height TEST_NUM_BLOCKS
    for (size_t block_no = 0; block_no < TEST_NUM_BLOCKS; block_no++) {
        auto *blk = db.find_block(block_no, TEST_NUM_BLOCKS+1);
	if (block_no == 10) {
	    assert(blk->height() == TEST_NUM_BLOCKS);
	    assert(reinterpret_cast<uint8_t *>(blk->data())[0] == 0x55);
	} else {
	    assert(blk->height() == blk->block_index());
	}
    }
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

	db.set_block_size(BLOCK_SIZE);
	db.set_bucket_size(16);
	
	std::cout << "Create blocks up to height " << TEST_NUM_BLOCKS << std::endl;
	uint8_t data[BLOCK_SIZE];
	for (size_t height = 0; height < TEST_NUM_BLOCKS; height++) {
	    memset(data, static_cast<uint8_t>(height & 0xff), BLOCK_SIZE);
	    db.new_block(data, BLOCK_SIZE, height, height);
	}
	db.flush();

	std::cout << "Check blocks..." << std::endl;

	check_blocks_1(db);
	check_blocks_2(db);
	
	// Let's modify a previous block at height TEST_NUM_BLOCKS
	// (i.e. one height beyond the current height)
	memset(data, 0x55, BLOCK_SIZE);
	db.new_block(data, BLOCK_SIZE, 10, TEST_NUM_BLOCKS);

	check_blocks_3(db);
	check_blocks_4(db);	

	// At this point we'll add 1000 new blocks at next height.
	size_t use_height = TEST_NUM_BLOCKS+1;
	size_t use_start_block_index = use_height;
	size_t num_extra_blocks = TEST_NUM_BLOCKS;
	for (size_t i = 0; i < num_extra_blocks; i++) {
  	    memset(data, static_cast<uint8_t>((0x80+i) & 0xff), BLOCK_SIZE);
	    db.new_block(data, BLOCK_SIZE, use_start_block_index+i,
			 use_height);
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

