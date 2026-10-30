/**
 * input_handler.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Input key, action and handler interface
 */
#ifndef INPUT_HANDLER_HPP
#define INPUT_HANDLER_HPP

#include <optional>

/**
 * Direction enum for snake movement
 */
enum class Direction {
	UP = 0,
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3
};

/**
 * Kind of key pressed
 */
enum class KeyKind { Char, Up, Down, Left, Right, CtrlC, Enter, Unknown };

/**
 * A decoded key press
 *
 * Attributes:
 * KeyKind kind: Kind of key pressed
 * char ch: Character, meaningful only when kind is KeyKind::Char, else '\0'
 */
struct Key {
	KeyKind kind;
	char ch;

	/**
	 * Compares two keys, ignoring ch unless both are character keys
	 *
	 * Parameters:
	 * const Key& o: The key to compare with
	 *
	 * Returns bool: true if the keys are the same key
	 */
	bool operator==(const Key& o) const {
		return kind == o.kind && (kind != KeyKind::Char || ch == o.ch);
	}
};

/**
 * Builds a character key
 *
 * Parameters:
 * char c: The character pressed
 *
 * Returns Key: A key of kind KeyKind::Char holding c
 */
inline Key charKey(char c) {
	return Key{KeyKind::Char, c};
}

/**
 * Builds a key that is not a character
 *
 * Parameters:
 * KeyKind k: The kind of key pressed
 *
 * Returns Key: A key of kind k with no character
 */
inline Key specialKey(KeyKind k) {
	return Key{k, '\0'};
}

/**
 * Game action a key can trigger
 */
enum class Action { MoveUp, MoveDown, MoveLeft, MoveRight, Quit, Restart, Confirm };

/**
 * Maps a key to an action
 */
class IInputHandler {
public:
	/**
	 * Destructor for the IInputHandler interface
	 */
	virtual ~IInputHandler() = default;

	/**
	 * Maps a key to an action
	 *
	 * Parameters:
	 * const Key& key: The key pressed
	 *
	 * Returns std::optional<Action>: The action, or nullopt if the key does nothing here
	 */
	virtual std::optional<Action> handle(const Key& key) const = 0;
};

#endif
