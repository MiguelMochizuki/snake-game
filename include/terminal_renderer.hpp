/**
 * terminal_renderer.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Terminal renderer class
 */
#ifndef TERMINAL_RENDERER_HPP
#define TERMINAL_RENDERER_HPP

#include "game_state.hpp"
#include <cstddef>
#include <iostream>

// Forward declaration
class WASDInputHandler;

/**
 * Renders game state to terminal/console
 *
 * Attributes:
 * std::ostream& out_: Stream that receives all output
 * bool firstRender_: Whether the next clear is the first one (full clear)
 * bool color_: Whether output carries ANSI SGR color codes
 */
class TerminalRenderer {
	std::ostream& out_;
	bool firstRender_ = true;
	bool color_;

public:
	/**
	 * Constructor for the TerminalRenderer class
	 *
	 * Parameters:
	 * std::ostream& out: Stream that receives all output (default: std::cout)
	 * bool color: Whether to emit ANSI SGR color codes (default: false)
	 */
	explicit TerminalRenderer(std::ostream& out = std::cout, bool color = false);

	/**
	 * Decides whether color should be on, following the NO_COLOR convention
	 *
	 * Returns bool: true unless NO_COLOR is set and non-empty
	 */
	static bool colorEnabledFromEnv();

	/**
	 * Destructor for the TerminalRenderer class
	 */
	~TerminalRenderer() = default;

	/**
	 * Resets the renderer state (for game restart)
	 */
	void reset();

	/**
	 * Clears the terminal screen
	 */
	void clear();

	/**
	 * Renders the game state
	 *
	 * Parameters:
	 * const GameState& state: The game state to render
	 */
	void render(const GameState& state);

	/**
	 * Prints the game over panel with statistics
	 *
	 * Parameters:
	 * int finalScore: The final score
	 * int bestScore: The best score
	 * size_t totalGames: Total number of games played
	 * double averageScore: Average score
	 */
	void printGameOver(int finalScore, int bestScore, size_t totalGames, double averageScore);

	/**
	 * Shows game over screen with statistics
	 *
	 * Parameters:
	 * int finalScore: The final score
	 * int bestScore: The best score
	 * size_t totalGames: Total number of games played
	 * double averageScore: Average score
	 * WASDInputHandler& inputHandler: Reference to the input handler
	 *
	 * Returns bool: true if restart was requested (R key)
	 */
	bool showGameOver(int finalScore, int bestScore, size_t totalGames,
	                  double averageScore, WASDInputHandler& inputHandler);
};

#endif
