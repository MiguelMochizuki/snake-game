/**
 * wasd_input_handler.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Maps WASD keys to movement actions
 */
#ifndef WASD_INPUT_HANDLER_HPP
#define WASD_INPUT_HANDLER_HPP

#include "snake_game/input/input_handler.hpp"

/**
 * Maps w/a/s/d, in either case, to movement actions
 */
class WASDInputHandler : public IInputHandler {
public:
	/**
	 * Maps a key to a movement action
	 *
	 * Parameters:
	 * const Key& key: The key pressed
	 *
	 * Returns std::optional<Action>: MoveUp/MoveDown/MoveLeft/MoveRight for wWsSaAdD, else nullopt
	 */
	std::optional<Action> handle(const Key& key) const override;
};

#endif
