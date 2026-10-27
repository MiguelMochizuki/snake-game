/**
 * terminal_key_source.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Key source reading the raw terminal
 */
#include "terminal_key_source.hpp"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <windows.h>
	#include <conio.h>
#else
	#include <errno.h>
	#include <fcntl.h>
	#include <string>
	#include <sys/select.h>
	#include <unistd.h>
#endif

/**
 * Constructor for the TerminalKeySource class. Enters raw mode
 *
 * POSIX: clears ICANON, ECHO, ISIG, IEXTEN and IXON, sets VMIN=VTIME=0 and O_NONBLOCK.
 * If stdin is not a tty only O_NONBLOCK is set. Windows: clears ENABLE_PROCESSED_INPUT
 */
TerminalKeySource::TerminalKeySource() {
#ifdef _WIN32
	HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode = 0;
	if (in != INVALID_HANDLE_VALUE && in != nullptr && GetConsoleMode(in, &mode)) {
		originalMode_ = mode;
		modeSaved_ = SetConsoleMode(in, mode & ~static_cast<DWORD>(ENABLE_PROCESSED_INPUT)) != 0;
	}
#else
	if (tcgetattr(STDIN_FILENO, &originalTerm_) == 0) {
		struct termios raw = originalTerm_;
		raw.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
		raw.c_iflag &= ~IXON;
		raw.c_cc[VMIN] = 0;
		raw.c_cc[VTIME] = 0;
		termiosSaved_ = tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0;
	}
	int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	if (flags != -1 && fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) != -1) {
		originalFlags_ = flags;
	}
#endif
}

/**
 * Destructor for the TerminalKeySource class. Restores the terminal
 */
TerminalKeySource::~TerminalKeySource() {
#ifdef _WIN32
	if (modeSaved_) {
		HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
		SetConsoleMode(in, originalMode_);
	}
#else
	if (originalFlags_ != -1) {
		fcntl(STDIN_FILENO, F_SETFL, originalFlags_);
	}
	if (termiosSaved_) {
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTerm_);
	}
#endif
}

/**
 * Reads every key available now, without blocking
 *
 * Returns std::vector<Key>: The keys, oldest first; empty if none
 */
std::vector<Key> TerminalKeySource::poll() {
	std::vector<Key> keys(queue_.begin(), queue_.end());
	queue_.clear();
#ifdef _WIN32
	while (_kbhit()) {
		int c = _getch();
		if (KeyDecoder::isWindowsPrefix(c)) {
			// A lone 0xE0/0x00 character must not block the tick waiting for a second byte
			int second = _kbhit() ? _getch() : 0;
			keys.push_back(KeyDecoder::decodeWindows(c, second));
		} else {
			keys.push_back(KeyDecoder::decodeWindows(c, 0));
		}
	}
#else
	char buf[256];
	bool any = false;
	ssize_t n;
	while ((n = read(STDIN_FILENO, buf, sizeof buf)) > 0) {
		any = true;
		for (const Key& k : decoder_.feed(std::string(buf, static_cast<size_t>(n)))) {
			keys.push_back(k);
		}
	}
	if (!any) {
		for (const Key& k : decoder_.idle()) {
			keys.push_back(k);
		}
	}
#endif
	return keys;
}

/**
 * Blocks until a key is available. The tty is in non-blocking raw mode (read() returns 0 or
 * fails with EAGAIN when no key is pending), so on POSIX this waits with select() and retries
 * on EAGAIN/EINTR; a read of 0 right after select() reported readable is a real EOF
 *
 * Returns std::optional<Key>: The key, or nullopt on EOF or a hard error
 */
std::optional<Key> TerminalKeySource::waitKey() {
	if (!queue_.empty()) {
		Key k = queue_.front();
		queue_.pop_front();
		return k;
	}
#ifdef _WIN32
	int c = _getch();
	return KeyDecoder::decodeWindows(c, KeyDecoder::isWindowsPrefix(c) ? _getch() : 0);
#else
	char buf[256];
	bool selected = false;
	while (true) {
		ssize_t n = read(STDIN_FILENO, buf, sizeof buf);
		if (n > 0) {
			for (const Key& k : decoder_.feed(std::string(buf, static_cast<size_t>(n)))) {
				queue_.push_back(k);
			}
			if (!queue_.empty()) {
				Key k = queue_.front();
				queue_.pop_front();
				return k;
			}
			selected = false;
			continue;
		}
		if (n == 0 && selected) {
			return std::nullopt;
		}
		if (n < 0 && errno == EINTR) {
			continue;
		}
		if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			return std::nullopt;
		}
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(STDIN_FILENO, &fds);
		int ready = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, nullptr);
		if (ready < 0 && errno != EINTR) {
			return std::nullopt;
		}
		selected = ready > 0;
	}
#endif
}

/**
 * Discards pending input, the key queue and any partial escape sequence
 */
void TerminalKeySource::flush() {
	queue_.clear();
	decoder_.reset();
#ifdef _WIN32
	while (_kbhit()) {
		_getch();
	}
#else
	tcflush(STDIN_FILENO, TCIFLUSH);
	char buf[256];
	while (read(STDIN_FILENO, buf, sizeof buf) > 0) {
	}
#endif
}
