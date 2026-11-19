/**
 * icon_main.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Draws the app icon artwork with the game's own sprites (a snake turning towards an apple) and writes it as a PPM.
 * Needs no SDL; tools/make_icon.py turns the picture into the icon files.
 * Usage: snake_icon OUT.ppm [SIZE]
 */
#include "snake_game/gui/pixel_surface.hpp"
#include "snake_game/gui/ppm.hpp"
#include "snake_game/gui/sprites.hpp"
#include <cstdio>
#include <cstdlib>

/**
 * Draws the artwork and writes it
 *
 * Parameters:
 * int argc: Argument count
 * char** argv: The output file and, optionally, the side of the square picture in pixels (default 1024)
 *
 * Returns int: 0 on success, 1 on bad arguments or a failed write
 */
int main(int argc, char** argv) {
	if (argc < 2) {
		std::fprintf(stderr, "usage: %s OUT.ppm [SIZE]\n", argv[0]);
		return 1;
	}
	const int size = argc > 2 ? std::atoi(argv[2]) : 1024;
	if (size < 64) {
		std::fprintf(stderr, "snake_icon: SIZE must be at least 64\n");
		return 1;
	}

	// A grid of four columns by two rows filling about 80% of the width: the snake comes in from the lower left, turns up
	// and heads for the apple on the right
	const int cell = size * 8 / 10 / 4;
	const int left = (size - 4 * cell) / 2;
	const int top = (size - 2 * cell) / 2;
	auto at = [&](int column, int row) { return SpriteCell{left + column * cell, top + row * cell}; };

	PixelSurface surface(size, size);
	surface.clear(theme::SURFACE);
	const SpritePalette palette{theme::BODY, theme::HEAD, theme::SCALE, theme::GOLD, theme::BACKGROUND,
		theme::FOOD, theme::FOOD, theme::STEM, theme::LEAF, theme::SURFACE};
	drawApple(surface, at(3, 0), cell, palette);
	drawSnake(surface, {at(2, 0), at(1, 0), at(1, 1), at(0, 1)}, cell, palette);

	if (!writePpm(surface.width(), surface.height(), surface.pixelsRgb(), argv[1])) {
		std::fprintf(stderr, "snake_icon: cannot write %s\n", argv[1]);
		return 1;
	}
	return 0;
}
