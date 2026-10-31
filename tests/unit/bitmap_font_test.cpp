/**
 * bitmap_font_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the embedded 8x8 bitmap font; built only with SNAKE_GAME_BUILD_GUI=ON
 */
#include <gtest/gtest.h>

#include <vector>

#include "snake_game/gui/bitmap_font.hpp"
#include "snake_game/gui/canvas.hpp"

namespace {

int litPixels(char c) {
	int n = 0;
	for (int row = 0; row < font::GLYPH; ++row) {
		for (int col = 0; col < font::GLYPH; ++col) {
			n += font::glyphPixel(c, col, row) ? 1 : 0;
		}
	}
	return n;
}

}  // namespace

TEST(BitmapFont, TextWidthScalesWithLengthAndScale) {
	EXPECT_EQ(font::textWidth("ABC", 2), 3 * 8 * 2);
	EXPECT_EQ(font::textWidth("", 3), 0);
	EXPECT_EQ(font::textHeight(2), 16);
}

TEST(BitmapFont, SpaceHasNoPixels) {
	EXPECT_EQ(litPixels(' '), 0);
}

TEST(BitmapFont, LetterIAndDotDiffer) {
	EXPECT_GT(litPixels('I'), 0);
	EXPECT_GT(litPixels('.'), 0);
	EXPECT_NE(litPixels('I'), litPixels('.'));
}

TEST(BitmapFont, DrawTextLightsPixelsInsideItsBox) {
	std::unique_ptr<Canvas> c = Canvas::offscreen(64, 32);
	c->clear(theme::BACKGROUND);
	font::drawText(*c, 4, 4, "Hi", 1, theme::NUMBER);
	std::vector<unsigned char> px = c->pixelsRgb();
	int inside = 0, outside = 0;
	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 64; ++x) {
			const size_t i = (static_cast<size_t>(y) * 64 + x) * 3;
			const bool text = px[i] == theme::NUMBER.r && px[i + 1] == theme::NUMBER.g && px[i + 2] == theme::NUMBER.b;
			const bool inBox = x >= 4 && x < 4 + 16 && y >= 4 && y < 4 + 8;
			if (text) {
				++(inBox ? inside : outside);
			}
		}
	}
	EXPECT_GT(inside, 0);
	EXPECT_EQ(outside, 0);
}
