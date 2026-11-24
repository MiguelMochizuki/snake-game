/**
 * sdl_menu_view_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for SdlMenuView on an off-screen canvas; built only with SNAKE_GAME_BUILD_GUI=ON
 */
#include <gtest/gtest.h>

#include <vector>

#include "snake_game/gui/canvas.hpp"
#include "snake_game/gui/sdl_app.hpp"
#include "snake_game/gui/sdl_menu_view.hpp"

namespace {

/**
 * Canvas, event pump and view wired together for one test
 */
struct Fixture {
	std::unique_ptr<Canvas> canvas;
	SdlApp app;
	SdlMenuView view;

	explicit Fixture(int w = 640, int h = 720) : canvas(Canvas::offscreen(w, h)), app(*canvas, false), view(*canvas, app) {}

	std::vector<unsigned char> blank() {
		canvas->clear(theme::BACKGROUND);
		return canvas->pixelsRgb();
	}
};

MenuSnapshot snapshot(MenuItem selected) {
	return MenuSnapshot{selected, Level::Normal, ScoreStats{42, 7}, true};
}

}  // namespace

TEST(SdlMenuView, MenuDrawsSelectedRowHighlighted) {
	Fixture f;
	std::vector<unsigned char> empty = f.blank();
	f.view.show(snapshot(MenuItem::Level));
	std::vector<unsigned char> level = f.canvas->pixelsRgb();
	f.view.show(snapshot(MenuItem::Quit));
	std::vector<unsigned char> quit = f.canvas->pixelsRgb();
	EXPECT_NE(level, empty);
	EXPECT_NE(quit, empty);
	EXPECT_NE(level, quit);
}

TEST(SdlMenuView, MenuShowsTheSpriteSnakeAndApple) {
	Fixture f;
	f.blank();
	f.view.show(snapshot(MenuItem::Start));
	// Colours that only the sprites use: the snake's eyes and the apple's leaf and stem
	bool eye = false, leaf = false, stem = false;
	const std::vector<unsigned char> px = f.canvas->pixelsRgb();
	for (size_t i = 0; i + 2 < px.size(); i += 3) {
		eye = eye || (px[i] == theme::GOLD.r && px[i + 1] == theme::GOLD.g && px[i + 2] == theme::GOLD.b);
		leaf = leaf || (px[i] == theme::LEAF.r && px[i + 1] == theme::LEAF.g && px[i + 2] == theme::LEAF.b);
		stem = stem || (px[i] == theme::STEM.r && px[i + 1] == theme::STEM.g && px[i + 2] == theme::STEM.b);
	}
	EXPECT_TRUE(eye);
	EXPECT_TRUE(leaf);
	EXPECT_TRUE(stem);
}

TEST(SdlMenuView, SoundSwitchChangesThePicture) {
	Fixture f;
	f.view.show(MenuSnapshot{MenuItem::Sound, Level::Normal, ScoreStats{0, 0}, true});
	std::vector<unsigned char> on = f.canvas->pixelsRgb();
	f.view.show(MenuSnapshot{MenuItem::Sound, Level::Normal, ScoreStats{0, 0}, false});
	EXPECT_NE(on, f.canvas->pixelsRgb());
}

TEST(SdlMenuView, PracticeSwitchChangesThePicture) {
	Fixture f;
	f.view.show(MenuSnapshot{MenuItem::Practice, Level::Normal, ScoreStats{0, 0}, true, false});
	std::vector<unsigned char> off = f.canvas->pixelsRgb();
	f.view.show(MenuSnapshot{MenuItem::Practice, Level::Normal, ScoreStats{0, 0}, true, true});
	EXPECT_NE(off, f.canvas->pixelsRgb());
}

TEST(SdlMenuView, ResetNoticeChangesThePicture) {
	Fixture f;
	f.view.show(MenuSnapshot{MenuItem::Start, Level::Normal, ScoreStats{0, 0}, true, false, false});
	std::vector<unsigned char> normal = f.canvas->pixelsRgb();
	f.view.show(MenuSnapshot{MenuItem::Start, Level::Normal, ScoreStats{0, 0}, true, false, true});
	EXPECT_NE(normal, f.canvas->pixelsRgb());
}

TEST(SdlMenuView, SaveFailureChangesThePicture) {
	Fixture f;
	f.view.show(MenuSnapshot{MenuItem::Start, Level::Normal, ScoreStats{0, 0, ""}, true});
	std::vector<unsigned char> normal = f.canvas->pixelsRgb();
	f.view.show(MenuSnapshot{MenuItem::Start, Level::Normal, ScoreStats{0, 0, "/no/such"}, true});
	EXPECT_NE(normal, f.canvas->pixelsRgb());
}

TEST(SdlMenuView, ConfirmClearIsNotBlank) {
	Fixture f;
	std::vector<unsigned char> empty = f.blank();
	f.view.showConfirmClear(Level::Normal);
	EXPECT_NE(empty, f.canvas->pixelsRgb());
}

TEST(SdlMenuView, ConfirmClearDiffersPerLevel) {
	Fixture f;
	f.view.showConfirmClear(Level::Easy);
	std::vector<unsigned char> easy = f.canvas->pixelsRgb();
	f.view.showConfirmClear(Level::Hard);
	EXPECT_NE(easy, f.canvas->pixelsRgb());
}

TEST(SdlMenuView, LeaderboardEmptyAndFilledDiffer) {
	Fixture f;
	f.view.showLeaderboard(LeaderboardSnapshot{Level::Hard, {}});
	std::vector<unsigned char> empty = f.canvas->pixelsRgb();
	f.view.showLeaderboard(LeaderboardSnapshot{Level::Hard, {30, 20, 10}});
	EXPECT_NE(empty, f.canvas->pixelsRgb());
}

TEST(SdlMenuView, HelpIsNotBlank) {
	Fixture f;
	std::vector<unsigned char> empty = f.blank();
	f.view.showHelp();
	EXPECT_NE(empty, f.canvas->pixelsRgb());
}

TEST(SdlMenuView, MenuSurvivesTinyCanvas) {
	Fixture f(20, 20);
	f.view.show(snapshot(MenuItem::Start));
	f.view.showLeaderboard(LeaderboardSnapshot{Level::Easy, {5}});
	f.view.showHelp();
}
