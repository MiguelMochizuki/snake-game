/**
 * arrow_input_handler.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Maps arrow keys to movement actions
 */
#ifndef ARROW_INPUT_HANDLER_HPP
#define ARROW_INPUT_HANDLER_HPP

#include "snake_game/input/input_handler.hpp"

/**
 * Maps the four arrow keys to movement actions
 */
class ArrowInputHandler : public IInputHandler {
public:
	/**
	 * Maps an arrow key to its movement action
	 *
	 * Parameters:
	 * const Key& key: The key pressed
	 *
	 * Returns std::optional<Action>: MoveUp, MoveDown, MoveLeft or MoveRight, or nullopt for any other key
	 */
	std::optional<Action> handle(const Key& key) const override;
};

#endif
