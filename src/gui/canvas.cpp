/**
 * canvas.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Canvas implementation on SDL3
 */
#include "snake_game/gui/canvas.hpp"
#include <SDL3/SDL.h>
#include <cstdio>

/**
 * SDL objects behind a Canvas
 *
 * Attributes:
 * SDL_Window* window: The window, nullptr when off-screen
 * SDL_Surface* surface: The target surface, nullptr for a window
 * SDL_Renderer* renderer: The renderer
 * int w, int h: Size of an off-screen canvas
 */
struct Canvas::Impl {
	SDL_Window* window = nullptr;
	SDL_Surface* surface = nullptr;
	SDL_Renderer* renderer = nullptr;
	int w = 0;
	int h = 0;
};

Canvas::Canvas() : impl_(new Impl) {
}

/**
 * Destroys the renderer, window and surface
 */
Canvas::~Canvas() {
	if (impl_->renderer) {
		SDL_DestroyRenderer(impl_->renderer);
	}
	if (impl_->window) {
		SDL_DestroyWindow(impl_->window);
	}
	if (impl_->surface) {
		SDL_DestroySurface(impl_->surface);
	}
}

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
std::unique_ptr<Canvas> Canvas::window(const std::string& title, int width, int height) {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		return nullptr;
	}
	std::unique_ptr<Canvas> canvas(new Canvas);
	if (!SDL_CreateWindowAndRenderer(title.c_str(), width, height, SDL_WINDOW_RESIZABLE, &canvas->impl_->window, &canvas->impl_->renderer)) {
		return nullptr;
	}
	return canvas;
}

/**
 * Creates an off-screen canvas that needs no display
 *
 * Parameters:
 * int width: Width in pixels
 * int height: Height in pixels
 *
 * Returns std::unique_ptr<Canvas>: The canvas, or nullptr on failure
 */
std::unique_ptr<Canvas> Canvas::offscreen(int width, int height) {
	std::unique_ptr<Canvas> canvas(new Canvas);
	canvas->impl_->w = width;
	canvas->impl_->h = height;
	canvas->impl_->surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGB24);
	if (!canvas->impl_->surface) {
		return nullptr;
	}
	canvas->impl_->renderer = SDL_CreateSoftwareRenderer(canvas->impl_->surface);
	if (!canvas->impl_->renderer) {
		return nullptr;
	}
	return canvas;
}

int Canvas::width() const {
	if (impl_->window) {
		int w = 0, h = 0;
		SDL_GetCurrentRenderOutputSize(impl_->renderer, &w, &h);
		return w;
	}
	return impl_->w;
}

int Canvas::height() const {
	if (impl_->window) {
		int w = 0, h = 0;
		SDL_GetCurrentRenderOutputSize(impl_->renderer, &w, &h);
		return h;
	}
	return impl_->h;
}

/**
 * Fills the whole canvas
 *
 * Parameters:
 * Color color: Fill colour
 */
void Canvas::clear(Color color) {
	SDL_SetRenderDrawColor(impl_->renderer, color.r, color.g, color.b, 255);
	SDL_RenderClear(impl_->renderer);
}

/**
 * Fills a rectangle
 *
 * Parameters:
 * int x, int y: Top-left corner
 * int w, int h: Size in pixels
 * Color color: Fill colour
 */
void Canvas::fillRect(int x, int y, int w, int h, Color color) {
	SDL_SetRenderDrawColor(impl_->renderer, color.r, color.g, color.b, 255);
	const SDL_FRect rect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h)};
	SDL_RenderFillRect(impl_->renderer, &rect);
}

/**
 * Fills a circle with one horizontal span per row
 *
 * Parameters:
 * int cx, int cy: Centre
 * int radius: Radius in pixels
 * Color color: Fill colour
 */
void Canvas::fillCircle(int cx, int cy, int radius, Color color) {
	for (int dy = -radius; dy <= radius; ++dy) {
		int dx = 0;
		while ((dx + 1) * (dx + 1) + dy * dy <= radius * radius) {
			++dx;
		}
		fillRect(cx - dx, cy + dy, 2 * dx + 1, 1, color);
	}
}

/**
 * Shows what was drawn; does nothing on an off-screen canvas
 */
void Canvas::present() {
	if (impl_->window) {
		SDL_RenderPresent(impl_->renderer);
	}
}

/**
 * Reads the canvas back
 *
 * Returns std::vector<unsigned char>: Row-major RGB bytes, width x height x 3
 */
std::vector<unsigned char> Canvas::pixelsRgb() const {
	SDL_Surface* read = SDL_RenderReadPixels(impl_->renderer, nullptr);
	if (!read) {
		return {};
	}
	SDL_Surface* rgb = SDL_ConvertSurface(read, SDL_PIXELFORMAT_RGB24);
	SDL_DestroySurface(read);
	if (!rgb) {
		return {};
	}
	std::vector<unsigned char> out(static_cast<size_t>(rgb->w) * rgb->h * 3);
	const unsigned char* src = static_cast<const unsigned char*>(rgb->pixels);
	for (int y = 0; y < rgb->h; ++y) {
		std::copy(src + static_cast<size_t>(y) * rgb->pitch, src + static_cast<size_t>(y) * rgb->pitch + rgb->w * 3, out.begin() + static_cast<size_t>(y) * rgb->w * 3);
	}
	SDL_DestroySurface(rgb);
	return out;
}

/**
 * Writes the canvas as a binary PPM (P6) image
 *
 * Parameters:
 * const std::string& path: Output file
 *
 * Returns bool: true if the file was written
 */
bool Canvas::writePpm(const std::string& path) const {
	const std::vector<unsigned char> px = pixelsRgb();
	if (px.empty()) {
		return false;
	}
	std::FILE* f = std::fopen(path.c_str(), "wb");
	if (!f) {
		return false;
	}
	std::fprintf(f, "P6\n%d %d\n255\n", width(), height());
	const bool ok = std::fwrite(px.data(), 1, px.size(), f) == px.size();
	return std::fclose(f) == 0 && ok;
}
