/**
 * food.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Class Food implementation
 */
#include "snake_game/game/food.hpp"
#include <iostream>

/**
 * Constructor for the Food class
 *
 * Parameters:
 * int x: The x-coordinate of the food
 * int y: The y-coordinate of the food
 */
Food::Food(int x, int y) : Entity(x, y) {}

/**
 * Food class concrete implementation of virtual function print
 */
void Food::print() const {
	std::cout << "*";
}
