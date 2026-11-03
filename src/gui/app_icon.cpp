/**
 * app_icon.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Builds the window icon surface from the embedded pixels
 */
#include "snake_game/gui/app_icon.hpp"

/**
 * Builds an SDL surface holding a copy of the icon
 *
 * Returns SDL_Surface*: The surface, to be freed with SDL_DestroySurface, or nullptr on failure
 */
SDL_Surface* makeIconSurface() {
	// The surface made from the array borrows its memory, so hand back a copy that owns its pixels
	SDL_Surface* borrowed = SDL_CreateSurfaceFrom(ICON_SIZE, ICON_SIZE, SDL_PIXELFORMAT_RGBA32, const_cast<unsigned char*>(ICON_RGBA), ICON_SIZE * 4);
	if (!borrowed) {
		return nullptr;
	}
	SDL_Surface* copy = SDL_DuplicateSurface(borrowed);
	SDL_DestroySurface(borrowed);
	return copy;
}
