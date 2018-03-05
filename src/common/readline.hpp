#pragma once

#ifndef _common_readline_hpp
#define _common_readline_hpp

#include <functional>
#include <vector>

namespace prologcoin { namespace common {

// Lightweight readline (that can handle keypresses as a
// base for a command line interface.)
class readline {
public:
    typedef std::function<bool(readline &, int)> callback_fn;

    static const int TIMEOUT = 10000;
    static const int KEY_UP = 1000;
    static const int KEY_DOWN = 1001;
    static const int KEY_LEFT = 1002;
    static const int KEY_RIGHT = 1003;

    readline();

    inline void set_callback( callback_fn callback )
      { callback_ = callback; }
    inline void set_accept_ctrl_c(bool b)
      { accept_ctrl_c_ = b; }

    std::string read();

    inline bool has_tick() const
      { return tick_; }
    inline void set_tick(bool t)
      { tick_ = t; }

    void add_char(char ch);
    void del_char();
    void go_back();
    void go_forward();

    void add_history(const std::string &str);
    void reset_history_search();
    void search_history_back();
    void search_history_forward();

    void clear_render();
    void render();

    bool has_standard_handling(int ch) {
	return (ch >= ' ' && ch <= 255) ||
	       (ch == 10 || ch == 127 ||
		ch == KEY_LEFT || ch == KEY_RIGHT ||
		ch == KEY_UP || ch == KEY_DOWN ||
		ch == 3);
    }

    static int getch(bool with_timeout);

private:
    void search_history(bool back);

    enum render_t {
	NOTHING,
	SIMPLE_ADD,
	SIMPLE_DEL,
	ALL
    };

    std::string buffer_;
    size_t position_;
    size_t old_position_;
    size_t old_size_;
    bool echo_;
    render_t render_;
    bool accept_ctrl_c_;
    callback_fn callback_;
    bool tick_;

    std::vector<std::string> history_;
    std::string search_;
    bool search_active_;
    int history_search_index_;
};

}};

#endif
