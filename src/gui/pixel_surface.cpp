/**
 * pixel_surface.cpp
 * Author: Miguel Mochizuki Silva
 * Description: PixelSurface implementation
 */
#include "snake_game/gui/pixel_surface.hpp"
#include <algorithm>

PixelSurface::PixelSurface(int width, int height)
	: width_(std::max(0, width)), height_(std::max(0, height)), rgb_(static_cast<size_t>(width_) * height_ * 3, 0) {
}

int PixelSurface::width() const {
	return width_;
}

int PixelSurface::height() const {
	return height_;
}

void PixelSurface::clear(Color color) {
	fillRect(0, 0, width_, height_, color);
}

/**
 * Fills a rectangle, clipped to the buffer; an empty or fully outside rectangle draws nothing
 *
 * Parameters:
 * int x, int y: Top-left corner
 * int w, int h: Size in pixels
 * Color color: Fill colour
 */
void PixelSurface::fillRect(int x, int y, int w, int h, Color color) {
	const int x0 = std::max(0, x);
	const int y0 = std::max(0, y);
	const int x1 = std::min(width_, x + w);
	const int y1 = std::min(height_, y + h);
	for (int py = y0; py < y1; ++py) {
		unsigned char* row = &rgb_[(static_cast<size_t>(py) * width_ + x0) * 3];
		for (int px = x0; px < x1; ++px, row += 3) {
			row[0] = color.r;
			row[1] = color.g;
			row[2] = color.b;
		}
	}
}

void PixelSurface::present() {
}

const std::vector<unsigned char>& PixelSurface::pixelsRgb() const {
	return rgb_;
}
