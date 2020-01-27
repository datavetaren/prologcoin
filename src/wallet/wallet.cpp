#include <iostream>
#include <fstream>
#include "wallet.hpp"
#include "../common/utime.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

namespace prologcoin { namespace wallet {

wallet::wallet(const std::string &wallet_file) : wallet_file_(wallet_file), interp_(*this, wallet_file), killed_(false)
{
}

wallet::~wallet()
{
    killed_ = true;
    thread_.join();
}

void wallet::load()
{
    std::ifstream ifs(wallet_file_);
    interp_.load_program(ifs);
}

void wallet::connect_node(std::shared_ptr<terminal> &node_term)
{
    terminal_ = node_term;
}

void wallet::node_pulse()
{
    if (terminal_ != nullptr) {
        terminal_->node_pulse();
    }
}

void wallet::print()
{
    interp_.print_db();
}

void wallet::reset()
{
    interp_.reset();
}

bool wallet::has_more()
{
    return interp_.has_more();
}

bool wallet::next()
{
    return interp_.next();
}
    
std::string wallet::execute(const std::string &cmd)
{
    // Execute arbitrary Prolog command
    try {
        term t = interp_.parse(cmd);
	if (interp_.execute(t)) {
	    return interp_.get_result();
	} else {
	    return "fail.";
	}
    } catch (term_parse_exception &ex) {
        return std::string("ERROR: ") + ex.what();
    } catch (interpreter_exception &ex) {
        return std::string("ERROR: ") + ex.what();
    } catch (token_exception &ex) {
        return std::string("ERROR: ") + ex.what();      
    } catch (std::runtime_error &ex) {
        return std::string("ERROR: ") + ex.what();
    } catch (...) {
        return "UNKNOWN ERROR";
    }
}

bool wallet::execute(term query)
{
    return interp_.execute(query);
}

std::string wallet::get_result()
{
    return interp_.get_result();
}

remote_return_t wallet::execute_at(term query, term_env &query_src, const std::string &where)
{
    uint64_t cost = 0;
    term query_term = terminal_->env().copy(query, env(), cost);

    bool old = terminal_->is_result_to_text();
    try {
	terminal_->set_result_to_text(false);
	if (!terminal_->execute(query_term)) {
	    terminal_->set_result_to_text(old);
	    return remote_return_t();
	}
    } catch (...) {
        terminal_->set_result_to_text(old);
	throw;
    }
    terminal_->set_result_to_text(old);
    term result_remote = terminal_->get_result();
    bool more_state = terminal_->has_more();
    bool at_end_state = terminal_->at_end();
    term result_term = query_src.copy(result_remote, terminal_->env(), cost);
    return remote_return_t(result_term, more_state, at_end_state, cost);
}

remote_return_t wallet::continue_at(term_env &query_src, const std::string &where)
{
    uint64_t cost = 0;
    bool old = terminal_->is_result_to_text();
    terminal_->set_result_to_text(false);
    try {
        if (!terminal_->next()) {
            return remote_return_t();
	}
    } catch (...) {
        terminal_->set_result_to_text(old);
	throw;
    }
    terminal_->set_result_to_text(old);    
    term result_remote = terminal_->get_result();
    bool more_state = terminal_->has_more();
    bool at_end_state = terminal_->at_end();
    term result_term = query_src.copy(result_remote, terminal_->env(), cost);
    return remote_return_t(result_term, more_state, at_end_state, cost);
}

bool wallet::delete_instance_at(term_env &query_src, const std::string &where)
{
    return terminal_->delete_instance();
}
    
    
}}
