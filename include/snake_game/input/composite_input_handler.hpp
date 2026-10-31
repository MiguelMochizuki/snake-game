/**
 * composite_input_handler.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Chains several input handlers, first match wins
 */
#ifndef COMPOSITE_INPUT_HANDLER_HPP
#define COMPOSITE_INPUT_HANDLER_HPP

#include <memory>
#include <vector>

#include "snake_game/input/input_handler.hpp"

/**
 * Asks each registered handler in order and returns the first action found
 *
 * Attributes:
 * std::vector<std::unique_ptr<IInputHandler>> handlers_: Handlers in registration order
 */
class CompositeInputHandler : public IInputHandler {
private:
	std::vector<std::unique_ptr<IInputHandler>> handlers_;

public:
	/**
	 * Registers a handler after the existing ones
	 *
	 * Parameters:
	 * std::unique_ptr<IInputHandler> handler: The handler to add, which takes ownership
	 */
	void add(std::unique_ptr<IInputHandler> handler);

	/**
	 * Returns the result of the first registered handler that maps the key
	 *
	 * Parameters:
	 * const Key& key: The key pressed
	 *
	 * Returns std::optional<Action>: The first non-empty action, or nullopt if none match
	 */
	std::optional<Action> handle(const Key& key) const override;
};

#endif
