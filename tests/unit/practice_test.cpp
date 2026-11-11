/**
 * practice_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests that practice mode plays a game without saving it to the leaderboard
 */
#include <gtest/gtest.h>

#include "engine_fakes.hpp"
#include "snake_game/game/game_engine.hpp"
#include "snake_game/score/score_manager.hpp"

using namespace fakes;

TEST(Practice, GamesAreNotSaved) {
	TempDir dir;
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	GameEngine engine(keys, input, renderer, 20, 20, 0, 42, dir.path.string());
	engine.setPractice(true);

	engine.run();

	EXPECT_EQ(renderer.gameOvers, 1);
	ScoreManager scores(dir.path.string());
	EXPECT_EQ(scores.stats(Level::Normal).games, 0);
}

TEST(Practice, SwitchingItOffSavesAgain) {
	TempDir dir;
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	GameEngine engine(keys, input, renderer, 20, 20, 0, 42, dir.path.string());
	engine.setPractice(true);
	engine.setPractice(false);

	engine.run();

	ScoreManager scores(dir.path.string());
	EXPECT_EQ(scores.stats(Level::Normal).games, 1);
}
