/**
 * app_icon_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the window icon embedded in the binary; built only with SNAKE_GAME_BUILD_GUI=ON
 */
#include <gtest/gtest.h>

#include <SDL3/SDL.h>

#include "snake_game/gui/app_icon.hpp"
#include "snake_game/gui/theme.hpp"

namespace {

/**
 * Reads one pixel of the embedded icon
 *
 * Parameters:
 * int x, int y: Pixel position
 *
 * Returns const unsigned char*: Pointer to its R, G, B, A bytes
 */
const unsigned char* iconPixel(int x, int y) {
	return &ICON_RGBA[(y * ICON_SIZE + x) * 4];
}

}  // namespace

TEST(AppIcon, DataHasTheDeclaredSize) {
	EXPECT_EQ(sizeof ICON_RGBA, static_cast<size_t>(ICON_SIZE) * ICON_SIZE * 4);
	EXPECT_GE(ICON_SIZE, 64);
}

TEST(AppIcon, IsARoundedSquare) {
	EXPECT_EQ(iconPixel(0, 0)[3], 0) << "the corner is transparent";
	EXPECT_EQ(iconPixel(ICON_SIZE - 1, ICON_SIZE - 1)[3], 0);
	EXPECT_EQ(iconPixel(ICON_SIZE / 2, ICON_SIZE / 2)[3], 255) << "the middle is opaque";
}

TEST(AppIcon, ContainsTheSnakeAndTheFood) {
	bool body = false, food = false;
	for (int y = 0; y < ICON_SIZE; ++y) {
		for (int x = 0; x < ICON_SIZE; ++x) {
			const unsigned char* p = iconPixel(x, y);
			if (p[3] != 255) {
				continue;
			}
			body = body || (p[0] == theme::BODY.r && p[1] == theme::BODY.g && p[2] == theme::BODY.b);
			food = food || (p[0] == theme::FOOD.r && p[1] == theme::FOOD.g && p[2] == theme::FOOD.b);
		}
	}
	EXPECT_TRUE(body);
	EXPECT_TRUE(food);
}

TEST(AppIcon, SurfaceMatchesTheData) {
	SDL_Surface* surface = makeIconSurface();
	ASSERT_NE(surface, nullptr);
	EXPECT_EQ(surface->w, ICON_SIZE);
	EXPECT_EQ(surface->h, ICON_SIZE);
	Uint8 r, g, b, a;
	ASSERT_TRUE(SDL_ReadSurfacePixel(surface, ICON_SIZE / 2, ICON_SIZE / 2, &r, &g, &b, &a));
	EXPECT_EQ(r, iconPixel(ICON_SIZE / 2, ICON_SIZE / 2)[0]);
	EXPECT_EQ(a, 255);
	SDL_DestroySurface(surface);
}
