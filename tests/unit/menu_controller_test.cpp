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

#include "arrow_input_handler.hpp"
#include "command_input_handler.hpp"
#include "composite_input_handler.hpp"
#include "menu_controller.hpp"
#include "wasd_input_handler.hpp"

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
};

/**
 * Fixture holding the real WASD, Arrow and Command composite
 */
class MenuControllerTest : public ::testing::Test {
protected:
	CompositeInputHandler input;
	MenuModel model;
	MenuStats stats{42, 7, 4.3};
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
		return runMenu(keys, input, view, model, stats);
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
	EXPECT_EQ(view.shown[2].stats.best, 42);
	EXPECT_EQ(view.shown[3].selected, MenuItem::Start);
}
