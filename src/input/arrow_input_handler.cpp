/**
 * arrow_input_handler.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Arrow key input handler implementation
 */
#include "snake_game/input/arrow_input_handler.hpp"

/**
 * Maps an arrow key to its movement action
 *
 * Parameters:
 * const Key& key: The key pressed
 *
 * Returns std::optional<Action>: MoveUp, MoveDown, MoveLeft or MoveRight, or nullopt for any other key
 */
std::optional<Action> ArrowInputHandler::handle(const Key& key) const {
	switch (key.kind) {
		case KeyKind::Up: return Action::MoveUp;
		case KeyKind::Down: return Action::MoveDown;
		case KeyKind::Left: return Action::MoveLeft;
		case KeyKind::Right: return Action::MoveRight;
		default: return std::nullopt;
	}
}
