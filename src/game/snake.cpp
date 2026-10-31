/**
 * snake.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Class Snake implementation
 */
#include "snake_game/game/snake.hpp"

/**
 * Constructor for the Snake class
 *
 * Parameters:
 * int x: The x-coordinate of the snake's head
 * int y: The y-coordinate of the snake's head
 * int initialSize: The initial size of the snake (default: 3)
 */
Snake::Snake(int x, int y, int initialSize) : shouldGrow_(false) {
	for (int i = 0; i < initialSize; ++i) {
		segments_.emplace_back(x - i, y);
	}
}

/**
 * Gets the head segment of the snake
 *
 * Returns const SnakeSegment*: Pointer to the head segment, or nullptr if empty
 */
const SnakeSegment* Snake::getHead() const {
	if (segments_.empty()) {
		return nullptr;
	}
	return &segments_.front();
}

/**
 * Gets the current length of the snake
 *
 * Returns size_t: The number of segments in the snake
 */
size_t Snake::getLength() const {
	return segments_.size();
}

/**
 * Gets all segments of the snake
 *
 * Returns const std::deque<SnakeSegment>&: Const reference to segments
 */
const std::deque<SnakeSegment>& Snake::getSegments() const {
	return segments_;
}

/**
 * Checks if the snake has a segment at a given position
 *
 * Parameters:
 * int x: The x-coordinate to check
 * int y: The y-coordinate to check
 *
 * Returns bool: true if a segment exists at the given position
 */
bool Snake::hasPosition(int x, int y) const {
	for (const auto& segment : segments_) {
		if (segment.getX() == x && segment.getY() == y) {
			return true;
		}
	}
	return false;
}

/**
 * Moves the snake to a new position
 *
 * Parameters:
 * int newX: The new x-coordinate of the snake's head
 * int newY: The new y-coordinate of the snake's head
 * bool ateFood: Whether the snake has eaten food and should grow
 */
void Snake::move(int newX, int newY, bool ateFood) {
	if (ateFood) {
		setGrow();
	}

	segments_.emplace_front(newX, newY);

	if (!shouldGrow_) {
		segments_.pop_back();
	} else {
		shouldGrow_ = false;
	}
}

/**
 * Sets the snake to grow on the next move
 */
void Snake::setGrow() {
	shouldGrow_ = true;
}
