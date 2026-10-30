/**
 * command_input_handler.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Command key input handler implementation
 */
#include "command_input_handler.hpp"

/**
 * Maps a command key to its action
 *
 * Parameters:
 * const Key& key: The key pressed
 *
 * Returns std::optional<Action>: Quit or Restart, or nullopt for any other key
 */
std::optional<Action> CommandInputHandler::handle(const Key& key) const {
	if (key.kind == KeyKind::CtrlC) {
		return Action::Quit;
	}
	if (key.kind == KeyKind::Enter) {
		return Action::Confirm;
	}
	if (key.kind != KeyKind::Char) {
		return std::nullopt;
	}
	switch (key.ch) {
		case 'q': case 'Q': return Action::Quit;
		case 'r': case 'R': return Action::Restart;
		case 'm': case 'M': return Action::Menu;
		default: return std::nullopt;
	}
}
