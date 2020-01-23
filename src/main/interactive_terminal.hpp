#pragma once

#ifndef _main_interactive_terminal_hpp
#define _main_interactive_terminal_hpp

#include "../common/readline.hpp"
#include "../terminal/terminal.hpp"

namespace prologcoin { namespace main {

//
// This is the main terminal window that acts like a Prolog prompt
// and establishes a direct link to the node on the local machine.
//
class interactive_terminal : public terminal::terminal {
public:
    interactive_terminal(unsigned short port);
    ~interactive_terminal();

    void run();

private:
    using term = common::term;
    using readline = common::readline;

    bool key_callback(int ch);

    void halt();

    std::string prompt_;
    bool stopped_;
    readline readline_;
    bool ctrl_c_;
    common::utime last_pulse_;
};

}}

#endif
