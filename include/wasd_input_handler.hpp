/**
 * wasd_input_handler.hpp
 * Author: Miguel Mochizuki Silva
 * Description: WASD input handler class
 */
#ifndef WASD_INPUT_HANDLER_HPP
#define WASD_INPUT_HANDLER_HPP

#include "input_handler.hpp"

/**
 * Input state structure to hold all input information
 */
struct InputState {
	Direction direction;    // Current direction
	bool quit;             // Quit flag
	bool directionChanged; // Whether direction was changed this frame

	InputState() : direction(Direction::RIGHT), quit(false), directionChanged(false) {}
};

/**
 * Handles WASD keyboard input for snake movement
 */
class WASDInputHandler {
private:
	InputState state_;     // Current input state
	bool isTerminalSetup_; // Track if terminal is in raw mode

	/**
	 * Reads a character without blocking
	 *
	 * Returns char: The character read, or 0 if none
	 */
	char readInput();

	/**
	 * Sets up terminal for raw input mode (Unix only)
	 */
	void setupTerminal();

	/**
	 * Restores terminal settings (Unix only)
	 */
	void restoreTerminal();

public:
	/**
	 * Constructor for the WASDInputHandler class
	 */
	WASDInputHandler();

	/**
	 * Destructor for the WASDInputHandler class
	 */
	~WASDInputHandler();

	/**
	 * Checks if direction changed this frame
	 *
	 * Returns bool: true if direction was changed
	 */
	bool directionChanged() const;

	/**
	 * Ensures terminal is in raw mode
	 */
	void ensureTerminal();

	/**
	 * Gets the current direction
	 *
	 * Returns Direction: The current direction
	 */
	Direction getDirection() const;

	/**
	 * Processes all pending input and updates state
	 * This should be called once per frame
	 */
	void processInput();

	/**
	 * Reads a character blocking (for game over screen)
	 *
	 * Returns char: The character read
	 */
	char readCharBlocking();

	/**
	 * Resets the direction changed flag
	 */
	void resetDirectionChanged();

	/**
	 * Resets quit flag to true when restarting the game
	 */
	void resetQuit();

	/**
	 * Checks if quit was pressed
	 *
	 * Returns bool: true if 'q' was pressed
	 */
	bool shouldQuit() const;
};

#endif
