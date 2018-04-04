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
    std::cout << "Create a loop back connection." << std::endl;
    self.new_standard_out_connection(ip_service("127.0.0.1",
						self_node::DEFAULT_PORT));

    self.set_master_hook(
			 [&success] (self_node &self) {
			     self.for_each_in_session(
			      [&success](in_session_state *session) {
					  if (session->heartbeats() >= 2) {
					      success = true;
					  }
			      });
			 });

    for (size_t i = 0; i < 10 && !success; i++) {
	utime::sleep(utime::ss(1));
    }

    std::cout << "We should have had at least 2 heartbeats by now." << std::endl;
    // We should have had 2 heartbeats by now
    self.stop();
    self.join();

    std::cout << "Success: " << success << std::endl;

    assert(success);
}

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

    std::cout << "Setup from port " << self_node::DEFAULT_PORT << " to " << self_node::DEFAULT_PORT+num_nodes-1 << " (= " << num_nodes << " nodes.)" << std::endl;
    std::vector<self_node *> nodes;
    for (size_t i = 0; i < num_nodes; i++) {
	auto *node = new self_node(self_node::DEFAULT_PORT+i);
	node->set_timer_interval(utime::ss(1));
	node->set_time_to_live(utime::ss(2));
	node->set_testing_mode(true);
	nodes.push_back(node);
    }

    //
    // For each node i, add the address node i + 1
    //
    std::cout << "Node with port I has a address reference to port I + 1." << std::endl;
    for (size_t i = 0; i < num_nodes; i++) {
	auto *node_i = nodes[i];

	//
	// Add new unverified address
	//
	// Node I has address to Node I+1 (mod N)
	//
	address_entry unverified(ip_address("127.0.0.1"),
				 self_node::DEFAULT_PORT+((i+1) % num_nodes),
				 ip_address("127.0.0.1"),
				 self_node::DEFAULT_PORT+((i+1) % num_nodes));

	node_i->book()().add(unverified);
    }

    // Start nodes
    std::cout << "Start all nodes..." << std::endl;
    for (auto *node : nodes) {
	node->start();
    }

    //
    // Check for maximum 60 seconds:
    //
    // Each node should have the other 9 addresses. Not all of them
    // verified perhaps.
    //
    std::cout << "Check for address propagation (max 60 seconds.)" << std::endl;
    std::unordered_set<unsigned short> ok_nodes;
    for (size_t i = 0; i < 60 && ok_nodes.size() < num_nodes; i++) {
	utime::sleep(utime::ss(1));
	for (auto *node : nodes) {
	    size_t count = 0;
	    unsigned short port = node->port();
	    if (ok_nodes.find(port) == ok_nodes.end()) {
		for (unsigned short p = self_node::DEFAULT_PORT;
		     p < self_node::DEFAULT_PORT+num_nodes; p++) {
		    ip_service ip("127.0.0.1", p);
		    if (node->book()().exists(ip)) count++;
		}
		if (count == num_nodes-1) {
		    ok_nodes.insert(port);
		    std::stringstream ss;
		    ss << "Node at port " << port << " is completed "
		       << "(connected to ";
		    bool first = true;
		    node->for_each_standard_out_connection(
			   [&](out_connection *conn) {
			       if (!first) ss << ", ";
			       ss << conn->ip().port();
			       first = false;
			   });
		    ss << ")" << std::endl;
		    std::cout << ss.str();
		}
	    }
	}
	if (ok_nodes.size() == num_nodes) {
	    std::cout << "All nodes completed." << std::endl;
	}
    }

    auto *stopped_node = nodes[4];

    //
    // Record address entry scores for each node to node about to be stopped
    // 
    std::cout << "Record scores to node at port " << stopped_node->port() << std::endl;
    std::map<unsigned short, int32_t> recorded_scores;
    for (auto *node : nodes) {
	node->book()().for_each_address_entry(
	      [&](const address_entry &e) {
		  if (e.port() == stopped_node->port()) {
		      recorded_scores[node->port()] = e.score();
		  }
	      });
    }

    //
    // Now stop node no. 4 (counting from 0) and see how scores
    // will be propagated.
    //
    std::cout << "Stopping node at port " << stopped_node->port() << std::endl;
    stopped_node->stop();
    stopped_node->join();
    std::cout << "Node stopped." << std::endl;

    //
    // Wait for 30 seconds and see how this is propagated through the
    // network.
    //
    std::cout << "Waiting for effect to propagate to other nodes (max 30 seconds)" << std::endl;
    size_t changes = 0;
    auto stopped_port = stopped_node->port();
    for (size_t i = 0; i < 30 && changes < 10; i++) {
	utime::sleep(utime::ss(1));

        for (auto *node : nodes) {
	    node->book()().for_each_address_entry(
	       [node,stopped_port,&changes,&recorded_scores]
	       (const address_entry &e) {
		   if (e.port() == stopped_port) {
		       if (e.score() != recorded_scores[node->port()]) {
			  changes++;
			  std::cout << "Node at port " << node->port()
				    << " downgraded score for node "
				    << stopped_port << " to "
				    << e.score() << std::endl;
			  recorded_scores[node->port()] = e.score();
		      }
		  }
	      });
	}
    }

    // Stop all nodes
    std::cout << "Stopping all nodes..." << std::endl;
    for (auto *node : nodes) {
	node->stop();
    }

    // Join threads
    std::cout << "Join thread..." << std::endl;
    for (auto *node : nodes) {
	node->join();
    }

    //
    // Did we succeed?
    //
    std::cout << "Check if successful..." << std::endl;
    bool succeed = true;
    if (ok_nodes.size() < num_nodes) {
	succeed = false;
	std::cout << "Failed to succeed address propagation for all nodes." << std::endl;
	std::cout << "The following ones failed:" << std::endl;
	for (auto *node : nodes) {
	    unsigned short port = node->port();
	    if (!ok_nodes.count(port)) {
		std::cout << "Node at port " << port << ":" << std::endl;
		std::cout << "---------------------------------------------" << std::endl;
		node->book()().print(std::cout);		
	    }
	}
    } else {
	std::cout << "Succeeded!" << std::endl;
    }

    std::cout << "Check scores to node at port " << stopped_port << std::endl;
    for (auto *node : nodes) {
	node->book()().for_each_address_entry(
	      [&](const address_entry &e) {
		  if (e.port() == stopped_port) {
		      std::cout << "Node at port " << node->port()
				<< " has score for node "
				    << stopped_node->port() << ": "
				    << e.score() << std::endl;
		  }
	      });
    }
    assert(changes >= 10);


    utime::sleep(utime::ss(1));

    // Delete nodes
    std::cout << "Delete all nodes." << std::endl;
    for (auto *node : nodes) {
	delete node;
    }

    assert(succeed);
}

int main(int argc, char *argv[])
{
    test_node_out();
    test_address_verifier();
    test_address_propagation();

    return 0;
}
