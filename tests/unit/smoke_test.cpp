/**
 * smoke_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Smoke test verifying the test binary links against snake_core
 */
#include <gtest/gtest.h>

#include "snake_game/game/snake.hpp"

TEST(Smoke, SnakeCoreLinks) {
	Snake s(5, 5);
	EXPECT_EQ(s.getLength(), 3u);
}
