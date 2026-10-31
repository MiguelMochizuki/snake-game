/**
 * entity.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Abstract base class for all game entities
 */
#ifndef ENTITY_HPP
#define ENTITY_HPP

/**
 * Abstract base class providing common entity functionality
 *
 * Attributes:
 * int x_: X-coordinate of the entity
 * int y_: Y-coordinate of the entity
 */
class Entity {
private:
	int x_;
	int y_;

public:
	/**
	 * Constructor for the Entity class
	 *
	 * Parameters:
	 * int x: The x-coordinate of the entity
	 * int y: The y-coordinate of the entity
	 */
	Entity(int x, int y);

	/**
	 * Destructor for the Entity class
	 */
	virtual ~Entity() = default;

	/**
	 * Gets the x-coordinate of the entity
	 *
	 * Returns int: The x-coordinate
	 */
	int getX() const;

	/**
	 * Gets the y-coordinate of the entity
	 *
	 * Returns int: The y-coordinate
	 */
	int getY() const;

	/**
	 * Virtual function for printing the entity's representation
	 */
	virtual void print() const = 0;

	/**
	 * Sets the x-coordinate of the entity
	 *
	 * Parameters:
	 * int x: The new x-coordinate
	 */
	void setX(int x);

	/**
	 * Sets the y-coordinate of the entity
	 *
	 * Parameters:
	 * int y: The new y-coordinate
	 */
	void setY(int y);
};

#endif
