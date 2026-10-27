/**
 * terminal_renderer.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Terminal renderer implementation
 */
#include "terminal_renderer.hpp"
#include "wasd_input_handler.hpp"
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <sstream>

namespace {

const char* const WALL = "\033[36m";
const char* const BODY = "\033[32m";
const char* const HEAD = "\033[1;92m";
const char* const FOOD = "\033[1;91m";
const char* const LABEL = "\033[1m";
const char* const NUMBER = "\033[1;33m";
const char* const DIM = "\033[2m";
const char* const RESTART = "\033[92m";
const char* const QUIT = "\033[91m";
const char* const RESET = "\033[0m";

/**
 * Writes text into a stream as runs of same-styled characters
 *
 * Attributes:
 * std::ostream& buf: Destination stream
 * bool color: Whether styles are emitted at all
 * const char* cur_: Style currently open (nullptr when plain)
 */
class Painter {
	std::ostream& buf_;
	bool color_;
	const char* cur_ = nullptr;

public:
	/**
	 * Parameters:
	 * std::ostream& buf: Destination stream
	 * bool color: Whether styles are emitted at all
	 */
	Painter(std::ostream& buf, bool color) : buf_(buf), color_(color) {
	}

	/**
	 * Appends text in a style, opening or closing SGR runs only when the style changes
	 *
	 * Parameters:
	 * const char* style: SGR sequence, or nullptr for plain text
	 * const std::string& text: Text to append
	 *
	 * Returns void:
	 */
	void put(const char* style, const std::string& text) {
		if (color_ && style != cur_) {
			if (cur_) {
				buf_ << RESET;
			}
			if (style) {
				buf_ << style;
			}
			cur_ = style;
		}
		buf_ << text;
	}

	/**
	 * Ends the line, closing any open style first
	 *
	 * Returns void:
	 */
	void endl() {
		if (cur_) {
			buf_ << RESET;
			cur_ = nullptr;
		}
		buf_ << '\n';
	}
};

/**
 * Formats a number right-aligned in a field
 *
 * Parameters:
 * T value: The number
 * int width: Field width
 * int precision: Digits after the point for floating values (-1 for integers)
 *
 * Returns std::string: The padded text
 */
template <typename T>
std::string padded(T value, int width, int precision = -1) {
	std::ostringstream s;
	s << std::setw(width);
	if (precision >= 0) {
		s << std::fixed << std::setprecision(precision);
	}
	s << value;
	return s.str();
}

}  // namespace

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
 * int finalScore: The final score
 * int bestScore: The best score
 * size_t totalGames: Total number of games played
 * double averageScore: Average score
 */
void TerminalRenderer::printGameOver(int finalScore, int bestScore, size_t totalGames, double averageScore) {
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
	p.put(nullptr, "  ");
	p.put(LABEL, "Final Score:");
	p.put(nullptr, "  ");
	p.put(NUMBER, padded(finalScore, 5));
	p.endl();
	p.put(nullptr, "  ");
	p.put(LABEL, "Best Score:");
	p.put(nullptr, "   ");
	p.put(NUMBER, padded(bestScore, 5));
	p.endl();
	p.put(nullptr, "  ");
	p.put(LABEL, "Games Played:");
	p.put(nullptr, " ");
	p.put(NUMBER, padded(totalGames, 5));
	p.endl();
	p.put(nullptr, "  ");
	p.put(LABEL, "Average Score:");
	p.put(NUMBER, padded(averageScore, 6, 1));
	p.endl();
	p.put(nullptr, std::string(30, ' '));
	p.endl();
	p.put(nullptr, "  ");
	p.put(RESTART, "[R]");
	p.put(nullptr, " Restart  ");
	p.put(QUIT, "[Q]");
	p.put(nullptr, " Quit       ");
	p.endl();
	p.put(WALL, rule);
	p.endl();
	out_ << buf.str();
}
