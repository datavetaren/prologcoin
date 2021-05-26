#include "../common/checked_cast.hpp"
#include "self_node.hpp"
#include "local_interpreter.hpp"
#include "session.hpp"
#include "task_reset.hpp"
#include "task_execute_query.hpp"
#include "../ec/builtins.hpp"
#include "../coin/builtins.hpp"
#include "../global/global_interpreter.hpp"
#include "../common/sha256.hpp"
#include "node_locker.hpp"
#include "sync.hpp"

namespace prologcoin { namespace node {

using namespace prologcoin::common;
using namespace prologcoin::interp;
using namespace prologcoin::db;

const con_cell local_interpreter::ME("me", 0);
const con_cell local_interpreter::COLON(":", 2);
const con_cell local_interpreter::COMMA(",", 2);

typedef meta_context_remote meta_context_operator_at;

bool me_builtins::list_load_2(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    term filename_term = args[0];
    term rest = args[1];

    bool done = false;

    // First check arguments...
    while (!done) {
        if (!interp.is_atom(filename_term)) {
	    interp.abort(interpreter_exception_wrong_arg_type("[...]: Argument was not a file name; was " + interp.to_string(filename_term)));
        }
	if (interp.is_dotted_pair(rest)) {
	    filename_term = interp.arg(rest, 0);
	    rest = interp.arg(rest, 1);
	} else if (!interp.is_empty_list(rest)) {
	    interp.abort(interpreter_exception_wrong_arg_type("[...]: List did not end with an empty list; was " + interp.to_string(rest)));
	} else {
	    done = true;
	}
    }

    filename_term = args[0];
    rest = args[1];

    while (!interp.is_empty_list(filename_term)) {
	std::string filename = interp.atom_name(filename_term) + ".pl";
	interp.load_file(filename);
	if (interp.is_dotted_pair(rest)) {
	    filename_term = interp.arg(rest, 0);
	} else {
	    filename_term = interp.EMPTY_LIST;
	}
    }

    return true;
}

bool me_builtins::operator_at_impl(interpreter_base &interp0, size_t arity, term args[], const std::string &name, interp::remote_execute_mode mode) {
    auto &interp = to_local(interp0);

    interp.root_check(name, 2);
    
    auto query = args[0];

    term where_term;
    term else_do;
    size_t timeout;

    std::tie(where_term, else_do, timeout) = interpreter::deconstruct_where(interp0, args[1]);

    if (where_term.tag() == tag_t::STR && interp.functor(where_term) == con_cell("local",1)) {
	term new_args[16];
	std::copy(args, args+arity, new_args);
	new_args[1] = interp.arg(args[1], 0);
	return interpreter::operator_at_impl(interp0, arity, new_args, name, mode);
    }

    // std::cout << "operator_at_2: query=" << interp.to_string(query) << " where=" << interp.to_string(where_term) << std::endl;

    if (!interp.is_atom(where_term)) {
	interp.abort(interpreter_exception_wrong_arg_type(name + "/2: Second argument must be an atom to represent a connection name; was " + interp.to_string(where_term)));
    }

    std::string where = interp.atom_name(where_term);

    if (where == "global") {
        // Then this is same as global(...)
        term local_args[1] = { query };
        return global_1(interp, 1, local_args);
    }

#define LL(interp) reinterpret_cast<local_interpreter &>(interp)
    
    remote_execution_proxy proxy(interp,
	[](interpreter_base &interp, term query, term else_do, const std::string &where, interp::remote_execute_mode mode, size_t timeout)
	   {return LL(interp).self().execute_at(query, else_do, interp, where, mode, timeout);},
        [](interpreter_base &interp, term query, term else_do, const std::string &where, interp::remote_execute_mode mode, size_t timeout)
	   {return LL(interp).self().continue_at(query, else_do, interp, where, mode, timeout);},
	[](interpreter_base &interp, const std::string &where)
	   {return LL(interp).self().delete_instance_at(interp, where);});
    proxy.set_mode(mode);
    proxy.set_timeout(timeout);

    return proxy.start(query, else_do, where);
}

    
bool me_builtins::operator_at_2(interpreter_base &interp0, size_t arity, term args[] )
{
    return operator_at_impl(interp0, arity, args, "@", MODE_NORMAL);
}

bool me_builtins::operator_at_silent_2(interpreter_base &interp0, size_t arity, term args[] )
{
    return operator_at_impl(interp0, arity, args, "@-", MODE_SILENT);
}

bool me_builtins::operator_at_parallel_2(interpreter_base &interp0, size_t arity, term args[] )
{
    return  operator_at_impl(interp0, arity, args, "@=", MODE_PARALLEL);
}
	
bool me_builtins::id_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    const std::string &id = interp.self().id();
    auto f = interp.functor(id, 0);
    return interp.unify(args[0], f);
}

bool me_builtins::name_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    const std::string &name = interp.self().name();
    auto f = interp.functor(name, 0);
    return interp.unify(args[0], f);
}

bool me_builtins::heartbeat_0(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    interp.session().heartbeat();
    return true;
}

bool me_builtins::version_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    int_cell ver_major(self_node::VERSION_MAJOR);
    int_cell ver_minor(self_node::VERSION_MINOR);
    auto ver = interp.new_term(con_cell("ver",2), {ver_major, ver_minor});
    return interp.unify(args[0], ver);
}

bool me_builtins::comment_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    term comment = interp.copy(interp.self().get_comment(),
			       interp.self().env());

    return interp.unify(args[0], comment);
}

bool me_builtins::datadir_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    term datadir = interp.string_to_list(interp.self().data_directory());

    return interp.unify(args[0], datadir);
}

bool me_builtins::peers_2(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);

    auto n_term = args[0];
    if (n_term.tag() != tag_t::INT) {
	interp.abort(interpreter_exception_wrong_arg_type("peers/2: First argument must be an integer; was " + interp.to_string(n_term)));
    }

    int64_t n = reinterpret_cast<int_cell &>(n_term).value();
    if (n < 1 || n > 100) {
	interp.abort(interpreter_exception_wrong_arg_type("peers/2: First argument must be a number within 1..100; was " + boost::lexical_cast<std::string>(n)));
    }

    auto book = interp.self().book();

    auto best = book().get_randomly_from_top_10_pt(static_cast<size_t>(n));
    auto rest = book().get_randomly_from_bottom_90_pt(static_cast<size_t>(n-best.size()));

    // Create a list out of entries
    term result = interp.EMPTY_LIST;
    for (auto &e : best) {
	result = interp.new_dotted_pair(e.to_term(interp), result);
    }
    for (auto &e : rest) {
	result = interp.new_dotted_pair(e.to_term(interp), result);
    }
    
    // Unify second arg with result

    return interp.unify(args[1], result);
}

bool me_builtins::add_address_2(interpreter_base &interp0, size_t arity, term args[] )
{
    term addr_term = args[0];
    term port_term = args[1];

    auto &interp = to_local(interp0);

    interp.root_check("add_address", arity);
    
    //
    // If the address is the empty list we figure out the IP address from
    // the remote endpoint.
    //
    bool addr_is_empty = interp.is_empty_list(addr_term);

    if (!addr_is_empty && !interp.is_atom(addr_term)) {
	interp.abort(interpreter_exception_wrong_arg_type("add_address/2: First argument must be an atom; was " + interp.to_string(addr_term)));
    }
    std::string addr;
    if (addr_is_empty) {
	if (auto *conn = interp.session().get_connection()) {
	    addr = conn->get_socket().remote_endpoint().address().to_string();
	}
    } else {
	addr = interp.atom_name(addr_term);
    }
    try {
	ip_address ip_addr(addr);
    } catch (boost::exception &ex) {
	interp.abort(interpreter_exception_wrong_arg_type("add_address/2: First argument must be a valid IP (v4 or v6) address; was '" + addr + "'"));
    }

    ip_address ip_addr(addr);

    if (port_term.tag() != tag_t::INT) {
	interp.abort(interpreter_exception_wrong_arg_type("add_address/2: Second argument must be a valid port number 0..65535; was " + interp.to_string(port_term)));
    }

    unsigned short port = 0;
    auto port_int = reinterpret_cast<const int_cell &>(port_term).value();
    try {
	port = checked_cast<unsigned short>(port_int);
    } catch (checked_cast_exception &ex) {
	interp.abort(interpreter_exception_wrong_arg_type("add_address/2: Second argument must be a valid port number 0..65535; was " + boost::lexical_cast<std::string>(port_int)));
    }

    address_entry entry(ip_addr, port);
    interp.self().book()().add(entry);
	
    return true;
}

bool me_builtins::connections_0(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);

    interp.root_check("connections", arity);
    
    std::stringstream ss;

    ss << std::setw(20) << "Address" << " Port" << std::setw(16) << "Name" << "  Ver" << " Comment" << std::endl;

    interp.self().for_each_standard_out_connection(
	      [&](out_connection *conn){
		  auto &ip_addr = conn->ip().addr();
		  auto port = conn->ip().port();
		  std::string ver;
		  std::string comment;
		  address_entry entry;
		  if (interp.self().book()().exists(conn->ip(), &entry)) {
		      ver = entry.version_str();
		      comment = entry.comment_str();
		  }

		  ss << std::setw(20) << ip_addr.str();
		  ss << std::setw(5) << port;
		  ss << std::setw(16) << conn->name();
		  ss << std::setw(5) << ver;
		  ss << " " << comment;
		  ss << std::endl;
	      });

    interp.add_text(ss.str());

    return true;
}

bool me_builtins::connections_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);

    interp.root_check("connections", arity);

    term lst = interp.EMPTY_LIST;
    term head = lst, tail = lst;

    interp.self().for_each_standard_out_connection(
	      [&](out_connection *conn){
		  auto name = conn->name();
		  term name_term;
		  if (interp.self().is_unique_connection_name(name)) {
		      name_term = interp.functor(name, 0);
		  } else {
		      name_term = interp.functor(conn->ip().str(), 0);
		  }
		  auto new_tail = interp.new_dotted_pair(name_term,
							 interp.EMPTY_LIST);
		  if (head == interp.EMPTY_LIST) {
		      head = tail = new_tail;
		  } else {
		      interp.set_arg(tail, 1, new_tail);
		      tail = new_tail;
		  }
	      });

    return interp.unify( args[0], head);
}

bool me_builtins::ready_2(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);

    interp.root_check("ready", arity);
    
    term arg = args[0];
    term result = interp.EMPTY_LIST;
    bool found = false;

    std::unordered_set<std::string> not_these;
    if (arity == 2) {
	auto not_these_list = args[1];
	while (interp.is_dotted_pair(not_these_list)) {
	    auto not_this_term = interp.arg(not_these_list, 0);
	    std::string not_this_str = interp.list_to_string(not_this_term);
	    not_these.insert(not_this_str);
	    not_these_list = interp.arg(not_these_list, 1);
	}
    }
    
    interp.self().for_each_standard_out_connection(
	      [&](out_connection *conn){
		  if (!found && conn->is_ready()) {
		      auto name = conn->name();
		      if (!interp.self().is_unique_connection_name(name)) {
			  name = conn->ip().str();
		      }
		      if (!not_these.count(name)) {
			  result = interp.functor(name, 0);
			  found = true;
		      }
		  }
	      });

    if (!found) {
	return false;
    }

    return interp.unify(arg, result);
}

bool me_builtins::mailbox_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);

    term name_term = args[0];
    if (!interp.is_atom(name_term)) {
	interp.abort(interpreter_exception_wrong_arg_type("mailbox/1: First argument must be an atom; was " + boost::lexical_cast<std::string>(name_term)));
    }

    auto name = interp.atom_name(name_term);
    interp.self().create_mailbox(name);
    
    return true;
}

bool me_builtins::check_mail_0(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);

    std::string msgs = interp.self().check_mail();
    interp.add_text(msgs);
    
    return true;
}

bool me_builtins::send_2(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    term mailbox_name_term = args[0];
    term message_term = args[1];

    if (!interp.is_atom(mailbox_name_term)) {
	interp.abort(interpreter_exception_wrong_arg_type("send/2: First argument must be an atom; was " + interp.to_string(mailbox_name_term)));
    }

    std::string mailbox_name = interp.atom_name(mailbox_name_term);

    std::string message;

    if (interp.is_atom(message_term)) {
	message = interp.atom_name(message_term);
    } else if (interp.is_dotted_pair(message_term)) {
	message = interp.list_to_string(message_term);
    } else {
	interp.abort(interpreter_exception_wrong_arg_type("send/2: Second argument must be an atom or string; was " + interp.to_string(message_term)));
    }

    std::string from = "";

    if (auto *conn = interp.session().get_connection()) {
	if (!conn->name().empty()) {
	    from = conn->name();
	} else {
	    from = conn->get_socket().remote_endpoint().address().to_string();
	}
    }

    interp.self().send_message(mailbox_name, from, message);

    return true;
}

bool me_builtins::initial_funds_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    interp0.set_debug(true);
    term arg = args[0];
    if (arg.tag() == tag_t::INT) {
	uint64_t val = static_cast<uint64_t>(reinterpret_cast<int_cell &>(arg).value());
	interp.self().set_initial_funds(val);
	return true;
    } else if (arg.tag().is_ref()) {
	auto val = static_cast<int64_t>(interp.self().get_initial_funds());
	if (val <= 0) val = 0;
	return interp.unify(arg, int_cell(val));
    } else {
	return false;
    }
}

bool me_builtins::maximum_funds_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    term arg = args[0];
    if (arg.tag() == tag_t::INT) {
      interp.root_check("maximum_funds", arity);
      
	uint64_t val = static_cast<uint64_t>(reinterpret_cast<int_cell &>(arg).value());
	interp.self().set_maximum_funds(val);
	return true;
    } else if (arg.tag().is_ref()) {
	auto val = static_cast<int64_t>(interp.self().get_maximum_funds());
	if (val <= 0) val = 0;
	return interp.unify(arg, int_cell(val));
    } else {
	return false;
    }
}

bool me_builtins::new_funds_per_second_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    interp.root_check("new_funds_per_second",arity);
    
    term arg = args[0];
    if (arg.tag() == tag_t::INT) {
	uint64_t val = static_cast<uint64_t>(reinterpret_cast<int_cell &>(arg).value());
	interp.self().set_new_funds_per_second(val);
	return true;
    } else if (arg.tag().is_ref()) {
	auto val = static_cast<int64_t>(int_cell::saturate(interp.self().new_funds_per_second()));
	return interp.unify(arg, int_cell(val));
    } else {
	return false;
    }
}

bool me_builtins::funds_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);
    term arg = args[0];
    auto funds = static_cast<int64_t>(int_cell::saturate(interp.session().available_funds()));
    return interp.unify(arg, int_cell(funds));
}

bool me_builtins::nolimit_0(interpreter_base &interp0, size_t arity, term args[] ) {
    auto &interp = to_local(interp0);
    auto maxing_it = std::numeric_limits<uint64_t>::max();
    interp.self().set_maximum_funds(maxing_it);
    interp.self().set_new_funds_per_second(maxing_it);
    return true;
}

bool me_builtins::drop_global_0(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();
    auto &g = interp.self().global();
    g.total_reset();
    return true;
}

bool me_builtins::gstat_1(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();
    auto &g = interp.self().global();
    if (arity == 0) {
	auto &prev_id = g.get_blockchain().tip().get_previous_id();	
	std::cout << "prevhash=16'" << hex::to_string(prev_id.hash(), prev_id.hash_size()) << std::endl;
	std::cout << "height=" << g.current_height() << std::endl;
	std::cout << "heap=" << g.env().heap_size() << std::endl;
	std::cout << "stack=" << g.env().stack_size() << std::endl;
	std::cout << "trail=" << g.env().trail_size() << std::endl;
	std::cout << "num_predicates=" << g.num_predicates() << std::endl;
	std::cout << "num_symbols=" << g.num_symbols() << std::endl;
	std::cout << "num_frozen_closures=" << g.num_frozen_closures() << std::endl;
	return true;
    } else {
	term result = interp.EMPTY_LIST;
	result = interp.new_dotted_pair(
		interp.new_term(interp.functor("new_frozen_closures",1),
				{int_cell(g.num_frozen_closures())}),
		result);
	result = interp.new_dotted_pair(
		interp.new_term(interp.functor("num_symbols",1),
				{int_cell(g.num_symbols())}),
		result);
	result = interp.new_dotted_pair(
		interp.new_term(interp.functor("num_predicates",1),
				{int_cell(g.num_predicates())}),
		result);
	result = interp.new_dotted_pair(
		interp.new_term(interp.functor("trail",1),
				{int_cell(g.env().trail_size())}),
		result);
	result = interp.new_dotted_pair(
		interp.new_term(interp.functor("stack",1),
				{int_cell(g.env().stack_size())}),
		result);
	result = interp.new_dotted_pair(
		interp.new_term(interp.functor("heap",1),
				{int_cell(g.env().heap_size())}),
		result);
	result = interp.new_dotted_pair(
		interp.new_term(interp.functor("height",1),
			        {int_cell(g.current_height())}),
		result);

	auto &prev_id = g.get_blockchain().tip().get_previous_id();
	term prev_hash = interp.new_big(prev_id.hash(), prev_id.hash_size());
	result = interp.new_dotted_pair(
		interp.new_term(interp.functor("prevhash",1), {prev_hash}),
		result);	
	return interp.unify(args[0], result);
    }
}

static size_t min_prefix(const global::meta_id &a, const global::meta_id &b) {
    size_t n = a.hash_size();
    auto ha = a.hash();
    auto hb = b.hash();
    for (size_t i = 0; i < n; i++) {
	if (ha[i] != hb[i]) {
	    return i+1;
	}
    }
    return n;
}

struct tree_node {
    global::meta_id id;
    size_t height;
    bool here;
    std::vector<tree_node *> children;

    tree_node() : id(), height(std::numeric_limits<size_t>::max()), here(false) { }
    tree_node(global::meta_id i, size_t h) : id(i), height(h), here(false) { }
    ~tree_node() {
	for (auto *n : children) {
	    delete n;
	}
    }

    void set_here() { here = true; }
    
    tree_node * add_child(const global::meta_id &child_id, size_t height) {
	auto child = new tree_node(child_id, height);
	children.push_back(child);
	return child;
    }

    bool has_children() const {
	return !children.empty();
    }

    tree_node * first_child() {
	return children[0];
    }

    size_t num_children() const {
	return children.size();
    }

    tree_node * get_child(size_t index) {
	return children[index];
    }
    
    const std::vector<tree_node *> & get_children() {
	return children;
    }
};

static void print_tree(tree_node *tree, size_t column_width, size_t id_bytes) {
    std::vector<tree_node *> prev_line;
    
    std::vector<std::pair<tree_node *, size_t> > path;
    for (tree_node *node = tree; node->has_children(); node=node->first_child()) {
	path.push_back(std::make_pair(node,0));
    }

    while (!path.empty()) {
	size_t col = 0;
	for (auto p : path) {
	    auto prev_node = (col < prev_line.size()) ? prev_line[col] : nullptr;
	    auto node = p.first->get_child(p.second);
	    std::string str;
	    if (prev_node != node) {
		if (node->here) {
		    str += "*";
		}
		str += boost::lexical_cast<std::string>(node->height);
		str += "(";
		str += hex::to_string(node->id.hash(), id_bytes);
		str += ")";
		if (col < path.size()-1) {
		    while (str.size() < column_width-1) {
			str += "-";
		    }
		    str += ">";
		}
	    } else {
		if (p.second < p.first->num_children() - 1) {
		    str = "|";
		}
	    }
	    std::cout << std::left << std::setw(column_width) << str;
	    col++;
	}
	prev_line.clear();
	for (auto p : path) {
	    prev_line.push_back(p.first->get_child(p.second));
	}
	std::cout << std::endl;
	bool cont = true;
	while (!path.empty() && cont) {
	    path.back().second++;
	    if (path.back().second >= path.back().first->num_children()) {
		path.pop_back();
		prev_line.pop_back();
	    } else {
		cont = false;
	    }
	}
	if (!path.empty()) {
	    for (tree_node *node = path.back().first->get_child(path.back().second);
		 node->has_children();
		 node = node->first_child()) {
		path.push_back(std::make_pair(node,0));
	    }
	}
    }
}

#if 0 // debugging
static void add_node(const std::string &str, tree_node *root) {
    uint8_t h[global::meta_id::HASH_SIZE];
    memset(h, 0, global::meta_id::HASH_SIZE);
    tree_node *node = root;
    for (size_t i = 0; i < str.size(); i++) {
	sha256 hash;
	hash.update(str.c_str(), i+1);
	hash.finalize(h);
	global::meta_id id(h);
	auto &children = node->get_children();
	bool found = false;
	size_t j;
	for (j = 0; j < children.size(); j++) {
	    tree_node *child = children[j];
	    if (child->id == id) {
		found = true;
		break;
	    }
	}
	if (found) {
	    node = children[j];
	} else {
	    node = node->add_child(id, i+100000);
	}
    }
}

static void set_here(const uint8_t *bytes, size_t n, tree_node *node) {
    if (memcmp(node->id.hash(), bytes, n) == 0) {
	node->set_here();
    } else {
	for (auto child : node->get_children()) {
	    set_here(bytes, n, child);
	}
    }
}

static void set_here(const std::string &str, tree_node *root) {
    uint8_t hash2[4];
    std::string str4 = str.substr(0,4);
    hex::from_string(str4, hash2, 2);
    set_here( hash2, 2, root );
}
#endif

//
// states.
//   Gives you nice summary on where you are.
bool me_builtins::chain_0(interpreter_base &interp0, size_t arity, term args[]) {
    using meta_id = global::meta_id;
    
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();
    auto &g = interp.self().global();
    auto &chain = g.get_blockchain();
    auto &tip = chain.tip();
    auto &tip_id = tip.get_id();

    const size_t hist = 3;

    auto start = tip_id;

    for (size_t i = 0; i < hist; i++) {
	auto prev = chain.previous(start);
	if (prev.is_zero()) {
	    break;
	}
	start = prev;
    }

    size_t height_limit = tip.get_height() + 2;
    auto start_height = tip.get_height();
    if (start_height < hist) {
	start_height = 0;
    } else {
	start_height -= hist;
    }

    tree_node tree;

#if 1
    // Construct tree
    tree_node *current = tree.add_child(start, start_height);
    std::stack<tree_node *> worklist;
    std::set<meta_id> all_ids;
    worklist.push(current);
    while (!worklist.empty()) {
	auto node = worklist.top();
	worklist.pop();
	auto id = node->id;
	if (id == tip_id) node->set_here();
	all_ids.insert(id);
	if (auto *e = chain.get_meta_entry(id)) {
	    if (e->get_height() >= height_limit) {
		continue;
	    }
	}
	auto fwd = chain.follows(id);
	for (auto &fid : fwd) {
	    if (auto *child_entry = chain.get_meta_entry(fid)) {
		auto next = node->add_child(fid, child_entry->get_height());
		worklist.push(next);
	    }
	}
    }
#else
    // Debugging

    // Add random nodes to make it easier to debug
    add_node( "abcdg", &tree);
    add_node( "abcdh", &tree);
    add_node( "abcdi", &tree);
    add_node( "abcdj", &tree);
    add_node( "abcdk", &tree);
    add_node( "abxq", &tree);
    add_node( "abxr", &tree);
    add_node( "bbdrr", &tree);
    add_node( "bbdrs", &tree);
    set_here( "ba78", &tree);
#endif
    
    // Find the minimum number of prefix digits of "all_ids" that are enough
    // to distinguish them from each other.
    meta_id prev;
    size_t min_bytes = 2;
    for (auto &id : all_ids) {
	if (!prev.is_zero()) {
	    min_bytes = std::max(min_bytes, min_prefix(prev, id));
	}
	prev = id;
    }

    //             #   (  ....           ) --->
    size_t width = 6 + 1 + 2*min_bytes + 1 + 4;
    
    print_tree(&tree, width, min_bytes);

    return true;
}

static term to_term(interpreter_base &interp0, const global::meta_id &id) {
    return interp0.new_big(id.hash(), id.hash_size());
}

std::set<global::meta_id> me_builtins::get_meta_ids(interpreter_base &interp0, const std::string &name, term id) {
    auto &interp = reinterpret_cast<local_interpreter &>(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();
    auto &chain = g.get_blockchain();

    if (interp0.is_empty_list(id)) {
	std::set<global::meta_id> r;
	r.insert(chain.genesis());
	return r;
    }
    
    uint8_t prefix[global::meta_id::HASH_SIZE];
    size_t prefix_len = 0;
    if (id.tag() != tag_t::INT && id.tag() != tag_t::BIG) {
	interp.abort(interpreter_exception_wrong_arg_type(name + ": (prefix) identifier must be an integer or bignum; was " + interp.to_string(id)));
    }
    if (id.tag() == tag_t::INT) {
	int64_t val = reinterpret_cast<int_cell &>(id).value();
	uint64_t uval = static_cast<uint64_t>(val);
	db::write_uint64(prefix, uval);
	std::reverse(prefix, &prefix[8]);
	// Skip leading 0s (unless last byte)
	size_t from = 0;
	while (from < 7 && prefix[from] == 0) from++;
	if (from > 0) {
	    memmove(&prefix[0], &prefix[from], 8-from);
	    memset(&prefix[8-from], 0, from);
	}
	prefix_len = 8-from;
    } else {
	auto big = reinterpret_cast<big_cell &>(id);
	prefix_len = interp.num_bytes(big);
	if (prefix_len > 32) {
	    interp.abort(interpreter_exception_wrong_arg_type(name + ": (prefix) identifier is bigger than 32 bytes; was " + interp.to_string(id)));
	}
	interp.get_big(big, prefix, prefix_len);
    }

    return chain.find_entries(prefix, prefix_len);
}

global::meta_id me_builtins::get_meta_id(interpreter_base &interp0, const std::string &name, term id) {
    auto ids = get_meta_ids(interp0, name, id);
    if (ids.empty()) {
	throw db_exception_meta_id_not_found(name + ": Did not find entry with id.");
    }
    if (ids.size() != 1) {
	throw interpreter_exception_wrong_arg_type(name + ": Id was not unique. Try a bigger number.");
    }
    return *ids.begin();
}

bool me_builtins::chain_3(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();
    auto &chain = g.get_blockchain();

    std::string name = (arity == 3) ? "chain/3" : "chain/2";

    if (args[0].tag() != tag_t::INT) {
	interp.abort(interpreter_exception_wrong_arg_type(name + ": First argument must be an integer; was " + interp.to_string(args[0])));
    }

    int64_t height = reinterpret_cast<int_cell &>(args[0]).value();
    if (height < 0) {
	interp.abort(interpreter_exception_wrong_arg_type(name + ": First argument must be a positive integer; was " + interp.to_string(args[0])));
    }

    uint8_t prefix[global::meta_id::HASH_SIZE];
    size_t prefix_len = 0;
    if (arity == 3) {
	if (args[1].tag() != tag_t::INT && args[1].tag() != tag_t::BIG) {
	    interp.abort(interpreter_exception_wrong_arg_type(name + ": Second argument must be an integer or bignum; was " + interp.to_string(args[0])));
	}
	if (args[1].tag() == tag_t::INT) {
	    int64_t val = reinterpret_cast<int_cell &>(args[1]).value();
	    uint64_t uval = static_cast<uint64_t>(val);
	    db::write_uint64(prefix, uval);
	    std::reverse(prefix, &prefix[8]);
	    // Skip leading 0s (unless last byte)
	    size_t from = 0;
	    while (from < 7 && prefix[from] == 0) from++;
	    if (from > 0) {
		memmove(&prefix[0], &prefix[from], 8-from);
		memset(&prefix[8-from], 0, from);
	    }
	    prefix_len = 8-from;
	} else {
	    auto big = reinterpret_cast<big_cell &>(args[1]);
	    prefix_len = interp.num_bytes(big);
	    if (prefix_len > 32) {
		interp.abort(interpreter_exception_wrong_arg_type(name + ": Second argument bignum is bigger than 32 bytes; was " + interp.to_string(args[1])));
	    }
	    interp.get_big(big, prefix, prefix_len);
	}
    }

    auto ids = chain.find_entries(height, prefix, prefix_len);

    term result = interp.EMPTY_LIST;

    auto it = ids.rbegin();
    
    while (it != ids.rend()) {
	result = interp.new_dotted_pair(to_term(interp0, *it), result);
	++it;
    }

    term result_arg = arity == 3 ? args[2] : args[1];
    
    return interp.unify(result_arg, result);
}

bool me_builtins::advance_0(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();
    g.advance();
    return true;
}

bool me_builtins::discard_0(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();
    g.discard();
    return true;
}

bool me_builtins::switch_1(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();

    auto id = get_meta_id(interp0, "switch/1", args[0]);
    
    if (!g.set_tip(id)) {
	throw interpreter_exception_wrong_arg_type("switch/1: Failed to switch.");
    }

    return true;
}

bool me_builtins::tip_1(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();
    auto const &id = g.tip_id();
    auto id_term = id.to_term(interp);
    return interp.unify(args[0], id_term);
}

bool me_builtins::current_1(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();
    auto &id = g.tip_id();
    return interp.unify(args[0], to_term(interp0, id));
}

bool me_builtins::height_1(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();
    return interp.unify(args[0], int_cell(static_cast<int64_t>(g.current_height())));
}

bool me_builtins::max_height_1(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();
    return interp.unify(args[0], int_cell(static_cast<int64_t>(g.max_height())));
}

bool me_builtins::goals_2(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();

    auto id = get_meta_id(interp0, "goals/2", args[0]);
    term result = g.db_get_block(interp0, id, false);
    return interp.unify(result, args[1]);
}

bool me_builtins::block_3(interpreter_base &interp0, size_t arity, term args[]) {
    std::string name = "block/" + boost::lexical_cast<std::string>(arity);
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();

    auto id = get_meta_id(interp0, name, args[0]);
    term block = g.db_get_block(interp0, id, true);
    if (!interp.unify(block, args[1])) {
	return false;
    }
    if (arity > 2) {
	term meta_info = g.db_get_meta(interp0, id, true);
	if (!interp.unify(meta_info, args[2])) {
	    return false;
	} 
    }
    return true;
}

bool me_builtins::block_hash_2(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    if (args[0].tag() != tag_t::BIG) {
	throw interpreter_exception_wrong_arg_type("block_hash/2: First argument must be a bignum; was " + interp.to_string(args[0]));
    }
    auto &big = reinterpret_cast<big_cell &>(args[0]);
    auto n = interp.num_bytes(big);
    std::vector<uint8_t> data(n);
    interp.get_big(big, &data[0], n);

    auto locked = interp.lock_node();
    auto &g = interp.self().global();
    auto hasher = g.get_blockchain().blocks_db().get_leaf_hasher();
    
    triedb_leaf leaf;
    leaf.read(&data[0]);
    hasher(&leaf);
    auto const &cleaf = leaf;
    const uint8_t *hash = cleaf.hash();

    term result = interp.new_big(hash, leaf.hash_size());
    return interp.unify(args[1], result);
}

bool me_builtins::meta_2(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();

    auto id = get_meta_id(interp0, "meta/2", args[0]);
    term result = g.db_get_meta(interp0, id, false);
    return interp.unify(result, args[1]);
}

bool me_builtins::meta_more_2(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();

    auto id = get_meta_id(interp0, "meta_more/2", args[0]);
    term result = g.db_get_meta(interp0, id, true);
    return interp.unify(result, args[1]);
    
}

bool me_builtins::validate_meta_1(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();
    auto &g = interp.self().global();
    global::meta_entry entry;
    g.db_parse_meta(interp, args[0], entry);
    auto const *found_entry = g.get_blockchain().get_meta_entry(entry.get_id());
    if (found_entry == nullptr) {
	return false;
    }
    if (*found_entry != entry) {
	return false;
    }

    node_hash block_hash, heap_hash, closure_hash, symbols_hash, program_hash;
    uint64_t block_num = 0, heap_num = 0, closure_num = 0, symbols_num = 0, program_num = 0;

    term lst = interp.arg(args[0], 0);
    while (interp.is_dotted_pair(lst)) {
	auto prop = interp.arg(lst, 0);
	if (prop.tag() == tag_t::STR && interp.functor(prop) == con_cell("db",3)) {
	    node_hash *hash_p = nullptr;
	    uint64_t *num_p = nullptr;
	    auto dbname = interp.arg(prop, 0);
	    if (dbname == con_cell("block",0)) {
		hash_p = &block_hash;
		num_p = &block_num;
	    } else if (dbname == con_cell("heap",0)) {
		hash_p = &heap_hash;
		num_p = &heap_num;
	    }  else if (dbname == con_cell("closure",0)) {
		hash_p = &closure_hash;
		num_p = &closure_num;		
	    } else if (dbname == con_cell("symbols",0)) {
		hash_p = &symbols_hash;
		num_p = &symbols_num;
	    } else if (dbname == con_cell("program",0)) {
		hash_p = &program_hash;
		num_p = &program_num;		
	    }

	    if (hash_p && num_p) {
		auto num_term = interp.arg(prop, 1);
		if (num_term.tag() != tag_t::INT) {
		    return false;
		}
		*num_p = static_cast<uint64_t>(reinterpret_cast<int_cell &>(num_term).value());
		auto hash_term = interp.arg(prop, 2);
		if (hash_term.tag() != tag_t::BIG) {
		    return false;
		}
		auto big = reinterpret_cast<big_cell &>(hash_term);
		size_t hash_size = interp.num_bytes(big);
		if (hash_size > 128) {
		    return false;
		}
		uint8_t hash_data[128];
		interp.get_big(hash_term, hash_data, hash_size);
		hash_p->set_hash(hash_data, hash_size);
	    }
	}
	lst = interp.arg(lst, 1);
    }


    blake2b_state s;
    blake2b_init(&s, BLAKE2B_OUTBYTES);

    // Add previous id
    blake2b_update(&s, entry.get_previous_id().hash(), global::meta_id::HASH_SIZE);

    auto const &block_hash_const = block_hash;
    auto const &heap_hash_const = heap_hash;
    auto const &closure_hash_const = closure_hash;
    auto const &symbols_hash_const = symbols_hash;
    auto const &program_hash_const = program_hash;
    blake2b_update(&s, block_hash_const.hash(), block_hash_const.hash_size());
    blake2b_update(&s, heap_hash_const.hash(), heap_hash_const.hash_size());
    blake2b_update(&s, closure_hash_const.hash(), closure_hash_const.hash_size());
    blake2b_update(&s, symbols_hash_const.hash(), symbols_hash_const.hash_size());
    blake2b_update(&s, program_hash_const.hash(), program_hash_const.hash_size());

    uint8_t data[1024];

    // Add number of entries in heap, closures, symbols and program.
    db::write_uint64(data, heap_num);
    blake2b_update(&s, data, sizeof(uint64_t));
    db::write_uint64(data, closure_num);
    blake2b_update(&s, data, sizeof(uint64_t));
    db::write_uint64(data, symbols_num);
    blake2b_update(&s, data, sizeof(uint64_t));
    db::write_uint64(data, program_num);
    blake2b_update(&s, data, sizeof(uint64_t));
    
    // Add version
    db::write_uint64(data, entry.get_version());
    blake2b_update(&s, data, sizeof(uint64_t));

    // Add height
    db::write_uint32(data, entry.get_height());
    blake2b_update(&s, data, sizeof(uint32_t));

    // Add nonce
    db::write_uint64(data, entry.get_nonce());
    blake2b_update(&s, data, sizeof(uint64_t));

    // Add timestamp
    auto &ts = entry.get_timestamp();
    ts.write(data);
    blake2b_update(&s, data, ts.serialization_size());

    // Add pow difficulty
    entry.get_pow_difficulty().write(data);
    blake2b_update(&s, data, entry.get_pow_difficulty().serialization_size());

    memset(data, 0, sizeof(data));
    blake2b_final(&s, data, BLAKE2B_OUTBYTES);

    // This hash should equal the id of the entry
    return memcmp(data, entry.get_id().hash(), entry.get_id().hash_size()) == 0;
}

bool me_builtins::metas_3(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();

    try {
	auto id = get_meta_id(interp0, "metas/3", args[0]);

	auto n_term = args[1];
	if (n_term.tag() != tag_t::INT) {
	    interp.abort(interpreter_exception_wrong_arg_type("metas/3: Second argument must be an integer; was " + interp.to_string(n_term)));
	}

	int64_t n = reinterpret_cast<int_cell &>(n_term).value();
	if (n < 1 || n > 1000) {
	    interp.abort(interpreter_exception_wrong_arg_type("metas/3: Second argument must be a number within 1..1000; was " + boost::lexical_cast<std::string>(n)));
	}    
    
	term result = g.db_get_metas(interp0, id, n);

	return interp.unify(result, args[2]);
    } catch (const db_exception_meta_id_not_found &) {
	return false;
    }
}

bool me_builtins::meta_roots_4(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();

    try {
	global::meta_id id = get_meta_id(interp0, "meta_roots/4", args[0]);
	auto spacing_term = args[1];
	if (spacing_term.tag() != tag_t::INT) {
	    interp.abort(interpreter_exception_wrong_arg_type("meta_roots/4: Second argument must be an integer; was " + interp.to_string(spacing_term)));
	}
	int64_t spacing = reinterpret_cast<int_cell &>(spacing_term).value();
	if (spacing < 1 || spacing > 1000) {
	    interp.abort(interpreter_exception_wrong_arg_type("meta_roots/4: Second argument must be a number within 1..1000; was " + boost::lexical_cast<std::string>(spacing)));	
	}
	
	auto n_term = args[2];
	if (n_term.tag() != tag_t::INT) {
	    interp.abort(interpreter_exception_wrong_arg_type("meta_roots/4: Thid argument must be an integer; was " + interp.to_string(n_term)));
	}
	
	int64_t n = reinterpret_cast<int_cell &>(n_term).value();
	if (n < 1 || n > 1000) {
	    interp.abort(interpreter_exception_wrong_arg_type("meta_roots/4: Third argument must be a number within 1..1000; was " + boost::lexical_cast<std::string>(n)));
	}
	term result = g.db_get_meta_roots(interp0, id, spacing, n);
	return interp.unify(result, args[3]);
    } catch (const db_exception_meta_id_not_found &) {
	return false;
    }
}

bool me_builtins::put_meta_1(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();

    try {
	g.db_put_meta(interp0, args[0]);
	g.get_blockchain().meta_db().flush();
    } catch (const global::global_db_exception &ex) {
	throw db_exception(ex.what());
    }
    return true;
}

bool me_builtins::put_metas_1(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();

    try {
	g.db_put_metas(interp0, args[0]);
	g.get_blockchain().meta_db().flush();	
    } catch (const global::global_db_exception &ex) {
	throw db_exception(ex.what());
    }

    return true;    
}

bool me_builtins::delay_put_metas_1(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();

    term lst = args[0];
    if (!interp.is_dotted_pair(lst)) {
	return false;
    }
    auto meta_term = interp.arg(lst, 0);
    global::meta_entry entry;

    try {
	if (!g.db_parse_meta(interp, meta_term, entry)) {
	    // Don't delay because of parse errors! We want to know and fail!
	    return false;
	}
    } catch (const global::global_db_exception &ex) {
	throw db_exception(ex.what());
    }
    auto &previd = entry.get_previous_id();
    if (previd.is_zero()) {
	return false;
    }
	 
    if (g.get_blockchain().get_meta_entry(previd)) {
	return false;
    }

    // We have a missing previous entry, so let's delay this
    return true;
}

bool me_builtins::branches_2(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    
    // First build graph
    struct meta_small {
	global::meta_id id;
	global::meta_id prev_id;
	size_t height;
    };
    global::meta_id longest_id;
    size_t longest_id_height = 0;
    bool ambiguous = false;
    std::unordered_map<global::meta_id, meta_small> small_map;
    std::map<global::meta_id, std::unordered_set<global::meta_id> > follows;
    auto lst = args[0];
    while (interp.is_dotted_pair(lst)) {
	auto meta_term = interp.arg(lst, 0);
	meta_small small;
	bool id_ok = false, previd_ok = false, height_ok = false;
	bool error = false;
	if (interp.is_functor(meta_term, con_cell("meta",1))) {
	    auto prop_lst = interp.arg(meta_term, 0);
	    while (!error && interp.is_dotted_pair(prop_lst)) {
		auto prop = interp.arg(prop_lst, 0);
		if (interp.is_functor(prop, con_cell("id",1))) {
		    if (small.id.from_term(interp, interp.arg(prop, 0))) {
			id_ok = true;
		    } else {
			error = true;
		    }
		} else if (interp.is_functor(prop, con_cell("previd",1))) {
		    if (small.prev_id.from_term(interp, interp.arg(prop, 0))) {
			previd_ok = true;
		    } else {
			error = true;
		    }
		} else if (interp.is_functor(prop, con_cell("height",1))) {
		    auto hterm = interp.arg(prop, 0);
		    if (hterm.tag() != tag_t::INT) {
			error = true;
			break;
		    }
		    auto v = reinterpret_cast<int_cell &>(hterm).value();
		    if (v >= 0) {
			small.height = static_cast<size_t>(v);
			height_ok = true;
			if (small.height > longest_id_height) {
			    longest_id_height = small.height;
			    longest_id = small.id;
			    ambiguous = false;
			} else if (small.height == longest_id_height) {
			    ambiguous = true;
			}
		    } else {
			error = true;
		    }
		}
		prop_lst = interp.arg(prop_lst, 1);
	    }
	    if (!error && id_ok && previd_ok && height_ok) {
		small_map[small.id] = small;
		follows[small.prev_id].insert(small.id);
	    }
	}
	lst = interp.arg(lst, 1);
    }

    // Find the longest chain so we can prune unncessary
    // branches; the longest chain is not considered to be a branch.
    std::unordered_set<global::meta_id> longest_chain;

    if (!ambiguous) {
	while (!longest_id.is_zero()) {
	    longest_chain.insert(longest_id);
	    longest_id = small_map[longest_id].prev_id;
	}
    }

    std::vector<meta_small> branchpoints;

    for (auto &e : follows) {
	auto &fids = e.second;

	if (fids.size() > 1) {
	    for (auto &fid : fids) {
		if (!longest_chain.count(fid)) {
		    branchpoints.push_back(small_map[fid]);
		}
	    }
	}
    }

    std::reverse(branchpoints.begin(), branchpoints.end());
    
    std::sort(branchpoints.begin(), branchpoints.end(),
	      [](const meta_small &a, const meta_small &b)
	      { if (a.height < b.height) {
		      return -1;
		  } else if (a.height > b.height) {
		      return 1;
		  } else {
		      if (a.id < b.id) {
			  return -1;
		      } if (a.id == b.id) {
			  return 0;
		      } else {
			  return 1;
		      }
		  }
	      });
    
    // At this point we have all branchpoints. Build list result.
    term head = interp.EMPTY_LIST, tail = interp.EMPTY_LIST;
    for (auto &b : branchpoints) {
	auto new_tail =
	    interp.new_dotted_pair(
		   interp.new_term( con_cell("branch", 2),
				    { int_cell(static_cast<int64_t>(b.height)),
				      b.id.to_term(interp) } ),
		   interp.EMPTY_LIST);
	if (head == interp.EMPTY_LIST) {
	    head = new_tail;
	    tail = new_tail;
	} else {
	    interp.set_arg(tail, 1, new_tail);
	    tail = new_tail;
	}
    }

    return interp.unify(args[1], head);
}

bool me_builtins::follow_3(interpreter_base &interp0, size_t arity, term args[]) {
    std::string name = "follow/2";
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();
    global::global &g = interp.self().global();
    global::meta_id id = get_meta_id(interp0, name, args[0]);

    auto num_term = args[1];
    if (num_term.tag() != tag_t::INT) {
	interp.abort(interpreter_exception_wrong_arg_type(name + ": Second argument must be an integer; was " + interp.to_string(num_term)));
    }
    int64_t v = reinterpret_cast<int_cell &>(num_term).value();
    if (v < 1 || v > 1000) {
	throw interpreter_exception_wrong_arg_type(name + ": Second argument must be a number within 1..1000; was " + boost::lexical_cast<std::string>(v));
    }
    size_t n = static_cast<size_t>(v);
    term path = g.db_best_path(interp, id, n);

    return interp.unify(args[2], path);
}

bool me_builtins::setup_commit_1(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    global::global &g = interp.self().global();

    term_serializer::buffer_t buf;
    term_serializer ser(interp);
    ser.write(buf, args[0]);
    
    g.setup_commit(buf);

    return true;
}

bool me_builtins::commit(local_interpreter &interp, term_serializer::buffer_t &buf, term t, bool naming)
{
    auto locked = interp.lock_node();
    global::global &g = interp.self().global();

    if (!g.has_interp()) {
	return false;
    }
    
    g.check_interp();
    
    g.set_naming(naming);

    if (t.tag() == tag_t::BIG) {
	// Assume this is an encoded leaf
	auto &big = reinterpret_cast<big_cell &>(t);
	auto n = interp.num_bytes(big);
	buf.resize(n);
	interp.get_big(big, &buf[0], n);
	triedb_leaf leaf;
	leaf.read(&buf[0]);
	n = leaf.custom_data_size();
	auto *p = leaf.custom_data();
	buf.resize(n);
	std::copy(&p[0], &p[n], &buf[0]);
    } else {
	// Serialize term
	term_serializer ser(interp);
	buf.clear();
	ser.write(buf, t);
    }

    if (!g.execute_commit(buf)) {
	return false;
    }

    return true;
}

bool me_builtins::commit_2(interpreter_base &interp0, size_t arity, term args[])
{
    // commit(X)
    // commit(X, naming)
    // Attempt to put X on the global interpreter.
    // Special if X is a clause: p(V) :- Body, then V is first bound
    // to the hash (by serialization) of Body, and then Body put on the
    // global interpreter. If X cannot be put on the global interpreter
    // this predicate fails. After X has been put on the global interpreter,
    // a cut is also executed on the global interpreter to ensure the
    // trail and stack becomes empty. The state of the global interpreter
    // is solely defined by the heap and the set of frozen closures (which
    // models UTXOs)

    auto &interp = to_local(interp0);

    interp.root_check("commit", arity);

    bool naming = arity >= 2 && args[1] == con_cell("naming",0);
    
    buffer_t buf;
    if (!commit(interp, buf, args[0], naming)) {
        return false;
    }
    
    return true;
}

bool me_builtins::global_impl(interpreter_base &interp0, size_t arity, term args[], bool silent)
{
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    

    interp.root_check("global", arity);
  
    global::global &g = interp.self().global();

    term t = args[0];
    
    // First serialize
    term_serializer ser(interp);
    buffer_t buf;
    buf.clear();
    ser.write(buf, t);

    if (!g.execute_goal(buf, silent)) {
	g.reset();
	return false;
    }

    // assert(g.is_clean());

    if (silent) {
	return true;
    } else {
	term t1 = ser.read(buf);
	return interp.unify(t, t1);
    }
}

bool me_builtins::global_1(interpreter_base &interp0, size_t arity, term args[] )
{
    return global_impl(interp0, arity, args, false);
}

bool me_builtins::global_silent_1(interpreter_base &interp0, size_t arity, term args[] )
{
    return global_impl(interp0, arity, args, true);
}
//
// Useful for testing. Adding arbitrary code to run when initializing syncing.
//
bool me_builtins::sync_init_1(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);    
    if (args[0].tag().is_ref()) {
	auto str = interp.self().syncer().get_init();
	if (str.empty()) {
	    return interp.unify(args[0], interp.EMPTY_LIST);
	} else {
	    // Attempt to parse it and return term
	    try {
		auto t = interp.parse(str + ".");
		return interp.unify(args[0], t);
	    } catch (std::runtime_error &ex) {
		return interp.unify(args[0], interp.string_to_list(ex.what()));
	    }
	}
    } else {
	// Set it
	std::string str = interp.to_string(args[0]);
	interp.self().syncer().set_init(str);
	return true;
    }
}

bool me_builtins::sync_0(interpreter_base &interp0, size_t arity, term args[])
{
    auto &interp = to_local(interp0);    
    interp.self().start_sync();
    return true;
}

bool me_builtins::sync_1(interpreter_base &interp0, size_t arity, term args[])
{
    auto &interp = to_local(interp0);

    auto rootid = get_meta_id(interp0, "sync/1", args[0]);
    interp.self().syncer().set_root(rootid);

    return true;
}


bool me_builtins::sync_complete_1(interpreter_base &interp0, size_t arity, term args[])
{
    auto &interp = to_local(interp0);
    auto c = interp.self().syncer().is_complete() ? con_cell("true",0) : con_cell("false",0);    
    return interp.unify(args[0], c);
}

bool me_builtins::sync_mode_1(interpreter_base &interp0, size_t arity, term args[])
{
    auto &interp = to_local(interp0);
    auto mode = interp.self().syncer().get_mode();
    if (mode.empty()) {
	return interp.unify(args[0], interp.EMPTY_LIST);
    } else {
	auto c = interp.functor(mode, 0);
	return interp.unify(args[0], c);
    }
}

bool me_builtins::syncing_meta_1(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto block = static_cast<int64_t>(interp.self().syncer().get_at_meta_block());
    return interp.unify(args[0], int_cell(block));
}

bool me_builtins::sync_progress_1(interpreter_base &interp0, size_t arity, term args[]) {
    std::string name = "sync_progress/" + boost::lexical_cast<std::string>(arity);
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();

    if (!interp.self().has_syncer()) {
	if (arity == 0) {
	    std::cout << "None" << std::endl;
	    return true;
	} else {
	    return interp.unify(args[0], interp.EMPTY_LIST);
	}
    }
    
    auto mode = interp.self().syncer().get_mode();
    auto progress = interp.self().syncer().get_progress();
    auto meta_id = interp.self().syncer().get_root();
    auto meta_id_term = meta_id.to_term(interp);
    auto meta_entry = interp.self().global().get_blockchain().get_meta_entry(meta_id);

    if (arity == 0) {
	if (interp.self().syncer().is_complete()) {
	    std::cout << "Completed" << std::endl;
	    return true;
	}
	if (mode == "wait") {
	    std::cout << "Waiting for selected root..." << std::endl;
	    return true;
	}
	auto meta_id_str = interp.to_string(meta_id_term);
	auto short_id_str = hex::to_string(meta_id.hash(), 2);	
	std::cout << "Syncing: Root    : " << meta_id_str << " (" << short_id_str << ")" << std::endl;
	if (meta_entry == nullptr) {
	    std::cout << "         Not found" << std::endl;
	    return false;
	}
	std::cout << "         Height  : " << meta_entry->get_height() << std::endl;
	std::cout << "         DB      : " << mode << std::endl;
	std::cout << "         Progress: " << progress << "%" << " [";
	size_t n = 50 * progress / 100;
	std::cout << std::string(n, '*') << std::string(50-n, ' ') << "]" << std::endl;
	return true;
    } else {
	assert(arity == 1);
	term lst = interp.EMPTY_LIST;
	if (mode == "meta") {
	    auto current = interp.self().syncer().get_at_meta_block();
	    lst = interp.new_dotted_pair(
	        interp.new_term(interp.functor("progress",1),
			{ int_cell(static_cast<int64_t>(progress)) }), lst);
	    lst = interp.new_dotted_pair(
	     interp.new_term(con_cell("height",1),{ int_cell(current) }), lst);
	    
	    lst = interp.new_dotted_pair(
	     interp.new_term(con_cell("mode",1), { con_cell("meta",0)}), lst);
	} else if (mode == "wait") {
	    lst = interp.new_dotted_pair(
	        interp.new_term(interp.functor("progress",1),
			{ int_cell(static_cast<int64_t>(progress)) }), lst);
	    lst = interp.new_dotted_pair(
	     interp.new_term(con_cell("mode",1), { con_cell("wait",0)}), lst);
	} else {
	    lst = interp.new_dotted_pair(
	        interp.new_term(interp.functor("progress",1),
			{ int_cell(static_cast<int64_t>(progress)) }), lst);
	    lst = interp.new_dotted_pair(
		interp.new_term(con_cell("db", 1),
			   { interp.functor(mode,0) }), lst);
	    lst = interp.new_dotted_pair(
	        interp.new_term(con_cell("root",1), { meta_id_term }),lst);
	    lst = interp.new_dotted_pair(
		interp.new_term(con_cell("mode",1), { con_cell("db",0)}), lst);
	}
	return interp.unify(args[0], lst);
    }
}

bool me_builtins::sync_point_2(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    auto &g = interp.self().global();

    std::string name = "sync_point/" + boost::lexical_cast<std::string>(arity);
    int64_t height = 0;
    term result;
    if (arity == 2) {
	auto height_term = args[0];
	if (height_term.tag() != tag_t::INT) {
	    throw interpreter_exception_wrong_arg_type(name + ": First argument must be an integer (a negative integer represents relative height from the tip.");
	}
	height = reinterpret_cast<int_cell &>(height_term).value();
	result = args[1];
    } else {
	height = 0;
	result = args[0];
    }
    
    auto sync_id = g.get_blockchain().sync_point(height);
    term term_sync_id = sync_id.to_term(interp);
    return interp.unify(result, term_sync_id);
}

bool me_builtins::ignore_pow_1(interpreter_base &interp0, size_t arity, term args[])
{
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();    
    if (args[0].tag().is_ref()) {
	auto c = interp.self().check_pow() ? con_cell("false",0) : con_cell("true",0);
	return interp.unify(args[0], c);
    } else if (args[0] == con_cell("false",0)) {
	interp.self().set_check_pow(true);
    } else if (args[0] == con_cell("true",0)) {
	interp.self().set_check_pow(false);
    } else {
	throw interpreter_exception_wrong_arg_type("ignore_pow/1: Argument must be a variable, or 'true' or 'false.'");
    }
    return true;
}

term me_builtins::build_leaf_term(interpreter_base &interp0, const merkle_leaf *lf, size_t pos)
{
    auto &interp = to_local(interp0);

    auto key = lf->key();

    auto hash_term = interp.new_big(lf->hash_size()*8);
    interp.set_big(hash_term, lf->hash(), lf->hash_size());
    term data_term = interp.EMPTY_LIST;
    if (lf->has_data()) {
	auto const &data = lf->data();
	if (data.data() != nullptr) {
	    data_term = interp.new_big(data.size()*8);
	    interp.set_big(data_term, data.data(), data.size());
	}
    }
    auto leaf_term = interp.new_term(con_cell("leaf",4),
				     {hash_term,
				      int_cell(checked_cast<int64_t>(pos)),
				      int_cell(checked_cast<int64_t>(key)),
				      data_term});
    return leaf_term;
}

term me_builtins::build_tree_term(interpreter_base &interp0, const merkle_branch *br, size_t pos)
{
    auto &interp = to_local(interp0);    
    
    term tail = interp.EMPTY_LIST;
    term lst = tail;

    auto m = br->mask();
    while (m) {
	size_t sub_index = common::lsb(m);
	m &= (static_cast<uint32_t>(-1) << sub_index) << 1;
	auto const &child = br->get_child(sub_index);
	auto const *node = child.get();
	term node_term;
	if (node->type() == merkle_node::BRANCH) {
	    auto const *sub_br = reinterpret_cast<const merkle_branch *>(node);
	    node_term = build_tree_term(interp0, sub_br, sub_index);
	} else {
	    auto const *sub_lf = reinterpret_cast<const merkle_leaf *>(node);
	    node_term = build_leaf_term(interp0, sub_lf, sub_index);
	}
	if (tail == interp.EMPTY_LIST) {
	    lst = tail = interp.new_dotted_pair(node_term, tail);
	} else {
	    term new_tail = interp.new_dotted_pair(node_term, interp.EMPTY_LIST);
	    interp.set_arg(tail, 1, new_tail);
	    tail = new_tail;
	}
    }
    term hash_term = interp.EMPTY_LIST;
    if (br->hash_size() != 0) {
	hash_term = interp.new_big(br->hash_size()*8);
	interp.set_big(hash_term, br->hash(), br->hash_size());
    }
    auto pos1 = int_cell(checked_cast<int64_t>(pos));
    auto depth = int_cell(checked_cast<int64_t>(br->depth()));
    return interp.new_term(con_cell("branch", 4), {hash_term,depth,pos1,lst});
}

//
//         <id>    <type> <from key> <to key>
// db_get(<root id>, heap, 10,         2000,                 X)
//  X will become a tree of data.
//
term me_builtins::db_get(interpreter_base &interp0, const std::string &name, size_t arity, term args[], bool compute_size_only) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();

    auto id = get_meta_id(interp, name, args[0]);
    
    if (args[1].tag() != tag_t::CON) {
	throw interpreter_exception_wrong_arg_type(name + ": Second argument must be an atom.");
    }

    triedb *db = nullptr;
    root_id root_id;

    std::tie(db,root_id) = get_db_root(interp0, args[1], id);
    if (db == nullptr) {
	throw interpreter_exception_wrong_arg_type(name + ": Second argument must be 'block', 'heap', 'closure', 'symbols' or 'program'");
    }

    if (root_id.is_zero()) {
	throw interpreter_exception_wrong_arg_type(name + ": Data is unavailable.");
    }

    auto from_key = args[2];
    if (from_key.tag() != tag_t::INT) {
	throw interpreter_exception_wrong_arg_type(name + ": Key must be an integer; was " + interp.to_string(from_key));
    }
    
    auto to_key = args[3];
    if (to_key.tag() != tag_t::INT) {
	throw interpreter_exception_wrong_arg_type(name + ": Key must be an integer; was " + interp.to_string(to_key));
    }

    auto from_key_val = reinterpret_cast<int_cell &>(from_key).value();
    auto to_key_val = reinterpret_cast<int_cell &>(to_key).value();

    merkle_root mtree;
    mtree.set_limit(2*triedb_params::MB); // Not more than 2 MB
    bool include_data = !compute_size_only;

    if (!db->get(root_id, from_key_val, to_key_val, include_data, mtree)) {
	if (mtree.total_size() > mtree.limit_size()) {
	    throw interpreter_exception_wrong_arg_type(name + ": Size of data set exceeded limit " + boost::lexical_cast<std::string>(mtree.limit_size()));
	}
	throw interpreter_exception_wrong_arg_type(name + ": Failed to get datal.");
    }

    term result = interp.EMPTY_LIST;

    if (compute_size_only) {
	result = int_cell(static_cast<int64_t>(mtree.total_size()));
    } else {
	result = build_tree_term(interp, &mtree, 0);
    }
    return result;
}

//
//         <id>    <type> <from key> <to key>
// db_get(<root id>, heap, 10,         2000,                 X)
//  X will become a tree of data.
//
bool me_builtins::db_get_5(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);    
    term result = db_get(interp0, "db_get/5", arity, args, false);
    return interp.unify(result, args[4]);
}

static size_t merkle_leaf_count(interpreter_base &interp0, term mtree_term, uint64_t from_key, uint64_t to_key)
{
    if (mtree_term.tag() != tag_t::STR) {
	return 0;
    }
    auto f = interp0.functor(mtree_term);
    size_t n = 0;
    if (f == con_cell("branch",4)) {
	auto children = interp0.arg(mtree_term, 3);
	while (interp0.is_dotted_pair(children)) {
	    auto child = interp0.arg(children, 0);
	    n += merkle_leaf_count(interp0, child, from_key, to_key);
	    children = interp0.arg(children, 1);
	}
    } else if (f == con_cell("leaf",4)) {
	auto key = interp0.arg(mtree_term, 2);
	if (key.tag() == tag_t::INT) {
	    auto k = static_cast<uint64_t>(reinterpret_cast<int_cell &>(key).value());
	    if (from_key <= k && k <= to_key) {
		n++;
	    }
	}
    }
    return n;
}

//
// db_num(<root id>, <type>, Num)
// Current number of symbols in database (used to calculate progress)
//
bool me_builtins::db_num_3(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();
    
    std::string name = "db_num/3";

    auto id = get_meta_id(interp, name, args[0]);

    if (args[1].tag() != tag_t::CON) {
	throw interpreter_exception_wrong_arg_type(name + ": Second argument must be an atom.");
    }

    triedb *db = nullptr;
    root_id root_id;

    std::tie(db,root_id) = get_db_root(interp0, args[1], id);
    if (db == nullptr) {
	throw interpreter_exception_wrong_arg_type(name + ": Second argument must be 'heap', 'closure', 'symbols' or 'program'");
    }

    if (root_id.is_zero()) {
	return interp.unify(args[2], int_cell(0));
    }

    return interp.unify(args[2], int_cell(static_cast<int64_t>(db->num_entries(root_id))));
}

bool me_builtins::db_num_4(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);

    std::string name = "db_num/4";
    
    auto mtree_term = args[0];

    auto from_key = args[1];
    if (from_key.tag() != tag_t::INT) {
	throw interpreter_exception_wrong_arg_type(name + ": Key must be an integer; was " + interp.to_string(from_key));
    }
    
    auto to_key = args[2];
    if (to_key.tag() != tag_t::INT) {
	throw interpreter_exception_wrong_arg_type(name + ": Key must be an integer; was " + interp.to_string(to_key));
    }

    auto from_key_val = reinterpret_cast<int_cell &>(from_key).value();
    auto to_key_val = reinterpret_cast<int_cell &>(to_key).value();

    auto n = merkle_leaf_count(interp0, mtree_term, from_key_val, to_key_val);

    return interp.unify(args[3], int_cell(static_cast<int64_t>(n)));
}

bool me_builtins::db_size_5(interpreter_base &interp0, size_t arity, term args[]) {
    auto &interp = to_local(interp0);    
    term result = db_get(interp0, "db_size/5", arity, args, true);
    return interp.unify(result, args[4]);
}

std::pair<triedb *, root_id> me_builtins::get_db_root(interpreter_base &interp0, term db_name, const global::meta_id &id) {
    db::triedb *db = nullptr;
    db::root_id rid;

    static con_cell BLOCK("block", 0);
    static con_cell HEAP("heap", 0);
    static con_cell CLOSURE("closure",0);
    static con_cell SYMBOLS("symbols",0);
    static con_cell PROGRAM("program",0);
    auto &interp = to_local(interp0);
    auto &chain = interp.self().global().get_blockchain();

    auto e = chain.get_meta_entry(id);    

    if (db_name == BLOCK) {
	db = &chain.blocks_db();
	rid = e->get_root_id_blocks();
    } else if (db_name == HEAP) {
	db = &chain.heap_db();
	rid = e->get_root_id_heap();
    } else if (db_name == CLOSURE) {
	db = &chain.closure_db();
	rid = e->get_root_id_closure();
    } else if (db_name == SYMBOLS) {
	db = &chain.symbols_db();
	rid = e->get_root_id_symbols();
    } else if (db_name == PROGRAM) {
	db = &chain.program_db();
	rid = e->get_root_id_program();
    }

    return std::make_pair(db, rid);
}

void me_builtins::set_db_root(interpreter_base &interp0, term db_name, const global::meta_id &id, const db::root_id &new_root_id) {
    static con_cell BLOCK("block",0);
    static con_cell HEAP("heap", 0);
    static con_cell CLOSURE("closure",0);
    static con_cell SYMBOLS("symbols",0);
    static con_cell PROGRAM("program",0);
    auto &interp = to_local(interp0);
    auto &chain = interp.self().global().get_blockchain();

    auto ep = chain.get_meta_entry(id);
    if (ep == nullptr) {
	throw interpreter_exception_wrong_arg_type("Cannot find meta entry for id");
    }

    auto e = *ep;

    if (db_name == BLOCK) {
	e.set_root_id_blocks(new_root_id);
    } else if (db_name == HEAP) {
	e.set_root_id_heap(new_root_id);
    } else if (db_name == CLOSURE) {
	e.set_root_id_closure(new_root_id);
    } else if (db_name == SYMBOLS) {
	e.set_root_id_symbols(new_root_id);
    } else if (db_name == PROGRAM) {
	e.set_root_id_program(new_root_id);
    }

    chain.update_meta_entry(e);
}

//
// db_keys(<root id>, <type>, <from key>, <num key>, X)
//
bool me_builtins::db_keys_5(interpreter_base &interp0, size_t arity, term args[]) {
    static const std::string name = "db_keys/5";

    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();

    auto id = get_meta_id(interp, name, args[0]);
    
    if (args[1].tag() != tag_t::CON) {
	throw interpreter_exception_wrong_arg_type(name + ": Second argument must be an atom.");
    }

    triedb *db = nullptr;
    root_id root_id;

    std::tie(db, root_id) = get_db_root(interp0, args[1], id);
    if (db == nullptr) {
	throw interpreter_exception_wrong_arg_type(name + ": Second argument must be 'heap', 'closure', 'symbols' or 'program'");	
    }

    if (root_id.is_zero()) {
	throw interpreter_exception_wrong_arg_type(name + ": Data is unavailable.");
    }    

    auto from_key = args[2];
    if (from_key.tag() != tag_t::INT) {
	throw interpreter_exception_wrong_arg_type(name + ": Key must be an integer; was " + interp.to_string(from_key));
    }
    
    auto num_keys = args[3];
    if (num_keys.tag() != tag_t::INT) {
	throw interpreter_exception_wrong_arg_type(name + ": Number of keys must be an integer; was " + interp.to_string(num_keys));
    }

    auto from_key_val = reinterpret_cast<int_cell &>(from_key).value();
    auto num_keys_val = reinterpret_cast<int_cell &>(num_keys).value();

    merkle_root mtree;

    size_t limit = 2*triedb_params::MB; // Not more than 2 MB
    mtree.set_limit(limit);
    mtree.set_num_keys(num_keys_val);

    if (!db->get(root_id, from_key_val, std::numeric_limits<uint64_t>::max(),
		 true, mtree)) {
	if (mtree.total_size() > mtree.limit_size()) {
	    throw interpreter_exception_wrong_arg_type(name + ": Size of data set exceeded limit " + boost::lexical_cast<std::string>(mtree.limit_size()));
	}
    }

    auto result = build_tree_term(interp, &mtree, 0);

    return interp.unify(result, args[4]);
}

term me_builtins::build_key_list(interpreter_base &interp0, const std::vector<uint64_t> &keys)
{
    term result = interp0.EMPTY_LIST;
    term tail = result;
    for (auto key : keys) {
	auto new_tail = interp0.new_dotted_pair(int_cell(static_cast<int64_t>(key)),
						interp0.EMPTY_LIST);
	if (interp0.is_empty_list(tail)) {
	    tail = new_tail;
	    result = tail;
	} else {
	    interp0.set_arg(tail, 1, new_tail);
	    tail = new_tail;
	}
    }
    return result;
}

//
// db_keys(MerkleTree, FromKey, NumKeys, Keys)
//
// Validate tree and extract keys
//
bool me_builtins::db_keys_4(interpreter_base &interp0, size_t arity, term args[]) {
    static const std::string name = "db_keys/4";
    
    auto &interp = to_local(interp0);

    merkle_root mtree;
    size_t pos = 0;
    if (!build_merkle_tree(interp0, args[0], mtree, pos)) {
	return false;
    }

    auto from_key = args[1];
    if (from_key.tag() != tag_t::INT) {
	throw interpreter_exception_wrong_arg_type(name + ": Key must be an integer; was " + interp.to_string(from_key));
    }
    
    auto num_keys = args[2];
    if (num_keys.tag() != tag_t::INT) {
	throw interpreter_exception_wrong_arg_type(name + ": Number of keys must be an integer; was " + interp.to_string(num_keys));
    }
    
    auto from_key_val = reinterpret_cast<int_cell &>(from_key).value();
    auto num_keys_val = reinterpret_cast<int_cell &>(num_keys).value();
    
    std::vector<uint64_t> keys;
    mtree.get_keys(from_key_val, num_keys_val, keys);

    if (!keys.empty()) {
	uint64_t min_key = keys[0];
	uint64_t max_key = keys.back();

	if (!mtree.validate(nullptr, min_key, max_key)) {
	    return false;
	}
    } else {
	if (!mtree.validate_end(nullptr, from_key_val)) {
	    return false;
	}
    }
		
    auto result = build_key_list(interp0, keys);

    return interp.unify(args[3], result);
}

//
// db_end(MerkleTree, Key)
//
// Check if Key is the last key (or in the range of the empty sub-tree
// at the right of the last key)
//
bool me_builtins::db_end_2(interpreter_base &interp0, size_t arity, term args[]) {
    static const std::string name = "db_end/2";

    merkle_root mtree;
    size_t pos = 0;
    if (!build_merkle_tree(interp0, args[0], mtree, pos)) {
	return false;
    }

    auto key_term = args[1];
    if (key_term.tag() != tag_t::INT) {
	throw interpreter_exception_wrong_arg_type(name + ": Key must be an integer; was " + interp0.to_string(key_term));
    }
    auto key = reinterpret_cast<int_cell &>(key_term).value();

    return mtree.validate_end(nullptr, key);
}

bool me_builtins::set_hash(interpreter_base &interp0, term t, merkle_node &mnode) {
    static const size_t MAX_HASH_SIZE_BYTES = 256;
    
    if (t.tag() != tag_t::BIG) {
	return false;
    }
    big_cell &bc = reinterpret_cast<big_cell &>(t);
    size_t num_bits = interp0.num_bits(bc);
    size_t num_bytes = (num_bits + 7) / 8;
    if (num_bytes > MAX_HASH_SIZE_BYTES) {
	return false;
    }
    uint8_t hash[MAX_HASH_SIZE_BYTES];
    interp0.get_big(bc, hash, num_bytes);
    mnode.set_hash(hash, num_bytes);

    return true;
}

bool me_builtins::set_depth(interpreter_base &interp0, term t, merkle_branch &mbr) {
    if (t.tag() != tag_t::INT) {
	return false;
    }
    int_cell &ic = reinterpret_cast<int_cell &>(t);
    if (ic.value() < 0 || ic.value() > 255) {
	return false;
    }
    mbr.set_depth(checked_cast<size_t>(ic.value()));

    return true;
}

bool me_builtins::check_position(interpreter_base &interp0, term t) {
    if (t.tag() != tag_t::INT) {
	return false;
    }
    int_cell &ic = reinterpret_cast<int_cell &>(t);
    if (ic.value() < 0 || ic.value() > 255) {
	return false;
    }

    return true;
}

bool me_builtins::build_merkle_tree(interpreter_base &interp0, term t, merkle_branch &br, size_t &pos)
{
    if (t.tag() != tag_t::STR) {
	return false;
    }
    auto f = interp0.functor(t);
    if (f != con_cell("branch", 4)) {
	return false;
    }
    auto hash_term = interp0.arg(t, 0);
    if (!set_hash(interp0, hash_term, br)) {
	return false;
    }
    auto depth_term = interp0.arg(t, 1);
    if (!set_depth(interp0, depth_term, br)) {
	return false;
    }
    auto pos_term = interp0.arg(t, 2);
    if (!check_position(interp0, pos_term)) {
	return false;
    }
    pos = checked_cast<size_t>(reinterpret_cast<int_cell &>(pos_term).value());

    auto children = interp0.arg(t, 3);
    if (!interp0.is_list(children)) {
	return false;
    }

    uint64_t num = 0;
    while (interp0.is_dotted_pair(children)) {
	term child = interp0.arg(children, 0);
	if (child.tag() != tag_t::STR) {
	    return false;
	}
	f = interp0.functor(child);
	merkle_node *child_node = nullptr;
	size_t child_pos = 0;
	if (f == con_cell("branch", 4)) {
	    auto *branch_child = new merkle_branch();
	    child_node = branch_child;
	    if (!build_merkle_tree(interp0, child, *branch_child, child_pos)) {
		delete branch_child;
		return false;
	    }
	    num += branch_child->num_entries();
	} else if (f == con_cell("leaf", 4)) {
	    auto *leaf_child = new merkle_leaf();
	    child_node = leaf_child;
	    if (!build_merkle_tree(interp0, child, *leaf_child, child_pos)) {
		delete leaf_child;
		return false;
	    }
	    num++;
	} else {
	    return false;
	}
	std::unique_ptr<merkle_node> child_ptr(child_node);
	br.set_child(child_pos, child_ptr);
	br.set_num_entries(num);
	children = interp0.arg(children, 1);
    }

    return true;
}

bool me_builtins::build_merkle_tree(interpreter_base &interp0, term t, merkle_leaf &lf, size_t &pos)
{
    if (t.tag() != tag_t::STR) {
	return false;
    }
    auto f = interp0.functor(t);
    if (f != con_cell("leaf", 4)) {
	return false;
    }
    auto hash_term = interp0.arg(t, 0);
    if (!set_hash(interp0, hash_term, lf)) {
	return false;
    }
    auto pos_term = interp0.arg(t, 1);
    if (!check_position(interp0, pos_term)) {
	return false;
    }
    pos = checked_cast<size_t>(reinterpret_cast<int_cell &>(pos_term).value());
    auto key_term = interp0.arg(t, 2);
    if (key_term.tag() != tag_t::INT) {
	return false;
    }
    auto key = reinterpret_cast<int_cell &>(key_term).value();
    if (key < 0) {
	return false;
    }
    auto key64 = static_cast<uint64_t>(key);
    lf.set_key(key64);
    
    auto data_term = interp0.arg(t, 3);
    if (data_term != interp0.EMPTY_LIST) {
	if (data_term.tag() != tag_t::BIG) {
	    return false;
	}
	auto &bc = reinterpret_cast<big_cell &>(data_term);
	auto num_bits = interp0.num_bits(bc);
	auto num_bytes = (num_bits + 7) / 8;
	std::unique_ptr<custom_data_t> data(new custom_data_t(num_bytes, true));
	interp0.get_big(bc, data.get()->data(), num_bytes);
	lf.set_data(data);
    }
    return true;
}

bool me_builtins::db_put_5(interpreter_base &interp0, size_t arity, term args[]) {
    static const std::string name = "db_put/5";
    
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();
    auto id = get_meta_id(interp, name, args[0]);
    
    if (args[1].tag() != tag_t::CON) {
	throw interpreter_exception_wrong_arg_type(name + ": Second argument must be an atom.");
    }

    triedb *db = nullptr;
    root_id root_id;

    auto db_name = args[1];
    
    std::tie(db, root_id) = get_db_root(interp0, db_name, id);
    if (db == nullptr) {
	throw interpreter_exception_wrong_arg_type(name + ": Second argument must be 'heap', 'closure', 'symbols' or 'program'");
    }

    auto from_key = args[2];
    if (from_key.tag() != tag_t::INT) {
	throw interpreter_exception_wrong_arg_type(name + ": Key must be an integer; was " + interp.to_string(from_key));
    }
    
    auto to_key = args[3];
    if (to_key.tag() != tag_t::INT) {
	throw interpreter_exception_wrong_arg_type(name + ": Key must be an integer; was " + interp.to_string(to_key));
    }

    auto from_key_val = reinterpret_cast<int_cell &>(from_key).value();
    auto to_key_val = reinterpret_cast<int_cell &>(to_key).value();

    term merkle_term = args[4];

    merkle_root mr;
    size_t pos = 0;
    if (!build_merkle_tree(interp0, merkle_term, mr, pos)) {
	return false;
    }

    if (!mr.validate(db, from_key_val, to_key_val)) {
	return false;
    }

    if (root_id.is_zero()) {
	// Create a new root
	root_id = db->new_root();
    }
    
    // At this point we assume it is safe to insert it
    db->update(root_id, mr);

    set_db_root(interp0, db_name, id, root_id);

    return true;
}

/*
 * db_put(Root, DB, Key, Value)
 */
bool me_builtins::db_put_4(interpreter_base &interp0, size_t arity, term args[]) {
    static const std::string name = "db_put/4";
    
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();
    auto id = get_meta_id(interp, name, args[0]);
    
    if (args[1].tag() != tag_t::CON) {
	throw interpreter_exception_wrong_arg_type(name + ": Second argument must be an atom.");
    }

    triedb *db = nullptr;
    root_id root_id;

    auto db_name = args[1];
    
    std::tie(db, root_id) = get_db_root(interp, db_name, id);
    if (db == nullptr) {
	throw interpreter_exception_wrong_arg_type(name + ": Second argument must be 'block', 'heap', 'closure', 'symbols' or 'program'");
    }

    auto key = args[2];
    if (key.tag() != tag_t::INT) {
	throw interpreter_exception_wrong_arg_type(name + ": Key must be an integer; was " + interp.to_string(key));
    }

    auto key_val = reinterpret_cast<int_cell &>(key).value();

    if (root_id.is_zero()) {
	// Create a new root
	root_id = db->new_root();
    }
    
    auto data = args[3];
    if (data.tag() == tag_t::BIG) {
	auto &big = reinterpret_cast<big_cell &>(data);
	size_t n = interp.num_bytes(big);
	std::vector<uint8_t> buf(n);
	interp.get_big(big, &buf[0], n);
	db->update(root_id, key_val, &buf[0], n);
    } else {
	// Serialize data
        term_serializer::buffer_t buf;
	term_serializer ser(interp0);
	ser.write(buf, data);
	db->update(root_id, key_val, &buf[0], buf.size());
    }

    set_db_root(interp0, db_name, id, root_id);
    
    return true;
}

/*
 * db_put(Root, DB, SerializedLeaf)
 */
bool me_builtins::db_put_3(interpreter_base &interp0, size_t arity, term args[]) {
    static const std::string name = "db_put/3";
    
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();
    auto id = get_meta_id(interp, name, args[0]);
    
    if (args[1].tag() != tag_t::CON) {
	throw interpreter_exception_wrong_arg_type(name + ": Second argument must be an atom.");
    }

    triedb *db = nullptr;
    root_id root_id;

    auto db_name = args[1];
    
    std::tie(db, root_id) = get_db_root(interp, db_name, id);
    if (db == nullptr) {
	throw interpreter_exception_wrong_arg_type(name + ": Second argument must be 'block', 'heap', 'closure', 'symbols' or 'program'");
    }

    auto leaf_data = args[2];
    if (leaf_data.tag() != tag_t::BIG) {
	throw interpreter_exception_wrong_arg_type(name + ": Serialized leaf data must be a bignum; " + interp.to_string(leaf_data));
    }
    auto &big = reinterpret_cast<big_cell &>(leaf_data);
    size_t n = interp.num_bytes(big);
    std::vector<uint8_t> buf(n);
    interp.get_big(big, &buf[0], n);
    
    triedb_leaf leaf;
    leaf.read(&buf[0]);

    if (root_id.is_zero()) {
	// Create a new root
	root_id = db->new_root();
    }
    
    db->update(root_id, leaf.key(), leaf.custom_data(), leaf.custom_data_size());

    set_db_root(interp0, db_name, id, root_id);

    return true;
}

bool me_builtins::ptask_0(interpreter_base &interp0, size_t arity, term args[]) {
    static const std::string name = "ptask/0";
    auto &interp = to_local(interp0);
    auto locked = interp.lock_node();
   
    interp.self().for_each_out_connection(
	[](out_connection *conn) {
	   std::cout << "------------ " << conn->name() << std::endl;
	   conn->print_task_queue();
	});
    return true;
}


local_interpreter::local_interpreter(in_session_state &session)
    : interp::interpreter("node"), session_(session), initialized_(false), ignore_text_(false)
{
    // Redirect standard output (standard std::cout) to an internal
    // stringstream.
    auto fs = new file_stream(*this, 0, "<stdout>");
    fs->open(standard_output_);
    tell_standard_output(*fs);
}

node_locker local_interpreter::lock_node()
{
    return self().lock_node();
}

self_node & local_interpreter::self()
{
    return session_.self();
}

bool local_interpreter::is_root() 
{
    return session_.is_root();
}

void local_interpreter::root_check(const std::string &name, size_t arity)
{
    if (!is_root()) {
        std::stringstream ss;
        ss << name << "/" << arity << ": Non-root is not allowed to use this predicate in this context.";
	abort(interpreter_exception_security(ss.str()));
    }
}

void local_interpreter::ensure_initialized()
{
    if (!initialized_) {
	initialized_ = true;
	// TODO: Only do this for authorized clients.
	enable_file_io();

	setup_standard_lib();

	ec::builtins::load(*this);
        coin::builtins::load(*this);

	setup_local_builtins();

	// Make it easier by importing multiple modules
	use_module(ME);
	use_module(con_cell("ec",0));
	use_module(con_cell("coin",0));

	// Retain state between queries (enables frozen closures in
	// "background" processes between queries)
	set_retain_state_between_queries(true);

	// Load startup file
	if (load_startup_file_) startup_file();
    }
}

void local_interpreter::setup_local_builtins()
{
    auto old_mod = current_module();

    remove_builtin(con_cell("@",2));
    remove_builtin(con_cell("@-",2));
    remove_builtin(con_cell("@=",2));
    remove_builtin(con_cell(".",2));
    
    set_current_module(con_cell("system",0));

    load_builtin(con_cell("@",2), &me_builtins::operator_at_2);
    load_builtin(con_cell("@-",2), &me_builtins::operator_at_silent_2);
    load_builtin(con_cell("@=",2), &me_builtins::operator_at_parallel_2);

    // [...] syntax to load programs.
    load_builtin(con_cell(".", 2), &me_builtins::list_load_2);

    // Basic
    load_builtin(ME, con_cell("id", 1), &me_builtins::id_1);
    load_builtin(ME, con_cell("name", 1), &me_builtins::name_1);
    load_builtin(ME, functor("heartbeat", 0), &me_builtins::heartbeat_0);
    load_builtin(ME, con_cell("version",1), &me_builtins::version_1);
    load_builtin(ME, con_cell("comment",1), &me_builtins::comment_1);
    load_builtin(ME, con_cell("datadir", 1), &me_builtins::datadir_1);
    
    // Address book & connections
    load_builtin(ME, con_cell("peers", 2), &me_builtins::peers_2);
    load_builtin(ME, functor("connections",0), &me_builtins::connections_0);
    load_builtin(ME, functor("connections",1), &me_builtins::connections_1);
    load_builtin(ME, functor("add_address",2), &me_builtins::add_address_2);
    load_builtin(ME, con_cell("ready", 1), &me_builtins::ready_2);
    load_builtin(ME, con_cell("ready", 2), &me_builtins::ready_2);

    // Mailbox
    load_builtin(ME, con_cell("mailbox",1), &me_builtins::mailbox_1);
    load_builtin(ME, functor("check_mail",0), &me_builtins::check_mail_0);
    load_builtin(ME, con_cell("send",2), &me_builtins::send_2);

    // Funding
    load_builtin(ME, functor("initial_funds",1), &me_builtins::initial_funds_1);
    load_builtin(ME, functor("new_funds_per_second",1), &me_builtins::new_funds_per_second_1);
    load_builtin(ME, con_cell("funds",1), &me_builtins::funds_1);
    load_builtin(ME, con_cell("nolimit",0), &me_builtins::nolimit_0);

    // Control global interpreter
    load_builtin(ME, functor("drop_global", 0), &me_builtins::drop_global_0);
    // gstat/0/1: Status of global interpreter
    load_builtin(ME, con_cell("gstat", 0), &me_builtins::gstat_1);
    load_builtin(ME, con_cell("gstat", 1), &me_builtins::gstat_1);
    // chain/0: View chain
    load_builtin(ME, con_cell("chain", 0), &me_builtins::chain_0);
    load_builtin(ME, con_cell("chain", 2), &me_builtins::chain_3);
    load_builtin(ME, con_cell("chain", 3), &me_builtins::chain_3);
    // advance/0: Go to next state
    load_builtin(ME, con_cell("advance", 0), &me_builtins::advance_0);
    // discard/0: Discard recent changes (since last advance/0)
    load_builtin(ME, con_cell("discard", 0), &me_builtins::discard_0);
    // switch/1: Go to a different state
    load_builtin(ME, con_cell("switch", 1), &me_builtins::switch_1);
    // tip/1: Get current tip id
    load_builtin(ME, con_cell("tip", 1), &me_builtins::tip_1);
    // height/1: Current height
    load_builtin(ME, con_cell("height", 1), &me_builtins::height_1);
    load_builtin(ME, functor("max_height", 1), &me_builtins::max_height_1);
    // current/1: Current id
    load_builtin(ME, con_cell("current",1), &me_builtins::current_1);
    // goals/2: Retrieve a local goals block
    load_builtin(ME, con_cell("goals", 2), &me_builtins::goals_2);
    // block/2: Retrieve the raw data of the goals block
    load_builtin(ME, con_cell("block", 2), &me_builtins::block_3);
    load_builtin(ME, con_cell("block", 3), &me_builtins::block_3);    
    // block_hash/2: Retrieve the hash from the raw data of the goals block
    load_builtin(ME, functor("block_hash", 2), &me_builtins::block_hash_2);
    // meta/2: Retrive meta information about a block
    load_builtin(ME, con_cell("meta", 2), &me_builtins::meta_2);
    // meta_more/2: Retrive meta information and more to enable
    //              DB synchronization and validation
    load_builtin(ME, functor("meta_more", 2), &me_builtins::meta_more_2);
    // validate_meta/1: Check that meta record matches the db roots
    load_builtin(ME, functor("validate_meta", 1), &me_builtins::validate_meta_1);
    // metas/3: Retrive meta information for several blocks
    load_builtin(ME, con_cell("metas", 3), &me_builtins::metas_3);
    // meta_roots/4: Retrive meta id root information with spacing
    load_builtin(ME, functor("meta_roots", 4), &me_builtins::meta_roots_4);
    // put_meta/1: Put meta information into meta chain
    load_builtin(ME, functor("put_meta", 1), &me_builtins::put_meta_1);
    // put_metas/1: Put list of meta information into meta chain
    load_builtin(ME, functor("put_metas", 1), &me_builtins::put_metas_1);
    load_builtin(ME, functor("delay_put_metas", 1), &me_builtins::delay_put_metas_1);
    // branches/2: Given a list of meta information, find all its branches
    load_builtin(ME, functor("branches", 2), &me_builtins::branches_2);
    load_builtin(ME, con_cell("follow", 3), &me_builtins::follow_3);
    
    // Commit to the global interpreter
    load_builtin(ME, functor("setup_commit", 1), &me_builtins::setup_commit_1);
    load_builtin(ME, con_cell("commit", 1), &me_builtins::commit_2);
    load_builtin(ME, con_cell("commit", 2), &me_builtins::commit_2);

    // Execute on global interpreter
    load_builtin(ME, con_cell("global", 1), &me_builtins::global_1);

    // Fast sync primitives
    load_builtin(ME, functor("sync_init", 1), &me_builtins::sync_init_1);
    load_builtin(ME, con_cell("sync", 0), &me_builtins::sync_0);
    load_builtin(ME, con_cell("sync", 1), &me_builtins::sync_1);
    load_builtin(ME, functor("sync_complete", 1), &me_builtins::sync_complete_1);
    load_builtin(ME, functor("sync_mode", 1), &me_builtins::sync_mode_1);
    load_builtin(ME, functor("syncing_meta", 1), &me_builtins::syncing_meta_1);
    load_builtin(ME, functor("sync_point", 1), &me_builtins::sync_point_2);
    load_builtin(ME, functor("sync_point", 2), &me_builtins::sync_point_2);
    load_builtin(ME, functor("sync_progress", 0), &me_builtins::sync_progress_1);    
    load_builtin(ME, functor("sync_progress", 1), &me_builtins::sync_progress_1);
    load_builtin(ME, functor("ignore_pow", 1), &me_builtins::ignore_pow_1);
    load_builtin(ME, con_cell("db_get", 5), &me_builtins::db_get_5);
    load_builtin(ME, con_cell("db_num", 3), &me_builtins::db_num_3);    
    load_builtin(ME, con_cell("db_num", 4), &me_builtins::db_num_4);
    load_builtin(ME, con_cell("db_size", 5), &me_builtins::db_size_5);
    load_builtin(ME, con_cell("db_keys", 5), &me_builtins::db_keys_5);
    load_builtin(ME, con_cell("db_keys", 4), &me_builtins::db_keys_4);
    load_builtin(ME, con_cell("db_end", 2), &me_builtins::db_end_2);
    load_builtin(ME, con_cell("db_put", 5), &me_builtins::db_put_5);
    load_builtin(ME, con_cell("db_put", 4), &me_builtins::db_put_4);
    load_builtin(ME, con_cell("db_put", 3), &me_builtins::db_put_3);
    load_builtin(ME, con_cell("ptask", 0), &me_builtins::ptask_0);

    set_current_module(old_mod);

    use_module(con_cell("system",0));
}

void local_interpreter::startup_file()
{
    if (!is_root()) {
	return;
    }

    boost::filesystem::path file(self().data_directory());
    file /= "startup.pl";

    std::string file_path = file.string();

    if (boost::filesystem::exists(file_path)) {
	try {
	    out() << "Loading " << file_path << std::endl;
	    load_file(file_path);
	} catch (const interpreter_exception &ex) {
	    out() << ex.what() << std::endl;
	} catch (const token_exception &ex) {
	    print_error_messages(out(), ex);
	} catch (const term_parse_exception &ex) {
	    print_error_messages(out(), ex);
	}
    }
}

void local_interpreter::local_reset()
{
    interpreter::reset();
}

bool local_interpreter::reset()
{
    interpreter::reset();
    std::unordered_set<task_reset *> resets;
    self().for_each_standard_out_connection( [&resets](out_connection *out)
		  {
		    auto *reset = out->create_reset_task();
	            resets.insert(reset);
	            out->schedule(reset); } );
    bool failed = false;
    while (!resets.empty()) {
	std::vector<task_reset *> to_remove;
	for (auto *task : resets) {
	    if (task->is_reset() || task->failed()) {
		if (task->failed()) failed = true;
		task->consume();
		to_remove.push_back(task);
	    }
	}
	for (auto *task : to_remove) {
	    resets.erase(task);
	}
	utime::sleep(utime::us(self().get_fast_timer_interval_microseconds()));
    }
    return !failed;
}


}}

