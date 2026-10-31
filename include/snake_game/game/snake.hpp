/**
 * snake.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Class Snake headers
 */
#ifndef SNAKE_HPP
#define SNAKE_HPP

#include "snake_game/game/snake_segment.hpp"
#include <deque>

/**
 * Represents the snake as a collection of segments
 *
 * Attributes:
 * std::deque<SnakeSegment> segments_: Snake body segments (head at front)
 * bool shouldGrow_: Flag indicating if snake should grow on next move
 */
class Snake {
private:
	std::deque<SnakeSegment> segments_;
	bool shouldGrow_;

public:
	/**
	 * Constructor for the Snake class
	 *
	 * Parameters:
	 * int x: The x-coordinate of the snake's head
	 * int y: The y-coordinate of the snake's head
	 * int initialSize: The initial size of the snake (default: 3)
	 */
	Snake(int x, int y, int initialSize = 3);

	/**
	 * Destructor for the Snake class
	 */
	~Snake() = default;

	/**
	 * Gets the head segment of the snake
	 *
	 * Returns const SnakeSegment*: Pointer to the head segment, or nullptr if empty
	 */
	const SnakeSegment* getHead() const;

	/**
	 * Gets the current length of the snake
	 *
	 * Returns size_t: The number of segments in the snake
	 */
	size_t getLength() const;

	/**
	 * Gets all segments of the snake
	 *
	 * Returns const std::deque<SnakeSegment>&: Const reference to segments
	 */
	const std::deque<SnakeSegment>& getSegments() const;

	/**
	 * Checks if the snake has a segment at a given position
	 *
	 * Parameters:
	 * int x: The x-coordinate to check
	 * int y: The y-coordinate to check
	 *
	 * Returns bool: true if a segment exists at the given position
	 */
	bool hasPosition(int x, int y) const;

	/**
	 * Moves the snake to a new position
	 *
	 * Parameters:
	 * int newX: The new x-coordinate of the snake's head
	 * int newY: The new y-coordinate of the snake's head
	 * bool ateFood: Whether the snake has eaten food and should grow
	 */
	void move(int newX, int newY, bool ateFood);

	/**
	 * Sets the snake to grow on the next move
	 */
	void setGrow();
};

#endif
