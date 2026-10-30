/**
 * level.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Difficulty levels and their helpers
 */
#ifndef LEVEL_HPP
#define LEVEL_HPP

#include <optional>

/**
 * Difficulty level, from slowest to fastest
 */
enum class Level { Easy, Normal, Hard };

/**
 * Gets the display name of a level
 *
 * Parameters:
 * Level level: The level
 *
 * Returns const char*: "Easy", "Normal" or "Hard"
 */
const char* levelName(Level level);

/**
 * Gets the frame time of a level
 *
 * Parameters:
 * Level level: The level
 *
 * Returns int: 300 for Easy, 200 for Normal, 100 for Hard (milliseconds per frame)
 */
int levelSpeedMs(Level level);

/**
 * Moves to a neighbouring level, wrapping around
 *
 * Parameters:
 * Level level: The starting level
 * int step: +1 for the next level (Easy, Normal, Hard, Easy...), -1 for the previous one
 *
 * Returns Level: The level reached
 */
Level nextLevel(Level level, int step);

/**
 * Gets the one-letter code a level is stored under
 *
 * Parameters:
 * Level level: The level
 *
 * Returns char: 'E', 'N' or 'H'
 */
char levelCode(Level level);

/**
 * Reads a level from its one-letter code
 *
 * Parameters:
 * char code: 'E', 'N' or 'H'
 *
 * Returns std::optional<Level>: The level, or nullopt for any other character
 */
std::optional<Level> levelFromCode(char code);

#endif
