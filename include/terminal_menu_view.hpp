/**
 * terminal_menu_view.hpp
 * Author: Miguel Mochizuki Silva
 * Description: ASCII start menu drawn in the terminal
 */
#ifndef TERMINAL_MENU_VIEW_HPP
#define TERMINAL_MENU_VIEW_HPP

#include "menu_view.hpp"
#include <iostream>

/**
 * Draws the start menu inside a 22 column frame, in the game's palette
 *
 * Attributes:
 * std::ostream& out_: Stream that receives all output
 * bool color_: Whether to emit ANSI SGR color codes
 */
class TerminalMenuView : public IMenuView {
private:
	std::ostream& out_;
	bool color_;

public:
	/**
	 * Constructor for the TerminalMenuView class
	 *
	 * Parameters:
	 * std::ostream& out: Stream that receives all output (default: std::cout)
	 * bool color: Whether to emit ANSI SGR color codes (default: false)
	 */
	explicit TerminalMenuView(std::ostream& out = std::cout, bool color = false);

	/**
	 * Clears the screen and draws the menu
	 *
	 * Parameters:
	 * const MenuSnapshot& snapshot: What to draw
	 *
	 * Returns void:
	 */
	void show(const MenuSnapshot& snapshot) override;
};

#endif
