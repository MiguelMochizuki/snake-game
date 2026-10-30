/**
 * main.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Main entry point
 */
#include "arrow_input_handler.hpp"
#include "command_input_handler.hpp"
#include "composite_input_handler.hpp"
#include "game_engine.hpp"
#include "menu_controller.hpp"
#include "score_manager.hpp"
#include "terminal_menu_view.hpp"
#include "terminal_key_source.hpp"
#include "wasd_input_handler.hpp"
#include "terminal_renderer.hpp"
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <optional>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

/**
 * Reads the frame duration from SNAKE_FRAME_MS
 *
 * Returns std::optional<int>: The parsed value if the whole string is an integer in [0, 10000], otherwise nullopt
 */
static std::optional<int> readFrameMs() {
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
static bool skipMenu() {
	const char* env = std::getenv("SNAKE_SKIP_MENU");
	return env && *env;
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
#ifdef _WIN32
	// Enable ANSI escape processing; failure is ignored (output stays usable)
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD mode = 0;
	if (console != INVALID_HANDLE_VALUE && GetConsoleMode(console, &mode)) {
		SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}
#endif
	TerminalKeySource keys;
	CompositeInputHandler input;
	input.add(std::make_unique<WASDInputHandler>());
	input.add(std::make_unique<ArrowInputHandler>());
	input.add(std::make_unique<CommandInputHandler>());
	const bool color = TerminalRenderer::colorEnabledFromEnv();
	GameEngine engine(keys, input, 20, 20, 200, readSeed(), color);
	MenuModel menu;
	bool showMenu = !skipMenu();
	do {
		int speedMs = 200;
		if (showMenu) {
			// Read the scores afresh: the round that just ended may have changed them
			ScoreManager scores;
			TerminalMenuView view(std::cout, color);
			MenuResult choice = runMenu(keys, input, view, menu,
				MenuStats{scores.bestScore(Level::Normal), scores.totalGames(Level::Normal), scores.averageScore(Level::Normal)});
			if (!choice.start) {
				return 0;
			}
			speedMs = choice.speedMs;
		}
		// A valid SNAKE_FRAME_MS (test hook) overrides the chosen level
		engine.setSpeed(readFrameMs().value_or(speedMs));
		showMenu = true;
	} while (engine.run() == GameEnd::Menu);
	return 0;
}
