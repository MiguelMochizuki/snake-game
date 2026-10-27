/**
 * composite_input_handler.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Composite input handler implementation
 */
#include "composite_input_handler.hpp"

/**
 * Registers a handler after the existing ones
 *
 * Parameters:
 * std::unique_ptr<IInputHandler> handler: The handler to add, which takes ownership
 */
void CompositeInputHandler::add(std::unique_ptr<IInputHandler> handler) {
	handlers_.push_back(std::move(handler));
}

/**
 * Returns the result of the first registered handler that maps the key
 *
 * Parameters:
 * const Key& key: The key pressed
 *
 * Returns std::optional<Action>: The first non-empty action, or nullopt if none match
 */
std::optional<Action> CompositeInputHandler::handle(const Key& key) const {
	for (const auto& handler : handlers_) {
		if (auto action = handler->handle(key)) {
			return action;
		}
	}
	return std::nullopt;
}
