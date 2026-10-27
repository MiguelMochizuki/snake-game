/**
 * game_engine.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Class GameEngine headers
 */
#ifndef GAME_ENGINE_HPP
#define GAME_ENGINE_HPP

#include "game_state.hpp"
#include "input_handler.hpp"
#include "key_source.hpp"
#include "terminal_renderer.hpp"
#include "score_manager.hpp"
#include <ctime>
#include <optional>

/**
 * What the player asked for during one tick
 *
 * Attributes:
 * bool quit: true if the player asked to end the round
 * std::optional<Direction> move: Direction of the last move key, if any
 */
struct TickInput {
	bool quit = false;
	std::optional<Direction> move;
};

/**
 * Main game engine - orchestrates the game loop
 *
 * Attributes:
 * ScoreManager scoreManager_: Manages score persistence
 * GameState state_: Current game state
 * IKeySource& keys_: Source of key presses
 * const IInputHandler& input_: Maps keys to actions
 * TerminalRenderer renderer_: Renderer for displaying the game
 * Direction direction_: Current direction
 * bool running_: Flag indicating if the game is running
 * int speedMs_: Game speed in milliseconds per frame
 */
class GameEngine {
private:
	ScoreManager scoreManager_;
	GameState state_;
	IKeySource& keys_;
	const IInputHandler& input_;
	TerminalRenderer renderer_;
	Direction direction_;
	bool running_;
	int speedMs_;

public:
	/**
	 * Constructor for the GameEngine class
	 *
	 * Parameters:
	 * IKeySource& keys: Source of key presses, must outlive the engine
	 * const IInputHandler& input: Maps keys to actions, must outlive the engine
	 * int rows: Number of rows in the board (default: 20)
	 * int cols: Number of columns in the board (default: 20)
	 * int speedMs: Game speed in milliseconds (default: 200)
	 * unsigned seed: Seed for food placement (default: current time)
	 * bool color: Whether to render with ANSI colors (default: false)
	 */
	GameEngine(IKeySource& keys, const IInputHandler& input, int rows = 20, int cols = 20, int speedMs = 200, unsigned seed = static_cast<unsigned>(std::time(nullptr)), bool color = false);

	/**
	 * Destructor for the GameEngine class
	 */
	~GameEngine() = default;

	/**
	 * Reads and maps every pending key for one tick
	 *
	 * Parameters:
	 * IKeySource& keys: Source of key presses
	 * const IInputHandler& handler: Maps keys to actions
	 *
	 * Returns TickInput: quit if any key mapped to Quit, else the last move; Restart is ignored
	 */
	static TickInput readTick(IKeySource& keys, const IInputHandler& handler);

	/**
	 * Waits on the game over screen for restart or quit
	 *
	 * Parameters:
	 * IKeySource& keys: Source of key presses
	 * const IInputHandler& handler: Maps keys to actions
	 *
	 * Returns bool: true on Restart; false on Quit or EOF. Moves and unmapped keys are ignored
	 */
	static bool awaitRestart(IKeySource& keys, const IInputHandler& handler);

	/**
	 * Tells whether two directions are opposite
	 *
	 * Parameters:
	 * Direction current: The direction the snake is moving
	 * Direction next: The requested direction
	 *
	 * Returns bool: true if next reverses current
	 */
	static bool isOpposite(Direction current, Direction next);

	/**
	 * Runs the main game loop
	 */
	void run();
};

#endif
