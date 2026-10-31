/**
 * level_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests for the Level helpers
 */
#include <gtest/gtest.h>

#include "snake_game/game/level.hpp"

TEST(Level, Names) {
	EXPECT_STREQ(levelName(Level::Easy), "Easy");
	EXPECT_STREQ(levelName(Level::Normal), "Normal");
	EXPECT_STREQ(levelName(Level::Hard), "Hard");
}

TEST(Level, Speeds) {
	EXPECT_EQ(levelSpeedMs(Level::Easy), 300);
	EXPECT_EQ(levelSpeedMs(Level::Normal), 200);
	EXPECT_EQ(levelSpeedMs(Level::Hard), 100);
}

TEST(Level, NextWraps) {
	EXPECT_EQ(nextLevel(Level::Normal, 1), Level::Hard);
	EXPECT_EQ(nextLevel(Level::Hard, 1), Level::Easy);
	EXPECT_EQ(nextLevel(Level::Easy, -1), Level::Hard);
	EXPECT_EQ(nextLevel(Level::Hard, -1), Level::Normal);
}

TEST(Level, CodesRoundTrip) {
	for (Level l : {Level::Easy, Level::Normal, Level::Hard}) {
		std::optional<Level> back = levelFromCode(levelCode(l));
		ASSERT_TRUE(back.has_value());
		EXPECT_EQ(*back, l);
	}
	EXPECT_EQ(levelCode(Level::Easy), 'E');
	EXPECT_EQ(levelCode(Level::Normal), 'N');
	EXPECT_EQ(levelCode(Level::Hard), 'H');
}

TEST(Level, UnknownCodesAreRejected) {
	EXPECT_EQ(levelFromCode('x'), std::nullopt);
	EXPECT_EQ(levelFromCode('n'), std::nullopt);
	EXPECT_EQ(levelFromCode('\0'), std::nullopt);
}
