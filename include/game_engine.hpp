/**
 * game_engine.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Class GameEngine headers
 */
#ifndef GAME_ENGINE_HPP
#define GAME_ENGINE_HPP

#include "game_state.hpp"
#include "wasd_input_handler.hpp"
#include "terminal_renderer.hpp"
#include "score_manager.hpp"

/**
 * Main game engine - orchestrates the game loop
 *
 * Attributes:
 * ScoreManager scoreManager_: Manages score persistence
 * GameState state_: Current game state
 * WASDInputHandler inputHandler_: Input handler for user interaction
 * TerminalRenderer renderer_: Renderer for displaying the game
 * Direction direction_: Current direction
 * bool running_: Flag indicating if the game is running
 * int speedMs_: Game speed in milliseconds per frame
 */
class GameEngine {
private:
	ScoreManager scoreManager_;
	GameState state_;
	WASDInputHandler inputHandler_;
	TerminalRenderer renderer_;
	Direction direction_;
	bool running_;
	int speedMs_;

public:
	/**
	 * Constructor for the GameEngine class
	 *
	 * Parameters:
	 * int rows: Number of rows in the board (default: 20)
	 * int cols: Number of columns in the board (default: 20)
	 * int speedMs: Game speed in milliseconds (default: 200)
	 * unsigned seed: Seed for food placement (default: current time)
	 */
	GameEngine(int rows = 20, int cols = 20, int speedMs = 200, unsigned seed = static_cast<unsigned>(std::time(nullptr)));

	/**
	 * Destructor for the GameEngine class
	 */
	~GameEngine() = default;

	/**
	 * Runs the main game loop
	 */
	void run();
};

#endif
