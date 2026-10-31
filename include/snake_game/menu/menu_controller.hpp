/**
 * menu_controller.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Runs the start menu loop over abstract keys, handlers and view
 */
#ifndef MENU_CONTROLLER_HPP
#define MENU_CONTROLLER_HPP

#include "snake_game/input/input_handler.hpp"
#include "snake_game/input/key_source.hpp"
#include "snake_game/menu/menu_model.hpp"
#include "snake_game/menu/menu_view.hpp"
#include "snake_game/score/score_source.hpp"

/**
 * What the player decided on the menu
 *
 * Attributes:
 * bool start: true to play, false to quit
 * int speedMs: Frame time of the chosen level, meaningful only when start is true
 * Level level: Level chosen
 * bool sound: Whether sound was left on
 */
struct MenuResult {
	bool start;
	int speedMs;
	Level level;
	bool sound;
};

/**
 * Runs the menu and its screens until the player starts, quits or the key source ends
 *
 * Parameters:
 * IKeySource& keys: Source of key presses
 * const IInputHandler& input: Maps keys to actions
 * IMenuView& view: Draws the menu
 * MenuModel& model: Menu state, updated as keys arrive
 * const IScoreSource& scores: Where the menu reads the figures and leaderboards from
 *
 * Returns MenuResult: start true with the chosen speed; false on Quit, Ctrl+C or EOF
 */
MenuResult runMenu(IKeySource& keys, const IInputHandler& input, IMenuView& view,
	MenuModel& model, const IScoreSource& scores);

#endif
