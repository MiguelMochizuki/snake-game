/**
 * bitmap_font.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Embedded 8x8 bitmap font for the graphical front-end
 */
#ifndef BITMAP_FONT_HPP
#define BITMAP_FONT_HPP

#include "snake_game/gui/canvas.hpp"
#include <string>

namespace font {

/** Side of one glyph in pixels */
constexpr int GLYPH = 8;

/**
 * Width of a text
 *
 * Parameters:
 * const std::string& text: The text
 * int scale: Pixels per glyph pixel
 *
 * Returns int: Width in pixels
 */
int textWidth(const std::string& text, int scale);

/**
 * Height of a line of text
 *
 * Parameters:
 * int scale: Pixels per glyph pixel
 *
 * Returns int: Height in pixels
 */
int textHeight(int scale);

/**
 * Tells whether one pixel of a glyph is lit
 *
 * Parameters:
 * char c: Character, ASCII 32 to 126; anything else is a filled box
 * int col: Column, 0 to 7
 * int row: Row, 0 to 7
 *
 * Returns bool: true if the pixel is lit
 */
bool glyphPixel(char c, int col, int row);

/**
 * Draws a text
 *
 * Parameters:
 * Canvas& canvas: Where to draw
 * int x, int y: Top-left corner
 * const std::string& text: The text
 * int scale: Pixels per glyph pixel, at least 1
 * Color color: Text colour
 */
void drawText(Canvas& canvas, int x, int y, const std::string& text, int scale, Color color);

}  // namespace font

#endif
