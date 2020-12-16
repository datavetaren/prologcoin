#include "../ec/builtins.hpp"
#include "../ec/mnemonic.hpp"
#include "../coin/builtins.hpp"
#include "../wallet/wallet.hpp"
#include "meta_interpreter.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/predicate.hpp>

using namespace prologcoin::common;
using namespace prologcoin::interp;
using namespace prologcoin::terminal;
using namespace prologcoin::node;
using namespace prologcoin::wallet;

namespace prologcoin { namespace main {

meta_interpreter::meta_interpreter(const std::string &home_dir) : home_dir_(home_dir), no_coin_security_(this->disable_coin_security()) {
    init();
}

meta_interpreter::~meta_interpreter() {
    for (auto &it : nodes_) {
	auto *term = it.second.second;
	delete term;
	auto *node = it.second.first;
	node->stop();
	node->join();
	delete node;
    }
    for (auto &it : wallets_) {
	auto *w = it.second;
	delete w;
    }
}

void meta_interpreter::init()
{
    set_debug_enabled();
    load_builtins_file_io();
    ec::builtins::load(*this);
    coin::builtins::load(*this);
    setup_standard_lib();
    set_current_module(con_cell("meta",0));
    setup_local_builtins();
    set_auto_wam(true);
}

void meta_interpreter::total_reset()
{
    interpreter::total_reset();
    disable_coin_security();
    init();
}

void meta_interpreter::setup_local_builtins()
{
    static const con_cell M("meta",0);
    load_builtin(M, con_cell("@",2), &meta_interpreter::operator_at_2);
    load_builtin(M, con_cell("@-",2), &meta_interpreter::operator_at_silent_2);
    load_builtin(M, functor("erase_all",0), &meta_interpreter::erase_all_0);
    load_builtin(M, con_cell("start", 2), &meta_interpreter::start_3);
    load_builtin(M, con_cell("start", 3), &meta_interpreter::start_3);
    load_builtin(M, con_cell("connect", 2), &meta_interpreter::connect_2);
}

terminal::terminal * meta_interpreter::get_node_terminal(const std::string &node_name) {
    auto it = nodes_.find(node_name);
    if (it == nodes_.end()) {
	return nullptr;
    }
    return it->second.second;
}

bool meta_interpreter::operator_at_impl(interpreter_base &interp0, size_t arity, term args[], bool silent) {
    static con_cell NODE("node", 1);
    static con_cell WALLET("wallet",1);

    auto query = args[0];
    auto where_term = args[1];
    auto &interp = reinterpret_cast<meta_interpreter &>(interp0);

    if (interp.functor(where_term) != NODE && interp.functor(where_term) != WALLET) {
        throw interp::interpreter_exception_wrong_arg_type("@/2: Second argument must be'node(...)' or 'wallet(...)'; was " + interp.to_string(where_term));
    }

    bool is_node = interp.functor(where_term) == NODE;

    auto name_term = interp.arg(where_term, 0);
    if (name_term.tag() != tag_t::CON) {
	throw interp::interpreter_exception_wrong_arg_type("@/2: Name must be an atom; was " + interp.to_string(name_term));
    }
    auto name = interp.atom_name(name_term);

    std::string where = name;
    if (is_node) {
	auto &nodes = interp.nodes_;
	auto it = nodes.find(where);
	if (it == nodes.end()) {
	    throw interp::interpreter_exception_wrong_arg_type("@/2: Cannot find node " + where);
	}
#define LL(x) reinterpret_cast<meta_interpreter &>(x)
	
	interp::remote_execution_proxy proxy(interp,
        [](interpreter_base &interp, term query, const std::string &where, bool silent)
	  {return LL(interp).execute_at_node(query, interp, where, silent);},
        [](interpreter_base &interp, const std::string &where)
	  {return LL(interp).continue_at_node(interp, where);},
	[](interpreter_base &interp, const std::string &where)
	  {return LL(interp).delete_instance_at_node(interp, where);});

	proxy.set_silent(silent);
	return proxy.start(query, where);

    } else {
	// Wallet
	auto &wallets = interp.wallets_;
	auto it = wallets.find(where);
	if (it == wallets.end()) {
	    throw interp::interpreter_exception_wrong_arg_type("@/2: Cannot find wallet " + where);
	}

	interp::remote_execution_proxy proxy(interp,
        [](interpreter_base &interp, term query, const std::string &where, bool silent)
	  {return LL(interp).execute_at_wallet(query, interp, where, silent);},
        [](interpreter_base &interp, const std::string &where)
	  {return LL(interp).continue_at_wallet(interp, where);},
	[](interpreter_base &interp, const std::string &where)
	  {return LL(interp).delete_instance_at_wallet(interp, where);});

	proxy.set_silent(silent);
	return proxy.start(query, where);
    }
}
	
bool meta_interpreter::operator_at_2(interpreter_base &interp, size_t arity, term args[]) {
    return operator_at_impl(interp, arity, args, false);
}

bool meta_interpreter::operator_at_silent_2(interpreter_base &interp, size_t arity, term args[]) {
    return operator_at_impl(interp, arity, args, true);
}

remote_return_t meta_interpreter::execute_at_node(term query, term_env &query_src, const std::string &where, bool silent)
{
    terminal::terminal *tterm = get_node_terminal(where);
    if (tterm == nullptr) {
	return remote_return_t();
    }
    
    uint64_t cost = 0;
    tterm->env().clear_names();
    term query_term = tterm->env().copy(query, env(), cost);

    bool old = tterm->is_result_to_text();
    try {
	tterm->set_result_to_text(false);
	if (!tterm->execute(query_term, silent)) {
	    tterm->set_result_to_text(old);
	    return remote_return_t();
	}
    } catch (...) {
        tterm->set_result_to_text(old);
	throw;
    }
    tterm->set_result_to_text(old);
    term result_remote = tterm->get_result();
    bool more_state = tterm->has_more();
    bool at_end_state = tterm->at_end();
    term result_term = query_src.copy(result_remote, tterm->env(), cost);
    return remote_return_t(result_term, more_state, at_end_state, cost);
}

remote_return_t meta_interpreter::continue_at_node(term_env &query_src, const std::string &where)
{
    terminal::terminal *tterm = get_node_terminal(where);
    if (tterm == nullptr) {
	return remote_return_t();
    }

    uint64_t cost = 0;
    bool old = tterm->is_result_to_text();
    tterm->set_result_to_text(false);
    tterm->set_has_more();
    try {
        if (!tterm->next()) {
            return remote_return_t();
	}
    } catch (...) {
        tterm->set_result_to_text(old);
	throw;
    }
    tterm->set_result_to_text(old);    
    term result_remote = tterm->get_result();
    bool more_state = tterm->has_more();
    bool at_end_state = tterm->at_end();
    term result_term = query_src.copy(result_remote, tterm->env(), cost);
    return remote_return_t(result_term, more_state, at_end_state, cost);
}

bool meta_interpreter::delete_instance_at_node(term_env &query_src, const std::string &where)
{
    terminal::terminal *tterm = get_node_terminal(where);
    if (tterm == nullptr) {
	return false;
    }
    
    return tterm->delete_instance();
}

remote_return_t meta_interpreter::execute_at_wallet(term query, term_env &query_src, const std::string &where, bool silent)
{
    auto it = wallets_.find(where);
    if (it == wallets_.end()) {
	return remote_return_t();
    }

    wallet::wallet *w = it->second;
    wallet_interpreter &wi = w->interp();
    
    term query_term = wi.copy(query, env());
    try {
	if (!wi.execute(query_term)) {
	    return remote_return_t();
	}
    } catch (...) {
	throw;
    }
    uint64_t cost = 0;    
    term result_remote = wi.get_result_term();
    bool more_state = wi.has_more();
    bool at_end_state = !more_state && wi.is_instance();
    term result_term = query_src.copy(result_remote, wi, cost);
    return remote_return_t(result_term, more_state, at_end_state, wi.accumulated_cost() + cost);
}

remote_return_t meta_interpreter::continue_at_wallet(term_env &query_src, const std::string &where)
{
    auto it = wallets_.find(where);
    if (it == wallets_.end()) {
	return remote_return_t();
    }

    wallet::wallet *w = it->second;
    wallet_interpreter &wi = w->interp();

    if (!wi.next()) {
	return remote_return_t();
    }
    uint64_t cost = 0;
    term result_remote = wi.get_result_term();
    bool more_state = wi.has_more();
    bool at_end_state = !more_state && wi.is_instance();
    term result_term = query_src.copy(result_remote, wi, cost);
    return remote_return_t(result_term, more_state, at_end_state, wi.accumulated_cost() + cost);
}

bool meta_interpreter::delete_instance_at_wallet(term_env &query_src, const std::string &where)
{
    auto it = wallets_.find(where);
    if (it == wallets_.end()) {
	return false;
    }

    wallet::wallet *w = it->second;
    wallet_interpreter &wi = w->interp();
    
    wi.delete_instance();
    return true;
}

void meta_interpreter::pulse() {
    for (auto &it : nodes_) {
	terminal::terminal *tterm = it.second.second;
	tterm->node_pulse();
	text_ << tterm->flush_text();
    }
}

std::string meta_interpreter::flush_text() {
    std::string t = text_.str();
    text_ = std::stringstream();
    return t;
}

bool meta_interpreter::erase_all_0(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = reinterpret_cast<meta_interpreter &>(interp0);
    boost::filesystem::path root_dir(interp.home_dir());
    boost::filesystem::directory_iterator end;
    for (boost::filesystem::directory_iterator it(root_dir); it != end; ++it) {
	if (boost::starts_with(it->path().filename().string(), "wallet_") ||
	    boost::starts_with(it->path().filename().string(), "node_")) {
	    boost::system::error_code ec;
	    boost::filesystem::remove_all(it->path());
	}
    }
    return true;
}

bool meta_interpreter::start_3(interpreter_base &interp0, size_t arity, term args[] ) {
    auto &interp = reinterpret_cast<meta_interpreter &>(interp0);

    std::string name = "start/" + boost::lexical_cast<std::string>(arity);

    con_cell at_type = con_cell("node", 0);
    
    if (args[0].tag() != tag_t::CON) {
	throw interpreter_exception_wrong_arg_type(
	   name + ": First argument must be an atom.");
    }

    if (args[0] != con_cell("node",0) &&
	args[0] != con_cell("wallet",0)) {
	throw interpreter_exception_wrong_arg_type(
	   name + ": First argument must be an atom (node | wallet)");
    }

    if (args[0] == con_cell("wallet",0)) {
	at_type = con_cell("wallet",0);
    }

    if (args[1].tag() != tag_t::CON) {
	throw interpreter_exception_wrong_arg_type(
	   name + ": Second argument must be an atom (a name)");
    }

    auto label = interp.atom_name(reinterpret_cast<con_cell &>(args[1]));
    unsigned short port = 0;

    if (at_type == con_cell("node",0) && arity > 2) {
	if (args[2].tag() != tag_t::INT) {
	    throw interpreter_exception_wrong_arg_type(
	       name + ": Third argument must be an integer (port number)");
	}
	port = static_cast<unsigned short>(reinterpret_cast<int_cell &>(args[2]).value());
    }

    if (at_type == con_cell("node",0)) {
	if (interp.nodes_.count(label)) {	
	    throw interpreter_exception_wrong_arg_type(
	       name + ": There's already a node with that name.");
	}

	boost::filesystem::path node_dir(interp.home_dir());
	node_dir /= std::string("node_") + label;
	boost::filesystem::create_directories(node_dir);
    
	auto *node = new self_node(node_dir.string(), port);
	node->set_name(label);
	node->start();

	// Connect a terminal to it
	auto *term = new terminal::terminal(port);
	size_t retry_cnt = 0;
	while (!term->connect() && retry_cnt < 10) {
	    utime::sleep(utime::ss(1));
	    retry_cnt++;
	}
	if (retry_cnt == 10) {
	    delete node;
	    delete term;
	    throw interpreter_exception_wrong_arg_type(
	       name + ": Timeout: Failed to start node." );
	}
	interp.nodes_.insert(std::make_pair(label, std::make_pair(node, term)));
    } else {
	if (interp.wallets_.count(label)) {
	    throw interpreter_exception_wrong_arg_type(
	       name + ": There's already a wallet with that name.");	    
	}
	boost::filesystem::path wallet_dir(interp.home_dir());
	wallet_dir /= std::string("wallet_") + label;
	boost::filesystem::create_directories(wallet_dir);
        std::string wallet_file = wallet_dir.string();
        wallet_file += boost::filesystem::path::preferred_separator;
	wallet_file += "wallet.pl";
	auto *w = new wallet::wallet(wallet_file);
	try {
	    w->load();
	} catch (term_parse_exception &ex) {
	    std::cout << term_parser::report_string(w->env(), ex);
	} catch (token_exception &ex) {
	    std::cout << term_parser::report_string(w->env(), ex);
	} catch (...) {
  	    std::cout << "Unknown error while loading wallet." << std::endl;
	}
	interp.wallets_.insert(std::make_pair(label, w));
    }

    return true;
}

static bool is_wallet(interpreter_base &interp0, term t) {
    return interp0.functor(t) == con_cell("wallet",1);
}

static bool is_node(interpreter_base &interp0, term t) {
    return interp0.functor(t) == con_cell("node",1);
}

static bool is_name(interpreter_base &interp0, term t) {
    return t.tag() == tag_t::CON;
}

static std::string get_inner_name(interpreter_base &interp0, term t) {
    if (is_wallet(interp0, t) || is_node(interp0, t)) {
	term name_term = interp0.arg(t, 0);
	if (!is_name(interp0, name_term)) {
	    return "";
	}
	return interp0.atom_name(reinterpret_cast<con_cell &>(name_term));
    }
    return "";
}

bool meta_interpreter::connect_2(interpreter_base &interp0, size_t arity, term args[] ) {
    auto &interp = reinterpret_cast<meta_interpreter &>(interp0);

    std::string name = "connect/" + boost::lexical_cast<std::string>(arity);

    if (is_wallet(interp0, args[0])) {
	// Connect wallet to node
	if (!is_node(interp0, args[1])) {
	    throw interp::interpreter_exception_wrong_arg_type(name + ": Second argument must be'node(...)' when connecting wallets; was " + interp.to_string(args[1]));
	}
	auto wallet_name = get_inner_name(interp0, args[0]);
	auto node_name = get_inner_name(interp0, args[1]);
	auto it = interp.wallets_.find(wallet_name);
	if (it == interp.wallets_.end()) {
	    throw interp::interpreter_exception_wrong_arg_type(name + ": Cannot find wallet " + wallet_name);
	}
	auto it2 = interp.nodes_.find(node_name);
	if (it2 == interp.nodes_.end()) {
	    throw interp::interpreter_exception_wrong_arg_type(name + ": Cannot find node " + node_name);
	}
	auto *wallet = it->second;
	auto *node_terminal = it2->second.second;
	wallet->connect_node(node_terminal);
	return true;
    }

    if (!is_node(interp0, args[0])) {
	throw interp::interpreter_exception_wrong_arg_type(name + ": First argument must be'node(...)' or 'wallet(...)'; was " + interp.to_string(args[0]));
    }

    if (!is_node(interp0, args[1])) {
	throw interp::interpreter_exception_wrong_arg_type(name + ": Second argument must be'node(...)' when connecting nodes; was " + interp.to_string(args[1]));
    }

    auto it1 = interp.nodes_.find(get_inner_name(interp0, args[0]));
    if (it1 == interp.nodes_.end()) {
	throw interp::interpreter_exception_wrong_arg_type(name + ": Cannot find node " + get_inner_name(interp0, args[0]));
    }
    
    auto it2 = interp.nodes_.find(get_inner_name(interp0, args[1]));
    if (it2 == interp.nodes_.end()) {
	throw interp::interpreter_exception_wrong_arg_type(name + ": Cannot find node " + get_inner_name(interp0, args[1]));
    }

    self_node *node1 = it1->second.first;
    self_node *node2 = it2->second.first;

    node1->book()().add("127.0.0.1", node2->port());
    node2->book()().add("127.0.0.1", node1->port());
    
    return true;
}

}}
