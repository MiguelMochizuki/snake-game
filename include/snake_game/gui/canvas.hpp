/**
 * canvas.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Drawing surface of the graphical front-end: an SDL window or an off-screen buffer
 */
#ifndef CANVAS_HPP
#define CANVAS_HPP

#include "snake_game/gui/draw_surface.hpp"
#include <memory>
#include <string>
#include <vector>

/**
 * Draws filled rectangles and circles on an SDL renderer, and reads them back; the same code serves a window and an off-screen surface
 *
 * Attributes:
 * std::unique_ptr<Impl> impl_: The SDL window, surface and renderer
 */
class Canvas : public IDrawSurface {
	struct Impl;
	std::unique_ptr<Impl> impl_;

	Canvas();

public:
	~Canvas() override;

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

	int width() const override;
	int height() const override;
	void clear(Color color) override;
	void fillRect(int x, int y, int w, int h, Color color) override;
	void fillCircle(int cx, int cy, int radius, Color color) override;
	void present() override;

	/**
	 * Reads the canvas back
	 *
	 * Returns std::vector<unsigned char>: Row-major RGB bytes, width x height x 3
	 */
	std::vector<unsigned char> pixelsRgb() const;
};

#endif
