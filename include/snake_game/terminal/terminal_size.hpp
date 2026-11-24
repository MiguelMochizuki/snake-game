/**
 * terminal_size.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Size of the terminal and the check that the game fits in it
 */
#ifndef TERMINAL_SIZE_HPP
#define TERMINAL_SIZE_HPP

#include <optional>
#include <string>

/**
 * Size of a terminal window
 *
 * Attributes:
 * int rows: Lines
 * int cols: Columns
 */
struct TerminalSize {
	int rows;
	int cols;
};

/**
 * Smallest terminal the game is drawn properly in: the board with its status line is 23 lines
 * (24 leaves one for the shell) and the widest panel is 30 columns
 */
constexpr TerminalSize MIN_TERMINAL_SIZE{24, 30};

/**
 * Asks the system for the size of the terminal on standard output
 *
 * Returns std::optional<TerminalSize>: The size, or nullopt when output is not a terminal or the system does not say
 */
std::optional<TerminalSize> queryTerminalSize();

/**
 * Tells whether the game would not fit
 *
 * Parameters:
 * const TerminalSize& size: The terminal's size
 *
 * Returns bool: true if it has fewer lines or columns than MIN_TERMINAL_SIZE
 */
bool terminalTooSmall(const TerminalSize& size);

/**
 * Builds the text shown when the terminal is too small
 *
 * Parameters:
 * const TerminalSize& size: The terminal's size
 *
 * Returns std::string: Lines ending in newlines, saying the current and the needed size
 */
std::string tooSmallNotice(const TerminalSize& size);

#endif
