/**
 * terminal_size.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Implementation of the terminal size functions
 */
#include "snake_game/terminal/terminal_size.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

std::optional<TerminalSize> queryTerminalSize() {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO info;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
		return std::nullopt;
	}
	return TerminalSize{info.srWindow.Bottom - info.srWindow.Top + 1, info.srWindow.Right - info.srWindow.Left + 1};
#else
	winsize ws{};
	// A pty nobody sized reports 0x0: that is "unknown", not "too small"
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0 || ws.ws_row == 0 || ws.ws_col == 0) {
		return std::nullopt;
	}
	return TerminalSize{ws.ws_row, ws.ws_col};
#endif
}

bool terminalTooSmall(const TerminalSize& size) {
	return size.rows < MIN_TERMINAL_SIZE.rows || size.cols < MIN_TERMINAL_SIZE.cols;
}

std::string tooSmallNotice(const TerminalSize& size) {
	return "Terminal too small: " + std::to_string(size.cols) + "x" + std::to_string(size.rows) + "\n"
		+ "Needs " + std::to_string(MIN_TERMINAL_SIZE.cols) + " columns by " + std::to_string(MIN_TERMINAL_SIZE.rows) + " lines.\n"
		+ "Enlarge it and press a key,\n"
		+ "or Q to quit.\n";
}
