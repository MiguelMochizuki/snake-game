/**
 * victory_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests that GameEngine ends a game that fills the board with the victory screen, not game over
 */
#include <gtest/gtest.h>

#include "engine_fakes.hpp"
#include "snake_game/game/game_engine.hpp"
#include "snake_game/score/score_manager.hpp"

using namespace fakes;

TEST(Victory, EngineShowsVictoryNotGameOver) {
	TempDir dir;
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	// A 1x4 board with seed 5: the snake starts three cells long and the first move eats the last food
	ScoreManager store(dir.path.string());
	GameEngine engine(keys, input, renderer, store, 1, 4, 0, 5);

	engine.run();

	EXPECT_EQ(renderer.victories, 1);
	EXPECT_EQ(renderer.gameOvers, 0);
	EXPECT_EQ(renderer.lastScore, 1);
	ScoreManager scores(dir.path.string());
	EXPECT_EQ(scores.stats(Level::Normal).games, 1);
	EXPECT_EQ(scores.stats(Level::Normal).best, 1);
}

TEST(Victory, RestartAfterAWinPlaysAgain) {
	TempDir dir;
	ScriptedKeys keys;
	keys.waits.push_back(charKey('r'));
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	ScoreManager store(dir.path.string());
	GameEngine engine(keys, input, renderer, store, 1, 4, 0, 5);

	engine.run();

	EXPECT_EQ(renderer.victories, 2);
	EXPECT_EQ(renderer.gameOvers, 0);
}

TEST(Victory, AWallCrashIsStillGameOver) {
	TempDir dir;
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	ScoreManager store(dir.path.string());
	GameEngine engine(keys, input, renderer, store, 20, 20, 0, 42);

	engine.run();

	EXPECT_EQ(renderer.victories, 0);
	EXPECT_EQ(renderer.gameOvers, 1);
}
