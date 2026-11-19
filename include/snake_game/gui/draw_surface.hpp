/**
 * draw_surface.hpp
 * Author: Miguel Mochizuki Silva
 * Description: What the graphical views need from a drawing surface, apart from SDL
 */
#ifndef DRAW_SURFACE_HPP
#define DRAW_SURFACE_HPP

#include "snake_game/gui/theme.hpp"

/**
 * A surface that filled shapes are drawn on; Canvas implements it on SDL
 */
class IDrawSurface {
public:
	/**
	 * Destructor for the IDrawSurface interface
	 */
	virtual ~IDrawSurface() = default;

	/**
	 * Returns int: Current width in pixels (follows a window resize)
	 */
	virtual int width() const = 0;

	/**
	 * Returns int: Current height in pixels (follows a window resize)
	 */
	virtual int height() const = 0;

	/**
	 * Fills the whole surface
	 *
	 * Parameters:
	 * Color color: Fill colour
	 */
	virtual void clear(Color color) = 0;

	/**
	 * Fills a rectangle
	 *
	 * Parameters:
	 * int x, int y: Top-left corner
	 * int w, int h: Size in pixels
	 * Color color: Fill colour
	 */
	virtual void fillRect(int x, int y, int w, int h, Color color) = 0;

	/**
	 * Fills a circle with one horizontal span per row; a surface may override it with something faster
	 *
	 * Parameters:
	 * int cx, int cy: Centre
	 * int radius: Radius in pixels
	 * Color color: Fill colour
	 */
	virtual void fillCircle(int cx, int cy, int radius, Color color) {
		for (int dy = -radius; dy <= radius; ++dy) {
			int dx = 0;
			while ((dx + 1) * (dx + 1) + dy * dy <= radius * radius) {
				++dx;
			}
			fillRect(cx - dx, cy + dy, 2 * dx + 1, 1, color);
		}
	}

	/**
	 * Shows what was drawn; does nothing on a surface that is not displayed
	 */
	virtual void present() = 0;
};

#endif
