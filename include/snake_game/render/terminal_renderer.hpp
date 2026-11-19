/**
 * terminal_renderer.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Terminal renderer class
 */
#ifndef TERMINAL_RENDERER_HPP
#define TERMINAL_RENDERER_HPP

#include "snake_game/game/game_state.hpp"
#include "snake_game/game/level.hpp"
#include "snake_game/render/renderer.hpp"
#include "snake_game/score/score_source.hpp"
#include "snake_game/terminal/painter.hpp"
#include <cstddef>
#include <iostream>

/**
 * Renders game state to terminal/console
 *
 * Attributes:
 * std::ostream& out_: Stream that receives all output
 * bool firstRender_: Whether the next clear is the first one (full clear)
 * bool color_: Whether output carries ANSI SGR color codes
 */
class TerminalRenderer : public IRenderer {
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
	void reset() override;

	/**
	 * Clears the terminal screen
	 */
	void clear() override;

	/**
	 * Renders the game state
	 *
	 * Parameters:
	 * const GameState& state: The game state to render
	 */
	void render(const GameState& state) override;

	/**
	 * Prints the end of game panel: a centered title, the statistics and the key hints
	 *
	 * Parameters:
	 * const std::string& title: Panel title; whatever does not fit in the 30 columns after leftPad is cut off
	 * terminal::Style titleStyle: SGR style of the title
	 * size_t leftPad: Spaces before the title (at most 30); the rest of the 30 columns is padded after it
	 * Level level: The level that was played
	 * int finalScore: The final score
	 * const ScoreStats& stats: Best, games played and average of that level
	 */
	void printPanel(const std::string& title, terminal::Style titleStyle, size_t leftPad, Level level, int finalScore, const ScoreStats& stats);

	/**
	 * Prints the game over panel with statistics
	 *
	 * Parameters:
	 * Level level: The level that was played
	 * int finalScore: The final score
	 * const ScoreStats& stats: Best, games played and average of that level
	 */
	void printGameOver(Level level, int finalScore, const ScoreStats& stats) override;

	/**
	 * Prints the victory panel with statistics: the game over panel with a yellow "YOU WIN!" title
	 *
	 * Parameters:
	 * Level level: The level that was played
	 * int finalScore: The final score
	 * const ScoreStats& stats: Best, games played and average of that level
	 */
	void printVictory(Level level, int finalScore, const ScoreStats& stats) override;

	/**
	 * Sleeps between game steps
	 *
	 * Parameters:
	 * int ms: Time to sleep in milliseconds
	 */
	void pause(int ms) override;
};

#endif
