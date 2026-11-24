/**
 * pause_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests that the game can be paused with P or by losing focus, and that a paused game is neither lost nor saved wrongly
 */
#include <gtest/gtest.h>

#include <deque>
#include <string>
#include <vector>

#include "engine_fakes.hpp"
#include "snake_game/game/game_engine.hpp"
#include "snake_game/score/score_manager.hpp"

using namespace fakes;

namespace {

/**
 * Key source that hands out one batch of keys per poll() and answers waitKey() from a script
 *
 * Attributes:
 * std::deque<std::vector<Key>> polls: What each poll() returns, in order; empty batches afterwards
 */
class ScriptedPolls : public ScriptedKeys {
public:
	std::deque<std::vector<Key>> polls;

	/**
	 * Gives the next batch
	 *
	 * Returns std::vector<Key>: The pending keys
	 */
	std::vector<Key> poll() override {
		if (polls.empty()) {
			return {};
		}
		std::vector<Key> batch = polls.front();
		polls.pop_front();
		return batch;
	}
};

/**
 * Sound that records which music calls the engine makes
 *
 * Attributes:
 * std::vector<std::string> events: The calls, in order
 */
class MusicLog : public ISound {
public:
	std::vector<std::string> events;

	void eat() override {}
	void gameOver() override {}
	void musicStart() override { events.push_back("start"); }
	void musicResume() override { events.push_back("resume"); }
	void musicStop() override { events.push_back("stop"); }
};

}  // namespace

TEST(AwaitResume, PResumes) {
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	keys.waits = {charKey('s'), specialKey(KeyKind::FocusLost), charKey('p')};
	int panels = 0;
	EXPECT_TRUE(GameEngine::awaitResume(keys, input, [&] { ++panels; }));
	EXPECT_EQ(panels, 1);
	EXPECT_TRUE(keys.waits.empty());
}

TEST(AwaitResume, EscResumesToo) {
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	keys.waits = {charKey('x'), specialKey(KeyKind::Escape)};
	EXPECT_TRUE(GameEngine::awaitResume(keys, input, [] {}));
}

TEST(AwaitChoice, EscGoesToTheMenu) {
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	keys.waits = {charKey('x'), specialKey(KeyKind::Escape)};
	EXPECT_EQ(GameEngine::awaitChoice(keys, input, [] {}), GameEnd::Menu);
}

TEST(AwaitResume, QuitAndEofDoNotResume) {
	CompositeInputHandler input = makeInput();
	ScriptedKeys quit;
	quit.waits = {charKey('q')};
	EXPECT_FALSE(GameEngine::awaitResume(quit, input, [] {}));
	ScriptedKeys ctrlC;
	ctrlC.waits = {specialKey(KeyKind::CtrlC)};
	EXPECT_FALSE(GameEngine::awaitResume(ctrlC, input, [] {}));
	ScriptedKeys eof;
	EXPECT_FALSE(GameEngine::awaitResume(eof, input, [] {}));
}

TEST(AwaitResume, MovesAndRestartsWhilePausedAreIgnored) {
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	keys.waits = {charKey('a'), charKey('r'), charKey('m'), charKey('p')};
	EXPECT_TRUE(GameEngine::awaitResume(keys, input, [] {}));
}

TEST(ReadTick, PauseAndLostFocusAskForAPause) {
	CompositeInputHandler input = makeInput();
	ScriptedPolls keys;
	keys.polls = {{charKey('p')}, {specialKey(KeyKind::FocusLost)}, {specialKey(KeyKind::Escape)}, {charKey('d')}};
	EXPECT_TRUE(GameEngine::readTick(keys, input).pause);
	EXPECT_TRUE(GameEngine::readTick(keys, input).pause);
	EXPECT_TRUE(GameEngine::readTick(keys, input).pause);
	EXPECT_FALSE(GameEngine::readTick(keys, input).pause);
}

TEST(Pause, PausedGameKeepsGoingAndIsSavedWhenItEnds) {
	TempDir dir;
	ScriptedPolls keys;
	keys.polls = {{charKey('p')}};
	keys.waits = {charKey('p')};  // resume, then the snake runs into the wall
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	ScoreManager store(dir.path.string());
	GameEngine engine(keys, input, renderer, store, 20, 20, 0, 42);

	engine.run();

	EXPECT_EQ(renderer.pausedPanels, 1);
	EXPECT_EQ(renderer.gameOvers, 1);
	ScoreManager scores(dir.path.string());
	EXPECT_EQ(scores.stats(Level::Normal).games, 1u);
}

TEST(Pause, QuittingWhilePausedAbandonsTheGame) {
	TempDir dir;
	ScriptedPolls keys;
	keys.polls = {{charKey('p')}};
	keys.waits = {charKey('q')};
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	ScoreManager store(dir.path.string());
	GameEngine engine(keys, input, renderer, store, 20, 20, 0, 42);

	engine.run();

	EXPECT_EQ(renderer.pausedPanels, 1);
	EXPECT_EQ(renderer.gameOvers, 1);
	ScoreManager scores(dir.path.string());
	EXPECT_EQ(scores.stats(Level::Normal).games, 0u);
}

TEST(Pause, TheBoardIsDrawnAgainFromScratchOnResume) {
	TempDir dir;
	ScriptedPolls keys;
	keys.polls = {{charKey('p')}};
	keys.waits = {charKey('p')};
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	ScoreManager store(dir.path.string());
	GameEngine engine(keys, input, renderer, store, 20, 20, 0, 42);

	engine.run();

	EXPECT_GE(renderer.resets, 1);
}

TEST(Pause, TheMusicStopsWhilePausedAndGoesOnFromWhereItWasInsteadOfPickingAnotherPiece) {
	TempDir dir;
	ScriptedPolls keys;
	keys.polls = {{charKey('p')}};
	keys.waits = {charKey('p')};
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	MusicLog sound;
	ScoreManager store(dir.path.string());
	GameEngine engine(keys, input, renderer, store, 20, 20, 0, 42);
	engine.setSound(&sound);

	engine.run();

	EXPECT_EQ(sound.events, (std::vector<std::string>{"start", "stop", "resume", "stop"}));
}
