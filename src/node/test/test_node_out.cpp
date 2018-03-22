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
    self.start();

    // Create a loop-back connection.
    self.new_out_connection(ip_service("127.0.0.1",
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

int main(int argc, char *argv[])
{
    test_node_out();

    return 0;
}
