/**
 * menu_model.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Start menu state, independent of any presentation
 */
#ifndef MENU_MODEL_HPP
#define MENU_MODEL_HPP

#include "input_handler.hpp"
#include "level.hpp"

/**
 * Row of the start menu
 */
enum class MenuItem { Start, Level, Leaderboard, Help, Sound, Quit };

/**
 * What an action did to the menu
 */
enum class MenuOutcome { None, Start, Quit, OpenLeaderboard, OpenHelp };

/**
 * Start menu state machine
 *
 * Attributes:
 * MenuItem selected_: Row currently selected
 * Level level_: Level currently chosen
 * bool sound_: Whether sound is on
 */
class MenuModel {
private:
	MenuItem selected_ = MenuItem::Start;
	Level level_ = Level::Normal;
	bool sound_ = true;

public:
	/**
	 * Gets the selected row
	 *
	 * Returns MenuItem: The selected row
	 */
	MenuItem selected() const;

	/**
	 * Gets the chosen level
	 *
	 * Returns Level: The chosen level
	 */
	Level level() const;

	/**
	 * Tells whether sound is on
	 *
	 * Returns bool: true if sound is on
	 */
	bool sound() const;

	/**
	 * Gets the frame time of the chosen level
	 *
	 * Returns int: 300 for Easy, 200 for Normal, 100 for Hard (milliseconds per frame)
	 */
	int speedMs() const;

	/**
	 * Applies an action: up/down move the selection (wrapping), left/right change the level
	 * on the Level row and toggle sound on the Sound row, Confirm acts on the selected row
	 * (starts, quits, opens a screen or toggles sound), Quit quits from any row
	 *
	 * Parameters:
	 * Action action: The action to apply
	 *
	 * Returns MenuOutcome: Start or Quit when the menu is done, OpenLeaderboard or OpenHelp to show a screen, otherwise None
	 */
	MenuOutcome apply(Action action);
};

#endif
