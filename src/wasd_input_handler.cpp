/**
 * wasd_input_handler.cpp
 * Author: Miguel Mochizuki Silva
 * Description: WASD input handler implementation
 */
#include "wasd_input_handler.hpp"

#ifdef _WIN32
	#include <conio.h>
#else
	#include <termios.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <sys/select.h>

	static struct termios originalTerm;
	static int originalFlags = -1;
#endif

/**
 * Constructor for the WASDInputHandler class
 */
WASDInputHandler::WASDInputHandler() : isTerminalSetup_(false) {
	setupTerminal();
}

/**
 * Destructor for the WASDInputHandler class
 */
WASDInputHandler::~WASDInputHandler() {
	restoreTerminal();
}

/**
 * Checks if direction changed this frame
 *
 * Returns bool: true if direction was changed
 */
bool WASDInputHandler::directionChanged() const {
	return state_.directionChanged;
}

/**
 * Ensures terminal is in raw mode
 */
void WASDInputHandler::ensureTerminal() {
	if (!isTerminalSetup_) {
		setupTerminal();
	}
}

/**
 * Gets the current direction
 *
 * Returns Direction: The current direction
 */
Direction WASDInputHandler::getDirection() const {
	return state_.direction;
}

/**
 * Processes all pending input and updates state
 * This should be called once per frame
 */
void WASDInputHandler::processInput() {
	ensureTerminal();

	char c;
	bool hasQuit = false;
	char lastValid = 0;

	// Read all available characters
#ifdef _WIN32
	while (_kbhit()) {
		c = _getch();
		if (c == 'q' || c == 'Q') {
			hasQuit = true;
		} else if (c == 'w' || c == 'W' || c == 's' || c == 'S' ||
		           c == 'a' || c == 'A' || c == 'd' || c == 'D') {
			lastValid = c;
		}
	}
#else
	while (read(STDIN_FILENO, &c, 1) > 0) {
		if (c == 'q' || c == 'Q') {
			hasQuit = true;
		} else if (c == 'w' || c == 'W' || c == 's' || c == 'S' ||
		           c == 'a' || c == 'A' || c == 'd' || c == 'D') {
			lastValid = c;
		}
	}
#endif

	// Update state
	if (hasQuit) {
		state_.quit = true;
	} else if (lastValid != 0) {
		state_.directionChanged = true;

		// Convert key to direction
		if (lastValid == 'w' || lastValid == 'W') {
			state_.direction = Direction::UP;
		} else if (lastValid == 's' || lastValid == 'S') {
			state_.direction = Direction::DOWN;
		} else if (lastValid == 'a' || lastValid == 'A') {
			state_.direction = Direction::LEFT;
		} else if (lastValid == 'd' || lastValid == 'D') {
			state_.direction = Direction::RIGHT;
		}
	}
}

/**
 * Reads a character blocking (for game over screen). The tty is in non-blocking raw mode
 * (read() returns 0 or fails with EAGAIN when no key is pending), so on POSIX this waits with
 * select() and retries on EAGAIN/EINTR; a read of 0 right after select() reported readable
 * is a real EOF
 *
 * Returns char: The character read, or 0 on EOF/error (POSIX only)
 */
char WASDInputHandler::readCharBlocking() {
	ensureTerminal();

#ifdef _WIN32
	return _getch();
#else
	char c;
	bool selected = false;
	while (true) {
		ssize_t n = read(STDIN_FILENO, &c, 1);
		if (n > 0) {
			return c;
		}
		if (n == 0 && selected) {
			return 0;
		}
		if (n < 0 && errno == EINTR) {
			continue;
		}
		if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			return 0;
		}
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(STDIN_FILENO, &fds);
		int ready = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, nullptr);
		if (ready < 0 && errno != EINTR) {
			return 0;
		}
		selected = ready > 0;
	}
#endif
}

/**
 * Reads a character without blocking
 *
 * Returns char: The character read, or 0 if none
 */
char WASDInputHandler::readInput() {
#ifdef _WIN32
	if (_kbhit()) {
		return _getch();
	}
	return 0;
#else
	char c;
	if (read(STDIN_FILENO, &c, 1) > 0) {
		return c;
	}
	return 0;
#endif
}

/**
 * Resets the direction changed flag
 */
void WASDInputHandler::resetDirectionChanged() {
	state_.directionChanged = false;
}

/**
 * Restores terminal settings (Unix only)
 */
void WASDInputHandler::restoreTerminal() {
#ifndef _WIN32
	if (!isTerminalSetup_) {
		return;
	}

	if (originalFlags != -1) {
		fcntl(STDIN_FILENO, F_SETFL, originalFlags);
	}
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTerm);

	isTerminalSetup_ = false;
#endif
}

/**
 * Resets quit flag to true when restarting the game
 */
void WASDInputHandler::resetQuit() {
	state_.quit = false;
}

/**
 * Sets up terminal for raw input mode (Unix only)
 */
void WASDInputHandler::setupTerminal() {
#ifndef _WIN32
	if (isTerminalSetup_) {
		return;
	}

	if (tcgetattr(STDIN_FILENO, &originalTerm) == -1) {
		return;
	}

	struct termios raw = originalTerm;
	raw.c_lflag &= ~(ICANON | ECHO);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 0;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
		return;
	}

	originalFlags = fcntl(STDIN_FILENO, F_GETFL, 0);
	if (originalFlags == -1) {
		return;
	}

	if (fcntl(STDIN_FILENO, F_SETFL, originalFlags | O_NONBLOCK) == -1) {
		return;
	}

	isTerminalSetup_ = true;
#endif
}

/**
 * Checks if quit was pressed
 *
 * Returns bool: true if 'q' was pressed
 */
bool WASDInputHandler::shouldQuit() const {
	return state_.quit;
}
