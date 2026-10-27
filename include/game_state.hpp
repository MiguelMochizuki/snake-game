/**
 * game_state.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Class GameState headers
 */
#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include "snake.hpp"
#include "food.hpp"
#include <vector>
#include <random>
#include <ctime>

/**
 * Represents the complete state of the game
 *
 * Attributes:
 * int rows_: Number of rows in the board
 * int cols_: Number of columns in the board
 * Snake snake_: Snake entity
 * Food food_: Food entity
 * std::vector<bool> occupied_: Flat bitmask for O(1) occupancy checks
 * int score_: Current score
 * int bestScore_: Current best score (loaded from file)
 * std::mt19937 rng_: Random generator for food placement, seeded once
 */
class GameState {
private:
	int rows_;
	int cols_;
	Snake snake_;
	Food food_;
	std::vector<bool> occupied_;
	int score_;
	int bestScore_;
	std::mt19937 rng_;

	/**
	 * Updates the occupancy bitmask based on current snake and food positions
	 */
	void updateOccupied();

public:
	/**
	 * Constructor for the GameState class
	 *
	 * Parameters:
	 * int rows: Number of rows in the board
	 * int cols: Number of columns in the board
	 * int bestScore: Initial best score from ScoreManager
	 * unsigned seed: Seed for the food placement generator (default: current time)
	 */
	GameState(int rows, int cols, int bestScore = 0, unsigned seed = static_cast<unsigned>(std::time(nullptr)));

	/**
	 * Destructor for the GameState class
	 */
	~GameState() = default;

	/**
	 * Gets the number of columns in the board
	 *
	 * Returns int: The number of columns
	 */
	int getCols() const;

	/**
	 * Gets the final score (for saving)
	 *
	 * Returns int: The final score
	 */
	int getFinalScore() const;

	/**
	 * Gets the food entity
	 *
	 * Returns const Food&: Const reference to the food
	 */
	const Food& getFood() const;

	/**
	 * Gets the occupancy grid (for rendering)
	 *
	 * Returns const std::vector<bool>&: Const reference to flat bitmask
	 */
	const std::vector<bool>& getOccupied() const;

	/**
	 * Gets the number of rows in the board
	 *
	 * Returns int: The number of rows
	 */
	int getRows() const;

	/**
	 * Gets the current score
	 *
	 * Returns int: The current score
	 */
	int getScore() const;

	/**
	 * Gets the best score
	 *
	 * Returns int: The best score achieved
	 */
	int getBestScore() const;

	/**
	 * Gets the snake entity
	 *
	 * Returns const Snake&: Const reference to the snake
	 */
	const Snake& getSnake() const;

	/**
	 * Increments the score and updates best score if necessary
	 */
	void incrementScore();

	/**
	 * Checks if a position is occupied (O(1) using bitmask)
	 *
	 * Parameters:
	 * int x: The x-coordinate to check
	 * int y: The y-coordinate to check
	 *
	 * Returns bool: true if occupied or out of bounds
	 */
	bool isOccupied(int x, int y) const;

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
	bool moveSnake(int direction);

	/**
	 * Resets the game state for a new game
	 */
	void reset();

	/**
	 * Sets the best score (used when loading from file)
	 *
	 * Parameters:
	 * int bestScore: The best score to set
	 */
	void setBestScore(int bestScore);

	/**
	 * Spawns food at a random unoccupied position
	 *
	 * Throws std::runtime_error: If the board is full
	 */
	void spawnFood();
};

#endif
