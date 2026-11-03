/**
 * user_data_dir.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Finds the per-user data folder through SDL, which knows each OS's convention
 */
#include "snake_game/gui/user_data_dir.hpp"

#include <SDL3/SDL.h>

/**
 * Finds, and creates if needed, the per-user data folder of the game
 *
 * Returns std::string: The folder without a trailing separator, or "data" if the OS gives none
 */
std::string userDataDir() {
	char* path = SDL_GetPrefPath(nullptr, "snake-game");
	if (!path) {
		return "data";
	}
	std::string dir = path;
	SDL_free(path);
	// SDL ends the path with a separator; the score files add their own
	while (dir.size() > 1 && (dir.back() == '/' || dir.back() == '\\')) {
		dir.pop_back();
	}
	return dir;
}
