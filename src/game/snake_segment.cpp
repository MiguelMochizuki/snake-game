/**
 * snake_segment.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Class SnakeSegment implementation
 */
#include "snake_game/game/snake_segment.hpp"
#include <iostream>

/**
 * Constructor for the SnakeSegment class
 *
 * Parameters:
 * int x: The x-coordinate of the segment
 * int y: The y-coordinate of the segment
 */
SnakeSegment::SnakeSegment(int x, int y) : Entity(x, y) {}

/**
 * SnakeSegment class concrete implementation of virtual function print
 */
void SnakeSegment::print() const {
	std::cout << "O";
}
