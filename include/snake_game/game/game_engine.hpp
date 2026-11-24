/**
 * game_engine.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Class GameEngine headers
 */
#ifndef GAME_ENGINE_HPP
#define GAME_ENGINE_HPP

#include "snake_game/game/game_state.hpp"
#include "snake_game/input/input_handler.hpp"
#include "snake_game/input/key_source.hpp"
#include "snake_game/game/level.hpp"
#include "snake_game/render/sound.hpp"
#include "snake_game/render/renderer.hpp"
#include "snake_game/score/score_source.hpp"
#include <deque>
#include <functional>
#include <optional>
#include <string>
#include <vector>

/**
 * What the player asked for during one tick
 *
 * Attributes:
 * bool quit: true if the player asked to end the round
 * bool pause: true if the player pressed pause or the window lost focus
 * std::vector<Direction> moves: Direction of every move key, in the order they were pressed
 */
struct TickInput {
	bool quit = false;
	bool pause = false;
	std::vector<Direction> moves;
};

/**
 * How a finished game ends
 */
enum class GameEnd { Restart, Menu, Quit };

/**
 * Main game engine - orchestrates the game loop
 *
 * Attributes:
 * IScoreStore& scores_: Where finished games are saved and the best score is read
 * GameState state_: Current game state
 * IKeySource& keys_: Source of key presses
 * const IInputHandler& input_: Maps keys to actions
 * IRenderer& renderer_: Renderer for displaying the game
 * Direction direction_: Current direction
 * std::deque<Direction> pending_: Turns pressed but not made yet, one is made per step
 * bool running_: Flag indicating if the game is running
 * int speedMs_: Game speed in milliseconds per frame
 * Level level_: Level that tags saved scores
 * ISound* sound_: Sound effects, nullptr for silence
 * bool practice_: Whether finished games are left out of the score file
 */
class GameEngine {
private:
	IScoreStore& scores_;
	GameState state_;
	IKeySource& keys_;
	const IInputHandler& input_;
	IRenderer& renderer_;
	Direction direction_;
	std::deque<Direction> pending_;
	bool running_;
	int speedMs_;
	Level level_ = Level::Normal;
	ISound* sound_ = nullptr;
	bool practice_ = false;

public:
	/**
	 * Constructor for the GameEngine class
	 *
	 * Parameters:
	 * IKeySource& keys: Source of key presses, must outlive the engine
	 * const IInputHandler& input: Maps keys to actions, must outlive the engine
	 * IRenderer& renderer: Draws the game and paces it, must outlive the engine
	 * IScoreStore& scores: Saves finished games and gives the best score, must outlive the engine
	 * int rows: Number of rows in the board
	 * int cols: Number of columns in the board
	 * int speedMs: Game speed in milliseconds
	 * unsigned seed: Seed for food placement
	 */
	GameEngine(IKeySource& keys, const IInputHandler& input, IRenderer& renderer, IScoreStore& scores, int rows, int cols, int speedMs, unsigned seed);

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
	 * Returns TickInput: quit if any key mapped to Quit, and every move in order; pause is set by Pause and AutoPause; Restart, Menu and Confirm are ignored
	 */
	static TickInput readTick(IKeySource& keys, const IInputHandler& handler);

	/**
	 * Shows the game over panel, discards keys typed before and while it appeared, then waits for restart, menu or quit
	 *
	 * Parameters:
	 * IKeySource& keys: Source of key presses
	 * const IInputHandler& handler: Maps keys to actions
	 * const std::function<void()>& showPanel: Draws the game over panel, called before the flush
	 *
	 * Returns GameEnd: Restart or Menu on those keys; Quit on Quit or EOF. Moves and unmapped keys are ignored
	 */
	static GameEnd awaitChoice(IKeySource& keys, const IInputHandler& handler,
		const std::function<void()>& showPanel);

	/**
	 * Shows the pause panel, discards keys typed before and while it appeared, then waits for the player to resume or quit
	 *
	 * Parameters:
	 * IKeySource& keys: Source of key presses
	 * const IInputHandler& handler: Maps keys to actions
	 * const std::function<void()>& showPanel: Draws the pause panel, called before the flush
	 *
	 * Returns bool: true on Pause (resume, with the keys typed while paused discarded); false on Quit or EOF. Moves, AutoPause and unmapped keys are ignored
	 */
	static bool awaitResume(IKeySource& keys, const IInputHandler& handler,
		const std::function<void()>& showPanel);

	/**
	 * Most turns kept waiting at once; a turn pressed with the queue full is ignored
	 */
	static constexpr std::size_t MAX_PENDING_TURNS = 2;

	/**
	 * Adds the turns of one tick to the queue of turns still to be made. A turn is kept only if it differs from, and is
	 * not the opposite of, the last turn in the queue (the current direction when the queue is empty), and only while
	 * the queue has room
	 *
	 * Parameters:
	 * std::deque<Direction>& pending: The queue, oldest turn first
	 * Direction current: The direction the snake is moving
	 * const std::vector<Direction>& moves: The turns pressed during the tick, in order
	 */
	static void queueTurns(std::deque<Direction>& pending, Direction current, const std::vector<Direction>& moves);

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
	 * Turns practice mode on or off
	 *
	 * Parameters:
	 * bool practice: true to play without saving scores to the leaderboard
	 */
	void setPractice(bool practice);

	/**
	 * Runs the main game loop, restarting on request, until the player asks for the menu or quits
	 *
	 * Returns GameEnd: Menu or Quit; the board is reset for the next run() after Menu
	 */
	GameEnd run();
};

#endif
