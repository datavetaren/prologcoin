#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "wallet.hpp"
#include "../common/utime.hpp"
#include "../ec/keys.hpp"
#include "../ec/mnemonic.hpp"
#include "../ec/builtins.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

namespace prologcoin { namespace wallet {

void wallet::erase(const std::string &wallet_file) {
    boost::filesystem::remove(wallet_file);
}

void wallet::erase_all_test_wallets(const std::string &dir) {
    boost::filesystem::directory_iterator at_end;
    for (boost::filesystem::directory_iterator it(dir); it != at_end; ++it){
	if (!boost::filesystem::is_regular_file(it->status())) {
	    continue;
	}
	auto filename = it->path().filename().string();
	if (boost::starts_with(filename, "test_wallet") &&
	    boost::ends_with(filename, ".pl")) {
	    auto path = it->path().string();
	    std::cout << "Erasing " << path << std::endl;
	    erase(path);
	}
    }
}

wallet::wallet(const std::string &wallet_file) : wallet_file_(wallet_file), new_file_(true), auto_save_(true), interp_(*this, wallet_file), killed_(false), terminal_(nullptr)
{
}

wallet::~wallet()
{
    killed_ = true;
}

void wallet::set_current_directory(const std::string &dir)
{
    interp_.set_current_directory(dir);
}
	
void wallet::total_reset()
{
    interp_.total_reset();
}
	
const std::string & wallet::get_file() const {
    return wallet_file_;
}

void wallet::set_file(const std::string &file) {
    wallet_file_ = file;
    new_file_ = true;
}

void wallet::load()
{
    auto fullpath = interp().get_full_path(wallet_file_);
    if (!fullpath.empty() && boost::filesystem::exists(fullpath)) {
        std::ifstream ifs(fullpath);
	auto old_module = interp_.current_module();
	interp_.set_current_module(con_cell("wallet",0));
	interp_.load_program(ifs);
	interp_.set_current_module(old_module);
    }
    new_file_ = false;
}

void wallet::check_dirty()
{
    if (!is_auto_save()) {
	return;
    }
    auto &meta = interp_.get_module_meta(con_cell("wallet",0));
    if (meta.has_changed()) {
        save();
    }
}

void wallet::save()
{
    if (wallet_file_.empty()) {
        return;
    }
    auto fullpath = interp().get_full_path(wallet_file_);
    // Check if wallet is non empty first
    auto &mod = interp().get_module(con_cell("wallet",0));
    if (mod.empty()) {
	// No need to save something empty
	return;
    }
    std::ofstream ofs(fullpath);
    interp_.save_program(con_cell("wallet",0),ofs);
}

void wallet::connect_node(terminal *node_term)
{
    terminal_ = node_term;
    terminal_->set_propagate_exceptions(true);
}

void wallet::node_pulse()
{
    if (terminal_) {
        terminal_->node_pulse();
    }
}

void wallet::print()
{
    interp_.print_db();
}

void wallet::reset()
{
    if (terminal_) terminal_->reset();
    interp_.reset();
}

bool wallet::has_more()
{
    return interp_.has_more();
}

bool wallet::next()
{
    try {
        bool r = interp_.next();
	check_dirty();
        return r;
    } catch (std::runtime_error &ex) {
        check_dirty();
	throw ex;
    }
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
	bool ok = false;
	if ((ok = execute(t))) {
	    auto r = interp_.get_result();
	    if (ok && new_file_) {
		interp_.total_reset();
		load();
	    }
	    return r;
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
    try {
        bool r = interp_.execute(query);
	check_dirty();
        return r;
    } catch (std::runtime_error &ex) {
        check_dirty();
	throw ex;
    }
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
    terminal_->env().clear_names();
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
    std::string nl = "\n";
    std::string nl2 = nl + nl;
    std::string template_source = R"PROG(

pubkey(Count, PubKey) :-
    master_pubkey(Master),
    ec:child_pubkey(Master, Count, ExtPubKey), 
    ec:normal_key(ExtPubKey, PubKey).

privkey(Count, PrivKey) :-
    master_privkey(Master),
    ec:child_privkey(Master, Count, ExtPrivKey),
    ec:normal_key(ExtPrivKey, PrivKey).

lastheap(0).
numkeys(0).

)PROG";

    ec::hd_keys hd(ec::builtins::get_secp256k1_ctx(interp_));
    term encrypted = ec::builtins::encrypt(interp_, sentence, passwd, 2048);
    std::string encrypted_str = interp_.to_string(encrypted);

    ec::mnemonic mn(interp_);
    mn.from_sentence(sentence); // Already checked before this call
    mn.compute_key(hd, "TREZOR"); // Be compatible with TREZOR

    std::stringstream ss;
    ss << "master_privkey(Master) :- current_predicate('$secret':master/1), '$secret':master(Master)." << std::endl;
    ss << "master_privkey(Master) :- system:password(Password), ec:encrypt(WordList, Password, 2048, " + encrypted_str +"), ec:master_key(WordList, Master, _), assert('$secret':master(Master))." << std::endl << std::endl;

    ss << "master_pubkey(58'" + hd.master_public().to_string() + ").";
    ss << std::endl << std::endl;

    ss << template_source;

    std::string total_program = ss.str();
    interp_.load_program(total_program);

    save();
}
    

    
    
}}
