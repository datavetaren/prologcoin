#include <algorithm>
#include <ctype.h>
#include <boost/algorithm/string/predicate.hpp>
#include "../common/term_serializer.hpp"
#include "../node/self_node.hpp"
#include "interactive_prompt.hpp"

namespace prologcoin { namespace main {

using namespace prologcoin::common;
using namespace prologcoin::node;
using namespace prologcoin::terminal;

interactive_prompt::interactive_prompt()
  : prompt_("?- "),
    stopped_(false),
    ctrl_c_(false),
    in_wallet_(false)
{
    readline_.set_tick(true);
    readline_.set_accept_ctrl_c(true);
    readline_.set_callback([this](readline &, int ch){
	    return key_callback(ch);
	});
}

interactive_prompt::~interactive_prompt()
{
}

bool interactive_prompt::connect_node(unsigned short port)
{
    node_terminal_ = new terminal(port);
    return node_terminal_->connect();
}

void interactive_prompt::connect_wallet(wallet *w)
{
    wallet_ = w;
    in_wallet_ = true;
}

void interactive_prompt::pulse()
{
    if (in_wallet_) {
        wallet_->node_pulse();
    } else {
        node_terminal_->node_pulse();
    }
}

std::string interactive_prompt::flush_text()
{
    std::string t = text_;
    t += node_terminal_->flush_text();
    text_.clear();
    return t;
}

void interactive_prompt::reset()
{
    if (in_wallet_) {
        wallet_->reset();
    } else {
        node_terminal_->reset();
    }
}

bool interactive_prompt::has_more()
{
    if (in_wallet_) {
        return wallet_->has_more();
    } else {
        return node_terminal_->has_more();
    }
}

bool interactive_prompt::next()
{
    bool r = false;
    if (in_wallet_) {
        if ((r = wallet_->next())) {
	    if (has_more()) {
	        add_text_output_no_nl(wallet_->get_result());
		add_text_output_no_nl(" ");		
	    } else {
	        add_text_output(wallet_->get_result());	      
	    }
	} else {
	    add_text_output("false.");
	}
    } else {
        r = node_terminal_->next();
    }
    if (!r) {
        reset();
    }
    return r;
}

void interactive_prompt::add_error(const std::string &msg)
{
    if (in_wallet_) {
        add_text_output("[ERROR]: " + msg);
    } else {
        node_terminal_->add_error(msg);
    }
}

void interactive_prompt::add_text_output(const std::string &str)
{
    if (!boost::ends_with(str, "\n")) {
        text_ += str + "\r\n";
    } else {
        text_ += str;
    }
}

void interactive_prompt::add_text_output_no_nl(const std::string &str)
{
    if (boost::ends_with(str,"\n")) {
        text_ += str.substr(0, str.size()-1);
    } else {
        text_ += str;
    }
}
 
bool interactive_prompt::key_callback(int ch)
{
    if (ch == 3) {
	ctrl_c_ = true;
	readline_.end_read();
    }

    if (ch == readline::TIMEOUT) {
	// Make a call to check_mail
	utime t0 = utime::now();
	bool do_pulse = (t0 - last_pulse_) >= utime::ss(1);
	if (do_pulse) {
	    last_pulse_ = t0;
	    pulse();
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

void interactive_prompt::halt()
{
    stopped_ = true;
}

term interactive_prompt::parse(const std::string &str)
{
    if (in_wallet_) {
        return wallet_->env().parse(str);
    } else {
        return node_terminal_->parse(str);
    }
}

void interactive_prompt::execute(term t)
{
    if (in_wallet_) {
        if (!wallet_->execute(t)) {
	    add_text_output("false.");
        } else {
	    if (has_more()) {
	        add_text_output_no_nl(wallet_->get_result());
		add_text_output_no_nl(" ");
	    } else {
	        add_text_output(wallet_->get_result());	      
	    }
	}
    } else {
        node_terminal_->execute(t);
    }
}

void interactive_prompt::run()
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
		    if (!in_wallet_) node_terminal_->set_has_more();
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
	        reset();
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
	    reset();
	} catch (...) {
	    add_error("Unknown error");
	    reset();
 	}
    }
}

}}
