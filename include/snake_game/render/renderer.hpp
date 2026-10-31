/**
 * renderer.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Renderer interface used by the game engine
 */
#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "snake_game/game/game_state.hpp"
#include "snake_game/game/level.hpp"
#include "snake_game/score/score_source.hpp"

/**
 * Draws the game and paces it; implemented by the terminal and graphical front-ends
 */
class IRenderer {
public:
	virtual ~IRenderer() = default;

	/**
	 * Resets the renderer state (for game restart)
	 */
	virtual void reset() = 0;

	/**
	 * Clears the screen
	 */
	virtual void clear() = 0;

	/**
	 * Draws the game state
	 *
	 * Parameters:
	 * const GameState& state: The game state to draw
	 */
	virtual void render(const GameState& state) = 0;

	/**
	 * Shows the game over panel with statistics
	 *
	 * Parameters:
	 * Level level: The level that was played
	 * int finalScore: The final score
	 * const ScoreStats& stats: Best, games played and average of that level
	 */
	virtual void printGameOver(Level level, int finalScore, const ScoreStats& stats) = 0;

	/**
	 * Waits between game steps, keeping the front-end responsive if it needs to
	 *
	 * Parameters:
	 * int ms: Time to wait in milliseconds
	 */
	virtual void pause(int ms) = 0;
};

#endif
