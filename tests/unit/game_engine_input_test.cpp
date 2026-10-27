/**
 * game_engine_input_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests for GameEngine input helpers using a scripted key source
 */
#include <gtest/gtest.h>

#include <deque>
#include <memory>

#include "arrow_input_handler.hpp"
#include "command_input_handler.hpp"
#include "composite_input_handler.hpp"
#include "game_engine.hpp"
#include "wasd_input_handler.hpp"

/**
 * Scripted key source
 *
 * Attributes:
 * std::deque<std::vector<Key>> batches: Results of successive poll() calls
 * std::deque<std::optional<Key>> waits: Results of successive waitKey() calls, nullopt once empty
 * std::deque<Key> stale: Keys waitKey() serves before `waits`; flush() discards them
 * int flushCount: Number of flush() calls
 */
class FakeKeySource : public IKeySource {
public:
	std::deque<std::vector<Key>> batches;
	std::deque<std::optional<Key>> waits;
	std::deque<Key> stale;
	int flushCount = 0;

	/**
	 * Returns the next scripted batch
	 *
	 * Returns std::vector<Key>: The batch, or empty when the script is exhausted
	 */
	std::vector<Key> poll() override {
		if (batches.empty()) {
			return {};
		}
		std::vector<Key> b = batches.front();
		batches.pop_front();
		return b;
	}

	/**
	 * Returns the next scripted key
	 *
	 * Returns std::optional<Key>: The key, or nullopt when the script is exhausted
	 */
	std::optional<Key> waitKey() override {
		if (!stale.empty()) {
			Key k = stale.front();
			stale.pop_front();
			return k;
		}
		if (waits.empty()) {
			return std::nullopt;
		}
		std::optional<Key> k = waits.front();
		waits.pop_front();
		return k;
	}

	/**
	 * Counts the call and discards the stale keys
	 */
	void flush() override {
		stale.clear();
		++flushCount;
	}
};

/**
 * Fixture holding the real WASD, Arrow and Command composite
 *
 * Attributes:
 * CompositeInputHandler input: The composite handler
 * FakeKeySource keys: The scripted key source
 */
class EngineInputTest : public ::testing::Test {
protected:
	CompositeInputHandler input;
	FakeKeySource keys;

	/**
	 * Registers the three handlers
	 */
	void SetUp() override {
		input.add(std::make_unique<WASDInputHandler>());
		input.add(std::make_unique<ArrowInputHandler>());
		input.add(std::make_unique<CommandInputHandler>());
	}
};

TEST_F(EngineInputTest, ReadTickLastMoveWins) {
	keys.batches = {{charKey('w'), specialKey(KeyKind::Left)}};
	TickInput t = GameEngine::readTick(keys, input);
	EXPECT_FALSE(t.quit);
	ASSERT_TRUE(t.move);
	EXPECT_EQ(*t.move, Direction::LEFT);
}

TEST_F(EngineInputTest, ReadTickQuitWins) {
	keys.batches = {{charKey('d'), charKey('q'), charKey('a')}, {specialKey(KeyKind::CtrlC)}};
	EXPECT_TRUE(GameEngine::readTick(keys, input).quit);
	EXPECT_TRUE(GameEngine::readTick(keys, input).quit);
}

TEST_F(EngineInputTest, ReadTickManyKeys) {
	std::vector<Key> many;
	for (int i = 0; i < 49; ++i) {
		many.push_back(charKey(i % 2 ? 's' : 'w'));
	}
	many.push_back(charKey('s'));
	keys.batches = {many};
	TickInput t = GameEngine::readTick(keys, input);
	ASSERT_TRUE(t.move);
	EXPECT_EQ(*t.move, Direction::DOWN);
}

TEST_F(EngineInputTest, ReadTickIgnoresRestartAndUnknown) {
	keys.batches = {{charKey('r'), specialKey(KeyKind::Unknown), charKey('z')}};
	TickInput t = GameEngine::readTick(keys, input);
	EXPECT_FALSE(t.quit);
	EXPECT_FALSE(t.move);
}

TEST_F(EngineInputTest, AwaitRestartOnR) {
	keys.waits = {charKey('w'), specialKey(KeyKind::Up), specialKey(KeyKind::Unknown), charKey('r')};
	EXPECT_TRUE(GameEngine::awaitRestart(keys, input));
}

TEST_F(EngineInputTest, AwaitRestartFlushesFirst) {
	keys.stale = {charKey('r')};
	keys.waits = {charKey('q')};
	EXPECT_FALSE(GameEngine::awaitRestart(keys, input));
	EXPECT_EQ(keys.flushCount, 1);
}

TEST_F(EngineInputTest, AwaitQuitOnQAndCtrlC) {
	keys.waits = {charKey('a'), charKey('q')};
	EXPECT_FALSE(GameEngine::awaitRestart(keys, input));
	keys.waits = {specialKey(KeyKind::CtrlC)};
	EXPECT_FALSE(GameEngine::awaitRestart(keys, input));
}

TEST_F(EngineInputTest, AwaitEofQuits) {
	EXPECT_FALSE(GameEngine::awaitRestart(keys, input));
}

TEST(GameEngineDirection, IsOppositePairs) {
	const Direction all[] = {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT};
	int opposites = 0;
	for (Direction a : all) {
		for (Direction b : all) {
			bool want = (a == Direction::UP && b == Direction::DOWN) || (a == Direction::DOWN && b == Direction::UP) ||
			            (a == Direction::LEFT && b == Direction::RIGHT) || (a == Direction::RIGHT && b == Direction::LEFT);
			EXPECT_EQ(GameEngine::isOpposite(a, b), want);
			opposites += want;
		}
	}
	EXPECT_EQ(opposites, 4);
}
