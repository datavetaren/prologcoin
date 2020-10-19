#include <boost/filesystem.hpp>
#include <common/random.hpp>
#include <common/checked_cast.hpp>
#include <common/test/test_home_dir.hpp>
#include <common/term_tools.hpp>
#include <ec/builtins.hpp>
#include <global/global.hpp>

using namespace prologcoin::common;
using namespace prologcoin::interp;
using namespace prologcoin::global;
using namespace prologcoin::ec;

std::string home_dir;
std::string test_dir;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

static size_t check_list(term_env &env, term t, bool recheck)
{
    size_t cnt = 0;
    while (env.is_dotted_pair(t)) {
        term el = env.arg(t, 0);
	assert(el.tag() == tag_t::INT);
	auto ival = reinterpret_cast<int_cell &>(el).value();
	if (static_cast<size_t>(ival) == cnt) cnt++;
	t = env.arg(t, 1);
    }
    assert(t == heap::EMPTY_LIST);
    return cnt;
}

static void recheck_global_basic(const std::string &test_dir, size_t heap_ref_pos)
{
    static const size_t NUMEL = 65536;

    std::cout << "Rechecking by reopening database..." << std::endl;

    global g(test_dir);

    std::cout << "Current height: " << g.current_height() << std::endl;
    
    term_env &env = g.interp();
    term my_ref = env.heap_get(heap_ref_pos);

    std::cout << "Checking list in first heap element..." << std::endl;

    assert(check_list(env, my_ref, true) == NUMEL);
    std::cout << "List is " << NUMEL << " elements." << std::endl;
}

static size_t setup_global_basic(const std::string &test_dir)
{
    // Remove existing database. Otherwise test will be confused.
    global::erase_db(test_dir);

    global g(test_dir);

    std::cout << "Data directory: " << test_dir << std::endl;
    std::cout << "STATUS: " << g.env().status() << std::endl;

    // Let's create a long list of integers and make sure it is correctly
    // persistently stored.

    static const size_t NUMEL = 65536;
    
    term_env &env = g.interp();
    term my_ref = env.new_ref();

    term list_head = heap::EMPTY_LIST;
    term list_tail = list_head;
    for (size_t i = 0; i < 65536; i++) {
        term new_pair = env.new_dotted_pair(int_cell(i),
					    heap::EMPTY_LIST);
	if (list_head == heap::EMPTY_LIST) {
	    list_head = new_pair;
	    list_tail = new_pair;
	} else {
	    env.set_arg(list_tail, 1, new_pair);
	    list_tail = new_pair;
	}
    }
    g.increment_height();

    // Let's bind the first var to list head

    size_t ref_heap_pos = reinterpret_cast<ref_cell &>(my_ref).index();
    
    std::cout << "Bind ref " << ref_heap_pos << std::endl;
    
    g.interp().unify(my_ref, list_head);

    my_ref = env.deref(my_ref);

    std::cout << "Ref " << ref_heap_pos << " is now bound." << std::endl;

    g.increment_height();

    std::cout << "Heap size is: " << g.interp().get_heap().size() << std::endl;
    
    assert(check_list(env, my_ref, false) == NUMEL);
    std::cout << "List is " << NUMEL << " elements." << std::endl;

    return ref_heap_pos;
}

static void test_global_basic()
{
    header("test_global_basic");

    size_t heap_ref_pos = setup_global_basic(test_dir);
    recheck_global_basic(test_dir, heap_ref_pos);
}

static void recheck_frozen_closures(std::vector<size_t> &all_frozen_closures0)
{
    std::cout << "Recheck begin" << std::endl;

    global g(test_dir);

    std::cout << "Current height: " << g.current_height() << std::endl;
    std::cout << "Current heap size: " << g.interp().heap_size() << std::endl;

    auto it0 = all_frozen_closures0.begin();

    std::vector<size_t> all_frozen_closures;

    size_t last_addr = 0;
    for (size_t i = 0; i < all_frozen_closures0.size(); i += 100) {
	auto check_closures_cmd = g.interp().parse("frozenk(" + boost::lexical_cast<std::string>(last_addr) + ", 100, X).");
	std::cout << "COMMAND: " << g.interp().to_string(check_closures_cmd) << std::endl;
	g.interp().execute(check_closures_cmd);
	auto lst = g.interp().get_result_term("X");
	size_t j = 0;
	// std::cout << "List length: " << g.interp().list_length(lst) << std::endl;
	while (g.interp().is_dotted_pair(lst)) {
	    auto addr_term = g.interp().arg(lst, 0);
	    assert(addr_term.tag() == tag_t::INT);
	    last_addr = checked_cast<size_t>(reinterpret_cast<int_cell &>(addr_term).value());
	    if (last_addr != *it0) {
		std::cout << "Difference at closure #" << (i+j) << ": Was " << last_addr << " Expected: " << *it0 << std::endl;
	    }
	    assert(last_addr == *it0);
	    ++it0;

	    all_frozen_closures.push_back(last_addr);

	    lst = g.interp().arg(lst, 1);
	    j++;
	}
	last_addr++;
    }
    std::cout << "Recheck: Total number of frozen closures: " << all_frozen_closures.size() << std::endl;
    assert(all_frozen_closures.size() == all_frozen_closures0.size());
}

void setup_frozen_closures(std::vector<size_t> &all_frozen_closures)
{
    static const size_t NUM_REWARDS = 1000;

    // Remove existing database. Otherwise test will be confused.
    global::erase_db(test_dir);

    global g(test_dir);

    std::cout << "Data directory: " << test_dir << std::endl;
    std::cout << "STATUS: " << g.env().status() << std::endl;
    
    prologcoin::ec::builtins::load(g.interp());

    auto myreward_cmd = g.interp().parse("ec:privkey(X), ec:pubkey(X,Y), ec:address(Y,Z), reward(Z).");
    for (size_t i = 0; i < NUM_REWARDS; i++) {
	g.interp().execute(myreward_cmd);
    }

    size_t last_addr = 0;
    for (size_t i = 0; i < NUM_REWARDS; i += 100) {
	auto check_closures_cmd = g.interp().parse("frozenk(" + boost::lexical_cast<std::string>(last_addr) + ", 100, X).");
	g.interp().execute(check_closures_cmd);
	auto lst = g.interp().get_result_term("X");
	while (g.interp().is_dotted_pair(lst)) {
	    auto addr_term = g.interp().arg(lst, 0);
	    assert(addr_term.tag() == tag_t::INT);
	    last_addr = checked_cast<size_t>(reinterpret_cast<int_cell &>(addr_term).value());
	    all_frozen_closures.push_back(last_addr);

	    lst = g.interp().arg(lst, 1);
	}
	last_addr++;
    }
    std::cout << "Total number of frozen closures: " << all_frozen_closures.size() << std::endl;
    assert(all_frozen_closures.size() == NUM_REWARDS);

#if 0
    bool first = true;
    for (auto cl : all_frozen_closures) {
	if (!first) std::cout << " ";
	std::cout << cl;
	first = false;
    }
    std::cout << std::endl;
#endif

    g.advance();
}

static void test_global_frozen_closures()
{
    header("test_global_frozen_closures");

    std::vector<size_t> all_frozen_closures;
    setup_frozen_closures(all_frozen_closures);
    recheck_frozen_closures(all_frozen_closures);
}

int main(int argc, char *argv[])
{
    home_dir = find_home_dir(argv[0]);
    test_dir = home_dir;
    test_dir = (boost::filesystem::path(test_dir) / "bin" / "test" / "global" / "triedb").string();

    random::set_for_testing(true);
  
    test_global_basic();
    test_global_frozen_closures();
    return 0;
}
