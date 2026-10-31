/**
 * food.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Class Food headers
 */
#ifndef FOOD_HPP
#define FOOD_HPP

#include "snake_game/game/entity.hpp"

/**
 * Represents food that can be eaten by the snake
 *
 * Attributes:
 * (inherits from Entity): x_, y_ coordinates
 */
class Food : public Entity {
public:
	/**
	 * Constructor for the Food class
	 *
	 * Parameters:
	 * int x: The x-coordinate of the food
	 * int y: The y-coordinate of the food
	 */
	Food(int x, int y);

	/**
	 * Destructor for the Food class
	 */
	virtual ~Food() = default;

	/**
	 * Food class concrete implementation of virtual function print
	 */
	void print() const override;
};

#endif
