/**
 * terminal_renderer.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Terminal renderer implementation
 */
#include "terminal_renderer.hpp"
#include "terminal_painter.hpp"
#include "wasd_input_handler.hpp"
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <sstream>

using namespace terminal;

/**
 * Constructor for the TerminalRenderer class
 *
 * Parameters:
 * std::ostream& out: Stream that receives all output
 * bool color: Whether to emit ANSI SGR color codes
 */
TerminalRenderer::TerminalRenderer(std::ostream& out, bool color) : out_(out), color_(color) {
}

/**
 * Decides whether color should be on, following the NO_COLOR convention
 *
 * Returns bool: true unless NO_COLOR is set and non-empty
 */
bool TerminalRenderer::colorEnabledFromEnv() {
	const char* env = std::getenv("NO_COLOR");
	return env == nullptr || *env == '\0';
}

/**
 * Resets the renderer state (for game restart)
 */
void TerminalRenderer::reset() {
	firstRender_ = true;
}

/**
 * Clears the terminal screen
 */
void TerminalRenderer::clear() {
	if (firstRender_) {
		out_ << "\033[2J\033[H";
		firstRender_ = false;
	} else {
		out_ << "\033[H";
	}
}

/**
 * Renders the game state
 *
 * Parameters:
 * const GameState& state: The game state to render
 */
void TerminalRenderer::render(const GameState& state) {
	const auto& grid = state.getOccupied();
	int rows = state.getRows();
	int cols = state.getCols();
	int foodX = state.getFood().getX();
	int foodY = state.getFood().getY();

	const SnakeSegment* head = state.getSnake().getHead();
	int headX = head ? head->getX() : -1;
	int headY = head ? head->getY() : -1;

	std::ostringstream buf;
	Painter p(buf, color_);
	const std::string rule(cols + 2, '#');

	p.put(WALL, rule);
	p.endl();

	for (int y = 0; y < rows; ++y) {
		p.put(WALL, "#");
		for (int x = 0; x < cols; ++x) {
			if (!grid[y * cols + x]) {
				p.put(nullptr, " ");
			} else if (x == foodX && y == foodY) {
				p.put(FOOD, "*");
			} else if (x == headX && y == headY) {
				p.put(HEAD, "O");
			} else {
				p.put(BODY, "O");
			}
		}
		p.put(WALL, "#");
		p.endl();
	}

	p.put(WALL, rule);
	p.endl();

	p.put(LABEL, "Score:");
	p.put(nullptr, " ");
	p.put(NUMBER, std::to_string(state.getScore()));
	p.endl();
	p.put(LABEL, "Best:");
	p.put(nullptr, " ");
	p.put(NUMBER, std::to_string(state.getBestScore()));
	p.endl();
	p.put(DIM, "Controls: WASD to move, Q to quit");
	p.endl();

	out_ << buf.str();
}

/**
 * Prints the game over panel with statistics
 *
 * Parameters:
 * Level level: The level that was played
 * int finalScore: The final score
 * const ScoreStats& stats: Best, games played and average of that level
 */
void TerminalRenderer::printGameOver(Level level, int finalScore, const ScoreStats& stats) {
	std::ostringstream buf;
	Painter p(buf, color_);
	const std::string rule(30, '=');
	buf << "\n";
	p.put(WALL, rule);
	p.endl();
	p.put(nullptr, "         ");
	p.put(FOOD, "GAME OVER!");
	p.put(nullptr, "           ");
	p.endl();
	p.put(nullptr, std::string(30, ' '));
	p.endl();
	const std::string name = levelName(level);
	p.put(nullptr, "  ");
	p.put(LABEL, "Level:");
	p.put(nullptr, std::string(13 - name.size(), ' '));
	p.put(NUMBER, name);
	p.endl();
	p.put(nullptr, "  ");
	p.put(LABEL, "Final Score:");
	p.put(nullptr, "  ");
	p.put(NUMBER, padded(finalScore, 5));
	p.endl();
	p.put(nullptr, "  ");
	p.put(LABEL, "Best Score:");
	p.put(nullptr, "   ");
	p.put(NUMBER, padded(stats.best, 5));
	p.endl();
	p.put(nullptr, "  ");
	p.put(LABEL, "Games Played:");
	p.put(nullptr, " ");
	p.put(NUMBER, padded(stats.games, 5));
	p.endl();
	p.put(nullptr, "  ");
	p.put(LABEL, "Average Score:");
	p.put(NUMBER, padded(stats.average, 6, 1));
	p.endl();
	p.put(nullptr, std::string(30, ' '));
	p.endl();
	p.put(nullptr, "  ");
	p.put(RESTART, "[R]");
	p.put(nullptr, " Restart  ");
	p.put(LABEL, "[M]");
	p.put(nullptr, " Menu       ");
	p.endl();
	p.put(nullptr, "  ");
	p.put(QUIT, "[Q]");
	p.put(nullptr, " Quit" + std::string(20, ' '));
	p.endl();
	p.put(WALL, rule);
	p.endl();
	out_ << buf.str();
}
