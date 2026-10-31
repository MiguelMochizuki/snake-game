/**
 * entity.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Abstract class Entity implementation
 */
#include "snake_game/game/entity.hpp"

/**
 * Constructor for the Entity class
 *
 * Parameters:
 * int x: The x-coordinate of the entity
 * int y: The y-coordinate of the entity
 */
Entity::Entity(int x, int y) : x_(x), y_(y) {}

/**
 * Gets the x-coordinate of the entity
 *
 * Returns int: The x-coordinate
 */
int Entity::getX() const {
	return x_;
}

/**
 * Gets the y-coordinate of the entity
 *
 * Returns int: The y-coordinate
 */
int Entity::getY() const {
	return y_;
}

/**
 * Sets the x-coordinate of the entity
 *
 * Parameters:
 * int x: The new x-coordinate
 */
void Entity::setX(int x) {
	x_ = x;
}

/**
 * Sets the y-coordinate of the entity
 *
 * Parameters:
 * int y: The new y-coordinate
 */
void Entity::setY(int y) {
	y_ = y;
}
