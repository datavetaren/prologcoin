#include "../common/checked_cast.hpp"
#include "self_node.hpp"
#include "local_interpreter.hpp"
#include "session.hpp"
#include "task_reset.hpp"
#include "../ec/builtins.hpp"
#include "../coin/builtins.hpp"
#include "../global/global_interpreter.hpp"

namespace prologcoin { namespace node {

using namespace prologcoin::common;
using namespace prologcoin::interp;

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

    
bool me_builtins::operator_at_2(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);

    interp.root_check("@", 2);
    
    auto query = args[0];
    auto where_term = args[1];

    // std::cout << "operator_at_2: query=" << interp.to_string(query) << " where=" << interp.to_string(where_term) << std::endl;

    if (!interp.is_atom(where_term)) {
	interp.abort(interpreter_exception_wrong_arg_type("@/2: Second argument must be an atom to represent a connection name; was " + interp.to_string(where_term)));
    }

    std::string where = interp.atom_name(where_term);

    if (where == "global") {
        // Then this is same as query(...)
        term local_args[1] = { query };
        return query_1(interp, 1, local_args);
    }

#define LL(interp) reinterpret_cast<local_interpreter &>(interp)
    
    remote_execution_proxy proxy(interp,
        [](interpreter_base &interp, term query, const std::string &where)
	   {return LL(interp).self().execute_at(query, interp, where);},
	[](interpreter_base &interp, const std::string &where)
	   {return LL(interp).self().continue_at(interp, where);},
	[](interpreter_base &interp, const std::string &where)
	   {return LL(interp).self().delete_instance_at(interp, where);});

    return proxy.start(query, where);
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

void me_builtins::preprocess_hashes(local_interpreter &interp, term t) {
    static const con_cell P("p", 1);

    static const common::con_cell op_comma(",", 2);
    static const common::con_cell op_semi(";", 2);
    static const common::con_cell op_imply("->", 2);
    static const common::con_cell op_clause(":-", 2);    

    if (t.tag() != common::tag_t::STR) {
        return;
    }
    
    auto f = interp.functor(t);

    // Scan for :- and subsitute the hash for body
    if (f == op_clause) {
        auto head = interp.arg(t, 0);
	if (head.tag() == tag_t::STR && interp.functor(head) == P) {
	    auto hash_var = interp.arg(head, 0);
	    if (hash_var.tag().is_ref()) {
	        uint8_t hash[ec::builtins::RAW_HASH_SIZE];
	        if (!ec::builtins::get_hashed_2_term(interp, t, hash)) {
		    return;
	        }
		term hash_term = interp.new_big(ec::builtins::RAW_HASH_SIZE*8);
		interp.set_big(hash_term, hash, ec::builtins::RAW_HASH_SIZE);
		if (!interp.unify(hash_var, hash_term)) {
		    return;
		}
	    }
	}
    }

    if (f == op_comma || f == op_semi || f == op_imply || f == op_clause) {
        preprocess_hashes(interp, interp.arg(t, 0));
	preprocess_hashes(interp, interp.arg(t, 1));
    }
}

bool me_builtins::commit(local_interpreter &interp, term_serializer::buffer_t &buf, term t, bool naming)
{
    global::global &g = interp.self().global();

    g.set_naming(naming);

    // Check if term is a clause:
    // p(X) :- Body
    // Then we compute the hash of Body (with X unbound) and bind X to the
    // hashed value. Then we apply commit on Body.
    //
    preprocess_hashes(interp, t);

    // First serialize
    term_serializer ser(interp);
    buf.clear();
    ser.write(buf, t);

    try {
        if (!g.execute_goal(buf, false)) {
	   g.reset();
           return false;
	}
    } catch (interpreter_exception &ex) {
        g.reset();
        throw ex;
    } catch (serializer_exception &ex) {
        g.reset();
        throw ex;
    }
	
    g.execute_cut();

    assert(g.is_clean());

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

bool me_builtins::query_1(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_local(interp0);

    interp.root_check("query", arity);
  
    global::global &g = interp.self().global();

    term t = args[0];
    
    // First serialize
    term_serializer ser(interp);
    buffer_t buf;
    buf.clear();
    ser.write(buf, t);

    try {
        if (!g.execute_goal(buf, true)) {
           return false;
	}
    } catch (interpreter_exception &ex) {
        g.reset();
        throw ex;
    } catch (serializer_exception &ex) {
        g.reset();
        throw ex;
    }

    assert(g.is_clean());

    term t1 = ser.read(buf);
    g.reset();
    return interp.unify(t, t1);
}

local_interpreter::local_interpreter(in_session_state &session)
    :session_(session), initialized_(false), ignore_text_(false)
{
    // Redirect standard output (standard std::cout) to an internal
    // stringstream.
    auto &fs = new_file_stream("<stdout>");
    fs.open(standard_output_);
    tell_standard_output(fs);
}

self_node & local_interpreter::self()
{
    return session_.self();
}

bool local_interpreter::is_root() 
{
    return session_.is_root();
}

void local_interpreter::root_check(const char *name, size_t arity)
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
	setup_standard_lib();

	// TODO: Only do this for authorized clients.
	load_builtins_file_io();

	ec::builtins::load(*this);
        coin::builtins::load(*this);

	setup_local_builtins();

	// Make it easier by importing multiple modules
	use_module(ME);
	use_module(con_cell("ec",0));
	use_module(con_cell("coin",0));

	// Load startup file
	startup_file();
    }
}

void local_interpreter::setup_local_builtins()
{
    load_builtin(con_cell("@",2), &me_builtins::operator_at_2);

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
    load_builtin(ME, functor("add_address",2), &me_builtins::add_address_2);

    // Mailbox
    load_builtin(ME, con_cell("mailbox",1), &me_builtins::mailbox_1);
    load_builtin(ME, functor("check_mail",0), &me_builtins::check_mail_0);
    load_builtin(ME, con_cell("send",2), &me_builtins::send_2);

    // Funding
    load_builtin(ME, functor("initial_funds",1), &me_builtins::initial_funds_1);
    load_builtin(ME, functor("new_funds_per_second",1), &me_builtins::new_funds_per_second_1);
    load_builtin(ME, con_cell("funds",1), &me_builtins::funds_1);

    // Commit to the global interpreter
    load_builtin(ME, con_cell("commit", 1), &me_builtins::commit_2);
    load_builtin(ME, con_cell("commit", 2), &me_builtins::commit_2);

    // Query the global interpreter (do not modify its state)
    load_builtin(ME, con_cell("query", 1), &me_builtins::query_1);
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
	utime::sleep(utime::us(self_node().get_fast_timer_interval_microseconds()));
    }
    return !failed;
}

void local_interpreter::load_file(const std::string &filename)
{
    using namespace prologcoin::common;

    std::ifstream infile(filename);
    if (!infile.good()) {
	throw interpreter_exception_file_not_found("Couldn't open file '" + filename + "'");
    }

    try {
        struct pre_action {
	    pre_action(interpreter &interp) : interp_(interp) { }

	    void operator () (term clause) {
		auto head = interp_.clause_head(clause);
		auto head_f = interp_.functor(head);

		if (head_f == interpreter_base::ACTION_BY) {
		    interp_.compile();
		}
	    }
	    interpreter &interp_;
	};
	load_program<pre_action>(infile);
	compile();
	infile.close();
    } catch (const syntax_exception &ex) {
	abort(ex);
    } catch (const interpreter_exception &ex) {
	abort(ex);
    } catch (const token_exception &ex) {
        throw ex;
    } catch (const term_parse_exception &ex) {
        throw ex;
    } catch (std::runtime_error &ex) {
	std::string msg("Unknown error: ");
	msg += ex.what();
	abort(interpreter_exception_unknown(msg));
    } catch (...) {
	abort(interpreter_exception_unknown("Unknown error"));
    }
}

}}

