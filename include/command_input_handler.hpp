/**
 * command_input_handler.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Maps quit and restart keys to actions
 */
#ifndef COMMAND_INPUT_HANDLER_HPP
#define COMMAND_INPUT_HANDLER_HPP

#include "input_handler.hpp"

/**
 * Maps q, Q and Ctrl+C to Quit, r, R to Restart, and Enter to Confirm
 */
class CommandInputHandler : public IInputHandler {
public:
	/**
	 * Maps a command key to its action
	 *
	 * Parameters:
	 * const Key& key: The key pressed
	 *
	 * Returns std::optional<Action>: Quit, Restart or Confirm, or nullopt for any other key
	 */
	std::optional<Action> handle(const Key& key) const override;
};

#endif
