/**
 * sprites.hpp
 * Author: Miguel Mochizuki Silva
 * Description: The snake and the apple drawn from rectangles and circles, with no SDL and no game types
 */
#ifndef SPRITES_HPP
#define SPRITES_HPP

#include "snake_game/gui/draw_surface.hpp"
#include <vector>

/** Below this cell side in pixels the sprites fall back to plain squares and a circle */
constexpr int MIN_SPRITE_CELL = 8;

/**
 * Where one cell sits on the surface
 *
 * Attributes:
 * int px, int py: Top-left corner of the cell in pixels
 */
struct SpriteCell {
	int px;
	int py;
};

/**
 * The colour of every part of the sprites, already adjusted by the caller (for instance dimmed for a fade)
 *
 * Attributes:
 * Color body, head: Snake body and head
 * Color scale: Dark scale marks on the body
 * Color eye, pupil: Eye and its slit
 * Color tongue: Forked tongue
 * Color apple: Apple body
 * Color stem, leaf: Apple stem and leaf
 * Color notch: The playfield colour, cut into the top of the apple
 */
struct SpritePalette {
	Color body, head, scale, eye, pupil, tongue, apple, stem, leaf, notch;
};

/**
 * Side of the square drawn inside a cell; the gap left is bridged between neighbouring segments
 *
 * Parameters:
 * int cellPx: Cell side in pixels
 *
 * Returns int: The square side, equal to cellPx for cells of up to 3 pixels
 */
int spriteSquare(int cellPx);

/**
 * Draws the snake: rounded body segments joined to their neighbours, and a head with eyes and a tongue facing its direction of travel
 *
 * Parameters:
 * IDrawSurface& surface: Where to draw
 * const std::vector<SpriteCell>& cells: The segments from head to tail, consecutive ones in adjacent cells; the direction comes from the first two, and a lone head faces right
 * int cellPx: Cell side in pixels
 * const SpritePalette& palette: Colours
 */
void drawSnake(IDrawSurface& surface, const std::vector<SpriteCell>& cells, int cellPx, const SpritePalette& palette);

/**
 * Draws an apple with a stem and a leaf, all inside its cell
 *
 * Parameters:
 * IDrawSurface& surface: Where to draw
 * SpriteCell cell: The cell
 * int cellPx: Cell side in pixels
 * const SpritePalette& palette: Colours
 */
void drawApple(IDrawSurface& surface, SpriteCell cell, int cellPx, const SpritePalette& palette);

#endif
