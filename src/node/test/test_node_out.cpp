#include <common/term_tools.hpp>
#include <node/self_node.hpp>
#include <node/terminal.hpp>
#include <node/session.hpp>

using namespace prologcoin::common;
using namespace prologcoin::node;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

#if 0
//
// This test does the following:
//
// 1) Starts a node
// 2) Creates an outgoing connection to itself (loop-back)
// 3) Check that this outgoing connection creates a session and
//    produces heartbeats.
// 4) Close down everything after 30s and check how many heartbeats
//    we've got. (It should be at least 2 heartbeats; heartbeats
//    are issued every 10 seconds.)
//
static void test_node_out()
{
    header("test_node_out()");

    bool success = false;
    
    self_node self;

    // Make everything quicker.
    self.set_timer_interval(utime::ss(1));
    self.start();

    // Create a loop-back connection.
    self.new_standard_out_connection(ip_service("127.0.0.1",
						self_node::DEFAULT_PORT));

    self.set_master_hook(
			 [&success] (self_node &self) {
			     self.for_each_in_session(
			      [&success,&self](in_session_state *session) {
					  if (session->heartbeats() >= 2) {
					      success = true;
					      self.stop();
					  }
			      });
			 });

    // We should have had 2 heartbeats by now
    self.join(utime::ss(30));
    self.stop();
    self.join();

    assert(success);
}
#endif

//
// Testing address verifier task
//
static void test_address_verifier()
{
    header("test_address_verifier()");

    self_node self;
    self_node self2(self_node::DEFAULT_PORT+1);
    std::string comment_str("foo(bar([17,4711],42))");
    self2.set_comment(comment_str + ".");

    self.set_timer_interval(utime::ss(1));
    self2.set_timer_interval(utime::ss(1));

    // Add new unverified address
    address_entry unverified(ip_address("127.0.0.1"),
			     self_node::DEFAULT_PORT+1,
			     ip_address("127.0.0.1"),
		   	     self_node::DEFAULT_PORT+1);

    self.book()().add(unverified);

    //
    // Start node
    //
    // This will create an address_verifier task, checking the
    // address above.
    //
    self.start();
    self2.start();

    // Poll to see if address gets verified (max 10 interval units)
    bool has_verified = false;
    for (size_t i = 0; i < 10 && !has_verified; i++) {
	has_verified = !self.book()().get_all_verified().empty();
	utime::sleep(utime::us(self.get_timer_interval_microseconds()));
    }

    //
    // Run this for maximum seconds
    //
    self.stop();
    self.join();
    self2.stop();
    self2.join();

    // Now check the address entries in self. It should have verified
    // its peer.

    auto verified = self.book()().get_all_verified();
    assert(verified.size() >= 1);

    std::cout << "Verified address entries: (should be 1)" << std::endl;
    std::cout << "---------------------------------------" << std::endl;

    self.book()().print(std::cout, verified);

    std::cout << "---------------------------------------" << std::endl;

    assert(verified.size() == 1);

    auto &e = verified[0];

    assert(self.book()().is_verified(e));
    assert(e.addr().str() == "127.0.0.1");
    assert(e.port() == self_node::DEFAULT_PORT+1);
    assert(e.score() >= address_entry::VERIFIED_INITIAL_SCORE);
    
    term_token_diff::assert_equal(e.comment_str(), comment_str,
				  "Unexpected comment:");
}

#if 0
//
// The purpose of this test is to setup 10 nodes with 1 address
// each (to its neighbor peer) and see how the addresses propagate
// through the gossip network.
//
static void test_address_propagation()
{
    header("test_address_propagation()");

    // bool success = false;

    const size_t num_nodes = 10;

    //
    // Setup nodes & start nodes
    //
    std::vector<self_node *> nodes;
    for (size_t i = 0; i < num_nodes; i++) {
	auto *node = new self_node(self_node::DEFAULT_PORT+i);
	node->set_timer_interval(utime::ss(1));
	nodes.push_back(node);
	node->start();
    }

    //
    // For each node i, add the address node i + 1
    //
    for (size_t i = 0; i < num_nodes; i++) {
	auto *node_i = nodes[i];
	auto *node_i1 = nodes[(i+1) % num_nodes];
	(void) node_i;
	(void) node_i1;
    }

    // Start nodes
    for (auto *node : nodes) {
	node->start();
    }

    // Wait 5 seconds
    utime::sleep(utime::ss(5));

    // Stop all nodes
    for (auto *node : nodes) {
	node->stop();
    }

    // Join threads
    for (auto *node : nodes) {
	node->join();
    }

    // Delete nodes
    for (auto *node : nodes) {
	delete node;
    }
}
#endif

int main(int argc, char *argv[])
{
    test_address_verifier();
    // test_node_out();
    // test_address_propagation();

    return 0;
}
