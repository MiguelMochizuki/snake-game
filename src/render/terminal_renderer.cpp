/**
 * terminal_renderer.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Terminal renderer implementation
 */
#include "snake_game/render/terminal_renderer.hpp"
#include <chrono>
#include <thread>
#include "snake_game/terminal/format.hpp"
#include "snake_game/terminal/painter.hpp"
#include "snake_game/input/wasd_input_handler.hpp"
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <algorithm>
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
				p.put(PLAIN, " ");
			} else if (x == headX && y == headY) {
				// Checked before the food: on a full board the old food cell is under the head
				p.put(HEAD, "@");
			} else if (x == foodX && y == foodY) {
				p.put(FOOD, "*");
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
	p.put(PLAIN, " ");
	p.put(NUMBER, std::to_string(state.getScore()));
	p.endl();
	p.put(LABEL, "Best:");
	p.put(PLAIN, " ");
	p.put(NUMBER, std::to_string(state.getBestScore()));
	p.endl();
	p.put(DIM, "Controls: WASD to move, P pause, Q end game");
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
	printPanel("GAME OVER!", FOOD, 9, level, finalScore, stats);
}

/**
 * Prints the victory panel with statistics
 *
 * Parameters:
 * Level level: The level that was played
 * int finalScore: The final score
 * const ScoreStats& stats: Best, games played and average of that level
 */
void TerminalRenderer::printVictory(Level level, int finalScore, const ScoreStats& stats) {
	printPanel("YOU WIN!", GOLD, 11, level, finalScore, stats);
}

void TerminalRenderer::printPanel(const std::string& title, Style titleStyle, size_t leftPad, Level level, int finalScore, const ScoreStats& stats) {
	std::ostringstream buf;
	Painter p(buf, color_);
	const std::string rule(30, '=');
	buf << "\n";
	p.put(WALL, rule);
	p.endl();
	// A title wider than the box is cut, so the line never grows past 30 columns
	const size_t pad = std::min<size_t>(leftPad, 30);
	const std::string shown = title.substr(0, 30 - pad);
	p.put(PLAIN, std::string(pad, ' '));
	p.put(titleStyle, shown);
	p.put(PLAIN, std::string(30 - pad - shown.size(), ' '));
	p.endl();
	p.put(PLAIN, std::string(30, ' '));
	p.endl();
	const std::string name = levelName(level);
	p.put(PLAIN, "  ");
	p.put(LABEL, "Level:");
	p.put(PLAIN, std::string(13 - name.size(), ' '));
	p.put(NUMBER, name);
	p.endl();
	p.put(PLAIN, "  ");
	p.put(LABEL, "Final Score:");
	p.put(PLAIN, "  ");
	p.put(NUMBER, padded(finalScore, 5));
	p.endl();
	p.put(PLAIN, "  ");
	p.put(LABEL, "Best Score:");
	p.put(PLAIN, "   ");
	p.put(NUMBER, padded(stats.best, 5));
	p.endl();
	p.put(PLAIN, "  ");
	p.put(LABEL, "Games Played:");
	p.put(PLAIN, " ");
	p.put(NUMBER, padded(stats.games, 5));
	p.endl();
	p.put(PLAIN, "  ");
	p.put(LABEL, "Average Score:");
	p.put(NUMBER, padded(stats.average, 6, 1));
	p.endl();
	p.put(PLAIN, std::string(30, ' '));
	p.endl();
	if (!stats.unsavedDir.empty()) {
		p.put(PLAIN, "  ");
		p.put(FOOD, "Scores not saved");
		p.put(PLAIN, std::string(12, ' '));
		p.endl();
	}
	p.put(PLAIN, "  ");
	p.put(RESTART, "[R]");
	p.put(PLAIN, " Restart  ");
	p.put(LABEL, "[M]");
	p.put(PLAIN, " Menu       ");
	p.endl();
	p.put(PLAIN, "  ");
	p.put(QUIT, "[Q]");
	p.put(PLAIN, " Quit" + std::string(20, ' '));
	p.endl();
	p.put(WALL, rule);
	p.endl();
	out_ << buf.str();
}

/**
 * Sleeps between game steps
 *
 * Parameters:
 * int ms: Time to sleep in milliseconds
 */
void TerminalRenderer::pause(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void TerminalRenderer::printPaused() {
	std::ostringstream buf;
	Painter p(buf, color_);
	const std::string rule(30, '=');
	buf << "\033[2J\033[H";
	p.put(WALL, rule);
	p.endl();
	p.put(PLAIN, std::string(12, ' '));
	p.put(GOLD, "PAUSED");
	p.put(PLAIN, std::string(12, ' '));
	p.endl();
	p.put(PLAIN, std::string(30, ' '));
	p.endl();
	p.put(PLAIN, "  ");
	p.put(RESTART, "[P]");
	p.put(PLAIN, " Resume" + std::string(18, ' '));
	p.endl();
	p.put(PLAIN, "  ");
	p.put(QUIT, "[Q]");
	p.put(PLAIN, " End game" + std::string(16, ' '));
	p.endl();
	p.put(WALL, rule);
	p.endl();
	out_ << buf.str();
}
