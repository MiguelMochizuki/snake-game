/**
 * terminal_size_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the terminal size check
 */
#include <gtest/gtest.h>

#include "snake_game/terminal/terminal_size.hpp"

TEST(TerminalSize, DefaultTerminalFits) {
	EXPECT_FALSE(terminalTooSmall(TerminalSize{24, 80}));
	EXPECT_FALSE(terminalTooSmall(MIN_TERMINAL_SIZE));
}

TEST(TerminalSize, TooFewLinesOrColumnsIsTooSmall) {
	EXPECT_TRUE(terminalTooSmall(TerminalSize{23, 80}));
	EXPECT_TRUE(terminalTooSmall(TerminalSize{24, 29}));
	EXPECT_TRUE(terminalTooSmall(TerminalSize{10, 20}));
}

TEST(TerminalSize, NoticeSaysWhatIsAndWhatIsNeeded) {
	const std::string text = tooSmallNotice(TerminalSize{20, 70});
	EXPECT_NE(text.find("70x20"), std::string::npos);
	EXPECT_NE(text.find("30 columns by 24 lines"), std::string::npos);
	EXPECT_EQ(text.back(), '\n');
}
