/**
 * sdl_renderer_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for SdlRenderer on an off-screen canvas; built only with SNAKE_GAME_BUILD_GUI=ON
 */
#include <gtest/gtest.h>

#include <vector>

#include "snake_game/game/game_state.hpp"
#include "snake_game/gui/canvas.hpp"
#include "snake_game/gui/sdl_app.hpp"
#include "snake_game/gui/sdl_renderer.hpp"

namespace {

Color pixelAt(const std::vector<unsigned char>& rgb, int width, int x, int y) {
	const size_t i = (static_cast<size_t>(y) * width + x) * 3;
	return Color{rgb[i], rgb[i + 1], rgb[i + 2]};
}

bool same(Color a, Color b) {
	return a.r == b.r && a.g == b.g && a.b == b.b;
}

}  // namespace

TEST(SdlRenderer, CellSizeIsLargestThatFits) {
	// 22 cells across and 22 down plus the HUD: 640 / 22 = 29, (720 - 80) / 22 = 29
	EXPECT_EQ(cellSizeFor(640, 720, 20, 20, 80), 29);
	EXPECT_EQ(cellSizeFor(640, 400, 20, 20, 80), 14);  // height limits: 320 / 22
	EXPECT_EQ(cellSizeFor(100, 100, 20, 20, 80), 1);
	EXPECT_EQ(cellSizeFor(0, 0, 20, 20, 80), 1);
}

TEST(SdlRenderer, LayoutCentresTheBoard) {
	Layout l = layoutFor(640, 720, 20, 20);
	EXPECT_GE(l.cell, 1);
	const int boardWidth = l.cell * 22;
	EXPECT_EQ(l.originX, (640 - boardWidth) / 2);
	EXPECT_GE(l.originY, 0);
}

TEST(SdlRenderer, RenderDrawsFoodAndSnakeColours) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(640, 720);
	SdlApp app(*canvas, false);
	SdlRenderer renderer(*canvas, app);
	GameState state(20, 20, 0, 42);
	renderer.clear();
	renderer.render(state);
	std::vector<unsigned char> px = canvas->pixelsRgb();
	const Layout l = layoutFor(640, 720, 20, 20);
	auto centre = [&](int cx, int cy) { return pixelAt(px, 640, l.originX + (cx + 1) * l.cell + l.cell / 2, l.originY + (cy + 1) * l.cell + l.cell / 2); };
	EXPECT_TRUE(same(centre(state.getFood().getX(), state.getFood().getY()), theme::FOOD));
	const SnakeSegment* head = state.getSnake().getHead();
	EXPECT_TRUE(same(centre(head->getX(), head->getY()), theme::HEAD));
	EXPECT_TRUE(same(pixelAt(px, 640, 0, 0), theme::BACKGROUND));
}

TEST(SdlRenderer, RenderTwiceIsDeterministic) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(640, 720);
	SdlApp app(*canvas, false);
	SdlRenderer renderer(*canvas, app);
	GameState state(20, 20, 0, 42);
	renderer.clear();
	renderer.render(state);
	std::vector<unsigned char> first = canvas->pixelsRgb();
	renderer.clear();
	renderer.render(state);
	EXPECT_EQ(first, canvas->pixelsRgb());
}

TEST(SdlRenderer, LastStateIsTheRenderedState) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(320, 360);
	SdlApp app(*canvas, false);
	SdlRenderer renderer(*canvas, app);
	GameState state(20, 20, 0, 42);
	renderer.render(state);
	EXPECT_EQ(renderer.lastState().getFood().getX(), state.getFood().getX());
	EXPECT_EQ(renderer.lastState().getScore(), state.getScore());
}

TEST(SdlRenderer, GameOverPanelChangesThePicture) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(640, 720);
	SdlApp app(*canvas, false);
	SdlRenderer renderer(*canvas, app);
	GameState state(20, 20, 0, 42);
	renderer.clear();
	renderer.render(state);
	std::vector<unsigned char> before = canvas->pixelsRgb();
	renderer.printGameOver(Level::Normal, 12, ScoreStats{30, 4, 9.5});
	EXPECT_NE(before, canvas->pixelsRgb());
}

TEST(SdlRenderer, TinyCanvasDoesNotCrash) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(10, 10);
	SdlApp app(*canvas, false);
	SdlRenderer renderer(*canvas, app);
	GameState state(20, 20, 0, 42);
	renderer.clear();
	renderer.render(state);
	renderer.printGameOver(Level::Hard, 0, ScoreStats{0, 0, 0.0});
	renderer.pause(1);
}
