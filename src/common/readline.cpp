#include <iostream>
#include "readline.hpp"
#if _WIN32
#include <conio.h>
#include <windows.h>
#else
// For all other operating systems we do termios
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#endif
#include <boost/algorithm/string/predicate.hpp>

namespace prologcoin { namespace common {

static bool global_handle_ctrl_c = false;

readline::readline() : keep_reading_(false), position_(0), old_position_(0), echo_(true), accept_ctrl_c_(false), ignore_callback_(false), callback_(nullptr), tick_(false), search_active_(false), history_search_index_(-1)
{
}

// Platform dependent!
#if _WIN32

static DWORD oldMode;
static bool oldModeSaved = false;

static HANDLE hKeyThread;
static HANDLE hKeyPressed;
static HANDLE hKeyMutex;
static bool keyThreadRun = false;
static bool keyThreadStopped = false;
static const int KEY_BUFFER_SIZE = 16;
static int keyBuffer[KEY_BUFFER_SIZE];
static DWORD keyBufferIndex = 0;
static DWORD keyBufferLength = 0;

static DWORD WINAPI KeyboardThread(LPVOID lpParam)
{
    while (!keyThreadStopped) {
        int ch = _getch();
	int ch2 = 0;
	if (ch == 0xe0 || ch == 0) {
	    ch2 = _getch();
	}
        WaitForSingleObject(hKeyMutex, INFINITE);
	if (keyBufferIndex < KEY_BUFFER_SIZE) {
            keyBuffer[keyBufferIndex++] = ch;
	}
	if (ch2 != 0 && keyBufferIndex < KEY_BUFFER_SIZE) {
  	    keyBuffer[keyBufferIndex++] = ch2;
	}
	SetEvent(hKeyPressed);
	ReleaseMutex(hKeyMutex);
    }
    return 0;
}

static void startKeyThread()
{
    keyThreadStopped = false;
    keyBufferIndex = 0;
    hKeyMutex = CreateMutex(NULL, FALSE, NULL);
    hKeyPressed = CreateEvent(NULL, FALSE, FALSE, NULL);
    hKeyThread = CreateThread(NULL, 0, &KeyboardThread, NULL, 0, NULL);
}

static void stopKeyThread()
{
    // We need to send a dummy key to unblock the getch() operation
    // We send CTRL+A, but perhaps there's another key we could send?
    keyThreadStopped = true;
    INPUT_RECORD ir1, ir2;
    ir1.EventType = KEY_EVENT;
    ir1.Event.KeyEvent.bKeyDown = TRUE;
    ir1.Event.KeyEvent.wRepeatCount = 1;
    ir1.Event.KeyEvent.wVirtualKeyCode = 1;
    ir1.Event.KeyEvent.wVirtualScanCode = 1;
    ir1.Event.KeyEvent.uChar.AsciiChar = 1;
    ir1.Event.KeyEvent.dwControlKeyState = 0;
    ir2 = ir1;
    ir2.Event.KeyEvent.bKeyDown = FALSE;
    INPUT_RECORD irs[2] = {ir1, ir2};
    DWORD dw = 0;
    WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &irs[0], 2, &dw);
    WaitForSingleObject(hKeyThread, INFINITE);
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    CloseHandle(hKeyThread);
    CloseHandle(hKeyMutex);
    CloseHandle(hKeyPressed);
}

static int getkey(int timeout)
{
    int ch = 0;
    while (ch == 0) {
        bool keyPress = false;
	WaitForSingleObject(hKeyMutex, INFINITE);
	if (keyBufferIndex > 0) {
	    keyPress = true;
	    ch = keyBuffer[0];
	    keyBufferIndex--;
	    memmove(&keyBuffer[0], &keyBuffer[1], keyBufferIndex*sizeof(int));
	}
	ReleaseMutex(hKeyMutex);
	if (!keyPress) {
	    if (WaitForSingleObject(hKeyPressed, timeout) == WAIT_TIMEOUT) {
	        return readline::TIMEOUT;
	    }
	}
    }
    return ch;
}

int readline::getch(bool with_timeout)
{
    int ch = with_timeout ? getkey(TIMEOUT_INTERVAL_MILLIS) : _getch();
    if (ch == 0xE0 || ch == 0) {
        ch = with_timeout ? getkey(TIMEOUT_INTERVAL_MILLIS) : _getch();
        switch (ch) {
	case 0x4B: ch = KEY_LEFT; break;
	case 0x4D: ch = KEY_RIGHT; break;
	case 0x48: ch = KEY_UP; break;
	case 0x50: ch = KEY_DOWN; break;
	default: ch = 0; break;
	}
    } else {
        switch (ch) {
	case 0x8: ch = 127; break; // Make Windows consistent with Unix
	case 13: ch = 10; break; // Make Windows consistent with Unix
	default: break;
	}
    }
    return ch;
}

void readline::enter_read()
{
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    if (GetConsoleMode(hin, &oldMode)) {
        SetConsoleMode(hin, oldMode & ~ENABLE_PROCESSED_INPUT);
	oldModeSaved = true;
    }
    startKeyThread();
}

void readline::leave_read()
{
    if (oldModeSaved) {
        HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(hin, oldMode);
	oldModeSaved = false;
    }
    stopKeyThread();
}

#else

static bool stdin_init = false;
static termios stdin_attr_old;
static struct sigaction old_hup;
static struct sigaction old_int;
static struct sigaction old_usr1;
static struct sigaction old_kill;
static bool global_ctrl_c_pressed = false;

static uint64_t current_timestamp() {
    struct timeval te; 
    gettimeofday(&te, NULL);
    uint64_t milliseconds = static_cast<uint64_t>(te.tv_sec)*1000 +
	                            te.tv_usec/1000;
    return milliseconds;
}

static void term_restore(int signo)
{
    static const int STDIN = 0;

    if (signo == SIGINT && global_handle_ctrl_c) {
	global_ctrl_c_pressed = true;
	return;
    }

    if (stdin_init) {
	tcsetattr(STDIN, TCSANOW, &stdin_attr_old);
	stdin_init = false;
    }

    switch (signo) {
    case SIGHUP: sigaction(SIGHUP, &old_hup, nullptr); raise(SIGHUP); break;
    case SIGINT: sigaction(SIGINT, &old_int, nullptr); raise(SIGINT); break;
    case SIGUSR1: sigaction(SIGUSR1, &old_usr1, nullptr); raise(SIGUSR1); break;
    case SIGKILL: sigaction(SIGKILL, &old_kill, nullptr); raise(SIGKILL); break;
    }
}

void readline::enter_read()
{
    static const int STDIN = 0;
    static const int STDOUT = 1;

    // Use termios to turn off line buffering
    if (!stdin_init) {
	tcgetattr(STDIN, &stdin_attr_old);
	stdin_init = true;

	struct sigaction sa;
	sa.sa_handler = &term_restore;
	sigfillset(&sa.sa_mask);

	sigaction(SIGHUP, &sa, &old_hup);
	sigaction(SIGUSR1, &sa, &old_usr1);
	sigaction(SIGINT, &sa, &old_int);
	sigaction(SIGKILL, &sa, &old_kill);
    }

    struct winsize ws;
    ws.ws_col = 80;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    column_width_ = ws.ws_col;

    struct termios term;
    tcgetattr(STDIN, &term);
    assert(sizeof(term_old_) >= sizeof(term));
    memcpy(&term_old_[0], &term, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN, TCSANOW, &term);

    start_column_ = 0;
    ignore_callback(true);
    write(STDOUT, "\033[6n", 4);
    char ch;
    for (size_t i = 0; ::read(STDIN, &ch, 1) == 1 && ch != ';'; i++) { }
    for (size_t i = 0; ::read(STDIN, &ch, 1) == 1 && ch != 'R'; i++) {
	if (ch >= '0' && ch <= '9') {
	    start_column_ *= 10;
	    start_column_ += ch - '0';
	}
    }
    if (start_column_ != 0) start_column_--;
    write(STDOUT, "\033[?7h", 5);

    ignore_callback(false);
}

void readline::leave_read()
{
    static const int STDIN = 0;

    struct termios term;
    memcpy(&term, &term_old_[0], sizeof(term));
    tcsetattr(STDIN, TCSANOW, &term);
}

int readline::getch(bool with_timeout)
{
    static const int STDIN = 0;

    bool keybuf_processing = !keybuf_.empty();

    if (!keybuf_processing) {
	fd_set readfds, writefds, errorfds;
    
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&errorfds);
	FD_SET(STDIN, &readfds);

	static uint64_t elapsed = 0;

	struct timeval tv0;
	int64_t to = TIMEOUT_INTERVAL_MILLIS - elapsed;
	if (to > TIMEOUT_INTERVAL_MILLIS) to = TIMEOUT_INTERVAL_MILLIS;
	if (to < 0) to = 0;
	tv0.tv_sec = to / 1000;
	tv0.tv_usec = (to % 1000) * 1000;

	struct timeval *tv = with_timeout ? &tv0 : nullptr;

	auto before = current_timestamp();
	int r = select(1, &readfds, &writefds, &errorfds, tv);
	auto after = current_timestamp();
	elapsed += (after-before);

	if (r == -1) {
	    if (global_ctrl_c_pressed) {
		global_ctrl_c_pressed = false;
		return 3;
	    }
	    return 0;
	}

	if (r == 0) {
	    elapsed = 0;
	    return TIMEOUT;
	}
    }
    
    int n = 0;
    if (keybuf_processing) {
	n = 1;
    } else {
	ioctl(STDIN, FIONREAD, &n);
    }

    char keybuf[32];
    if (n > static_cast<int>(sizeof(keybuf))) {
	n = sizeof(keybuf);
    }

    int ch = 0;

    if (keybuf_processing) {
	keybuf[0] = keybuf_.front();
	keybuf_.pop();
	n = 1;
    } else { 
	::read(STDIN, keybuf, n);
    }

    if (n == 1) {
	ch = keybuf[0];
	switch (ch) {
	case 1: ch = KEY_HOME; break;
	case 5: ch = KEY_END; break;
	default: break;
	}
    } else if (n == 3) {
	if (keybuf[0] == 27 && keybuf[1] == '[') {
	    switch (keybuf[2]) {
	    case 'A': ch = KEY_UP; break;
	    case 'B': ch = KEY_DOWN; break;
	    case 'C': ch = KEY_RIGHT; break;
	    case 'D': ch = KEY_LEFT; break;
            case 'H': ch = KEY_HOME; break;
            case 'F': ch = KEY_END; break;
	    }
	}
    }

    // Could be a paste command. So we store it in our own buffer.
    if (!keybuf_processing && n > 1 && ch == 0) {
	ch = keybuf[0];
	for (int i = 1; i < n; i++) {
	    keybuf_.push(keybuf[i]);
	}
    }

    return ch;
}
#endif

void readline::add_char(char ch)
{
    reset_history_search();
    if (position_ == buffer_.size()) {
	buffer_ += ch;
	position_ = buffer_.size();
	render_ = SIMPLE_ADD;
    } else {
	buffer_.insert(position_, 1, ch);
	position_++;
	render_ = ALL;
    }
}

void readline::del_char()
{
    if (position_ == 0) {
	render_ = NOTHING;
	return;
    }
    position_--;
    if (position_ == buffer_.size()-1) {
	buffer_.pop_back();
	render_ = SIMPLE_DEL;
    } else {
	buffer_.erase(position_, 1);
	render_ = ALL;
    }
    reset_history_search();
}

void readline::go_back()
{
    if (position_ == 0) {
	return;
    }
    position_--;
    render_ = ALL;
}

void readline::go_beginning()
{
    if (position_ == 0) {
	return;
    }
    position_ = 0;
    render_ = ALL;
}

void readline::go_forward()
{
    if (position_ == buffer_.size()) {
	return;
    }
    position_++;
    render_ = ALL;
}

void readline::go_end() {
    if (position_ == buffer_.size()) {
	return;
    }
    position_ = buffer_.size();
    render_ = ALL;
}

void readline::clear_render()
{
    old_position_ = 0;
    old_size_ = 0;
    render_ = ALL;
}

void readline::render()
{
    switch (render_) {
    case NOTHING: break;
    case SIMPLE_ADD: std::cout << buffer_.back();
	if ((position_ + start_column_) % column_width_ == 0) {
	    std::cout << " \b";
	}
	break;
    case SIMPLE_DEL: std::cout << "\b ";
	if ((position_ + start_column_) % column_width_ != column_width_-1) std::cout << "\b";
	break;
    case ALL: {
	size_t blank_out = (buffer_.size() < old_size_) ?
	    old_size_ - buffer_.size() : 0;
	std::cout << std::string(old_position_, '\b')
		  << buffer_ << std::string(blank_out, ' ')
		  << std::string(buffer_.size()-position_+blank_out, '\b');
	break;
        }
    }
    if (render_ != NOTHING) std::cout.flush();
}

void readline::clear_line()
{
    std::cout << std::string(buffer_.size(), '\b')
	      << std::string(buffer_.size(), ' ')
	      << std::string(buffer_.size(), '\b');
}

void readline::add_history(const std::string &str)
{
    history_.push_back(str);
    reset_history_search();
}

void readline::reset_history_search()
{
    search_.clear();
    history_search_index_ = history_.size();
    search_active_ = false;
}

void readline::search_history(bool back)
{
    if (history_.empty()) {
	return;
    }

    if (!search_active_) {
	search_ = buffer_;
	search_active_ = true;
    }

    if (back) {
	history_search_index_--;
	if (history_search_index_ < 0) {
	    history_search_index_ = history_.size();
	}
    } else {
	history_search_index_++;
	if (history_search_index_ > static_cast<int>(history_.size())) {
	    history_search_index_ = 0;
	}
    }

    for (size_t cnt = 0; cnt < history_.size(); cnt++) {
	auto hist = history_search_index_ < static_cast<int>(history_.size()) ?
	    history_[history_search_index_] : search_;
	if (boost::starts_with(hist, search_)) {
	    buffer_ = hist;
	    position_ = buffer_.size();
	    break;
	}
	history_search_index_ += back ? -1 : 1;
	if (history_search_index_ == -1) {
	    history_search_index_ = history_.size();
	}
    }
    render_ = ALL;
}

void readline::search_history_back()
{
    search_history(true);
}

void readline::search_history_forward()
{
    search_history(false);
}

void readline::end_read()
{
    keep_reading_ = false;
}

std::string readline::read()
{
    enter_read();

    global_handle_ctrl_c = accept_ctrl_c_;
    keep_reading_ = true;

    position_ = 0;
    buffer_.clear();

    while (!std::cin.eof() && keep_reading_) {
	int ch = getch(tick_);
	if (ch != -1) {
	    bool r = (callback_ != nullptr && !ignore_callback_) ? callback_(*this, ch) : true;
	    if (r) {
		render_ = NOTHING;
		old_position_ = position_;
		old_size_ = buffer_.size();
		if ((ch >= ' ' && ch <= 255) && ch != 127) {
		    auto c = (ch <= 255) ? static_cast<char>(ch) : '?';
		    add_char(c);
		} else {
		    switch (ch) {
		    case 127: del_char(); break;
		    case KEY_LEFT: go_back(); break;
		    case KEY_RIGHT: go_forward(); break;
		    case KEY_UP: search_history_back(); break;
		    case KEY_DOWN: search_history_forward(); break;
		    case KEY_HOME: go_beginning(); break;
		    case KEY_END: go_end(); break;
		    case 3: case 10: keep_reading_ = false; break;
		    }
		}
		if (echo_) render();
	    }
	}
    }
    leave_read();
    return std::string(buffer_.data(), buffer_.size());
}

}}

