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
#include "level.hpp"
#include "sound.hpp"
#include "terminal_renderer.hpp"
#include "score_manager.hpp"
#include <ctime>
#include <functional>
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
 * How a finished game ends
 */
enum class GameEnd { Restart, Menu, Quit };

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
 * Level level_: Level that tags saved scores
 * ISound* sound_: Sound effects, nullptr for silence
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
	Level level_ = Level::Normal;
	ISound* sound_ = nullptr;

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
	 * Returns TickInput: quit if any key mapped to Quit, else the last move; Restart, Menu and Confirm are ignored
	 */
	static TickInput readTick(IKeySource& keys, const IInputHandler& handler);

	/**
	 * Discards keys typed before the prompt, shows the game over panel, then waits for restart, menu or quit
	 *
	 * Parameters:
	 * IKeySource& keys: Source of key presses
	 * const IInputHandler& handler: Maps keys to actions
	 * const std::function<void()>& showPanel: Draws the game over panel, called after the flush
	 *
	 * Returns GameEnd: Restart or Menu on those keys; Quit on Quit or EOF. Moves and unmapped keys are ignored
	 */
	static GameEnd awaitChoice(IKeySource& keys, const IInputHandler& handler,
		const std::function<void()>& showPanel);

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
	 * Sets the level and game speed for the next rounds
	 *
	 * Parameters:
	 * Level level: Level that tags the saved scores and sets the best score shown
	 * int speedMs: Game speed in milliseconds per frame
	 */
	void setLevel(Level level, int speedMs);

	/**
	 * Sets the sound effects
	 *
	 * Parameters:
	 * ISound* sound: Receives eat and game over events, must outlive the engine; nullptr for silence
	 */
	void setSound(ISound* sound);

	/**
	 * Runs the main game loop, restarting on request, until the player asks for the menu or quits
	 *
	 * Returns GameEnd: Menu or Quit; the board is reset for the next run() after Menu
	 */
	GameEnd run();
};

#endif
