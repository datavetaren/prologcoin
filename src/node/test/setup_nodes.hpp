#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <common/utime.hpp>
#include <node/self_node.hpp>
#include <terminal/terminal.hpp>
#include <memory>
#include <functional>
#include <common/term_tools.hpp>

//
// Provide a simple infrastructure to setup N nodes and wait until they are connected.
// This is a convenience utility for tests such as "test_operator_at".
//

namespace prologcoin { namespace node {

using terminal = prologcoin::terminal::terminal;
    
class setup_nodes {
public:
    struct node_spec {
	std::string name;
	unsigned short port;
        std::string data_dir;
    };

    //
    // Arg structure is:
    //
    inline setup_nodes( const std::initializer_list<node_spec> &args )
    {
	for (auto &ns : args) {
	    prologcoin::global::global::erase_db(ns.data_dir);
  	    auto *node = new self_node(ns.data_dir, ns.port);
	    node->set_name(ns.name);
	    node->set_testing_mode(true); // Faster propagation
	    node_map_[ns.name] = node;
	    nodes_.push_back(node);
	}
    }

    inline void start()
    {
	for (auto *node : nodes_) {
	    std::string qname = "'" + node->name() + "'";
	    std::cout << "Starting node " << std::setw(12) << qname << " at port " << node->port() << std::endl;
	    node->start();
	}
	add_addresses();
	wait_connections();
    }

    inline void stop()
    {
	for (auto *node : nodes_) {
	    std::string qname = "'" + node->name() + "'";
	    std::cout << "Stopping node " << std::setw(12) << qname << " at port " << node->port() << std::endl;
	    node->stop();
	}
	for (auto *node : nodes_) {
	    node->join();
	}
	for (auto *node : nodes_) {
	    delete node;
	}
	std::cout << "Network stopped." << std::endl;
    }
    
    inline void add_addresses()
    {
	for (auto *node : nodes_) {
	    for (auto *peer : nodes_) {
		if (peer != node) {
		    node->book()().add("127.0.0.1", peer->port());
		}
	    }
	}
    }

    inline void wait_connections()
    {
	using namespace prologcoin::common;

	std::cout << "Waiting for full connections..." << std::endl;
	size_t num_peers = nodes_.size() - 1;
	uint64_t elapsed_time = 0;
	for (auto *node : nodes_) {
	    std::cout << "Waiting for node " << node->name() << std::endl;
	    size_t cnt = 0;
	    while (cnt != num_peers) {
		cnt = 0;
		node->for_each_standard_out_connection([&cnt](out_connection *) {cnt++;});
		utime::sleep(utime::us(node->get_fast_timer_interval_microseconds()));
		elapsed_time += node->get_fast_timer_interval_microseconds();

		if (elapsed_time > 120*(1000*1000)) {
		    throw std::runtime_error("Timeout expired; Failed to setup connections.");
		}
	    }
	    std::cout << "Succeeded with node " << node->name() << std::endl;
	}
	std::cout << "Established full connections." << std::endl;
    }

    inline self_node * get_node(const std::string &name) 
    {
	return node_map_[name];
    }

    inline std::unique_ptr<terminal> new_terminal(const std::string &name)
    {
	auto *node = node_map_[name];
        std::unique_ptr<terminal> tm(new terminal(node->port()));
	assert(tm->connect());
	return tm;
    }

    inline void remove_nl(std::string &s)
    {
	for (auto &ch : s) {
	    if (ch == '\n') {
		ch = ' ';
	    }
	}
    }

    inline void check_result( std::unique_ptr<terminal> &tm,
			      const std::vector<std::string> &expects )
    {
	size_t cnt = 0;
	for (auto &expect : expects) {
	    auto actual = tm->flush_text();
	    remove_nl(actual);
	    std::cout << "Actual: " << actual << std::endl;
	    std::cout << "Expect: " << expect << std::endl;
	    common::term_token_diff::assert_equal(actual, expect);
	    cnt++;
	    if (cnt < expects.size()) {
		tm->next();
	    }
	}
	bool r = tm->next();
	if (r) {
	    auto actual = tm->flush_text();
	    remove_nl(actual);
	    std::cout << "Actual: " << actual << std::endl;
	    std::cout << "Expect: <nothing>" << std::endl;
	}
	assert(!r);
	tm->flush_text();
    }

private:
    std::unordered_map<std::string, self_node *> node_map_;
    std::vector<self_node *> nodes_;
};

}}



