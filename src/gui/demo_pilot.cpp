/**
 * demo_pilot.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Demo autopilot implementation
 */
#include "snake_game/gui/demo_pilot.hpp"
#include <array>
#include <queue>

namespace {

struct Step {
	Direction dir;
	int dx;
	int dy;
};

const std::array<Step, 4> STEPS = {{{Direction::UP, 0, -1}, {Direction::DOWN, 0, 1}, {Direction::LEFT, -1, 0}, {Direction::RIGHT, 1, 0}}};

/**
 * Tells whether a cell is on the board and not body
 *
 * Parameters:
 * const DemoBoard& b: The board
 * int x, int y: The cell
 *
 * Returns bool: true if the snake could move there
 */
bool freeCell(const DemoBoard& b, int x, int y) {
	return x >= 0 && x < b.cols && y >= 0 && y < b.rows && !b.blocked[static_cast<size_t>(y) * b.cols + x];
}

/**
 * Counts the free cells reachable from a cell, the cell included
 *
 * Parameters:
 * const DemoBoard& b: The board
 * int x, int y: The start cell, which must be free
 *
 * Returns int: Number of reachable free cells
 */
int reachable(const DemoBoard& b, int x, int y) {
	std::vector<bool> seen(b.blocked.size(), false);
	std::queue<std::pair<int, int>> todo;
	todo.push({x, y});
	seen[static_cast<size_t>(y) * b.cols + x] = true;
	int count = 0;
	while (!todo.empty()) {
		auto [cx, cy] = todo.front();
		todo.pop();
		++count;
		for (const Step& s : STEPS) {
			const int nx = cx + s.dx, ny = cy + s.dy;
			if (freeCell(b, nx, ny) && !seen[static_cast<size_t>(ny) * b.cols + nx]) {
				seen[static_cast<size_t>(ny) * b.cols + nx] = true;
				todo.push({nx, ny});
			}
		}
	}
	return count;
}

}  // namespace

std::optional<Direction> demoMoveOn(const DemoBoard& board) {
	// Breadth-first search from the head; first[i] is the first move of the path that reached cell i
	std::vector<int> first(board.blocked.size(), -1);
	std::queue<std::pair<int, int>> todo;
	for (size_t i = 0; i < STEPS.size(); ++i) {
		const int nx = board.headX + STEPS[i].dx, ny = board.headY + STEPS[i].dy;
		if (freeCell(board, nx, ny) && first[static_cast<size_t>(ny) * board.cols + nx] < 0) {
			first[static_cast<size_t>(ny) * board.cols + nx] = static_cast<int>(i);
			todo.push({nx, ny});
		}
	}
	while (!todo.empty()) {
		auto [cx, cy] = todo.front();
		todo.pop();
		if (cx == board.foodX && cy == board.foodY) {
			return STEPS[first[static_cast<size_t>(cy) * board.cols + cx]].dir;
		}
		for (const Step& s : STEPS) {
			const int nx = cx + s.dx, ny = cy + s.dy;
			const size_t idx = static_cast<size_t>(ny) * board.cols + nx;
			if (freeCell(board, nx, ny) && first[idx] < 0 && !(nx == board.headX && ny == board.headY)) {
				first[idx] = first[static_cast<size_t>(cy) * board.cols + cx];
				todo.push({nx, ny});
			}
		}
	}
	// Food unreachable: keep to the roomiest side
	std::optional<Direction> best;
	int bestRoom = -1;
	for (const Step& s : STEPS) {
		const int nx = board.headX + s.dx, ny = board.headY + s.dy;
		if (freeCell(board, nx, ny)) {
			const int room = reachable(board, nx, ny);
			if (room > bestRoom) {
				bestRoom = room;
				best = s.dir;
			}
		}
	}
	return best;
}

std::optional<Direction> demoMove(const GameState& state) {
	DemoBoard board{state.getRows(), state.getCols(), std::vector<bool>(static_cast<size_t>(state.getRows()) * state.getCols(), false), 0, 0, state.getFood().getX(), state.getFood().getY()};
	const std::vector<bool>& occupied = state.getOccupied();
	for (size_t i = 0; i < occupied.size(); ++i) {
		board.blocked[i] = occupied[i];
	}
	// The grid marks the food cell as occupied too; the pilot must be able to step onto it
	board.blocked[static_cast<size_t>(board.foodY) * board.cols + board.foodX] = false;
	const SnakeSegment* head = state.getSnake().getHead();
	board.headX = head->getX();
	board.headY = head->getY();
	return demoMoveOn(board);
}

/**
 * Constructor for the DemoKeys class
 *
 * Parameters:
 * const SdlRenderer& renderer: Renderer of the game being played
 * SdlApp& app: Event pump
 * std::function<bool()> stop: Returns true when the demo should end
 */
DemoKeys::DemoKeys(const SdlRenderer& renderer, SdlApp& app, std::function<bool()> stop) : renderer_(renderer), app_(app), stop_(std::move(stop)) {
}

/**
 * Returns std::vector<Key>: Keys from the window (a close is a Q), then the autopilot's move or a Q when the demo should end
 */
std::vector<Key> DemoKeys::poll() {
	app_.pump(0);
	std::vector<Key> keys(app_.keys().begin(), app_.keys().end());
	app_.keys().clear();
	if (stop_ && stop_()) {
		keys.push_back(charKey('q'));
		return keys;
	}
	if (std::optional<Direction> move = demoMove(renderer_.lastState())) {
		switch (*move) {
			case Direction::UP: keys.push_back(specialKey(KeyKind::Up)); break;
			case Direction::DOWN: keys.push_back(specialKey(KeyKind::Down)); break;
			case Direction::LEFT: keys.push_back(specialKey(KeyKind::Left)); break;
			case Direction::RIGHT: keys.push_back(specialKey(KeyKind::Right)); break;
		}
	}
	return keys;
}

/**
 * Returns std::optional<Key>: Q once at the game over panel, then nullopt so a demo run ends
 */
std::optional<Key> DemoKeys::waitKey() {
	if (quitSent_) {
		return std::nullopt;
	}
	quitSent_ = true;
	return charKey('q');
}

/**
 * Does nothing; the autopilot has no stale keys
 */
void DemoKeys::flush() {
}
