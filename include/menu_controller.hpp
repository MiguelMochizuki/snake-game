/**
 * menu_controller.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Runs the start menu loop over abstract keys, handlers and view
 */
#ifndef MENU_CONTROLLER_HPP
#define MENU_CONTROLLER_HPP

#include "input_handler.hpp"
#include "key_source.hpp"
#include "menu_model.hpp"
#include "menu_view.hpp"

/**
 * What the player decided on the menu
 *
 * Attributes:
 * bool start: true to play, false to quit
 * int speedMs: Frame time of the chosen level, meaningful only when start is true
 */
struct MenuResult {
	bool start;
	int speedMs;
};

/**
 * Runs the menu until the player starts, quits or the key source ends
 *
 * Parameters:
 * IKeySource& keys: Source of key presses
 * const IInputHandler& input: Maps keys to actions
 * IMenuView& view: Draws the menu
 * MenuModel& model: Menu state, updated as keys arrive
 * const MenuStats& stats: Score figures to show
 *
 * Returns MenuResult: start true with the chosen speed; false on Quit, Ctrl+C or EOF
 */
MenuResult runMenu(IKeySource& keys, const IInputHandler& input, IMenuView& view,
	MenuModel& model, const MenuStats& stats);

#endif
