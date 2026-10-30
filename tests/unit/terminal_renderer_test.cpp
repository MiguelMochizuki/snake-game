/**
 * terminal_renderer_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Golden snapshot and escape sequence tests for TerminalRenderer
 */
#include <gtest/gtest.h>

#include <sstream>

#include "game_state.hpp"
#include "golden.hpp"
#include "terminal_renderer.hpp"

TEST(TerminalRendererGolden, InitialBoard) {
	std::ostringstream out;
	TerminalRenderer renderer(out);
	GameState state(20, 20, 0, 42);
	renderer.render(state);
	expectGolden("board_seed42_initial.txt", out.str());
}

TEST(TerminalRendererGolden, AfterMoves) {
	std::ostringstream out;
	TerminalRenderer renderer(out);
	GameState state(20, 20, 0, 42);
	for (int dir : {0, 0, 2, 2}) {
		state.moveSnake(dir);
	}
	renderer.render(state);
	expectGolden("board_seed42_after_moves.txt", out.str());
}

TEST(TerminalRendererGolden, GameOverPanel) {
	std::ostringstream out;
	TerminalRenderer renderer(out);
	renderer.printGameOver(Level::Hard, 3, {12, 7, 4.2857});
	expectGolden("game_over_panel.txt", out.str());
}

TEST(TerminalRenderer, ClearEscapes) {
	std::ostringstream out;
	TerminalRenderer renderer(out);
	renderer.clear();
	EXPECT_EQ(out.str(), "\033[2J\033[H");
	out.str("");
	renderer.clear();
	EXPECT_EQ(out.str(), "\033[H");
	out.str("");
	renderer.reset();
	renderer.clear();
	EXPECT_EQ(out.str(), "\033[2J\033[H");
}
