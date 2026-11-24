/**
 * abandon_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests that a game the player walks away from is not saved, while a finished one is
 */
#include <gtest/gtest.h>

#include "engine_fakes.hpp"
#include "snake_game/game/game_engine.hpp"
#include "snake_game/score/score_manager.hpp"

using namespace fakes;

namespace {

/**
 * Key source whose first poll() is a q, as if the player pressed Q (or Ctrl+C) in the middle of the game
 */
class QuitsMidGame : public ScriptedKeys {
public:
	bool sent = false;

	/**
	 * Gives the q once, then nothing
	 *
	 * Returns std::vector<Key>: The pending keys
	 */
	std::vector<Key> poll() override {
		if (sent) {
			return {};
		}
		sent = true;
		return {charKey('q')};
	}
};

}  // namespace

TEST(Abandon, QuittingMidGameDoesNotSaveButStillShowsThePanel) {
	TempDir dir;
	QuitsMidGame keys;
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	ScoreManager store(dir.path.string());
	GameEngine engine(keys, input, renderer, store, 20, 20, 0, 42);

	engine.run();

	EXPECT_EQ(renderer.gameOvers, 1);
	ScoreManager scores(dir.path.string());
	EXPECT_EQ(scores.stats(Level::Normal).games, 0u);
}

TEST(Abandon, DyingStillSaves) {
	TempDir dir;
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	ScoreManager store(dir.path.string());
	GameEngine engine(keys, input, renderer, store, 20, 20, 0, 42);

	engine.run();

	ScoreManager scores(dir.path.string());
	EXPECT_EQ(scores.stats(Level::Normal).games, 1u);
}
