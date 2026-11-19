/**
 * pixel_surface_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the in-memory PixelSurface and the PPM writer; they need no SDL
 */
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "snake_game/gui/pixel_surface.hpp"
#include "snake_game/gui/ppm.hpp"
#include "snake_game/gui/theme.hpp"

namespace {

/**
 * Reads one pixel from a surface
 *
 * Parameters:
 * const PixelSurface& s: The surface
 * int x, int y: Pixel position
 *
 * Returns Color: The pixel colour
 */
Color pixelAt(const PixelSurface& s, int x, int y) {
	const size_t i = (static_cast<size_t>(y) * s.width() + x) * 3;
	const std::vector<unsigned char>& rgb = s.pixelsRgb();
	return Color{rgb[i], rgb[i + 1], rgb[i + 2]};
}

/**
 * Compares two colours
 *
 * Parameters:
 * Color a, Color b: The colours
 *
 * Returns bool: true if all three channels match
 */
bool same(Color a, Color b) {
	return a.r == b.r && a.g == b.g && a.b == b.b;
}

}  // namespace

TEST(PixelSurface, ClearFillsEverything) {
	PixelSurface s(8, 6);
	s.clear(theme::FOOD);
	EXPECT_EQ(s.width(), 8);
	EXPECT_EQ(s.height(), 6);
	EXPECT_EQ(s.pixelsRgb().size(), static_cast<size_t>(8 * 6 * 3));
	for (int y = 0; y < 6; ++y) {
		for (int x = 0; x < 8; ++x) {
			ASSERT_TRUE(same(pixelAt(s, x, y), theme::FOOD));
		}
	}
}

TEST(PixelSurface, FillRectDrawsInsideTheBuffer) {
	PixelSurface s(16, 16);
	s.clear(theme::BACKGROUND);
	s.fillRect(4, 5, 3, 2, theme::HEAD);
	EXPECT_TRUE(same(pixelAt(s, 4, 5), theme::HEAD));
	EXPECT_TRUE(same(pixelAt(s, 6, 6), theme::HEAD));
	EXPECT_TRUE(same(pixelAt(s, 7, 6), theme::BACKGROUND));
	EXPECT_TRUE(same(pixelAt(s, 6, 7), theme::BACKGROUND));
}

TEST(PixelSurface, FillRectClipsToTheBuffer) {
	PixelSurface s(8, 8);
	s.clear(theme::BACKGROUND);
	s.fillRect(-5, -5, 7, 7, theme::HEAD);    // sticks out of the top-left corner
	s.fillRect(6, 6, 20, 20, theme::FOOD);    // sticks out of the bottom-right corner
	s.fillRect(100, 100, 4, 4, theme::GOLD);  // entirely outside
	s.fillRect(2, 2, -3, 4, theme::GOLD);     // empty: negative size
	EXPECT_TRUE(same(pixelAt(s, 0, 0), theme::HEAD));
	EXPECT_TRUE(same(pixelAt(s, 1, 1), theme::HEAD));
	EXPECT_TRUE(same(pixelAt(s, 2, 2), theme::BACKGROUND));
	EXPECT_TRUE(same(pixelAt(s, 7, 7), theme::FOOD));
	EXPECT_EQ(s.pixelsRgb().size(), static_cast<size_t>(8 * 8 * 3));
}

TEST(PixelSurface, FillCircleIsRoundNotSquare) {
	PixelSurface s(64, 48);
	s.clear(theme::BACKGROUND);
	s.fillCircle(32, 24, 10, theme::FOOD);
	EXPECT_TRUE(same(pixelAt(s, 32, 24), theme::FOOD));
	EXPECT_TRUE(same(pixelAt(s, 22, 24), theme::FOOD));
	EXPECT_TRUE(same(pixelAt(s, 22, 14), theme::BACKGROUND));  // corner of the bounding box
}

TEST(PixelSurface, FillCircleNearTheEdgeIsClipped) {
	PixelSurface s(16, 16);
	s.clear(theme::BACKGROUND);
	s.fillCircle(0, 0, 10, theme::FOOD);
	EXPECT_TRUE(same(pixelAt(s, 0, 0), theme::FOOD));
	EXPECT_EQ(s.pixelsRgb().size(), static_cast<size_t>(16 * 16 * 3));
}

TEST(PixelSurface, WritePpmWritesHeaderAndPixels) {
	PixelSurface s(5, 3);
	s.clear(theme::BACKGROUND);
	const std::filesystem::path path = std::filesystem::temp_directory_path() / "snake_pixel_surface_test.ppm";
	ASSERT_TRUE(writePpm(s.width(), s.height(), s.pixelsRgb(), path.string()));
	std::ifstream in(path, std::ios::binary);
	std::stringstream content;
	content << in.rdbuf();
	in.close();
	std::filesystem::remove(path);
	const std::string header = "P6\n5 3\n255\n";
	EXPECT_EQ(content.str().substr(0, header.size()), header);
	EXPECT_EQ(content.str().size(), header.size() + 5 * 3 * 3);
}
