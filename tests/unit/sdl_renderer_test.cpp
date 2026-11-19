/**
 * sdl_renderer_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for SdlRenderer on an off-screen canvas; built only with SNAKE_GAME_BUILD_GUI=ON
 */
#include <gtest/gtest.h>

#include <chrono>
#include <vector>

#include <SDL3/SDL.h>

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

TEST(SdlRenderer, VictoryPanelChangesThePicture) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(640, 720);
	SdlApp app(*canvas, false);
	SdlRenderer renderer(*canvas, app);
	GameState state(20, 20, 0, 42);
	renderer.clear();
	renderer.render(state);
	std::vector<unsigned char> before = canvas->pixelsRgb();
	renderer.printVictory(Level::Normal, 12, ScoreStats{30, 4, 9.5});
	std::vector<unsigned char> victory = canvas->pixelsRgb();
	EXPECT_NE(before, victory);
	renderer.clear();
	renderer.render(state);
	renderer.printGameOver(Level::Normal, 12, ScoreStats{30, 4, 9.5});
	EXPECT_NE(victory, canvas->pixelsRgb());
}

TEST(SdlRenderer, VictoryDoesNotWaitOffScreen) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(640, 720);
	SdlApp app(*canvas, false);
	SdlRenderer renderer(*canvas, app);
	GameState state(20, 20, 0, 42);
	renderer.clear();
	renderer.render(state);
	const auto start = std::chrono::steady_clock::now();
	renderer.printVictory(Level::Normal, 12, ScoreStats{30, 4, 9.5});
	// The fade lasts 600 ms on a window; off-screen there is nobody to watch it
	EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::milliseconds(200));
}

TEST(SdlRenderer, ClosingTheWindowDuringTheFadeStillDrawsThePanel) {
	// The reference picture: the same win with no fade to interrupt
	std::unique_ptr<Canvas> plain = Canvas::offscreen(640, 720);
	SdlApp plainApp(*plain, false);
	SdlRenderer plainRenderer(*plain, plainApp);
	GameState state(20, 20, 0, 42);
	plainRenderer.clear();
	plainRenderer.render(state);
	plainRenderer.printVictory(Level::Normal, 12, ScoreStats{30, 4, 9.5});

	std::unique_ptr<Canvas> canvas = Canvas::offscreen(640, 720);
	SdlApp app(*canvas, true);
	SdlRenderer renderer(*canvas, app);
	renderer.clear();
	renderer.render(state);
	app.pump(0);
	SDL_Event close;
	SDL_zero(close);
	close.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
	SDL_PushEvent(&close);
	const auto start = std::chrono::steady_clock::now();
	renderer.printVictory(Level::Normal, 12, ScoreStats{30, 4, 9.5});
	const auto took = std::chrono::steady_clock::now() - start;

	EXPECT_TRUE(app.closeRequested());
	// Only the first of the 12 steps runs (about 50 ms), not the whole 600 ms
	EXPECT_LT(took, std::chrono::milliseconds(300));
	EXPECT_EQ(canvas->pixelsRgb(), plain->pixelsRgb());
}

TEST(SdlRenderer, VictoryOnATinyCanvasDoesNotCrash) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(10, 10);
	SdlApp app(*canvas, false);
	SdlRenderer renderer(*canvas, app);
	GameState state(20, 20, 0, 42);
	renderer.clear();
	renderer.render(state);
	renderer.printVictory(Level::Hard, 0, ScoreStats{0, 0, 0.0});
}

TEST(SdlRenderer, VictoryDarkensTheBoard) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(640, 720);
	SdlApp app(*canvas, false);
	SdlRenderer renderer(*canvas, app);
	GameState state(20, 20, 0, 42);
	ASSERT_FALSE(state.getFood().getX() == 0 && state.getFood().getY() == 0);
	const Layout l = layoutFor(640, 720, 20, 20);
	// The centre of the top-left cell: empty playfield, well away from the centred panel
	const int px = l.originX + l.cell + l.cell / 2;
	const int py = l.originY + l.cell + l.cell / 2;
	renderer.clear();
	renderer.render(state);
	const Color before = pixelAt(canvas->pixelsRgb(), 640, px, py);
	renderer.printVictory(Level::Normal, 12, ScoreStats{30, 4, 9.5});
	const Color after = pixelAt(canvas->pixelsRgb(), 640, px, py);
	EXPECT_LT(after.r, before.r);
	EXPECT_LT(after.g, before.g);
	EXPECT_LT(after.b, before.b);
}

TEST(SdlRenderer, TheWinningFrameDrawsTheHeadNotTheFood) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(640, 720);
	SdlApp app(*canvas, false);
	SdlRenderer renderer(*canvas, app);
	GameState state(1, 4, 0, 5);
	state.moveSnake(3);
	ASSERT_TRUE(state.isWon());
	const Layout l = layoutFor(640, 720, 4, 1);
	const SnakeSegment* head = state.getSnake().getHead();
	// A corner of the head cell: the food is a circle and leaves the corners alone, the head fills the square
	const int px = l.originX + (head->getX() + 1) * l.cell + 1;
	const int py = l.originY + (head->getY() + 1) * l.cell + 1;
	renderer.clear();
	renderer.render(state);
	EXPECT_TRUE(same(pixelAt(canvas->pixelsRgb(), 640, px, py), theme::HEAD));
}

TEST(SdlRenderer, RedrawAfterVictoryKeepsTheBoardDark) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(640, 720);
	SdlApp app(*canvas, false);
	SdlRenderer renderer(*canvas, app);
	GameState state(20, 20, 0, 42);
	renderer.clear();
	renderer.render(state);
	renderer.printVictory(Level::Normal, 12, ScoreStats{30, 4, 9.5});
	const std::vector<unsigned char> shown = canvas->pixelsRgb();
	app.redraw();
	EXPECT_EQ(shown, canvas->pixelsRgb());
}

TEST(SdlRenderer, ResetRestoresFullBrightness) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(640, 720);
	SdlApp app(*canvas, false);
	SdlRenderer renderer(*canvas, app);
	GameState state(20, 20, 0, 42);
	renderer.clear();
	renderer.render(state);
	const std::vector<unsigned char> fresh = canvas->pixelsRgb();
	renderer.printVictory(Level::Normal, 12, ScoreStats{30, 4, 9.5});
	renderer.reset();
	renderer.clear();
	renderer.render(state);
	EXPECT_EQ(fresh, canvas->pixelsRgb());
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

TEST(SdlRenderer, GameOverPanelRedrawsTheBoardInsteadOfTrustingTheBackBuffer) {
	std::unique_ptr<Canvas> canvas = Canvas::offscreen(640, 720);
	SdlApp app(*canvas, false);
	SdlRenderer renderer(*canvas, app);
	GameState state(20, 20, 0, 42);
	renderer.clear();
	renderer.render(state);
	// On a real window the buffer behind present() may hold an older frame: the game over panel must not depend on it
	canvas->clear(Color{200, 0, 200});
	renderer.printGameOver(Level::Normal, 0, ScoreStats{});
	std::vector<unsigned char> px = canvas->pixelsRgb();
	// The panel hides the head, so check that the stale pixels outside it are gone
	EXPECT_TRUE(same(pixelAt(px, 640, 0, 0), theme::BACKGROUND));
}
