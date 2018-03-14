#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <node/address_book.hpp>

using namespace prologcoin::node;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static void test_ip4_addresses()
{                                //  v l r   1 2 3 5 6
    header("ip4_addresses");     //  a o o   9 5 9 7 5
                                 //  l c u t 1 4 2 3 9
                                 //  i a t o 8 4 7 7 8
                                 //  d l e r
    static const char *ip_matrix[]
	      = { "127.0.0.1"     , "1 1 0 0 0 0 0 0 0",
                  "192.168.1.1"   , "1 0 0 0 1 0 0 0 0",
                  "10.0.0.5"      , "1 0 0 0 1 0 0 0 0",
                  "192.18.5.5"    , "1 0 0 0 0 1 0 0 0",
                  "169.254.5.5"   , "1 0 0 0 0 0 1 0 0",
                  "192.0.2.0"     , "1 0 0 0 0 0 0 1 0",
                  "198.51.100.1"  , "1 0 0 0 0 0 0 1 0",
                  "203.0.113.1"   , "1 0 0 0 0 0 0 1 0",
                  "100.64.1.1"    , "1 0 0 0 0 0 0 0 1",

		   nullptr
                };

    typedef bool (ip_address::*predicate_fn)() const;

    static const size_t NUM_PREDICATES = 19;

    static const predicate_fn predicate [NUM_PREDICATES] = {
	&ip_address::is_valid,
	&ip_address::is_local,
	&ip_address::is_routable,
	&ip_address::is_tor,
	&ip_address::is_rfc_1918,
	&ip_address::is_rfc_2544,
	&ip_address::is_rfc_3927,
	&ip_address::is_rfc_5737,
	&ip_address::is_rfc_6598,
	&ip_address::is_v6,
	&ip_address::is_rfc_3849,
	&ip_address::is_rfc_3964,
	&ip_address::is_rfc_4193,
	&ip_address::is_rfc_4380,
	&ip_address::is_rfc_4843,
	&ip_address::is_rfc_4862,
	&ip_address::is_rfc_6052,
	&ip_address::is_rfc_6145,
	&ip_address::is_rfc_7343
    };

    static const char * predicate_name[NUM_PREDICATES] = {
	"is_valid",
	"is_local",
	"is_routable",
	"is_tor",
	"is_rfc_1918",
	"is_rfc_2544",
	"is_rfc_3927",
	"is_rfc_5737",
	"is_rfc_6598",
	"is_v6",
	"is_rfc_3849",
	"is_rfc_3964",
	"is_rfc_4193",
	"is_rfc_4380",
	"is_rfc_4843",
	"is_rfc_4862",
	"is_rfc_6052",
	"is_rfc_6145",
	"is_rfc_7343"
    };

    std::string nl = "\n";
    std::string prefix = "|" + std::string(29,' ');

    std::cout << prefix << "        r r r r r" << nl;
    std::cout << prefix << "        f f f f f" << nl;
    std::cout << prefix << "v l r   c c c c c" << nl;
    std::cout << prefix << "a o o   1 2 3 5 6" << nl;
    std::cout << prefix << "l c u t 9 5 9 7 5" << nl;
    std::cout << prefix << "i a t o 1 4 2 3 9" << nl;
    std::cout << prefix << "d l e r 8 4 7 7 8" << nl;

    for (size_t i = 0; ip_matrix[i] != nullptr; i += 2) {
	std::string addr = ip_matrix[i];
	std::string flags = ip_matrix[i+1];
	std::cout << "Check: " << std::setw(20) << addr << "   " << flags << std::endl;

	ip_address ip_addr(addr);

	size_t predicate_index = 0;
	for (char ch : flags) {
	    if (ch == ' ') continue;
	    bool r = (ip_addr.*(predicate[predicate_index]))();
	    auto *name = predicate_name[predicate_index];
	    bool v = r ? ch == '1' : ch == '0';
	    if (!v) {
		std::cout << "Error: Failed to verify predicate " << name << "\n";
		std::cout << "Expect: " << ch << "\n";
		std::cout << "Actual: " << (r ? "1" : "0") << "\n";
	    }
	    assert(v);
	    predicate_index++;
	}
	for (; predicate_index < NUM_PREDICATES; predicate_index++) {
	    bool r = (ip_addr.*(predicate[predicate_index]))();
	    auto *name = predicate_name[predicate_index];
	    if (r) {
		std::cout << "Error: Failed to verify predicate " << name << "\n";
		std::cout << "Expect: 0 (because it's an IPv6 predicate.\n";
		std::cout << "Actual: " << (r ? "1" : "0") << "\n";
	    }
	    assert(!r);
	}
    }
}

static void test_ip6_addresses()
{

static const char *ip_matrix[]
  = { "2001:0db8:85a3:0000:0000:8a2e:0370:7334", "0 0 0 0 1 1 0 0 0 0 0 0 0 0",
      "2002:0900:00ff::bbbb"                   , "1 0 1 0 1 0 1 0 0 0 0 0 0 0",
      "fc00:1234:5678::cccc"                   , "1 0 0 0 1 0 0 1 0 0 0 0 0 0",
      "2001:0000:4136:e378:8000:63bf:3fff:fdd2", "1 0 1 0 1 0 0 0 1 0 0 0 0 0",
      "2001:0010:0001:0002:0003:0004:0005:0006", "1 0 0 0 1 0 0 0 0 1 0 0 0 0",
      "FE80:0000:0000:0000:0001:0002:0003:0004", "1 0 0 0 1 0 0 0 0 0 1 0 0 0",
      "0064:FF9B::8.8.8.8",                      "1 0 1 0 1 0 0 0 0 0 0 1 0 0",
      "::FFFF:0:8.8.8.8",                        "1 0 1 0 1 0 0 0 0 0 0 0 1 0",
      "2001:0020:0001:0002:0003:0004:0005:0006", "1 0 0 0 1 0 0 0 0 0 0 0 0 1",
       nullptr
    };

    typedef bool (ip_address::*predicate_fn)() const;

    static const size_t NUM_PREDICATES = 19;

    static const predicate_fn predicate [NUM_PREDICATES] = {
	&ip_address::is_valid,
	&ip_address::is_local,
	&ip_address::is_routable,
	&ip_address::is_tor,
	&ip_address::is_v6,
	&ip_address::is_rfc_3849,
	&ip_address::is_rfc_3964,
	&ip_address::is_rfc_4193,
	&ip_address::is_rfc_4380,
	&ip_address::is_rfc_4843,
	&ip_address::is_rfc_4862,
	&ip_address::is_rfc_6052,
	&ip_address::is_rfc_6145,
	&ip_address::is_rfc_7343,
	&ip_address::is_rfc_1918,
	&ip_address::is_rfc_2544,
	&ip_address::is_rfc_3927,
	&ip_address::is_rfc_5737,
	&ip_address::is_rfc_6598
    };

    static const char * predicate_name[NUM_PREDICATES] = {
	"is_valid",
	"is_local",
	"is_routable",
	"is_tor",
	"is_v6",
	"is_rfc_3849",
	"is_rfc_3964",
	"is_rfc_4193",
	"is_rfc_4380",
	"is_rfc_4843",
	"is_rfc_4862",
	"is_rfc_6052",
	"is_rfc_6145",
	"is_rfc_7343",
	"is_rfc_1918",
	"is_rfc_2544",
	"is_rfc_3927",
	"is_rfc_5737",
	"is_rfc_6598"
    };

    std::string nl = "\n";
    std::string prefix = "|" + std::string(48,' ');

                                              //  v l r     3 3 4 4 4 4 6 6 7
    header("ip6_addresses");                  //  a o o     8 9 1 3 8 8 0 1 3
                                              //  l c u t i 4 6 9 8 4 6 5 4 4
                                              //  i a t o p 9 4 3 0 3 2 2 5 3
                                              //  d l e r 6

    std::cout << prefix << "          r r r r r r r r r" << nl;
    std::cout << prefix << "          f f f f f f f f f" << nl;
    std::cout << prefix << "v l r     c c c c c c c c c" << nl;
    std::cout << prefix << "a o o     3 3 4 4 4 4 6 6 7" << nl;
    std::cout << prefix << "l c u t i 8 9 1 3 8 8 0 1 3" << nl;
    std::cout << prefix << "i a t o p 4 6 9 8 4 6 5 4 4" << nl;
    std::cout << prefix << "d l e r 6 9 4 3 0 3 2 2 5 3" << nl;

    for (size_t i = 0; ip_matrix[i] != nullptr; i += 2) {
	std::string addr = ip_matrix[i];
	std::string flags = ip_matrix[i+1];
	std::cout << "Check: " << std::setw(39) << addr << "   " << flags << std::endl;

	ip_address ip_addr(addr);

	size_t predicate_index = 0;
	for (char ch : flags) {
	    if (ch == ' ') continue;
	    bool r = (ip_addr.*(predicate[predicate_index]))();
	    auto *name = predicate_name[predicate_index];
	    bool v = r ? ch == '1' : ch == '0';
	    if (!v) {
		std::cout << "Error: Failed to verify predicate " << name << "\n";
		std::cout << "Expect: " << ch << "\n";
		std::cout << "Actual: " << (r ? "1" : "0") << "\n";
	    }
	    assert(v);
	    predicate_index++;
	}
	for (; predicate_index < NUM_PREDICATES; predicate_index++) {
	    bool r = (ip_addr.*(predicate[predicate_index]))();
	    auto *name = predicate_name[predicate_index];
	    if (r) {
		std::cout << "Error: Failed to verify predicate " << name << "\n";
		std::cout << "Expect: 0 (because it's an IPv6 predicate.\n";
		std::cout << "Actual: " << (r ? "1" : "0") << "\n";
	    }
	    assert(!r);
	}
    }
}

static void test_ip_groups()
{
    static const char *addrs[]
  = { "2001:0db8:85a3:0000:0000:8a2e:0370:7334", "0.0.0.0",
      "2002:0900:00ff::bbbb"                   , "9.0.0.0",
      "fc00:1234:5678::cccc"                   , "0.0.0.0",
      "2001:0000:4136:e378:8000:63bf:3fff:fdd2", "0.0.0.1",
      "2001:0010:0001:0002:0003:0004:0005:0006", "0.0.0.0",
      "FE80:0000:0000:0000:0001:0002:0003:0004", "0.0.0.0",
      "0064:FF9B::8.8.8.8",                      "8.8.0.0",
      "::FFFF:0:8.8.8.8",                        "8.8.0.0",
      "2001:0020:0001:0002:0003:0004:0005:0006", "0.0.0.0",
      "2001:0DB8:1234:5678:0001:0002:0003:0004", "0.0.0.0",
      "2a03:0f80:ed16:0ca7:ea75:b12d:02af:9e2a", "42.3.15.128",
      nullptr
    };

    header("test_ip_groups");

    for (size_t i = 0; addrs[i] != nullptr; i += 2) {
	std::string addr = addrs[i];
	std::string expect = addrs[i+1];
	ip_address ip(addr);
	std::string addr2 = ip.str();
	std::string group = ip_address::group_str(ip.group());
	std::cout << "Check: " << std::setw(40) << addr << " -- " << group << std::endl;
	std::cout << "Expect:" << std::setw(40) << addr2 << " -- " << expect << std::endl;
	assert(group == expect);
    }
}

int main(int argc, char *argv[])
{
    test_ip4_addresses();
    test_ip6_addresses();
    test_ip_groups();
    return 0;
}
