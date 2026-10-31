/**
 * terminal_key_source_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests for TerminalKeySource (POSIX, stdin replaced by a pipe)
 */
#ifndef _WIN32

#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <unistd.h>

#include "snake_game/input/terminal_key_source.hpp"

/**
 * Fixture that replaces STDIN_FILENO with a pipe and restores it on teardown
 *
 * Attributes:
 * int savedStdin_: Duplicate of the original stdin
 */
class KeySourceTest : public ::testing::Test {
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

TEST_F(KeySourceTest, PollDecodesPipeBytes) {
	feed("w\x1b[Dq");
	TerminalKeySource ks;
	std::vector<Key> want = {charKey('w'), specialKey(KeyKind::Left), charKey('q')};
	EXPECT_EQ(ks.poll(), want);
}

TEST_F(KeySourceTest, WaitKeyReturnsNext) {
	feed("r");
	TerminalKeySource ks;
	EXPECT_EQ(ks.waitKey(), charKey('r'));
}

TEST_F(KeySourceTest, WaitKeyEofIsNullopt) {
	feed("");
	TerminalKeySource ks;
	EXPECT_FALSE(ks.waitKey());
}

TEST_F(KeySourceTest, WaitKeyQueuesRest) {
	feed("ab");
	TerminalKeySource ks;
	EXPECT_EQ(ks.waitKey(), charKey('a'));
	EXPECT_EQ(ks.waitKey(), charKey('b'));
	EXPECT_FALSE(ks.waitKey());
}

TEST_F(KeySourceTest, FlushDiscards) {
	feed("abc", false);
	TerminalKeySource ks;
	ks.flush();
	EXPECT_TRUE(ks.poll().empty());
}

#endif
