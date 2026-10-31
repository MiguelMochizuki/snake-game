/**
 * terminal_menu_view_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Golden and behaviour tests for TerminalMenuView
 */
#include <gtest/gtest.h>

#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "golden.hpp"
#include "snake_game/menu/terminal_menu_view.hpp"

namespace {

const std::string CLEAR = "\033[2J\033[H";

/**
 * Renders a snapshot through a TerminalMenuView
 *
 * Parameters:
 * const MenuSnapshot& snapshot: What to draw
 * bool color: Whether the view emits colors
 *
 * Returns std::string: Everything the view wrote
 */
std::string render(const MenuSnapshot& snapshot, bool color) {
	std::ostringstream os;
	TerminalMenuView view(os, color);
	view.show(snapshot);
	return os.str();
}

/**
 * Removes SGR color sequences and the leading screen clear
 *
 * Parameters:
 * const std::string& s: Raw view output
 *
 * Returns std::string: Plain text
 */
std::string plain(const std::string& s) {
	std::string t = std::regex_replace(s, std::regex("\033\\[[0-9;]*m"), "");
	if (t.rfind(CLEAR, 0) == 0) {
		t.erase(0, CLEAR.size());
	}
	return t;
}

/**
 * Splits text into lines
 *
 * Parameters:
 * const std::string& s: The text
 *
 * Returns std::vector<std::string>: The lines without newlines
 */
std::vector<std::string> lines(const std::string& s) {
	std::vector<std::string> out;
	std::istringstream in(s);
	std::string line;
	while (std::getline(in, line)) {
		out.push_back(line);
	}
	return out;
}

/**
 * Finds the first line containing a text
 *
 * Parameters:
 * const std::vector<std::string>& ls: The lines
 * const std::string& needle: Text to look for
 *
 * Returns std::string: The line, or empty if none contains it
 */
std::string lineWith(const std::vector<std::string>& ls, const std::string& needle) {
	for (const std::string& l : ls) {
		if (l.find(needle) != std::string::npos) {
			return l;
		}
	}
	return "";
}

/**
 * Renders the leaderboard through a TerminalMenuView
 *
 * Parameters:
 * const LeaderboardSnapshot& snapshot: What to draw
 * bool color: Whether the view emits colors
 *
 * Returns std::string: Everything the view wrote
 */
std::string renderBoard(const LeaderboardSnapshot& snapshot, bool color) {
	std::ostringstream os;
	TerminalMenuView view(os, color);
	view.showLeaderboard(snapshot);
	return os.str();
}

/**
 * Renders the help screen through a TerminalMenuView
 *
 * Parameters:
 * bool color: Whether the view emits colors
 *
 * Returns std::string: Everything the view wrote
 */
std::string renderHelp(bool color) {
	std::ostringstream os;
	TerminalMenuView view(os, color);
	view.showHelp();
	return os.str();
}

/**
 * Checks the shared rules of every menu screen: framed rows exactly 22 columns, other lines at most 22,
 * and no more than 24 lines in total
 *
 * Parameters:
 * const std::string& raw: Raw view output
 */
void expectFits(const std::string& raw) {
	std::vector<std::string> ls = lines(plain(raw));
	int framed = 0;
	for (const std::string& l : ls) {
		EXPECT_LE(l.size(), 22u) << l;
		if (!l.empty() && l[0] == '#') {
			EXPECT_EQ(l.size(), 22u) << l;
			EXPECT_EQ(l.back(), '#') << l;
			++framed;
		}
	}
	EXPECT_GT(framed, 0);
	EXPECT_LE(ls.size(), 24u);
}

const MenuSnapshot DEFAULT_SNAPSHOT{MenuItem::Start, Level::Normal, ScoreStats{42, 7, 4.3}, true};

}  // namespace

TEST(TerminalMenuView, PlainMatchesGolden) {
	expectGolden("menu_default.txt", render(DEFAULT_SNAPSHOT, false));
}

TEST(TerminalMenuView, ColorMatchesGolden) {
	expectGolden("menu_default_color.txt", render(DEFAULT_SNAPSHOT, true));
}

TEST(TerminalMenuView, ColorStripsToPlain) {
	EXPECT_EQ(plain(render(DEFAULT_SNAPSHOT, true)), plain(render(DEFAULT_SNAPSHOT, false)));
}

TEST(TerminalMenuView, FrameRowsAreExactly22ColumnsAndFitTheBoard) {
	for (MenuItem item : {MenuItem::Start, MenuItem::Level, MenuItem::Quit}) {
		for (Level level : {Level::Easy, Level::Normal, Level::Hard}) {
			std::vector<std::string> ls = lines(plain(render({item, level, {123456, 99999, 1234.5}, true}, false)));
			int framed = 0;
			for (const std::string& l : ls) {
				EXPECT_LE(l.size(), 22u) << l;
				if (!l.empty() && l[0] == '#') {
					EXPECT_EQ(l.size(), 22u) << l;
					EXPECT_EQ(l.back(), '#') << l;
					++framed;
				}
			}
			EXPECT_GT(framed, 0);
			EXPECT_LE(ls.size(), 24u);
		}
	}
}

TEST(TerminalMenuView, MarkerFollowsSelectionWithoutColor) {
	std::vector<std::string> start = lines(plain(render({MenuItem::Start, Level::Normal, {0, 0, 0}, true}, false)));
	EXPECT_NE(lineWith(start, "> Start"), "");
	EXPECT_EQ(lineWith(start, "> Level"), "");
	std::vector<std::string> level = lines(plain(render({MenuItem::Level, Level::Normal, {0, 0, 0}, true}, false)));
	EXPECT_NE(lineWith(level, "> Level"), "");
	EXPECT_EQ(lineWith(level, "> Start"), "");
	std::vector<std::string> quit = lines(plain(render({MenuItem::Quit, Level::Normal, {0, 0, 0}, true}, false)));
	EXPECT_NE(lineWith(quit, "> Quit"), "");
}

TEST(TerminalMenuView, ShowsEachLevelBetweenBrackets) {
	EXPECT_NE(plain(render({MenuItem::Level, Level::Easy, {0, 0, 0}, true}, false)).find("<Easy>"), std::string::npos);
	EXPECT_NE(plain(render({MenuItem::Level, Level::Normal, {0, 0, 0}, true}, false)).find("<Normal>"), std::string::npos);
	EXPECT_NE(plain(render({MenuItem::Level, Level::Hard, {0, 0, 0}, true}, false)).find("<Hard>"), std::string::npos);
}

TEST(TerminalMenuView, BannerIsBoxedAndCentered) {
	std::vector<std::string> ls = lines(plain(render(DEFAULT_SNAPSHOT, false)));
	EXPECT_EQ(ls[2], "#   ##############   #");
	EXPECT_EQ(ls[3], "#   # SNAKE GAME #   #");
	EXPECT_EQ(ls[4], "#   ##############   #");
}

TEST(TerminalMenuView, ContentAlignsWithTheBannerLeftEdge) {
	std::vector<std::string> ls = lines(plain(render(DEFAULT_SNAPSHOT, false)));
	for (const char* needle : {"OOOOOOO", "> Start", "Best:", "Games:", "Avg:"}) {
		EXPECT_EQ(lineWith(ls, needle).find_first_not_of("# "), 4u) << needle;
	}
}

TEST(TerminalMenuView, StatsLines) {
	std::vector<std::string> ls = lines(plain(render(DEFAULT_SNAPSHOT, false)));
	EXPECT_NE(lineWith(ls, "Best:").find("42"), std::string::npos);
	EXPECT_NE(lineWith(ls, "Games:").find("7"), std::string::npos);
	EXPECT_NE(lineWith(ls, "Avg:").find("4.3"), std::string::npos);
}

TEST(TerminalMenuView, PlainOutputHasNoEscapeBesidesTheClear) {
	std::string out = render(DEFAULT_SNAPSHOT, false);
	ASSERT_EQ(out.rfind(CLEAR, 0), 0u);
	EXPECT_EQ(out.find('\033', CLEAR.size()), std::string::npos);
}

TEST(TerminalMenuView, ColorOutputUsesTheGamePalette) {
	std::string out = render(DEFAULT_SNAPSHOT, true);
	EXPECT_NE(out.find("\033[36m"), std::string::npos);    // cyan frame
	EXPECT_NE(out.find("\033[32m"), std::string::npos);    // green snake and title
	EXPECT_NE(out.find("\033[1;92m"), std::string::npos);  // bright head
	EXPECT_NE(out.find("\033[1;91m"), std::string::npos);  // red food
	EXPECT_NE(out.find("\033[1;33m"), std::string::npos);  // yellow numbers
}

TEST(TerminalMenuView, SoundRowShowsOnAndOff) {
	std::string on = plain(render({MenuItem::Sound, Level::Normal, {0, 0, 0}, true}, false));
	std::string off = plain(render({MenuItem::Sound, Level::Normal, {0, 0, 0}, false}, false));
	EXPECT_NE(on.find("Sound <On>"), std::string::npos);
	EXPECT_NE(off.find("Sound <Off>"), std::string::npos);
}

TEST(TerminalMenuView, MarkerReachesTheNewRows) {
	for (const char* label : {"> Leaderboard", "> How to play", "> Sound"}) {
		MenuItem item = std::string(label) == "> Leaderboard" ? MenuItem::Leaderboard
			: std::string(label) == "> How to play" ? MenuItem::Help : MenuItem::Sound;
		std::vector<std::string> ls = lines(plain(render({item, Level::Normal, {0, 0, 0}, true}, false)));
		EXPECT_NE(lineWith(ls, label), "") << label;
		EXPECT_EQ(lineWith(ls, "> Start"), "") << label;
	}
}

TEST(TerminalMenuView, LeaderboardMatchesGolden) {
	LeaderboardSnapshot snap{Level::Normal, {42, 30, 30, 7}};
	expectGolden("menu_leaderboard.txt", renderBoard(snap, false));
	expectGolden("menu_leaderboard_color.txt", renderBoard(snap, true));
	EXPECT_EQ(plain(renderBoard(snap, true)), plain(renderBoard(snap, false)));
}

TEST(TerminalMenuView, EmptyLeaderboardSaysNoGamesYet) {
	LeaderboardSnapshot snap{Level::Hard, {}};
	expectGolden("menu_leaderboard_empty.txt", renderBoard(snap, false));
	EXPECT_NE(plain(renderBoard(snap, false)).find("No games yet"), std::string::npos);
	EXPECT_NE(plain(renderBoard(snap, false)).find("<Hard>"), std::string::npos);
}

TEST(TerminalMenuView, LeaderboardFitsWithTenAndHugeScores) {
	expectFits(renderBoard({Level::Easy, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}}, false));
	expectFits(renderBoard({Level::Normal, {2147483647, 123456, 9}}, false));
	expectFits(renderBoard({Level::Normal, {2147483647, 123456, 9}}, true));
	expectFits(renderBoard({Level::Hard, {}}, false));
}

TEST(TerminalMenuView, LeaderboardRanksFromOne) {
	std::vector<std::string> ls = lines(plain(renderBoard({Level::Normal, {42, 30}}, false)));
	EXPECT_NE(lineWith(ls, "1.").find("42"), std::string::npos);
	EXPECT_NE(lineWith(ls, "2.").find("30"), std::string::npos);
}

TEST(TerminalMenuView, HelpMatchesGoldenAndFits) {
	expectGolden("menu_help.txt", renderHelp(false));
	expectGolden("menu_help_color.txt", renderHelp(true));
	EXPECT_EQ(plain(renderHelp(true)), plain(renderHelp(false)));
	expectFits(renderHelp(false));
	expectFits(renderHelp(true));
}

TEST(TerminalMenuView, SubScreensHaveNoEscapeBesidesTheClearWithoutColor) {
	for (const std::string& out : {renderBoard({Level::Normal, {5}}, false), renderHelp(false)}) {
		ASSERT_EQ(out.rfind(CLEAR, 0), 0u);
		EXPECT_EQ(out.find('\033', CLEAR.size()), std::string::npos);
	}
}
