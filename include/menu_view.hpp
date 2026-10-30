/**
 * menu_view.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Presentation interface of the start menu
 */
#ifndef MENU_VIEW_HPP
#define MENU_VIEW_HPP

#include "menu_model.hpp"
#include <cstddef>

/**
 * Score figures shown on the menu
 *
 * Attributes:
 * int best: Best score
 * std::size_t games: Games played
 * double average: Average score over the last games
 */
struct MenuStats {
	int best;
	std::size_t games;
	double average;
};

/**
 * Everything a view needs to draw the menu
 *
 * Attributes:
 * MenuItem selected: Selected row
 * Level level: Chosen level
 * MenuStats stats: Score figures
 */
struct MenuSnapshot {
	MenuItem selected;
	Level level;
	MenuStats stats;
};

/**
 * Draws the start menu; a terminal or a GUI implements it
 */
class IMenuView {
public:
	/**
	 * Destructor for the IMenuView interface
	 */
	virtual ~IMenuView() = default;

	/**
	 * Draws the menu
	 *
	 * Parameters:
	 * const MenuSnapshot& snapshot: What to draw
	 *
	 * Returns void:
	 */
	virtual void show(const MenuSnapshot& snapshot) = 0;
};

#endif
