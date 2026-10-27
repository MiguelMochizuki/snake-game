/**
 * wasd_input_handler_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests pinning WASD input handling (non-Windows, stdin replaced by a pipe)
 */
#ifndef _WIN32

#include <gtest/gtest.h>

#include <string>
#include <unistd.h>

#include <sstream>

#include "terminal_renderer.hpp"
#include "wasd_input_handler.hpp"

/**
 * Fixture that replaces STDIN_FILENO with a pipe and restores it on teardown
 *
 * Attributes:
 * int savedStdin_: Duplicate of the original stdin
 */
class InputTest : public ::testing::Test {
protected:
	int savedStdin_ = -1;

	/**
	 * Saves the original stdin
	 */
	void SetUp() override {
		savedStdin_ = dup(STDIN_FILENO);
		ASSERT_NE(savedStdin_, -1);
	}

	/**
	 * Restores the original stdin
	 */
	void TearDown() override {
		if (savedStdin_ != -1) {
			dup2(savedStdin_, STDIN_FILENO);
			close(savedStdin_);
		}
	}

	/**
	 * Makes stdin a pipe pre-filled with the given bytes
	 *
	 * Parameters:
	 * const std::string& bytes: Bytes to make readable
	 * bool closeWrite: Close the write end so reads hit EOF (default true)
	 */
	void feed(const std::string& bytes, bool closeWrite = true) {
		int fds[2];
		ASSERT_EQ(pipe(fds), 0);
		if (!bytes.empty()) {
			ASSERT_EQ(write(fds[1], bytes.data(), bytes.size()), static_cast<ssize_t>(bytes.size()));
		}
		if (closeWrite) {
			close(fds[1]);
		}
		dup2(fds[0], STDIN_FILENO);
		close(fds[0]);
	}
};

TEST_F(InputTest, LastValidKeyWins) {
	feed("wad");
	WASDInputHandler h;
	h.processInput();
	EXPECT_EQ(h.getDirection(), Direction::RIGHT);
	EXPECT_TRUE(h.directionChanged());
}

TEST_F(InputTest, QuitOverridesDirection) {
	feed("wqa");
	WASDInputHandler h;
	h.processInput();
	EXPECT_TRUE(h.shouldQuit());
	EXPECT_FALSE(h.directionChanged());
}

TEST_F(InputTest, UppercaseAccepted) {
	feed("S");
	WASDInputHandler h;
	h.processInput();
	EXPECT_EQ(h.getDirection(), Direction::DOWN);
}

TEST_F(InputTest, OtherKeysIgnored) {
	feed("xz1");
	WASDInputHandler h;
	h.processInput();
	EXPECT_FALSE(h.directionChanged());
	EXPECT_EQ(h.getDirection(), Direction::RIGHT);
}

TEST_F(InputTest, NoInput) {
	feed("");
	WASDInputHandler h;
	h.processInput();
	EXPECT_FALSE(h.directionChanged());
	EXPECT_FALSE(h.shouldQuit());
}

TEST_F(InputTest, ResetFlags) {
	feed("q");
	WASDInputHandler h;
	h.processInput();
	feed("w");
	h.processInput();
	EXPECT_TRUE(h.shouldQuit());
	EXPECT_TRUE(h.directionChanged());
	h.resetQuit();
	h.resetDirectionChanged();
	EXPECT_FALSE(h.shouldQuit());
	EXPECT_FALSE(h.directionChanged());
}

TEST_F(InputTest, ReadCharBlockingReturnsNext) {
	feed("r");
	WASDInputHandler h;
	EXPECT_EQ(h.readCharBlocking(), 'r');
}

// Pinned quirk: EOF yields 0
TEST_F(InputTest, ReadCharBlockingEofReturnsZero) {
	feed("");
	WASDInputHandler h;
	EXPECT_EQ(h.readCharBlocking(), 0);
}

// Game-over screen must quit on EOF instead of spinning forever
TEST_F(InputTest, ShowGameOverReturnsFalseOnEof) {
	ASSERT_NO_FATAL_FAILURE(feed(""));
	WASDInputHandler h;
	std::ostringstream os;
	TerminalRenderer r(os);
	EXPECT_FALSE(r.showGameOver(0, 0, 1, 0.0, h));
}

#endif
