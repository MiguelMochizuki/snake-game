/**
 * menu_model.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Start menu state implementation
 */
#include "menu_model.hpp"

namespace {

const int ITEM_COUNT = 3;
const int LEVEL_COUNT = 3;

}  // namespace

/**
 * Gets the display name of a level
 *
 * Parameters:
 * Level level: The level
 *
 * Returns const char*: "Easy", "Normal" or "Hard"
 */
const char* levelName(Level level) {
	switch (level) {
		case Level::Easy: return "Easy";
		case Level::Normal: return "Normal";
		case Level::Hard: return "Hard";
	}
	return "";
}

/**
 * Gets the selected row
 *
 * Returns MenuItem: The selected row
 */
MenuItem MenuModel::selected() const {
	return selected_;
}

/**
 * Gets the chosen level
 *
 * Returns Level: The chosen level
 */
Level MenuModel::level() const {
	return level_;
}

/**
 * Gets the frame time of the chosen level
 *
 * Returns int: 300 for Easy, 200 for Normal, 100 for Hard (milliseconds per frame)
 */
int MenuModel::speedMs() const {
	switch (level_) {
		case Level::Easy: return 300;
		case Level::Normal: return 200;
		case Level::Hard: return 100;
	}
	return 200;
}

/**
 * Applies an action to the menu
 *
 * Parameters:
 * Action action: The action to apply
 *
 * Returns MenuOutcome: Start or Quit when the menu is done, otherwise None
 */
MenuOutcome MenuModel::apply(Action action) {
	int item = static_cast<int>(selected_);
	int level = static_cast<int>(level_);
	switch (action) {
		case Action::MoveUp:
			selected_ = static_cast<MenuItem>((item + ITEM_COUNT - 1) % ITEM_COUNT);
			break;
		case Action::MoveDown:
			selected_ = static_cast<MenuItem>((item + 1) % ITEM_COUNT);
			break;
		case Action::MoveLeft:
			if (selected_ == MenuItem::Level) {
				level_ = static_cast<Level>((level + LEVEL_COUNT - 1) % LEVEL_COUNT);
			}
			break;
		case Action::MoveRight:
			if (selected_ == MenuItem::Level) {
				level_ = static_cast<Level>((level + 1) % LEVEL_COUNT);
			}
			break;
		case Action::Confirm:
			if (selected_ == MenuItem::Start) {
				return MenuOutcome::Start;
			}
			if (selected_ == MenuItem::Quit) {
				return MenuOutcome::Quit;
			}
			break;
		case Action::Quit:
			return MenuOutcome::Quit;
		case Action::Restart:
			break;
	}
	return MenuOutcome::None;
}
