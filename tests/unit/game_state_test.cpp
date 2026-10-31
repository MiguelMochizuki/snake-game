/**
 * game_state_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests pinning GameState behavior
 */
#include <gtest/gtest.h>

#include "snake_game/game/game_state.hpp"

#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

const int DX[4] = {0, 0, -1, 1};
const int DY[4] = {-1, 1, 0, 0};

/**
 * Moves the snake n times in one direction
 *
 * Parameters:
 * GameState& gs: The game state to move
 * int dir: The direction (0=UP, 1=DOWN, 2=LEFT, 3=RIGHT)
 * int n: Number of moves
 *
 * Throws std::runtime_error: If any move collides (propagated from moveSnake)
 */
void step(GameState& gs, int dir, int n) {
	for (int i = 0; i < n; ++i) {
		gs.moveSnake(dir);
	}
}

/**
 * Steers the head to the food along a shortest path that avoids the body
 *
 * Parameters:
 * GameState& gs: The game state to steer (ends having eaten the food)
 * bool* lastAte: If not null, receives the result of the final moveSnake call
 *
 * Returns int: The direction of the final (eating) move, or -1 if unreachable
 */
int steerToFood(GameState& gs, bool* lastAte = nullptr) {
	const int rows = gs.getRows();
	const int cols = gs.getCols();
	const int hx = gs.getSnake().getHead()->getX();
	const int hy = gs.getSnake().getHead()->getY();
	const int fx = gs.getFood().getX();
	const int fy = gs.getFood().getY();

	std::vector<int> from(rows * cols, -1);
	std::vector<bool> seen(rows * cols, false);
	std::queue<int> q;
	seen[hy * cols + hx] = true;
	q.push(hy * cols + hx);
	while (!q.empty() && !seen[fy * cols + fx]) {
		int c = q.front();
		q.pop();
		for (int d = 0; d < 4; ++d) {
			int nx = c % cols + DX[d];
			int ny = c / cols + DY[d];
			if (nx < 0 || nx >= cols || ny < 0 || ny >= rows || seen[ny * cols + nx]) {
				continue;
			}
			if (gs.getSnake().hasPosition(nx, ny)) {
				continue;
			}
			seen[ny * cols + nx] = true;
			from[ny * cols + nx] = d;
			q.push(ny * cols + nx);
		}
	}
	if (!seen[fy * cols + fx]) {
		return -1;
	}
	std::vector<int> path;
	for (int x = fx, y = fy; x != hx || y != hy;) {
		int d = from[y * cols + x];
		path.push_back(d);
		x -= DX[d];
		y -= DY[d];
	}
	bool ate = false;
	for (auto it = path.rbegin(); it != path.rend(); ++it) {
		ate = gs.moveSnake(*it);
	}
	if (lastAte) {
		*lastAte = ate;
	}
	return path.front();  // path was built from the food backwards
}

}  // namespace

TEST(GameState, InitialSnakeCentered) {
	GameState gs(20, 20, 7, 1);
	EXPECT_EQ(gs.getSnake().getHead()->getX(), 10);
	EXPECT_EQ(gs.getSnake().getHead()->getY(), 10);
	EXPECT_EQ(gs.getSnake().getLength(), 3u);
	EXPECT_EQ(gs.getScore(), 0);
	EXPECT_EQ(gs.getBestScore(), 7);
}

TEST(GameState, FoodNeverOnSnake) {
	for (unsigned seed = 0; seed < 200; ++seed) {
		// Known quirk, pinned by InitialFoodCanLandOnSnakeQuirk: seed 92 places
		// the first food on the snake body.
		if (seed == 92) {
			continue;
		}
		GameState gs(20, 20, 0, seed);
		EXPECT_FALSE(gs.getSnake().hasPosition(gs.getFood().getX(), gs.getFood().getY())) << "seed " << seed;
	}
}

// Pinned quirk: constructor/reset call spawnFood() before updateOccupied(), so the first food ignores the snake. Fixing this flips this test.
TEST(GameState, InitialFoodCanLandOnSnakeQuirk) {
	GameState gs(20, 20, 0, 92);
	EXPECT_TRUE(gs.getSnake().hasPosition(gs.getFood().getX(), gs.getFood().getY()));
}

// Pinned quirk: reset() spawns food against the stale mask of the previous game. Fixing this flips this test.
TEST(GameState, ResetFoodCanLandOnSnakeQuirk) {
	GameState gs(20, 20, 0, 57);
	step(gs, 0, 4);
	gs.reset();
	EXPECT_TRUE(gs.getSnake().hasPosition(gs.getFood().getX(), gs.getFood().getY()));
}

TEST(GameState, IsOccupiedOutOfBounds) {
	GameState gs(20, 20, 0, 1);
	EXPECT_TRUE(gs.isOccupied(-1, 0));
	EXPECT_TRUE(gs.isOccupied(20, 0));
	EXPECT_TRUE(gs.isOccupied(0, -1));
	EXPECT_TRUE(gs.isOccupied(0, 20));
}

TEST(GameState, IsOccupiedFoodCell) {
	GameState gs(20, 20, 0, 1);
	EXPECT_TRUE(gs.isOccupied(gs.getFood().getX(), gs.getFood().getY()));
}

TEST(GameState, WallCollisionRight) {
	GameState gs(20, 20, 0, 1);
	EXPECT_NO_THROW(step(gs, 3, 9));
	EXPECT_THROW(gs.moveSnake(3), std::runtime_error);
}

TEST(GameState, WallCollisionUp) {
	GameState gs(20, 20, 0, 1);
	EXPECT_NO_THROW(step(gs, 0, 10));
	EXPECT_THROW(gs.moveSnake(0), std::runtime_error);
}

TEST(GameState, WallCollisionDown) {
	GameState gs(20, 20, 0, 1);
	EXPECT_NO_THROW(step(gs, 1, 9));
	EXPECT_THROW(gs.moveSnake(1), std::runtime_error);
}

TEST(GameState, WallCollisionLeft) {
	GameState gs(20, 20, 0, 1);
	EXPECT_NO_THROW(step(gs, 0, 1));
	EXPECT_NO_THROW(step(gs, 2, 10));
	EXPECT_THROW(gs.moveSnake(2), std::runtime_error);
}

TEST(GameState, ImmediateReverseIsSelfCollision) {
	GameState gs(20, 20, 0, 1);
	EXPECT_THROW(gs.moveSnake(2), std::runtime_error);
}

TEST(GameState, MoveIntoTailCellIsAllowed) {
	GameState gs(20, 20, 0, 42);
	int d = steerToFood(gs);
	ASSERT_GE(d, 0);
	ASSERT_EQ(gs.getSnake().getLength(), 4u);
	const int hx = gs.getSnake().getHead()->getX();
	const int hy = gs.getSnake().getHead()->getY();
	const int opposite[4] = {1, 0, 3, 2};
	const int t = (d == 0 || d == 1) ? 2 : 0;  // perpendicular turn kept in bounds below
	ASSERT_TRUE(hx + DX[t] >= 0 && hy + DY[t] >= 0);
	// 2x2 loop: turn, back, turn back, forward -> head lands on the old head cell (now the tail)
	ASSERT_NO_THROW(gs.moveSnake(t));
	ASSERT_NO_THROW(gs.moveSnake(opposite[d]));
	ASSERT_NO_THROW(gs.moveSnake(opposite[t]));
	EXPECT_NO_THROW(gs.moveSnake(d));
	EXPECT_EQ(gs.getSnake().getHead()->getX(), hx);
	EXPECT_EQ(gs.getSnake().getHead()->getY(), hy);
}

TEST(GameState, EatingIncrementsScoreAndGrows) {
	GameState gs(20, 20, 0, 42);
	const int fx = gs.getFood().getX();
	const int fy = gs.getFood().getY();
	bool ate = false;
	ASSERT_GE(steerToFood(gs, &ate), 0);
	EXPECT_TRUE(ate);
	EXPECT_EQ(gs.getScore(), 1);
	EXPECT_EQ(gs.getSnake().getLength(), 4u);
	EXPECT_TRUE(gs.getFood().getX() != fx || gs.getFood().getY() != fy);
}

TEST(GameState, BestScoreFollowsScore) {
	GameState gs(20, 20, 0, 42);
	ASSERT_GE(steerToFood(gs), 0);
	EXPECT_EQ(gs.getBestScore(), 1);
}

TEST(GameState, ResetKeepsBestScore) {
	GameState gs(20, 20, 0, 42);
	ASSERT_GE(steerToFood(gs), 0);
	gs.reset();
	EXPECT_EQ(gs.getScore(), 0);
	EXPECT_EQ(gs.getSnake().getLength(), 3u);
	EXPECT_EQ(gs.getSnake().getHead()->getX(), 10);
	EXPECT_EQ(gs.getSnake().getHead()->getY(), 10);
	EXPECT_EQ(gs.getBestScore(), 1);
}

TEST(GameState, InvalidDirectionThrowsSelfCollision) {
	GameState gs(20, 20, 0, 1);
	try {
		gs.moveSnake(4);
		FAIL() << "expected exception";
	} catch (const std::runtime_error& e) {
		EXPECT_NE(std::string(e.what()).find("Self collision"), std::string::npos);
	}
}

TEST(GameState, BoardFullThrows) {
	// Seed 5 puts the first food on (3,0), the only cell not covered by the snake
	GameState gs(1, 4, 0, 5);
	ASSERT_EQ(gs.getFood().getX(), 3);
	ASSERT_EQ(gs.getFood().getY(), 0);
	try {
		gs.moveSnake(3);
		FAIL() << "expected exception";
	} catch (const std::runtime_error& e) {
		EXPECT_STREQ(e.what(), "Board is full!");
	}
}
