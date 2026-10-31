/**
 * env_options.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Test and demo hooks read from the environment, shared by the terminal and graphical executables
 */
#ifndef ENV_OPTIONS_HPP
#define ENV_OPTIONS_HPP

#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <optional>

/**
 * Reads the frame duration from SNAKE_FRAME_MS
 *
 * Returns std::optional<int>: The parsed value if the whole string is an integer in [0, 10000], otherwise nullopt
 */
inline std::optional<int> readFrameMs() {
	const char* env = std::getenv("SNAKE_FRAME_MS");
	if (!env || !std::isdigit(static_cast<unsigned char>(*env))) {
		return std::nullopt;
	}
	errno = 0;
	char* end = nullptr;
	long value = std::strtol(env, &end, 10);
	if (errno != 0 || *end != '\0' || value < 0 || value > 10000) {
		return std::nullopt;
	}
	return static_cast<int>(value);
}

/**
 * Tells whether the start menu is skipped, from SNAKE_SKIP_MENU
 *
 * Returns bool: true if SNAKE_SKIP_MENU is set and non-empty
 */
inline bool skipMenu() {
	const char* env = std::getenv("SNAKE_SKIP_MENU");
	return env && *env;
}

/**
 * Reads the food placement seed from SNAKE_SEED
 *
 * Returns unsigned: The parsed value if the whole string is an unsigned 32-bit integer, otherwise the current time
 */
inline unsigned readSeed() {
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

#endif
