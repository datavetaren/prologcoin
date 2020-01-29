#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "wallet.hpp"
#include "../common/utime.hpp"
#include "../ec/keys.hpp"
#include "../ec/mnemonic.hpp"
#include "../ec/builtins.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

namespace prologcoin { namespace wallet {

wallet::wallet(const std::string &wallet_file) : wallet_file_(wallet_file), interp_(*this, wallet_file), killed_(false), terminal_(nullptr)
{
}

wallet::~wallet()
{
    killed_ = true;
}

void wallet::load()
{
    if (boost::filesystem::exists(wallet_file_)) {
        std::ifstream ifs(wallet_file_);
	auto old_module = interp_.current_module();
	interp_.set_current_module(con_cell("wallet",0));
	interp_.load_program(ifs);
	interp_.set_current_module(old_module);
    }
}

void wallet::save()
{
    std::ofstream ofs(wallet_file_);
    interp_.save_program(con_cell("wallet",0),ofs);
}

void wallet::connect_node(terminal *node_term)
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

term wallet::parse(const std::string &cmd)
{
    return interp_.parse(cmd);
}

std::string wallet::to_string(term t)
{
    return interp_.to_string(t);
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

term wallet::get_result_term()
{
    return interp_.get_result_term();
}

term wallet::get_result_term(const std::string &varname)
{
    return interp_.get_result_term(varname);
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

// Generate a new wallet file with some initial source code.
// In particular create a new seed for keys. Return the sentence.
void wallet::create(const std::string &passwd, term sentence)
{
    std::string template_source = R"PROG(

pubkey(Count, PubKey) :- master_pubkey(Master), ec:child_pubkey(Master, Count, PubKey).
privkey(Count, PrivKey) :- master_privkey(Master), ec:child_privkey(Master, Count, PrivKey).

)PROG";

    ec::hd_keys hd(ec::builtins::get_secp256k1_ctx(interp_));
    term encrypted = ec::builtins::encrypt(interp_, sentence, passwd, 2048);
    std::string encrypted_str = interp_.to_string(encrypted);

    ec::mnemonic mn(interp_);
    mn.from_sentence(sentence); // Already checked before this call
    mn.compute_key(hd, "TREZOR"); // Be compatible with TREZOR

    std::string master_pubkey_source = "master_pubkey(58'" + hd.master_public().to_string() + ").";
    interp_.load_program(master_pubkey_source);

    std::string master_privkey_source = "master_privkey(Master) :- wallet_memory:password(Password), ec:encrypt(WordList, Password, 1024, encrypted_str), ec:master_key(WordList, Master, _).";
    interp_.load_program(master_privkey_source);
    interp_.load_program(template_source);

    save();
}
    

    
    
}}
