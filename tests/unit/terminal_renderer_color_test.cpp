/**
 * terminal_renderer_color_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Golden and behaviour tests for the colored (Arcade palette) TerminalRenderer output
 */
#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>

#include "snake_game/game/game_state.hpp"
#include "golden.hpp"
#include "snake_game/render/terminal_renderer.hpp"

namespace {

/**
 * Removes every SGR sequence (ESC [ digits/semicolons m)
 *
 * Parameters:
 * const std::string& s: Colored text
 *
 * Returns std::string: Text without SGR sequences
 */
std::string stripSgr(const std::string& s) {
	return std::regex_replace(s, std::regex("\033\\[[0-9;]*m"), "");
}

/**
 * Reads a golden file through expectGolden's directory
 *
 * Parameters:
 * const std::string& name: Golden file name
 *
 * Returns std::string: File contents
 */
std::string readGolden(const std::string& name) {
	std::ifstream in(std::string(SNAKE_GOLDEN_DIR) + "/" + name, std::ios::binary);
	return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

/**
 * Renders the seed 42 initial board with color on
 *
 * Returns std::string: The colored board
 */
std::string coloredBoard() {
	std::ostringstream out;
	TerminalRenderer renderer(out, true);
	GameState state(20, 20, 0, 42);
	renderer.render(state);
	return out.str();
}

/**
 * Renders the sample game-over panel with color on
 *
 * Returns std::string: The colored panel
 */
std::string coloredPanel() {
	std::ostringstream out;
	TerminalRenderer renderer(out, true);
	renderer.printGameOver(Level::Hard, 3, {12, 7, 4.2857});
	return out.str();
}

}  // namespace

TEST(TerminalRendererColor, ColorBoard) {
	expectGolden("board_seed42_initial_color.txt", coloredBoard());
}

TEST(TerminalRendererColor, ColorPanel) {
	expectGolden("game_over_panel_color.txt", coloredPanel());
}

TEST(TerminalRendererColor, ColorStripsToPlain) {
	EXPECT_EQ(stripSgr(coloredBoard()), readGolden("board_seed42_initial.txt"));
	EXPECT_EQ(stripSgr(coloredPanel()), readGolden("game_over_panel.txt"));
}

TEST(TerminalRendererColor, HeadDistinct) {
	const std::string board = coloredBoard();
	const std::string head = "\033[1;92mO";
	size_t first = board.find(head);
	ASSERT_NE(first, std::string::npos);
	EXPECT_EQ(board.find(head, first + 1), std::string::npos);
}

TEST(TerminalRendererColor, EmptyNoColorKeepsColor) {
#ifdef _WIN32
	// _putenv_s(name, "") deletes the variable on Windows, so "set but empty" is POSIX-only
	const char* old = std::getenv("NO_COLOR");
	std::string saved = old ? old : "";
	_putenv_s("NO_COLOR", "");
	EXPECT_TRUE(TerminalRenderer::colorEnabledFromEnv());
	_putenv_s("NO_COLOR", "1");
	EXPECT_FALSE(TerminalRenderer::colorEnabledFromEnv());
	_putenv_s("NO_COLOR", saved.c_str());
#else
	const char* old = std::getenv("NO_COLOR");
	std::string saved = old ? old : "";
	setenv("NO_COLOR", "", 1);
	EXPECT_TRUE(TerminalRenderer::colorEnabledFromEnv());
	setenv("NO_COLOR", "1", 1);
	EXPECT_FALSE(TerminalRenderer::colorEnabledFromEnv());
	unsetenv("NO_COLOR");
	EXPECT_TRUE(TerminalRenderer::colorEnabledFromEnv());
	if (old) {
		setenv("NO_COLOR", saved.c_str(), 1);
	}
#endif
}
