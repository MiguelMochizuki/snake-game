/**
 * sound_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests for the sound implementations
 */
#include <gtest/gtest.h>

#include <sstream>

#include "snake_game/render/sound.hpp"

TEST(TerminalBell, EatWritesOneBell) {
	std::ostringstream os;
	TerminalBell bell(os);
	ISound& sound = bell;
	sound.eat();
	EXPECT_EQ(os.str(), "\a");
}

TEST(TerminalBell, GameOverWritesAnotherBell) {
	std::ostringstream os;
	TerminalBell bell(os);
	ISound& sound = bell;
	sound.eat();
	sound.gameOver();
	EXPECT_EQ(os.str(), "\a\a");
}

TEST(NullSound, WritesNothingAndNeedsNoStream) {
	NullSound quiet;
	ISound& sound = quiet;
	sound.eat();
	sound.gameOver();
	SUCCEED();
}
