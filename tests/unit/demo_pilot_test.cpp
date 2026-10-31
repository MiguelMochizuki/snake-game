/**
 * demo_pilot_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the demo autopilot; built only with SNAKE_GAME_BUILD_GUI=ON
 */
#include <gtest/gtest.h>

#include <vector>

#include "snake_game/game/game_state.hpp"
#include "snake_game/gui/demo_pilot.hpp"

namespace {

/**
 * Empty board with the head and food placed
 *
 * Parameters:
 * int rows, int cols: Board size
 * int hx, int hy: Head position
 * int fx, int fy: Food position
 *
 * Returns DemoBoard: The board; blocked cells are added by the test
 */
DemoBoard board(int rows, int cols, int hx, int hy, int fx, int fy) {
	return DemoBoard{rows, cols, std::vector<bool>(static_cast<size_t>(rows) * cols, false), hx, hy, fx, fy};
}

}  // namespace

TEST(DemoPilot, HeadsTowardsFoodInOpenBoard) {
	EXPECT_EQ(demoMoveOn(board(10, 10, 2, 5, 7, 5)), Direction::RIGHT);
	EXPECT_EQ(demoMoveOn(board(10, 10, 2, 5, 2, 1)), Direction::UP);
	EXPECT_EQ(demoMoveOn(board(10, 10, 6, 2, 6, 8)), Direction::DOWN);
	EXPECT_EQ(demoMoveOn(board(10, 10, 6, 2, 1, 2)), Direction::LEFT);
}

TEST(DemoPilot, NeverStepsIntoWallOrBody) {
	DemoBoard b = board(10, 10, 0, 0, 5, 5);
	b.blocked[0 * 10 + 1] = true;  // body to the right of the head
	EXPECT_EQ(demoMoveOn(b), Direction::DOWN);
}

TEST(DemoPilot, GoesAroundAnObstacle) {
	DemoBoard b = board(10, 10, 2, 5, 7, 5);
	b.blocked[5 * 10 + 3] = true;  // body straight ahead
	std::optional<Direction> move = demoMoveOn(b);
	ASSERT_TRUE(move.has_value());
	EXPECT_TRUE(*move == Direction::UP || *move == Direction::DOWN);
}

TEST(DemoPilot, FallsBackWhenFoodUnreachable) {
	DemoBoard b = board(10, 10, 0, 0, 5, 5);
	for (int y = 0; y < 10; ++y) {
		b.blocked[y * 10 + 3] = true;  // a wall of body cells splits the board
	}
	std::optional<Direction> move = demoMoveOn(b);
	ASSERT_TRUE(move.has_value());
	EXPECT_TRUE(*move == Direction::RIGHT || *move == Direction::DOWN);
}

TEST(DemoPilot, NulloptWhenBoxedIn) {
	DemoBoard b = board(10, 10, 0, 0, 5, 5);
	b.blocked[0 * 10 + 1] = true;
	b.blocked[1 * 10 + 0] = true;
	EXPECT_FALSE(demoMoveOn(b).has_value());
}

TEST(DemoPilot, PlaysARealStateWithoutDying) {
	GameState state(20, 20, 0, 42);
	for (int step = 0; step < 300; ++step) {
		std::optional<Direction> move = demoMove(state);
		ASSERT_TRUE(move.has_value()) << "step " << step;
		ASSERT_NO_THROW(state.moveSnake(static_cast<int>(*move))) << "step " << step;
	}
	EXPECT_GT(state.getScore(), 0);
}
