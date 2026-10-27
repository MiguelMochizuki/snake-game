/**
 * seams_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the seed, data directory and output stream seams
 */
#include <gtest/gtest.h>

#include "game_state.hpp"
#include "score_manager.hpp"
#include "terminal_renderer.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

TEST(GameStateSeed, SameSeedSameFood) {
	GameState a(20, 20, 0, 42);
	GameState b(20, 20, 0, 42);
	EXPECT_EQ(a.getFood().getX(), b.getFood().getX());
	EXPECT_EQ(a.getFood().getY(), b.getFood().getY());
}

TEST(GameStateSeed, FoodSequenceIsPortable) {
	// Derived from std::mt19937 seeded with 42; its output is fixed by the
	// C++ standard, so this value is the same on every platform
	// (first outputs 1608637542 % 20 = 2, 3421126067 % 20 = 7).
	GameState s(20, 20, 0, 42);
	EXPECT_EQ(s.getFood().getX(), 2);
	EXPECT_EQ(s.getFood().getY(), 7);
}

TEST(ScoreManagerDir, UsesGivenDirectory) {
	fs::path tmp = fs::temp_directory_path() /
		("snake_seams_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
	fs::create_directories(tmp);
	{
		ScoreManager m(tmp.string());
		m.saveScore(5);
	}
	std::ifstream in(tmp / "best_score.txt");
	std::stringstream content;
	content << in.rdbuf();
	in.close();
	fs::remove_all(tmp);
	EXPECT_EQ(content.str(), "5\n");
}

TEST(TerminalRendererStream, WritesToGivenStream) {
	std::ostringstream os;
	TerminalRenderer r(os);
	r.clear();
	EXPECT_EQ(os.str(), "\033[2J\033[H");
	r.clear();
	EXPECT_EQ(os.str(), "\033[2J\033[H\033[H");
}
