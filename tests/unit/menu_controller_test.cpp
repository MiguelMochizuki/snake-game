/**
 * menu_controller_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests for runMenu using a scripted key source and a recording view
 */
#include <gtest/gtest.h>

#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "snake_game/input/arrow_input_handler.hpp"
#include "snake_game/input/command_input_handler.hpp"
#include "snake_game/input/composite_input_handler.hpp"
#include "snake_game/menu/menu_controller.hpp"
#include "snake_game/score/score_source.hpp"
#include "snake_game/input/wasd_input_handler.hpp"

namespace {

/**
 * Scripted key source that also records the order of flush and show calls
 *
 * Attributes:
 * std::deque<Key> keys: Keys waitKey() serves, nullopt once empty
 * std::vector<std::string>* log: Shared event log ("flush", "show")
 */
class ScriptedKeys : public IKeySource {
public:
	std::deque<Key> keys;
	std::vector<std::string>* log;

	/**
	 * Parameters:
	 * std::vector<Key> script: The keys to serve
	 * std::vector<std::string>* sharedLog: Where flush() records itself
	 */
	ScriptedKeys(std::vector<Key> script, std::vector<std::string>* sharedLog)
		: keys(script.begin(), script.end()), log(sharedLog) {
	}

	/**
	 * Returns nothing; the menu never polls
	 *
	 * Returns std::vector<Key>: Always empty
	 */
	std::vector<Key> poll() override {
		return {};
	}

	/**
	 * Returns the next scripted key
	 *
	 * Returns std::optional<Key>: The key, or nullopt at end of script
	 */
	std::optional<Key> waitKey() override {
		if (keys.empty()) {
			return std::nullopt;
		}
		Key k = keys.front();
		keys.pop_front();
		return k;
	}

	/**
	 * Records the call
	 */
	void flush() override {
		log->push_back("flush");
	}
};

/**
 * View that records every snapshot it is shown
 *
 * Attributes:
 * std::vector<MenuSnapshot> shown: Snapshots in order
 * std::vector<std::string>* log: Shared event log
 */
class RecordingView : public IMenuView {
public:
	std::vector<MenuSnapshot> shown;
	std::vector<LeaderboardSnapshot> boards;
	int helpShown = 0;
	std::vector<std::string>* log;

	/**
	 * Parameters:
	 * std::vector<std::string>* sharedLog: Where show() records itself
	 */
	explicit RecordingView(std::vector<std::string>* sharedLog) : log(sharedLog) {
	}

	/**
	 * Records the snapshot
	 *
	 * Parameters:
	 * const MenuSnapshot& snapshot: What was drawn
	 */
	void show(const MenuSnapshot& snapshot) override {
		shown.push_back(snapshot);
		log->push_back("show");
	}

	/**
	 * Records the leaderboard
	 *
	 * Parameters:
	 * const LeaderboardSnapshot& snapshot: What was drawn
	 */
	void showLeaderboard(const LeaderboardSnapshot& snapshot) override {
		boards.push_back(snapshot);
		log->push_back("leaderboard");
	}

	/**
	 * Records the help screen
	 */
	void showHelp() override {
		++helpShown;
		log->push_back("help");
	}
};

/**
 * Score source with different figures per level
 */
class FakeScores : public IScoreSource {
public:
	/**
	 * Gets canned figures: Easy 10/1/10.0, Normal 42/7/4.3, Hard 99/3/50.0
	 *
	 * Parameters:
	 * Level level: The level
	 *
	 * Returns ScoreStats: The figures
	 */
	ScoreStats stats(Level level) const override {
		switch (level) {
			case Level::Easy: return ScoreStats{10, 1, 10.0};
			case Level::Normal: return ScoreStats{42, 7, 4.3};
			case Level::Hard: return ScoreStats{99, 3, 50.0};
		}
		return ScoreStats{0, 0, 0.0};
	}

	/**
	 * Gets canned scores: Easy {10}, Normal {42, 30, 30, 7}, Hard none
	 *
	 * Parameters:
	 * Level level: The level
	 * std::size_t n: Maximum number of scores
	 *
	 * Returns std::vector<int>: The scores
	 */
	std::vector<int> topScores(Level level, std::size_t n) const override {
		std::vector<int> all;
		if (level == Level::Easy) all = {10};
		if (level == Level::Normal) all = {42, 30, 30, 7};
		if (all.size() > n) all.resize(n);
		return all;
	}
};

/**
 * Fixture holding the real WASD, Arrow and Command composite
 */
class MenuControllerTest : public ::testing::Test {
protected:
	CompositeInputHandler input;
	MenuModel model;
	FakeScores scores;
	std::vector<std::string> log;

	/**
	 * Registers the three handlers
	 */
	void SetUp() override {
		input.add(std::make_unique<WASDInputHandler>());
		input.add(std::make_unique<ArrowInputHandler>());
		input.add(std::make_unique<CommandInputHandler>());
	}

	/**
	 * Runs the menu on a script
	 *
	 * Parameters:
	 * std::vector<Key> script: Keys to serve
	 * RecordingView& view: View to show on
	 *
	 * Returns MenuResult: What runMenu returned
	 */
	MenuResult run(std::vector<Key> script, RecordingView& view) {
		ScriptedKeys keys(std::move(script), &log);
		return runMenu(keys, input, view, model, scores);
	}
};

Key enter() {
	return specialKey(KeyKind::Enter);
}

}  // namespace

TEST_F(MenuControllerTest, EnterStartsWithNormalSpeed) {
	RecordingView view(&log);
	MenuResult r = run({enter()}, view);
	EXPECT_TRUE(r.start);
	EXPECT_EQ(r.speedMs, 200);
}

TEST_F(MenuControllerTest, LevelChosenWithKeysSetsSpeed) {
	RecordingView view(&log);
	MenuResult r = run({specialKey(KeyKind::Down), specialKey(KeyKind::Right), specialKey(KeyKind::Up), enter()}, view);
	EXPECT_TRUE(r.start);
	EXPECT_EQ(r.speedMs, 100);
}

TEST_F(MenuControllerTest, WasdKeysWorkToo) {
	RecordingView view(&log);
	MenuResult r = run({charKey('s'), charKey('a'), charKey('w'), enter()}, view);
	EXPECT_TRUE(r.start);
	EXPECT_EQ(r.speedMs, 300);
}

TEST_F(MenuControllerTest, QuitRowAndQKeyDoNotStart) {
	RecordingView view(&log);
	EXPECT_FALSE(run({specialKey(KeyKind::Up), enter()}, view).start);
	MenuModel fresh;
	model = fresh;
	RecordingView view2(&log);
	EXPECT_FALSE(run({charKey('q')}, view2).start);
}

TEST_F(MenuControllerTest, CtrlCQuits) {
	RecordingView view(&log);
	EXPECT_FALSE(run({specialKey(KeyKind::CtrlC)}, view).start);
}

TEST_F(MenuControllerTest, EofQuitsWithoutHanging) {
	RecordingView view(&log);
	EXPECT_FALSE(run({}, view).start);
}

TEST_F(MenuControllerTest, UnknownAndUnregisteredKeysAreIgnored) {
	RecordingView view(&log);
	MenuResult r = run({specialKey(KeyKind::Unknown), charKey('x'), charKey('r'), enter()}, view);
	EXPECT_TRUE(r.start);
	EXPECT_EQ(r.speedMs, 200);
	EXPECT_EQ(view.shown.back().selected, MenuItem::Start);
}

TEST_F(MenuControllerTest, FlushesStaleKeysBeforeFirstDraw) {
	RecordingView view(&log);
	run({enter()}, view);
	ASSERT_GE(log.size(), 2u);
	EXPECT_EQ(log[0], "flush");
	EXPECT_EQ(log[1], "show");
}

TEST_F(MenuControllerTest, ShowsOnceUpFrontAndAfterEachKey) {
	RecordingView view(&log);
	run({specialKey(KeyKind::Down), specialKey(KeyKind::Right), specialKey(KeyKind::Up), enter()}, view);
	ASSERT_EQ(view.shown.size(), 4u);
	EXPECT_EQ(view.shown[0].selected, MenuItem::Start);
	EXPECT_EQ(view.shown[1].selected, MenuItem::Level);
	EXPECT_EQ(view.shown[2].level, Level::Hard);
	EXPECT_EQ(view.shown[2].stats.best, 99);  // Hard, the level just chosen
	EXPECT_EQ(view.shown[3].selected, MenuItem::Start);
}

TEST_F(MenuControllerTest, StatsFollowTheLevel) {
	RecordingView view(&log);
	run({specialKey(KeyKind::Down), specialKey(KeyKind::Right), specialKey(KeyKind::Right)}, view);
	ASSERT_GE(view.shown.size(), 3u);
	EXPECT_EQ(view.shown[0].stats.best, 42);
	EXPECT_EQ(view.shown[1].stats.best, 42);
	EXPECT_EQ(view.shown[2].level, Level::Hard);
	EXPECT_EQ(view.shown[2].stats.best, 99);
	EXPECT_EQ(view.shown[3].level, Level::Easy);
	EXPECT_EQ(view.shown[3].stats.best, 10);
}

TEST_F(MenuControllerTest, LeaderboardShowsSwitchesLevelAndGoesBack) {
	RecordingView view(&log);
	MenuResult r = run({specialKey(KeyKind::Down), specialKey(KeyKind::Down), enter(),
		specialKey(KeyKind::Right), enter(), specialKey(KeyKind::Up), specialKey(KeyKind::Up), enter()}, view);
	ASSERT_EQ(view.boards.size(), 2u);
	EXPECT_EQ(view.boards[0].level, Level::Normal);
	EXPECT_EQ(view.boards[0].scores, (std::vector<int>{42, 30, 30, 7}));
	EXPECT_EQ(view.boards[1].level, Level::Hard);
	EXPECT_TRUE(view.boards[1].scores.empty());
	EXPECT_TRUE(r.start);
	EXPECT_EQ(r.level, Level::Normal);
}

TEST_F(MenuControllerTest, HelpShowsAndQGoesBack) {
	RecordingView view(&log);
	MenuResult r = run({specialKey(KeyKind::Down), specialKey(KeyKind::Down), specialKey(KeyKind::Down), enter(),
		charKey('q'), specialKey(KeyKind::Up), specialKey(KeyKind::Up), specialKey(KeyKind::Up), enter()}, view);
	EXPECT_EQ(view.helpShown, 1);
	EXPECT_TRUE(r.start);
}

TEST_F(MenuControllerTest, CtrlCInSubScreensReturnsToTheMenu) {
	RecordingView view(&log);
	MenuResult r = run({specialKey(KeyKind::Down), specialKey(KeyKind::Down), enter(),
		specialKey(KeyKind::CtrlC), specialKey(KeyKind::Up), specialKey(KeyKind::Up), enter()}, view);
	EXPECT_EQ(view.boards.size(), 1u);
	EXPECT_TRUE(r.start);
}

TEST_F(MenuControllerTest, EofInsideSubScreensQuits) {
	RecordingView leaderboardView(&log);
	EXPECT_FALSE(run({specialKey(KeyKind::Down), specialKey(KeyKind::Down), enter()}, leaderboardView).start);
	EXPECT_EQ(leaderboardView.boards.size(), 1u);
	MenuModel fresh;
	model = fresh;
	RecordingView helpView(&log);
	EXPECT_FALSE(run({specialKey(KeyKind::Down), specialKey(KeyKind::Down), specialKey(KeyKind::Down), enter()}, helpView).start);
	EXPECT_EQ(helpView.helpShown, 1);
}

TEST_F(MenuControllerTest, ResultCarriesLevelAndSound) {
	RecordingView view(&log);
	MenuResult r = run({specialKey(KeyKind::Down), specialKey(KeyKind::Right), specialKey(KeyKind::Down),
		specialKey(KeyKind::Down), specialKey(KeyKind::Down), specialKey(KeyKind::Right),
		specialKey(KeyKind::Up), specialKey(KeyKind::Up), specialKey(KeyKind::Up), specialKey(KeyKind::Up),
		enter()}, view);
	EXPECT_TRUE(r.start);
	EXPECT_EQ(r.level, Level::Hard);
	EXPECT_EQ(r.speedMs, 100);
	EXPECT_FALSE(r.sound);
}
