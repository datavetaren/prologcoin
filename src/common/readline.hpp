#pragma once

#ifndef _common_readline_hpp
#define _common_readline_hpp

#include <functional>
#include <vector>
#include <queue>

namespace prologcoin { namespace common {

// Lightweight readline (that can handle keypresses as a
// base for a command line interface.)
class readline {
public:
    typedef std::function<bool(readline &, int)> callback_fn;

    static const int TIMEOUT = 10000;
    static const int KEY_SPECIAL_FIRST = 1000;
    static const int KEY_UP = 1000;
    static const int KEY_DOWN = 1001;
    static const int KEY_LEFT = 1002;
    static const int KEY_RIGHT = 1003;
    static const int KEY_HOME = 1004;
    static const int KEY_END = 1005;
    static const int KEY_WORD_BACK = 1006;
    static const int KEY_WORD_FORWARD = 1007;
    static const int KEY_DEL = 1008;
    static const int KEY_SPECIAL_LAST = 1100;

    readline();

    inline void set_callback( callback_fn callback )
      { callback_ = callback; }
    inline void ignore_callback( bool b ) 
      { ignore_callback_ = b; }
    inline void set_accept_ctrl_c(bool b)
      { accept_ctrl_c_ = b; }

    std::string read();

    inline bool has_tick() const
      { return tick_; }
    inline void set_tick(bool t)
      { tick_ = t; }

    void end_read();

    void add_char(char ch);
    void del_char();
    void del_backspace();
    void do_paste();
    void go_back();
    void go_beginning();
    void go_forward();
    void go_end();
    void go_word_back();
    void go_word_forward();

    void add_history(const std::string &str);
    void reset_history_search();
    void search_history_back();
    void search_history_forward();

    void clear_render();
    void render_simple_del();
    void render_all();
    void render();
    void clear_line();

    bool has_standard_handling(int ch) {
	return (ch >= ' ' && ch <= 255) ||
	       ch == 10 || ch == 127 || ch == 22 || ch == 3 ||
	      (ch >= KEY_SPECIAL_FIRST && ch <= KEY_SPECIAL_LAST);
    }

    void enter_read();
    void leave_read();
    int getch(bool with_timeout);

    static void check_key();
  
private:
    static const int TIMEOUT_INTERVAL_MILLIS = 100;

    void search_history(bool back);

    enum render_t {
	NOTHING,
	SIMPLE_ADD,
	SIMPLE_DEL,
	ALL
    };

    bool keep_reading_;
    std::string buffer_;
    size_t position_;
    size_t old_position_;
    size_t old_size_;
    size_t start_column_;
    size_t column_width_;
    bool echo_;
    render_t render_;
    bool accept_ctrl_c_;
    bool ignore_callback_;
    callback_fn callback_;
    bool tick_;

    std::queue<char> keybuf_;

    std::vector<std::string> history_;
    std::string search_;
    bool search_active_;
    int history_search_index_;

#if _WIN32
#else
    char term_old_[1024]; // Make it big enough to remember termios
                          // We don't want platform details to leak in
                          // a header file.
#endif
};

}};

#endif
