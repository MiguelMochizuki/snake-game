/**
 * menu_controller.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Start menu loop implementation
 */
#include "snake_game/menu/menu_controller.hpp"

namespace {

const std::size_t LEADERBOARD_SIZE = 10;

/**
 * Builds the result of a finished menu
 *
 * Parameters:
 * const MenuModel& model: The menu state
 * bool start: Whether the player chose to play
 *
 * Returns MenuResult: The result
 */
MenuResult resultOf(const MenuModel& model, bool start) {
	return MenuResult{start, model.speedMs(), model.level(), model.sound(), model.practice()};
}

/**
 * Asks whether to delete every saved score and does it on Enter; Q keeps them, other keys are ignored
 *
 * Parameters:
 * IKeySource& keys: Source of key presses
 * const IInputHandler& input: Maps keys to actions
 * IMenuView& view: Draws the question
 * IScoreSource& scores: Cleared when the player confirms
 *
 * Returns bool: false if the key source ended (the program should quit), true otherwise
 */
bool runConfirmClear(IKeySource& keys, const IInputHandler& input, IMenuView& view, IScoreSource& scores) {
	view.showConfirmClear();
	while (true) {
		std::optional<Key> key = keys.waitKey();
		if (!key) {
			return false;
		}
		std::optional<Action> action = input.handle(*key);
		if (action == Action::Confirm) {
			scores.clear();
			return true;
		}
		if (action == Action::Quit) {
			return true;
		}
	}
}

/**
 * Shows the leaderboard until the player goes back; Left and Right change the level shown, C offers to clear every score
 *
 * Parameters:
 * IKeySource& keys: Source of key presses
 * const IInputHandler& input: Maps keys to actions
 * IMenuView& view: Draws the leaderboard
 * IScoreSource& scores: Where the scores come from, and where they are cleared
 * Level level: Level to start on
 *
 * Returns bool: false if the key source ended (the program should quit), true when the player went back
 */
bool runLeaderboard(IKeySource& keys, const IInputHandler& input, IMenuView& view,
		IScoreSource& scores, Level level) {
	while (true) {
		view.showLeaderboard(LeaderboardSnapshot{level, scores.topScores(level, LEADERBOARD_SIZE)});
		std::optional<Key> key = keys.waitKey();
		if (!key) {
			return false;
		}
		std::optional<Action> action = input.handle(*key);
		if (action == Action::MoveLeft) {
			level = nextLevel(level, -1);
		} else if (action == Action::MoveRight) {
			level = nextLevel(level, 1);
		} else if (action == Action::Clear) {
			if (!runConfirmClear(keys, input, view, scores)) {
				return false;
			}
		} else if (action == Action::Confirm || action == Action::Quit) {
			return true;
		}
	}
}

/**
 * Shows the how-to-play screen until the player goes back
 *
 * Parameters:
 * IKeySource& keys: Source of key presses
 * const IInputHandler& input: Maps keys to actions
 * IMenuView& view: Draws the screen
 *
 * Returns bool: false if the key source ended (the program should quit), true when the player went back
 */
bool runHelp(IKeySource& keys, const IInputHandler& input, IMenuView& view) {
	view.showHelp();
	while (true) {
		std::optional<Key> key = keys.waitKey();
		if (!key) {
			return false;
		}
		std::optional<Action> action = input.handle(*key);
		if (action == Action::Confirm || action == Action::Quit) {
			return true;
		}
	}
}

}  // namespace

/**
 * Runs the menu and its screens until the player starts, quits or the key source ends
 *
 * Parameters:
 * IKeySource& keys: Source of key presses
 * const IInputHandler& input: Maps keys to actions
 * IMenuView& view: Draws the menu
 * MenuModel& model: Menu state, updated as keys arrive
 * IScoreSource& scores: Where the menu reads the figures and leaderboards from, and clears them on request
 * ISound* sound: Plays a blip on Up/Down and Confirm while the menu's sound switch is on, nullptr for silence
 *
 * Returns MenuResult: start true with the chosen speed; false on Quit, Ctrl+C or EOF
 */
MenuResult runMenu(IKeySource& keys, const IInputHandler& input, IMenuView& view,
		MenuModel& model, IScoreSource& scores, ISound* sound) {
	keys.flush();
	while (true) {
		view.show(MenuSnapshot{model.selected(), model.level(), scores.stats(model.level()), model.sound(), model.practice()});
		std::optional<Key> key = keys.waitKey();
		if (!key) {
			return resultOf(model, false);
		}
		std::optional<Action> action = input.handle(*key);
		if (!action) {
			continue;
		}
		const MenuOutcome outcome = model.apply(*action);
		// Judged after the action, so switching sound Off is itself silent
		if (sound && model.sound()) {
			if (*action == Action::MoveUp || *action == Action::MoveDown) {
				sound->menuMove();
			} else if (*action == Action::Confirm) {
				sound->menuSelect();
			}
		}
		switch (outcome) {
			case MenuOutcome::Start:
				return resultOf(model, true);
			case MenuOutcome::Quit:
				return resultOf(model, false);
			case MenuOutcome::OpenLeaderboard:
				if (!runLeaderboard(keys, input, view, scores, model.level())) {
					return resultOf(model, false);
				}
				break;
			case MenuOutcome::OpenHelp:
				if (!runHelp(keys, input, view)) {
					return resultOf(model, false);
				}
				break;
			case MenuOutcome::None:
				break;
		}
	}
}
