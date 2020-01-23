#include <iostream>
#include <fstream>
#include "wallet.hpp"
#include "../common/utime.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

namespace prologcoin { namespace wallet {

wallet::wallet(const std::string &wallet_file) : wallet_file_(wallet_file), interp_(wallet_file), killed_(false)
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

void wallet::start(unsigned short port, const std::string &address)
{
    thread_ = boost::thread([&](){ run(); });
}
    
void wallet::stop() {
    killed_ = true;
}

void wallet::join() {
    thread_.join();
}

void wallet::run()
{
    while (!killed_) {
        std::cout << "Tic" << std::endl;
	std::cout.flush();
        utime::sleep(utime::ss(1));
    }
}

void wallet::print()
{
    interp_.print_db();
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
    
}}
