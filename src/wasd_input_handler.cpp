/**
 * wasd_input_handler.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Maps WASD keys to movement actions
 */
#include "wasd_input_handler.hpp"

/**
 * Maps a key to a movement action
 *
 * Parameters:
 * const Key& key: The key pressed
 *
 * Returns std::optional<Action>: MoveUp/MoveDown/MoveLeft/MoveRight for wWsSaAdD, else nullopt
 */
std::optional<Action> WASDInputHandler::handle(const Key& key) const {
	if (key.kind != KeyKind::Char) {
		return std::nullopt;
	}
	switch (key.ch) {
		case 'w': case 'W': return Action::MoveUp;
		case 's': case 'S': return Action::MoveDown;
		case 'a': case 'A': return Action::MoveLeft;
		case 'd': case 'D': return Action::MoveRight;
		default: return std::nullopt;
	}
}
