/**
 * main.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Main entry point
 */
#include "game_engine.hpp"
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <ctime>

/**
 * Reads the frame duration from SNAKE_FRAME_MS
 *
 * Returns int: The parsed value if the whole string is an integer in [0, 10000], otherwise 200
 */
static int readFrameMs() {
	const char* env = std::getenv("SNAKE_FRAME_MS");
	if (!env || !std::isdigit(static_cast<unsigned char>(*env))) {
		return 200;
	}
	errno = 0;
	char* end = nullptr;
	long value = std::strtol(env, &end, 10);
	if (errno != 0 || *end != '\0' || value < 0 || value > 10000) {
		return 200;
	}
	return static_cast<int>(value);
}

/**
 * Reads the food placement seed from SNAKE_SEED
 *
 * Returns unsigned: The parsed value if the whole string is an unsigned 32-bit integer, otherwise the current time
 */
static unsigned readSeed() {
	unsigned fallback = static_cast<unsigned>(std::time(nullptr));
	const char* env = std::getenv("SNAKE_SEED");
	if (!env || !std::isdigit(static_cast<unsigned char>(*env))) {
		return fallback;
	}
	errno = 0;
	char* end = nullptr;
	unsigned long value = std::strtoul(env, &end, 10);
	if (errno != 0 || *end != '\0' || value > UINT32_MAX) {
		return fallback;
	}
	return static_cast<unsigned>(value);
}

/**
 * Program entry point
 *
 * Returns int: Exit status
 */
int main() {
	GameEngine engine(20, 20, readFrameMs(), readSeed());
	engine.run();
	return 0;
}
