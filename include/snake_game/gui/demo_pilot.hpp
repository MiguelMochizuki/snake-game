/**
 * demo_pilot.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Autopilot that plays the game for demos and recordings
 */
#ifndef DEMO_PILOT_HPP
#define DEMO_PILOT_HPP

#include "snake_game/game/game_state.hpp"
#include "snake_game/gui/sdl_app.hpp"
#include "snake_game/gui/sdl_renderer.hpp"
#include "snake_game/input/input_handler.hpp"
#include "snake_game/input/key_source.hpp"
#include <functional>
#include <optional>
#include <vector>

/**
 * What the autopilot sees of a board
 *
 * Attributes:
 * int rows, int cols: Board size
 * std::vector<bool> blocked: Row-major, true for body cells (the food cell is not blocked)
 * int headX, int headY: Head position
 * int foodX, int foodY: Food position
 */
struct DemoBoard {
	int rows;
	int cols;
	std::vector<bool> blocked;
	int headX;
	int headY;
	int foodX;
	int foodY;
};

/**
 * Picks the next move: the first step of a shortest path to the food over free cells; if the food cannot be reached, the free neighbour with the most reachable cells
 *
 * Parameters:
 * const DemoBoard& board: The board
 *
 * Returns std::optional<Direction>: The move, or nullopt if the head has no free neighbour
 */
std::optional<Direction> demoMoveOn(const DemoBoard& board);

/**
 * Picks the next move for a game state
 *
 * Parameters:
 * const GameState& state: The state
 *
 * Returns std::optional<Direction>: The move, or nullopt if the head has no free neighbour
 */
std::optional<Direction> demoMove(const GameState& state);

/**
 * Key source that plays by itself: one move key per poll, window events still handled, and Q once at the game over panel
 *
 * Attributes:
 * const SdlRenderer& renderer_: Source of the current state
 * SdlApp& app_: Event pump, so a window close is noticed
 * std::function<bool()> stop_: Asked on every poll; when true a Q is sent to end the game
 * bool quitSent_: Whether Q was already given at the game over panel
 */
class DemoKeys : public IKeySource {
	const SdlRenderer& renderer_;
	SdlApp& app_;
	std::function<bool()> stop_;
	bool quitSent_ = false;

public:
	/**
	 * Constructor for the DemoKeys class
	 *
	 * Parameters:
	 * const SdlRenderer& renderer: Renderer of the game being played
	 * SdlApp& app: Event pump
	 * std::function<bool()> stop: Returns true when the demo should end (default: never)
	 */
	DemoKeys(const SdlRenderer& renderer, SdlApp& app, std::function<bool()> stop = nullptr);

	std::vector<Key> poll() override;
	std::optional<Key> waitKey() override;
	void flush() override;
};

#endif
