#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <common/fast_hash.hpp>
#include <common/test/test_home_dir.hpp>
#include <node/address_book.hpp>
#include <node/self_node.hpp>

using namespace prologcoin::common;
using namespace prologcoin::node;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void next_address(ip_address &ip)
{
    size_t n = ip.bytes_size();
    for (size_t i = 0; i < n; i++) {
	uint8_t b = ++ip.to_bytes()[n-i-1];
	if (b != 0) {
	    break;
	}
    }
}

static void fill_with_random(address_book &book, size_t n)
{
    fast_hash h;
    h << 1234;

    utime t;

    static const char * comments [] = {
	"[].",
	"[gpu(123)].",
	"[foo,bar,baz(42)].",
	nullptr
    };

    ip_address self;

    uint8_t src_addr[4];
    for (size_t x = 0; x < 4; x++) {
	h << x;
	src_addr[x] = static_cast<uint8_t>(h);
    }
    ip_address src;
    src.set_addr(src_addr, 4);

    term_env env;

    size_t k = 0;
    bool ip4 = true;
    for (size_t i = 0; i < n;) {
	h << i;
	ip_address ip;
	if (ip4) {
	    ip.set_addr( static_cast<uint32_t>(h) );
	} else {
	    uint8_t addr[16] = {0x20,0x01,0x11,0x22};
	    for (size_t x = 4; x < 16; x++) {
		h << x;
		addr[x] = static_cast<uint8_t>(h);
	    }
	    ip.set_addr(addr, 16);
	}
	ip4 = !ip4;

	for (size_t j = 0; j < 8 && i < n; j++, i++) {
	    if (i % 100 == 0) {
		for (size_t x = 0; x < 4; x++) {
		    h << x;
		    src_addr[x] = static_cast<uint8_t>(h);
	        }
	        src.set_addr(src_addr, 4);
	    }

	    bool use_self = (i % 2) == 0;
	    address_entry e(ip, (use_self ? self : src),
			    self_node::DEFAULT_PORT);
	    e.set_time(t);
	    h << j;
	    e.set_score(static_cast<uint32_t>(h) % 10000);

	    if (i % 5 == 0) {
		e.set_comment( env, comments[k++] );
		if (comments[k] == nullptr) k = 0;
	    }

	    book.add(e);
	    next_address(ip);

	    t += utime::ss(1);
	}
    }
}

static void test_address_book_simple()
{
    header("test_address_book_simple");

    term_env env;

    address_book book;
    fill_with_random(book, 20);

    book.print(std::cout);

    const std::string &home_dir = find_home_dir();
    std::string outfile = home_dir + "/bin/test/node/address_book.txt";

    book.save(outfile);

    const std::string infile = outfile;

    address_book book2;
    book2.load(infile);

    // Address book that was read back in should be identical to the
    // address book written.

    assert(book == book2);
}

static void test_address_book_big()
{
    header("test_address_book_big");

    std::cout << "create book\n";

    utime t1 = utime::now();
    address_book book;
    fill_with_random(book, 10000);
    utime t2 = utime::now();
    std::cout << "(took " << (t2-t1).in_ms() << " milliseconds)" << std::endl;

    std::cout << "stat=" << book.stat() << std::endl;
    std::cout << "integrity check" << std::endl;
    book.integrity_check();

    const std::string &home_dir = find_home_dir();
    std::string outfile = home_dir + "/bin/test/node/address_book_big.txt";

    std::cout << "save book\n";

    t1 = utime::now();
    book.save(outfile);
    t2 = utime::now();
    std::cout << "(took " << (t2-t1).in_ms() << " milliseconds)" << std::endl;

    const std::string infile = outfile;

    std::cout << "load book\n";

    t1 = utime::now();
    address_book book2;
    book2.load(infile);
    t2 = utime::now();
    std::cout << "(took " << (t2-t1).in_ms() << " milliseconds)" << std::endl;
    book2.integrity_check();

    // Address book that was read back in should be identical to the
    // address book written.

    std::cout << "compare books\n";

    t1 = utime::now();
    assert(book == book2);
    t2 = utime::now();
    std::cout << "(took " << (t2-t1).in_ms() << " milliseconds)" << std::endl;

    std::cout << std::endl;
    std::cout << "The 10 entries from top 10%:" << std::endl << std::endl;
    auto top = book.get_randomly_from_top_10_pt(10);
    book.print(std::cout, top);
    size_t top_score = 0;
    for (auto &e : top) {
	top_score += e.score();
    }
    auto avg_score = top_score / top.size();
    std::cout << std::endl;
    std::cout << "Average score: " << avg_score << std::endl << std::endl;
    assert(avg_score >= 8000);

    std::cout << "Get 10 entries from the bottom 90%:" << std::endl << std::endl;
    auto bot = book.get_randomly_from_bottom_90_pt(10);
    book.print(std::cout, bot);
    size_t bot_score = 0;
    for (auto &e : bot) {
	bot_score += e.score();
    }
    avg_score = bot_score / bot.size();
    std::cout << std::endl;
    std::cout << "Average score: " << avg_score << std::endl << std::endl;

    std::cout << "Get 10 entries from unverified:" << std::endl << std::endl;
    auto unv = book.get_randomly_from_unverified(10);
    book.print(std::cout, unv);

}

static void test_address_book_medium()
{
    header("test_address_book_medium");

    std::cout << "Create book with 100 entries. Top 10% = 10 entries." << std::endl;

    address_book book;
    fill_with_random(book, 100);
    book.integrity_check();

    std::cout << "Get 50 entries from top 10%. Will fail..." << std::endl;

    auto r = book.get_randomly_from_top_10_pt(50);
    std::cout << "Got " << r.size() << " entries out of 50." << std::endl;

    assert(r.size() <= 10);
}


// 
// If many addresses come from the same source (possibly multiple IP
// addresses from the same network), then at most 100 addresses will
// be accumulated before spilling.
//
namespace prologcoin { namespace node {
class test_address_book {
public:
static void test_address_book_spilling()
{
    header("test_address_book_spilling");

    ip_address ip("192.168.1.1");
    ip_address src("127.0.0.1");

    address_book book;

    size_t n = 1000;

    std::cout << "Create " << n << " entries in the same network..." << std::endl;
    for (size_t i = 0; i < 1000; i++) {
	address_entry e(ip, src, 1234);
	book.add(e);
	next_address(ip);
    }
    book.integrity_check();

    std::cout << book.stat() << std::endl;

    assert(book.size() == address_book::MAX_GROUP_SIZE);
    assert(book.num_unverified() == book.size());
    assert(book.num_spilled() == n - book.size());

    // Then keep adding with different sources

    std::cout << "Create " << n << " entries with different sources..." << std::endl;

    for (size_t i = 0; i < n; i++) {
	address_entry e(ip, src, 1234);
	book.add(e);
	next_address(ip);
	next_address(src);
    }
    book.integrity_check();

    std::cout << book.stat() << std::endl;

    assert(book.size() == address_book::MAX_GROUP_SIZE);
    assert(book.num_spilled() == 2*n - book.size());
    assert(book.num_unverified() == book.size());

    // Then add some verified entries (= entries with no source)

    std::cout << "Create " << n << " verified entries from same network..." << std::endl;
    
    ip_address nosrc;
    int max_score = 0;
    for (size_t i = 0; i < n; i++) {
	address_entry e(ip, nosrc, 1234);
	e.set_score(100+10*i);
	max_score = std::max(e.score(), max_score);
	book.add(e);
	next_address(ip);
    }
    book.integrity_check();
    std::cout << book.stat() << std::endl;

    // Heavy spilling should not affect the top entries...
    std::cout << "max_score=" << max_score << std::endl;
    auto top = book.get_from_top_10_pt(10);
    book.print(std::cout, top);
    size_t sc = max_score;
    for (auto &e : top) {
	assert(e.score() == sc);
	sc -= 10;
    }
}
};

}}

int main(int argc, char *argv[] )
{
    find_home_dir(argv[0]);

    test_address_book_simple();
    test_address_book_big();
    test_address_book_medium();
    test_address_book::test_address_book_spilling();

    return 0;
}
