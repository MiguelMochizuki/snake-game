/**
 * canvas_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the off-screen Canvas; built only with SNAKE_GAME_BUILD_GUI=ON
 */
#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "snake_game/gui/canvas.hpp"
#include "snake_game/gui/theme.hpp"

namespace {

/**
 * Reads one pixel from a row-major RGB buffer
 *
 * Parameters:
 * const std::vector<unsigned char>& rgb: Pixels from Canvas::pixelsRgb
 * int width: Canvas width
 * int x, int y: Pixel position
 *
 * Returns Color: The pixel colour
 */
Color pixelAt(const std::vector<unsigned char>& rgb, int width, int x, int y) {
	const size_t i = (static_cast<size_t>(y) * width + x) * 3;
	return Color{rgb[i], rgb[i + 1], rgb[i + 2]};
}

bool same(Color a, Color b) {
	return a.r == b.r && a.g == b.g && a.b == b.b;
}

}  // namespace

TEST(Canvas, OffscreenCanvasHasRequestedSize) {
	std::unique_ptr<Canvas> c = Canvas::offscreen(64, 48);
	ASSERT_NE(c, nullptr);
	EXPECT_EQ(c->width(), 64);
	EXPECT_EQ(c->height(), 48);
	EXPECT_EQ(c->pixelsRgb().size(), static_cast<size_t>(64 * 48 * 3));
}

TEST(Canvas, FillRectChangesPixels) {
	std::unique_ptr<Canvas> c = Canvas::offscreen(64, 48);
	c->clear(theme::BACKGROUND);
	c->fillRect(10, 10, 5, 5, theme::FOOD);
	std::vector<unsigned char> px = c->pixelsRgb();
	EXPECT_TRUE(same(pixelAt(px, 64, 12, 12), theme::FOOD));
	EXPECT_TRUE(same(pixelAt(px, 64, 0, 0), theme::BACKGROUND));
}

TEST(Canvas, FillCircleIsRoundNotSquare) {
	std::unique_ptr<Canvas> c = Canvas::offscreen(64, 48);
	c->clear(theme::BACKGROUND);
	c->fillCircle(32, 24, 10, theme::FOOD);
	std::vector<unsigned char> px = c->pixelsRgb();
	EXPECT_TRUE(same(pixelAt(px, 64, 32, 24), theme::FOOD));
	EXPECT_TRUE(same(pixelAt(px, 64, 22, 14), theme::BACKGROUND));  // corner of the bounding box
}

TEST(Canvas, WritePpmWritesHeaderAndPixels) {
	std::unique_ptr<Canvas> c = Canvas::offscreen(64, 48);
	c->clear(theme::BACKGROUND);
	const std::filesystem::path path = std::filesystem::temp_directory_path() / "snake_canvas_test.ppm";
	ASSERT_TRUE(c->writePpm(path.string()));
	std::ifstream in(path, std::ios::binary);
	std::stringstream content;
	content << in.rdbuf();
	in.close();
	std::filesystem::remove(path);
	const std::string header = "P6\n64 48\n255\n";
	EXPECT_EQ(content.str().substr(0, header.size()), header);
	EXPECT_EQ(content.str().size(), header.size() + 64 * 48 * 3);
}
