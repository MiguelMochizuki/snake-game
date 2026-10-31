/**
 * terminal_painter.hpp
 * Author: Miguel Mochizuki Silva
 * Description: ANSI styles and the styled-text painter shared by the terminal views
 */
#ifndef TERMINAL_PAINTER_HPP
#define TERMINAL_PAINTER_HPP

#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>

namespace terminal {

inline constexpr const char* WALL = "\033[36m";
inline constexpr const char* BODY = "\033[32m";
inline constexpr const char* HEAD = "\033[1;92m";
inline constexpr const char* FOOD = "\033[1;91m";
inline constexpr const char* LABEL = "\033[1m";
inline constexpr const char* NUMBER = "\033[1;33m";
inline constexpr const char* DIM = "\033[2m";
inline constexpr const char* RESTART = "\033[92m";
inline constexpr const char* QUIT = "\033[91m";
inline constexpr const char* RESET = "\033[0m";

/**
 * Writes text into a stream as runs of same-styled characters
 *
 * Attributes:
 * std::ostream& buf: Destination stream
 * bool color: Whether styles are emitted at all
 * const char* cur_: Style currently open (nullptr when plain)
 */
class Painter {
	std::ostream& buf_;
	bool color_;
	const char* cur_ = nullptr;

public:
	/**
	 * Parameters:
	 * std::ostream& buf: Destination stream
	 * bool color: Whether styles are emitted at all
	 */
	Painter(std::ostream& buf, bool color) : buf_(buf), color_(color) {
	}

	/**
	 * Appends text in a style, opening or closing SGR runs only when the style changes
	 *
	 * Parameters:
	 * const char* style: SGR sequence, or nullptr for plain text
	 * const std::string& text: Text to append
	 *
	 * Returns void:
	 */
	void put(const char* style, const std::string& text) {
		if (color_ && style != cur_) {
			if (cur_) {
				buf_ << RESET;
			}
			if (style) {
				buf_ << style;
			}
			cur_ = style;
		}
		buf_ << text;
	}

	/**
	 * Ends the line, closing any open style first
	 *
	 * Returns void:
	 */
	void endl() {
		if (cur_) {
			buf_ << RESET;
			cur_ = nullptr;
		}
		buf_ << '\n';
	}
};

/**
 * Formats a number right-aligned in a field
 *
 * Parameters:
 * T value: The number
 * int width: Field width
 * int precision: Digits after the point for floating values (-1 for integers)
 *
 * Returns std::string: The padded text
 */
template <typename T>
std::string padded(T value, int width, int precision = -1) {
	std::ostringstream s;
	s << std::setw(width);
	if (precision >= 0) {
		s << std::fixed << std::setprecision(precision);
	}
	s << value;
	return s.str();
}

}  // namespace terminal

#endif
