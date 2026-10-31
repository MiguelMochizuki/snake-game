/**
 * snake_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests pinning Snake behavior
 */
#include <gtest/gtest.h>

#include "snake_game/game/snake.hpp"

TEST(Snake, ConstructorLaysBodyLeftOfHead) {
	Snake s(10, 10);
	ASSERT_EQ(s.getLength(), 3u);
	const auto& seg = s.getSegments();
	for (int i = 0; i < 3; ++i) {
		EXPECT_EQ(seg[i].getX(), 10 - i);
		EXPECT_EQ(seg[i].getY(), 10);
	}
}

TEST(Snake, ZeroSizeHasNullHead) {
	Snake s(0, 0, 0);
	EXPECT_EQ(s.getHead(), nullptr);
	EXPECT_EQ(s.getLength(), 0u);
}

TEST(Snake, MoveWithoutFoodKeepsLength) {
	Snake s(10, 10);
	s.move(11, 10, false);
	EXPECT_EQ(s.getLength(), 3u);
	EXPECT_EQ(s.getHead()->getX(), 11);
	EXPECT_EQ(s.getHead()->getY(), 10);
	EXPECT_EQ(s.getSegments().back().getX(), 9);
	EXPECT_EQ(s.getSegments().back().getY(), 10);
}

TEST(Snake, MoveWithFoodGrowsByOne) {
	Snake s(10, 10);
	s.move(11, 10, true);
	EXPECT_EQ(s.getLength(), 4u);
	s.move(12, 10, false);
	EXPECT_EQ(s.getLength(), 4u);
}

TEST(Snake, HasPosition) {
	Snake s(10, 10);
	EXPECT_TRUE(s.hasPosition(10, 10));
	EXPECT_TRUE(s.hasPosition(9, 10));
	EXPECT_TRUE(s.hasPosition(8, 10));
	EXPECT_FALSE(s.hasPosition(0, 0));
}
