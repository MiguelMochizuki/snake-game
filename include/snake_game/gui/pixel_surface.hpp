/**
 * pixel_surface.hpp
 * Author: Miguel Mochizuki Silva
 * Description: In-memory drawing surface, so sprites and the icon can be drawn and tested without SDL
 */
#ifndef PIXEL_SURFACE_HPP
#define PIXEL_SURFACE_HPP

#include "snake_game/gui/draw_surface.hpp"
#include <vector>

/**
 * Draws filled rectangles and circles on a plain RGB buffer
 *
 * Attributes:
 * int width_, int height_: Size in pixels
 * std::vector<unsigned char> rgb_: Row-major RGB bytes, width x height x 3
 */
class PixelSurface : public IDrawSurface {
	int width_;
	int height_;
	std::vector<unsigned char> rgb_;

public:
	/**
	 * Constructor for the PixelSurface class
	 *
	 * Parameters:
	 * int width: Width in pixels
	 * int height: Height in pixels
	 */
	PixelSurface(int width, int height);

	int width() const override;
	int height() const override;
	void clear(Color color) override;
	void fillRect(int x, int y, int w, int h, Color color) override;
	void present() override;

	/**
	 * Returns const std::vector<unsigned char>&: Row-major RGB bytes, width x height x 3
	 */
	const std::vector<unsigned char>& pixelsRgb() const;
};

#endif
