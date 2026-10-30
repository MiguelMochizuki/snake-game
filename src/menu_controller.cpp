/**
 * menu_controller.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Start menu loop implementation
 */
#include "menu_controller.hpp"

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
		MenuModel& model, const MenuStats& stats) {
	keys.flush();
	while (true) {
		view.show(MenuSnapshot{model.selected(), model.level(), stats});
		std::optional<Key> key = keys.waitKey();
		if (!key) {
			return MenuResult{false, model.speedMs()};
		}
		std::optional<Action> action = input.handle(*key);
		if (!action) {
			continue;
		}
		MenuOutcome outcome = model.apply(*action);
		if (outcome == MenuOutcome::Start) {
			return MenuResult{true, model.speedMs()};
		}
		if (outcome == MenuOutcome::Quit) {
			return MenuResult{false, model.speedMs()};
		}
	}
}
