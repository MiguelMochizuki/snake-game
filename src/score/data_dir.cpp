/**
 * data_dir.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Picks the folder where the terminal game keeps its score files
 */
#include "snake_game/score/data_dir.hpp"

#include <cstdlib>
#include <filesystem>

namespace {

/**
 * Reads an environment variable
 *
 * Parameters:
 * const char* name: The variable
 *
 * Returns std::string: Its value, empty if unset
 */
std::string env(const char* name) {
	const char* value = std::getenv(name);
	return value ? value : "";
}

/**
 * Finds the per-user data folder of the OS, the one every app of the user shares
 *
 * Returns std::string: The folder, empty if the environment does not say
 */
std::string osDataBase() {
#ifdef _WIN32
	return env("APPDATA");
#elif defined(__APPLE__)
	const std::string home = env("HOME");
	return home.empty() ? "" : home + "/Library/Application Support";
#else
	const std::string xdg = env("XDG_DATA_HOME");
	if (!xdg.empty()) {
		return xdg;
	}
	const std::string home = env("HOME");
	return home.empty() ? "" : home + "/.local/share";
#endif
}

}  // namespace

std::string defaultDataDir() {
	const std::string chosen = env("SNAKE_DATA_DIR");
	if (!chosen.empty()) {
		return chosen;
	}
	// Scores kept next to where the game was started by earlier versions stay where they are, v1 files included so they still get imported
	std::error_code ec;
	for (const char* name : {"scores_v2.txt", "score_history.txt", "best_score.txt", "score_checksum.txt"}) {
		if (std::filesystem::exists(std::filesystem::path("data") / name, ec)) {
			return "data";
		}
	}
	const std::string base = osDataBase();
	return base.empty() ? "data" : base + "/snake-game-terminal";
}
