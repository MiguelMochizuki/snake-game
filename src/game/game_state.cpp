/**
 * game_state.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Class GameState implementation
 */
#include "snake_game/game/game_state.hpp"
#include <stdexcept>
#include <algorithm>

/**
 * Constructor for the GameState class
 *
 * Parameters:
 * int rows: Number of rows in the board
 * int cols: Number of columns in the board
 * int bestScore: Initial best score from ScoreManager
 * unsigned seed: Seed for the food placement generator
 */
GameState::GameState(int rows, int cols, int bestScore, unsigned seed)
	: rows_(rows)
	, cols_(cols)
	, snake_(cols / 2, rows / 2, 3)
	, food_(0, 0)
	, occupied_(rows * cols, false)
	, score_(0)
	, bestScore_(bestScore)
	, rng_(seed) {
	spawnFood();
	updateOccupied();
}

/**
 * Gets the number of columns in the board
 *
 * Returns int: The number of columns
 */
int GameState::getCols() const {
	return cols_;
}

/**
 * Gets the final score (for saving)
 *
 * Returns int: The final score
 */
int GameState::getFinalScore() const {
	return score_;
}

/**
 * Gets the food entity
 *
 * Returns const Food&: Const reference to the food
 */
const Food& GameState::getFood() const {
	return food_;
}

/**
 * Gets the occupancy grid (for rendering)
 *
 * Returns const std::vector<std::vector<bool>>&: Const reference to grid
 */
const std::vector<bool>& GameState::getOccupied() const {
	return occupied_;
}

/**
 * Gets the number of rows in the board
 *
 * Returns int: The number of rows
 */
int GameState::getRows() const {
	return rows_;
}

/**
 * Gets the current score
 *
 * Returns int: The current score
 */
int GameState::getScore() const {
	return score_;
}

/**
 * Gets the best score
 *
 * Returns int: The best score achieved
 */
int GameState::getBestScore() const {
	return bestScore_;
}

/**
 * Gets the snake entity
 *
 * Returns const Snake&: Const reference to the snake
 */
const Snake& GameState::getSnake() const {
	return snake_;
}

/**
 * Increments the score and updates best score if necessary
 */
void GameState::incrementScore() {
	score_++;
	if (score_ > bestScore_) {
		bestScore_ = score_;
	}
}

/**
 * Checks if a position is occupied (O(1) using bitmask)
 *
 * Parameters:
 * int x: The x-coordinate to check
 * int y: The y-coordinate to check
 *
 * Returns bool: true if occupied or out of bounds
 */
bool GameState::isOccupied(int x, int y) const {
	if (x < 0 || x >= cols_ || y < 0 || y >= rows_) {
		return true;
	}
	return occupied_[y * cols_ + x];
}

/**
 * Moves the snake in a given direction
 *
 * Parameters:
 * int direction: The direction to move (0=UP, 1=DOWN, 2=LEFT, 3=RIGHT)
 *
 * Returns bool: true if food was eaten
 *
 * Throws std::runtime_error: If the snake collides with a wall or itself
 */
bool GameState::moveSnake(int direction) {
	const SnakeSegment* head = snake_.getHead();
	if (!head) {
		throw std::runtime_error("Snake has no head!");
	}

	int newX = head->getX();
	int newY = head->getY();

	switch (direction) {
		case 0: newY--; break;
		case 1: newY++; break;
		case 2: newX--; break;
		case 3: newX++; break;
		default: break;
	}

	if (newX < 0 || newX >= cols_ || newY < 0 || newY >= rows_) {
		throw std::runtime_error("Game Over: Wall collision!");
	}

	bool ateFood = (newX == food_.getX() && newY == food_.getY());

	if (!ateFood) {
		const auto& segments = snake_.getSegments();
		size_t checkUntil = segments.size() - 1;
		for (size_t i = 0; i < checkUntil; ++i) {
			if (segments[i].getX() == newX && segments[i].getY() == newY) {
				throw std::runtime_error("Game Over: Self collision!");
			}
		}
	}

	snake_.move(newX, newY, ateFood);

	updateOccupied();

	if (ateFood) {
		spawnFood();
		incrementScore();
		updateOccupied();
	}

	return ateFood;
}

/**
 * Resets the game state for a new game
 */
void GameState::reset() {
	snake_ = Snake(cols_ / 2, rows_ / 2, 3);
	score_ = 0;
	spawnFood();
	updateOccupied();
}

/**
 * Sets the best score (used when loading from file)
 *
 * Parameters:
 * int bestScore: The best score to set
 */
void GameState::setBestScore(int bestScore) {
	bestScore_ = bestScore;
}

/**
 * Spawns food at a random unoccupied position
 *
 * Throws std::runtime_error: If the board is full
 */
void GameState::spawnFood() {
	for (int attempt = 0; attempt < 100; ++attempt) {
		int x = static_cast<int>(rng_() % static_cast<unsigned>(cols_));
		int y = static_cast<int>(rng_() % static_cast<unsigned>(rows_));

		if (!isOccupied(x, y)) {
			food_.setX(x);
			food_.setY(y);
			return;
		}
	}

	for (int y = 0; y < rows_; ++y) {
		for (int x = 0; x < cols_; ++x) {
			if (!isOccupied(x, y)) {
				food_.setX(x);
				food_.setY(y);
				return;
			}
		}
	}

	throw std::runtime_error("Board is full!");
}

/**
 * Updates the occupancy bitmask based on current snake and food positions
 */
void GameState::updateOccupied() {
	std::fill(occupied_.begin(), occupied_.end(), false);

	for (const auto& segment : snake_.getSegments()) {
		occupied_[segment.getY() * cols_ + segment.getX()] = true;
	}

	occupied_[food_.getY() * cols_ + food_.getX()] = true;
}
