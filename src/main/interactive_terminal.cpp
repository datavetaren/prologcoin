#include <algorithm>
#include <ctype.h>
#include <boost/algorithm/string/predicate.hpp>
#include "../common/term_serializer.hpp"
#include "../node/self_node.hpp"
#include "../node/local_interpreter.hpp"
#include "interactive_terminal.hpp"

namespace prologcoin { namespace main {

using namespace prologcoin::common;
using namespace prologcoin::node;

interactive_terminal::interactive_terminal(unsigned short port)
  : terminal(port),
    prompt_("?- "),
    stopped_(false),
    ctrl_c_(false)
{
    readline_.set_tick(true);
    readline_.set_accept_ctrl_c(true);
    readline_.set_callback([this](readline &, int ch){
	    return key_callback(ch);
	});
}

interactive_terminal::~interactive_terminal()
{
}

bool interactive_terminal::key_callback(int ch)
{
    if (ch == 3) {
	ctrl_c_ = true;
	readline_.end_read();
    }

    if (ch == readline::TIMEOUT) {
	// Make a call to check_mail
	utime t0 = utime::now();
	bool do_check_mail = (t0 - last_mail_check_) >= utime::ss(1);

	if (do_check_mail) {
	    last_mail_check_ = t0;
	    auto query_check_mail = env().new_term(local_interpreter::COLON,
						   {local_interpreter::ME,
						    env().functor("check_mail",0)});
	    bool old = is_result_to_text();
	    set_result_to_text(false);
	    execute(query_check_mail);
	    set_result_to_text(old);
	}

	std::string text = flush_text();

	if (!text.empty()) {
	    readline_.clear_line();
	    std::cout << std::string(prompt_.size(), '\b')
		      << std::string(prompt_.size(), ' ')
	   	      << std::string(prompt_.size(), '\b');
	    std::cout << text;
	    std::cout << prompt_;
	    std::cout.flush();
	    readline_.clear_render();
	    readline_.render();
	}
	return false;
    }

    bool r = readline_.has_standard_handling(ch);

    if (r && has_more()) {
	// Single keystrokes if in query mode.
	readline_.end_read();
    }

    return r;
}

void interactive_terminal::halt()
{
    stopped_ = true;
}

void interactive_terminal::run()
{
    while (!stopped_) {
	// Set prompt
	if (has_more()) {
	    prompt_ = "";
	} else {
	    prompt_ = "?- ";
	}

	std::cout << prompt_;
	std::cout.flush();

	ctrl_c_ = false;
	std::string cmd = readline_.read();
	std::cout << "\n";

	if (ctrl_c_) {
	    reset();
	    std::cout << "Enter 'halt.' to exit terminal." << std::endl;
	    continue;
	}

	if (stopped_) {
	    continue;
	}
	try {
	    if (has_more()) {
		if (cmd != ";" && !cmd.empty()) {
		    add_error("Unknown command. Only ';' or ENTER is allowed.");
		    set_has_more();
		    continue;
		}
		if (cmd.empty()) {
		    reset();
		    continue;
		}
		next();
		continue;
	    }

	    if (cmd.empty()) {
		continue;
	    }

	    readline_.add_history(cmd);

	    term t = parse(cmd);
	    if (t == term()) {
		continue;
	    }
	    if (t == con_cell("halt",0)) {
		break;
	    }
	    execute(t);
	} catch (std::runtime_error &ex) {
	    add_error(ex.what());
	} catch (...) {
	    add_error("Unknown error");
 	}
    }
}

}}
