/**
 * turn_queue_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests that turns pressed within one step are kept and made one per step
 */
#include <gtest/gtest.h>

#include <deque>
#include <vector>

#include "engine_fakes.hpp"
#include "snake_game/game/game_engine.hpp"
#include "snake_game/score/score_manager.hpp"

using namespace fakes;

namespace {

using Turns = std::deque<Direction>;
using Moves = std::vector<Direction>;

}  // namespace

TEST(QueueTurns, TwoTurnsInOneTickAreBothKept) {
	Turns q;
	GameEngine::queueTurns(q, Direction::RIGHT, Moves{Direction::UP, Direction::LEFT});
	EXPECT_EQ(q, (Turns{Direction::UP, Direction::LEFT}));
}

TEST(QueueTurns, OppositeOfTheCurrentDirectionIsIgnored) {
	Turns q;
	GameEngine::queueTurns(q, Direction::RIGHT, Moves{Direction::LEFT});
	EXPECT_TRUE(q.empty());
}

TEST(QueueTurns, TheSameDirectionIsNotQueued) {
	Turns q;
	GameEngine::queueTurns(q, Direction::RIGHT, Moves{Direction::RIGHT, Direction::RIGHT});
	EXPECT_TRUE(q.empty());
	GameEngine::queueTurns(q, Direction::RIGHT, Moves{Direction::UP, Direction::UP, Direction::UP});
	EXPECT_EQ(q, (Turns{Direction::UP}));
}

TEST(QueueTurns, EachTurnIsCheckedAgainstTheLastOneQueued) {
	Turns q;
	// Up is fine after Right; Down is the opposite of the Up already waiting, so it is dropped
	GameEngine::queueTurns(q, Direction::RIGHT, Moves{Direction::UP, Direction::DOWN});
	EXPECT_EQ(q, (Turns{Direction::UP}));
}

TEST(QueueTurns, AFullQueueIgnoresTheNewTurns) {
	Turns q;
	GameEngine::queueTurns(q, Direction::RIGHT, Moves{Direction::UP, Direction::LEFT, Direction::DOWN});
	EXPECT_EQ(q, (Turns{Direction::UP, Direction::LEFT}));
	GameEngine::queueTurns(q, Direction::RIGHT, Moves{Direction::DOWN});
	EXPECT_EQ(q, (Turns{Direction::UP, Direction::LEFT}));
}

TEST(QueueTurns, TurnsPressedLaterJoinWhatIsWaiting) {
	Turns q{Direction::UP};
	GameEngine::queueTurns(q, Direction::RIGHT, Moves{Direction::LEFT});
	EXPECT_EQ(q, (Turns{Direction::UP, Direction::LEFT}));
}

namespace {

/**
 * Key source that hands out one batch of keys per poll()
 *
 * Attributes:
 * std::deque<std::vector<Key>> polls: What each poll() returns, in order; empty batches afterwards
 */
class Batches : public ScriptedKeys {
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

}  // namespace

TEST(TurnQueue, UpThenLeftInOneTickMakesBothTurns) {
	TempDir dir;
	Batches keys;
	keys.polls = {{charKey('w'), charKey('a')}};
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	ScoreManager store(dir.path.string());
	GameEngine engine(keys, input, renderer, store, 20, 20, 0, 42);

	engine.run();

	// heads[0] is the board before the first step: the snake starts in the middle heading right
	ASSERT_GE(renderer.heads.size(), 3u);
	EXPECT_EQ(renderer.heads[1], (std::pair<int, int>{10, 9}));
	EXPECT_EQ(renderer.heads[2], (std::pair<int, int>{9, 9}));
}
