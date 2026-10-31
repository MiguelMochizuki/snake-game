/**
 * snake_segment.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Class SnakeSegment headers
 */
#ifndef SNAKE_SEGMENT_HPP
#define SNAKE_SEGMENT_HPP

#include "snake_game/game/entity.hpp"

/**
 * Represents a single segment of the snake's body
 *
 * Attributes:
 * (inherits from Entity): x_, y_ coordinates
 */
class SnakeSegment : public Entity {
public:
	/**
	 * Constructor for the SnakeSegment class
	 *
	 * Parameters:
	 * int x: The x-coordinate of the segment
	 * int y: The y-coordinate of the segment
	 */
	SnakeSegment(int x, int y);

	/**
	 * Destructor for the SnakeSegment class
	 */
	virtual ~SnakeSegment() = default;

	/**
	 * SnakeSegment class concrete implementation of virtual function print
	 */
	void print() const override;
};

#endif
