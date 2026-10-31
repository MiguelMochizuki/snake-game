/**
 * menu_model.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Start menu state implementation
 */
#include "snake_game/menu/menu_model.hpp"

namespace {

const int ITEM_COUNT = 6;

}  // namespace

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
 * Tells whether sound is on
 *
 * Returns bool: true if sound is on
 */
bool MenuModel::sound() const {
	return sound_;
}

/**
 * Gets the frame time of the chosen level
 *
 * Returns int: 300 for Easy, 200 for Normal, 100 for Hard (milliseconds per frame)
 */
int MenuModel::speedMs() const {
	return levelSpeedMs(level_);
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
	switch (action) {
		case Action::MoveUp:
			selected_ = static_cast<MenuItem>((item + ITEM_COUNT - 1) % ITEM_COUNT);
			break;
		case Action::MoveDown:
			selected_ = static_cast<MenuItem>((item + 1) % ITEM_COUNT);
			break;
		case Action::MoveLeft:
			if (selected_ == MenuItem::Level) {
				level_ = nextLevel(level_, -1);
			} else if (selected_ == MenuItem::Sound) {
				sound_ = !sound_;
			}
			break;
		case Action::MoveRight:
			if (selected_ == MenuItem::Level) {
				level_ = nextLevel(level_, 1);
			} else if (selected_ == MenuItem::Sound) {
				sound_ = !sound_;
			}
			break;
		case Action::Confirm:
			if (selected_ == MenuItem::Start) {
				return MenuOutcome::Start;
			}
			if (selected_ == MenuItem::Quit) {
				return MenuOutcome::Quit;
			}
			if (selected_ == MenuItem::Leaderboard) {
				return MenuOutcome::OpenLeaderboard;
			}
			if (selected_ == MenuItem::Help) {
				return MenuOutcome::OpenHelp;
			}
			if (selected_ == MenuItem::Sound) {
				sound_ = !sound_;
			}
			break;
		case Action::Quit:
			return MenuOutcome::Quit;
		case Action::Restart:
		case Action::Menu:
			break;
	}
	return MenuOutcome::None;
}
