/**
 * level.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Difficulty level helpers implementation
 */
#include "level.hpp"

namespace {

const int LEVEL_COUNT = 3;

}  // namespace

/**
 * Gets the display name of a level
 *
 * Parameters:
 * Level level: The level
 *
 * Returns const char*: "Easy", "Normal" or "Hard"
 */
const char* levelName(Level level) {
	switch (level) {
		case Level::Easy: return "Easy";
		case Level::Normal: return "Normal";
		case Level::Hard: return "Hard";
	}
	return "";
}

/**
 * Gets the frame time of a level
 *
 * Parameters:
 * Level level: The level
 *
 * Returns int: 300 for Easy, 200 for Normal, 100 for Hard (milliseconds per frame)
 */
int levelSpeedMs(Level level) {
	switch (level) {
		case Level::Easy: return 300;
		case Level::Normal: return 200;
		case Level::Hard: return 100;
	}
	return 200;
}

/**
 * Moves to a neighbouring level, wrapping around
 *
 * Parameters:
 * Level level: The starting level
 * int step: +1 for the next level, -1 for the previous one
 *
 * Returns Level: The level reached
 */
Level nextLevel(Level level, int step) {
	int index = static_cast<int>(level);
	return static_cast<Level>((index + step + LEVEL_COUNT) % LEVEL_COUNT);
}

/**
 * Gets the one-letter code a level is stored under
 *
 * Parameters:
 * Level level: The level
 *
 * Returns char: 'E', 'N' or 'H'
 */
char levelCode(Level level) {
	switch (level) {
		case Level::Easy: return 'E';
		case Level::Normal: return 'N';
		case Level::Hard: return 'H';
	}
	return '?';
}

/**
 * Reads a level from its one-letter code
 *
 * Parameters:
 * char code: 'E', 'N' or 'H'
 *
 * Returns std::optional<Level>: The level, or nullopt for any other character
 */
std::optional<Level> levelFromCode(char code) {
	switch (code) {
		case 'E': return Level::Easy;
		case 'N': return Level::Normal;
		case 'H': return Level::Hard;
		default: return std::nullopt;
	}
}
