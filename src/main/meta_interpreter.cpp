#include "../ec/builtins.hpp"
#include "../ec/mnemonic.hpp"
#include "../coin/builtins.hpp"
#include "meta_interpreter.hpp"
#include <boost/filesystem/path.hpp>

using namespace prologcoin::common;
using namespace prologcoin::interp;
using namespace prologcoin::terminal;
using namespace prologcoin::node;

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
    load_builtin(M, con_cell("start", 2), &meta_interpreter::start_2);
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

    auto query = args[0];
    auto where_term = args[1];
    auto &interp = reinterpret_cast<meta_interpreter &>(interp0);

    if (interp.functor(where_term) != NODE) {
        throw interp::interpreter_exception_wrong_arg_type("@/2: Second argument must be'node(...)'; was " + interp.to_string(where_term));
    }

    auto node_name = interp.arg(where_term, 0);
    if (node_name.tag() != tag_t::CON) {
	throw interp::interpreter_exception_wrong_arg_type("@/2: Node name must be an atom; was " + interp.to_string(node_name));
    }

    auto &nodes = interp.nodes_;
    std::string where = interp.atom_name(node_name);
    auto it = nodes.find(where);
    if (it == nodes.end()) {
	throw interp::interpreter_exception_wrong_arg_type("@/2: Cannot find node " + where);
    }

#define LL(x) reinterpret_cast<meta_interpreter &>(x)
	
    interp::remote_execution_proxy proxy(interp,
	[](interpreter_base &interp, term query, const std::string &where, bool silent)
	{return LL(interp).execute_at(query, interp, where, silent);},
        [](interpreter_base &interp, const std::string &where)
	{return LL(interp).continue_at(interp, where);},
	[](interpreter_base &interp, const std::string &where)
	{return LL(interp).delete_instance_at(interp, where);});

    proxy.set_silent(silent);
    return proxy.start(query, where);

    return true;
}
	
bool meta_interpreter::operator_at_2(interpreter_base &interp, size_t arity, term args[]) {
    return operator_at_impl(interp, arity, args, false);
}

bool meta_interpreter::operator_at_silent_2(interpreter_base &interp, size_t arity, term args[]) {
    return operator_at_impl(interp, arity, args, true);
}

remote_return_t meta_interpreter::execute_at(term query, term_env &query_src, const std::string &where, bool silent)
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

remote_return_t meta_interpreter::continue_at(term_env &query_src, const std::string &where)
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

bool meta_interpreter::delete_instance_at(term_env &query_src, const std::string &where)
{
    terminal::terminal *tterm = get_node_terminal(where);
    if (tterm == nullptr) {
	return false;
    }
    
    return tterm->delete_instance();
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
	
bool meta_interpreter::start_2(interpreter_base &interp0, size_t arity, term args[] ) {
    auto &interp = reinterpret_cast<meta_interpreter &>(interp0);
    
    if (args[0].tag() != tag_t::CON) {
	throw interpreter_exception_wrong_arg_type(
	   "start/2: First argument must be an atom.");
    }
    if (args[1].tag() != tag_t::INT) {
	throw interpreter_exception_wrong_arg_type(
	   "start/2: Second argument must be an integer (port number)");			   
    }
    auto name = interp.atom_name(reinterpret_cast<con_cell &>(args[0]));
    auto port = reinterpret_cast<int_cell &>(args[1]).value();

    if (interp.nodes_.count(name)) {
	throw interpreter_exception_wrong_arg_type(
	   "start/2: There's already a node with that name.");
    }

    boost::filesystem::path node_dir(interp.home_dir());
    node_dir /= std::string("node_") + name;
    
    auto *node = new self_node(node_dir.string(), port);
    node->set_name(name);
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
	   "start/2: Timeout: Failed to start node." );
    }
    interp.nodes_.insert(std::make_pair(name, std::make_pair(node, term)));

    return true;
}

}}
