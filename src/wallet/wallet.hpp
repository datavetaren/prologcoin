#pragma once

#ifndef _wallet_wallet_hpp
#define _wallet_wallet_hpp

#include "../terminal/terminal.hpp"
#include "wallet_interpreter.hpp"

namespace prologcoin { namespace wallet {

//
// The wallet is a pretty isolated piece of code. It loads in prolog source
// code (the wallet) in its own interpreter and communicate to the node
// via a (non-interactive) terminal. Terminals are socket based. This make
// the load balancing somewhat easier, so that the wallet can download and
// process things and thus will not starve computer threads to death on
// initial synchronizatons/wallet sweeps. It also enables wallets being run
// on a different computer (which could be on a different network.)
//
class wallet {
public:
    using terminal = prologcoin::terminal::terminal;
  
    // Wallet file is some Prolog source code (.pl) representing a wallet.
    wallet(const std::string &wallet_file);
    ~wallet();

    void load();
  
    // Start the thread that will talk to the node.
    void start(unsigned short port = terminal::DEFAULT_PORT, const std::string &host = "127.0.0.1");

    void run();
    void stop();
    void join();
    void print();

    std::string execute(const std::string &cmd);
  
private:
    std::string wallet_file_;
    wallet_interpreter interp_;
    bool killed_;

    // The wallet is run in a specific thread and will open
    // a terminal to the node
    boost::thread thread_;

    // This is the terminal to the node.
    std::unique_ptr<terminal> terminal_;
};
    
}}

#endif
