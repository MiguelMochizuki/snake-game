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
enum class MenuItem { Start, Level, Quit };

/**
 * What an action did to the menu
 */
enum class MenuOutcome { None, Start, Quit };

/**
 * Start menu state machine
 *
 * Attributes:
 * MenuItem selected_: Row currently selected
 * Level level_: Level currently chosen
 */
class MenuModel {
private:
	MenuItem selected_ = MenuItem::Start;
	Level level_ = Level::Normal;

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
	 * Gets the frame time of the chosen level
	 *
	 * Returns int: 300 for Easy, 200 for Normal, 100 for Hard (milliseconds per frame)
	 */
	int speedMs() const;

	/**
	 * Applies an action: up/down move the selection (wrapping), left/right change the level
	 * on the Level row, Confirm acts on the selected row, Quit quits from any row
	 *
	 * Parameters:
	 * Action action: The action to apply
	 *
	 * Returns MenuOutcome: Start or Quit when the menu is done, otherwise None
	 */
	MenuOutcome apply(Action action);
};

#endif
