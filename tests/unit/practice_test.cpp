/**
 * practice_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests that a practice session shows its own best and a practice panel, and leaves the saved figures alone
 */
#include <gtest/gtest.h>

#include "engine_fakes.hpp"
#include "snake_game/game/game_engine.hpp"
#include "snake_game/score/score_manager.hpp"

using namespace fakes;

namespace {

/**
 * Plays one game that ends against the wall, with 20 already saved as the best of Normal
 *
 * Parameters:
 * bool practice: Whether practice is on
 * RecordingRenderer& renderer: Receives the calls
 * const std::string& dir: Score folder
 */
void playOne(bool practice, RecordingRenderer& renderer, const std::string& dir) {
	{
		ScoreManager seed(dir);
		seed.saveScore(Level::Normal, 20);
	}
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	ScoreManager store(dir);
	GameEngine engine(keys, input, renderer, store, 20, 20, 0, 42);
	// Set in both orders over the tests below: the best shown must not depend on it
	engine.setPractice(practice);
	engine.setLevel(Level::Normal, 0);
	engine.run();
}

}  // namespace

TEST(Practice, StatusLineStartsFromZeroNotTheSavedBest) {
	TempDir dir;
	RecordingRenderer renderer;
	playOne(true, renderer, dir.path.string());
	EXPECT_TRUE(renderer.practice);
	ASSERT_FALSE(renderer.bests.empty());
	EXPECT_EQ(renderer.bests.front(), 0);
}

TEST(Practice, NormalPlayStillStartsFromTheSavedBest) {
	TempDir dir;
	RecordingRenderer renderer;
	playOne(false, renderer, dir.path.string());
	EXPECT_FALSE(renderer.practice);
	ASSERT_FALSE(renderer.bests.empty());
	EXPECT_EQ(renderer.bests.front(), 20);
}

TEST(Practice, PanelGetsTheSessionBestAndNoGamesAndSavedFiguresAreUntouched) {
	TempDir dir;
	RecordingRenderer renderer;
	playOne(true, renderer, dir.path.string());
	EXPECT_EQ(renderer.gameOvers, 1);
	EXPECT_EQ(renderer.lastStats.games, 0u);
	EXPECT_EQ(renderer.lastStats.best, renderer.lastScore);
	EXPECT_TRUE(renderer.lastStats.unsavedDir.empty());
	ScoreManager scores(dir.path.string());
	EXPECT_EQ(scores.stats(Level::Normal).games, 1u);
	EXPECT_EQ(scores.stats(Level::Normal).best, 20);
}

TEST(Practice, TheOrderOfSetLevelAndSetPracticeDoesNotMatter) {
	TempDir dir;
	{
		ScoreManager seed(dir.path.string());
		seed.saveScore(Level::Normal, 20);
	}
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	ScoreManager store(dir.path.string());
	GameEngine engine(keys, input, renderer, store, 20, 20, 0, 42);
	engine.setLevel(Level::Normal, 0);
	engine.setPractice(true);
	engine.run();
	ASSERT_FALSE(renderer.bests.empty());
	EXPECT_EQ(renderer.bests.front(), 0);
}
