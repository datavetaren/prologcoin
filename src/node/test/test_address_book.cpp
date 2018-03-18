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

    term_env env;

    size_t k = 0;
    for (size_t i = 0; i < n;) {
	h << i;
	ip_address ip;
	if (i % 2 == 0) {
	    ip.set_addr( static_cast<uint32_t>(h) );
	} else {
	    uint8_t addr[16] = {0x20,0x01,0x11,0x22};
	    for (size_t x = 4; x < 16; x++) {
		h << x;
		addr[x] = static_cast<uint8_t>(h);
	    }
	    ip.set_addr(addr, 16);
	}

	for (size_t j = 0; j < 8 && i < n; j++, i++) {
	    address_entry e(ip, self_node::DEFAULT_PORT);
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

static void test_address_book()
{
    header("test_address_book");

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

    // Address book that was read back in should be identical to the
    // address book written.

    std::cout << "compare books\n";

    t1 = utime::now();
    assert(book == book2);
    t2 = utime::now();
    std::cout << "(took " << (t2-t1).in_ms() << " milliseconds)" << std::endl;
}

int main(int argc, char *argv[] )
{
    find_home_dir(argv[0]);

    test_address_book();
    test_address_book_big();

    return 0;
}
