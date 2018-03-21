#include <node/self_node.hpp>
#include <node/terminal.hpp>

using namespace prologcoin::node;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

#if 0
static void test_node_start_stop()
{
    header( "test_node_start_stop()" );

    self_node self;
    self.start();

    sleep(30);

    std::cout << "DONE\n";

    self.stop();

    self.join();
}
#endif

static void start_server()
{
    header("start_server()");

    self_node self;
    self.start();
    self.join();
}

static void start_server2()
{
    header("start_server2()");

    self_node self;

    self.book()().add("127.0.0.1", 1000);
    self.book()().add("127.0.0.1", 1001);
    self.book()().add("127.0.0.1", 1002);

    self.start();
    self.join();
}

static void start_client()
{
    header("start_client()");

    terminal termin;
    termin.run();
}

int main(int argc, char *argv[])
{
    header("test_node_simple()");

    if (argc == 2) {
	if (strcmp(argv[1], "-server") == 0) {
	    start_server();
	} else if (strcmp(argv[1], "-server2") == 0) {
	    start_server2();
	} else if (strcmp(argv[1], "-client") == 0) {
	    start_client();
	} else {
	    std::cout << "Unrecognized command: " << argv[1] << "\n";
	}
	return 0;
    }
    return 0;
    // test_node_start_stop();
}
