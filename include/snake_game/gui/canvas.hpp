/**
 * canvas.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Drawing surface of the graphical front-end: an SDL window or an off-screen buffer
 */
#ifndef CANVAS_HPP
#define CANVAS_HPP

#include "snake_game/gui/theme.hpp"
#include <memory>
#include <string>
#include <vector>

/**
 * Draws filled rectangles and circles on an SDL renderer; the same code serves a window and an off-screen surface
 *
 * Attributes:
 * std::unique_ptr<Impl> impl_: The SDL window, surface and renderer
 */
class Canvas {
	struct Impl;
	std::unique_ptr<Impl> impl_;

	Canvas();

public:
	~Canvas();

	/**
	 * Opens a resizable window
	 *
	 * Parameters:
	 * const std::string& title: Window title
	 * int width: Initial width in pixels
	 * int height: Initial height in pixels
	 *
	 * Returns std::unique_ptr<Canvas>: The canvas, or nullptr if no window can be created
	 */
	static std::unique_ptr<Canvas> window(const std::string& title, int width, int height);

	/**
	 * Creates an off-screen canvas that needs no display (SDL's software renderer over a surface)
	 *
	 * Parameters:
	 * int width: Width in pixels
	 * int height: Height in pixels
	 *
	 * Returns std::unique_ptr<Canvas>: The canvas, or nullptr on failure
	 */
	static std::unique_ptr<Canvas> offscreen(int width, int height);

	/**
	 * Returns int: Current width in pixels (follows a window resize)
	 */
	int width() const;

	/**
	 * Returns int: Current height in pixels (follows a window resize)
	 */
	int height() const;

	/**
	 * Fills the whole canvas
	 *
	 * Parameters:
	 * Color color: Fill colour
	 */
	void clear(Color color);

	/**
	 * Fills a rectangle
	 *
	 * Parameters:
	 * int x, int y: Top-left corner
	 * int w, int h: Size in pixels
	 * Color color: Fill colour
	 */
	void fillRect(int x, int y, int w, int h, Color color);

	/**
	 * Fills a circle
	 *
	 * Parameters:
	 * int cx, int cy: Centre
	 * int radius: Radius in pixels
	 * Color color: Fill colour
	 */
	void fillCircle(int cx, int cy, int radius, Color color);

	/**
	 * Shows what was drawn; does nothing on an off-screen canvas
	 */
	void present();

	/**
	 * Reads the canvas back
	 *
	 * Returns std::vector<unsigned char>: Row-major RGB bytes, width x height x 3
	 */
	std::vector<unsigned char> pixelsRgb() const;

	/**
	 * Writes the canvas as a binary PPM (P6) image
	 *
	 * Parameters:
	 * const std::string& path: Output file
	 *
	 * Returns bool: true if the file was written
	 */
	bool writePpm(const std::string& path) const;
};

#endif
