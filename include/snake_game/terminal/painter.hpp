/**
 * painter.hpp
 * Author: Miguel Mochizuki Silva
 * Description: ANSI styles and the styled-text painter shared by the terminal views
 */
#ifndef TERMINAL_PAINTER_HPP
#define TERMINAL_PAINTER_HPP

#include <ostream>
#include <string>

namespace terminal {

/**
 * Colour roles of the terminal views; PLAIN is the terminal's default style
 */
enum Style { PLAIN, WALL, BODY, HEAD, FOOD, LABEL, NUMBER, DIM, RESTART, QUIT, GOLD };

/**
 * Gets the ANSI sequence of a style
 *
 * Parameters:
 * Style style: The style
 *
 * Returns const char*: The SGR sequence, "" for PLAIN
 */
inline constexpr const char* sgr(Style style) {
	switch (style) {
		case WALL: return "\033[36m";
		case BODY: return "\033[32m";
		case HEAD: return "\033[1;92m";
		case FOOD: return "\033[1;91m";
		case LABEL: return "\033[1m";
		case NUMBER: return "\033[1;33m";
		case DIM: return "\033[2m";
		case RESTART: return "\033[92m";
		case QUIT: return "\033[91m";
		case GOLD: return "\033[1;93m";
		case PLAIN: break;
	}
	return "";
}

inline constexpr const char* RESET_SEQUENCE = "\033[0m";

/**
 * Writes text into a stream as runs of same-styled characters
 *
 * Attributes:
 * std::ostream& buf: Destination stream
 * bool color: Whether styles are emitted at all
 * Style cur_: Style currently open (PLAIN when none)
 */
class Painter {
	std::ostream& buf_;
	bool color_;
	Style cur_ = PLAIN;

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
	 * Style style: Style of the text, PLAIN for none
	 * const std::string& text: Text to append
	 *
	 * Returns void:
	 */
	void put(Style style, const std::string& text) {
		if (color_ && style != cur_) {
			if (cur_ != PLAIN) {
				buf_ << RESET_SEQUENCE;
			}
			buf_ << sgr(style);
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
		if (cur_ != PLAIN) {
			buf_ << RESET_SEQUENCE;
			cur_ = PLAIN;
		}
		buf_ << '\n';
	}
};

}  // namespace terminal

#endif
