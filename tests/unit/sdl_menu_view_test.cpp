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
	return MenuSnapshot{selected, Level::Normal, ScoreStats{42, 7, 12.5}, true};
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

TEST(SdlMenuView, SoundSwitchChangesThePicture) {
	Fixture f;
	f.view.show(MenuSnapshot{MenuItem::Sound, Level::Normal, ScoreStats{0, 0, 0.0}, true});
	std::vector<unsigned char> on = f.canvas->pixelsRgb();
	f.view.show(MenuSnapshot{MenuItem::Sound, Level::Normal, ScoreStats{0, 0, 0.0}, false});
	EXPECT_NE(on, f.canvas->pixelsRgb());
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
