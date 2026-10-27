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

/**
 * Constructor for the TerminalRenderer class
 *
 * Parameters:
 * std::ostream& out: Stream that receives all output
 */
TerminalRenderer::TerminalRenderer(std::ostream& out) : out_(out) {
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

	std::ostringstream buf;

	for (int x = 0; x < cols + 2; ++x) {
		buf << '#';
	}
	buf << '\n';

	for (int y = 0; y < rows; ++y) {
		buf << '#';
		for (int x = 0; x < cols; ++x) {
			if (grid[y * cols + x]) {
				buf << (x == foodX && y == foodY ? '*' : 'O');
			} else {
				buf << ' ';
			}
		}
		buf << "#\n";
	}

	for (int x = 0; x < cols + 2; ++x) {
		buf << '#';
	}
	buf << '\n';

	buf << "Score: " << state.getScore() << '\n';
	buf << "Best: " << state.getBestScore() << '\n';
	buf << "Controls: WASD to move, Q to quit\n";

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
	buf << "\n";
	buf << "==============================\n";
	buf << "         GAME OVER!           \n";
	buf << "                              \n";
	buf << "  Final Score:  " << std::setw(5) << finalScore << "\n";
	buf << "  Best Score:   " << std::setw(5) << bestScore << "\n";
	buf << "  Games Played: " << std::setw(5) << totalGames << "\n";
	buf << "  Average Score:" << std::setw(6) << std::fixed << std::setprecision(1) << averageScore << "\n";
	buf << "                              \n";
	buf << "  [R] Restart  [Q] Quit       \n";
	buf << "==============================\n";
	out_ << buf.str();
}

/**
 * Shows game over screen with statistics
 *
 * Parameters:
 * int finalScore: The final score
 * int bestScore: The best score
 * size_t totalGames: Total number of games played
 * double averageScore: Average score
 * WASDInputHandler& inputHandler: Reference to the input handler
 *
 * Returns bool: true if restart was requested (R key)
 */
bool TerminalRenderer::showGameOver(int finalScore, int bestScore, size_t totalGames,
                                   double averageScore, WASDInputHandler& inputHandler) {
	printGameOver(finalScore, bestScore, totalGames, averageScore);

	while (true) {
		char choice = inputHandler.readCharBlocking();
		if (choice == 'r' || choice == 'R') {
			return true;
		} else if (choice == 'q' || choice == 'Q') {
			return false;
		}
	}
}
