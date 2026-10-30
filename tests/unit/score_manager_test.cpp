/**
 * score_manager_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests pinning ScoreManager persistence and integrity behavior
 */
#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "score_manager.hpp"

#include <cstdint>

#ifdef _WIN32
	#include <process.h>
	#define SNAKE_GETPID _getpid
#else
	#include <unistd.h>
	#define SNAKE_GETPID getpid
#endif

namespace fs = std::filesystem;

/**
 * Fixture giving each test its own temp directory, removed on teardown
 *
 * Attributes:
 * fs::path dir_: Unique directory for the running test
 */
class ScoreManagerTest : public ::testing::Test {
protected:
	fs::path dir_;

	/**
	 * Creates the unique temp directory
	 */
	void SetUp() override {
		const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
		auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
		dir_ = fs::temp_directory_path() /
			(std::string("snake_sm_") + info->name() + "_" +
			 std::to_string(SNAKE_GETPID()) + "_" + std::to_string(stamp));
		fs::create_directories(dir_);
	}

	/**
	 * Removes the temp directory (never throws)
	 */
	void TearDown() override {
		std::error_code ec;
		fs::remove_all(dir_, ec);
	}

	/**
	 * Reads a file from the temp directory
	 *
	 * Parameters:
	 * const std::string& name: File name relative to the temp directory
	 *
	 * Returns std::string: File contents, empty if missing
	 */
	std::string read(const std::string& name) const {
		std::ifstream f(dir_ / name);
		std::stringstream ss;
		ss << f.rdbuf();
		return ss.str();
	}

	/**
	 * Writes a file into the temp directory, replacing it
	 *
	 * Parameters:
	 * const std::string& name: File name relative to the temp directory
	 * const std::string& content: Content to write
	 */
	void write(const std::string& name, const std::string& content) const {
		std::ofstream f(dir_ / name, std::ios::trunc);
		f << content;
	}

	/**
	 * Gets the data directory path as a string
	 *
	 * Returns std::string: Temp directory path
	 */
	std::string dir() const {
		return dir_.string();
	}
};

/**
 * Computes the checksum the pre-2.0 game wrote, so migration can be tested with genuine old files
 *
 * Parameters:
 * int best: Content of best_score.txt
 * const std::vector<int>& history: Content of score_history.txt
 *
 * Returns uint64_t: The old checksum
 */
static uint64_t oldChecksum(int best, const std::vector<int>& history) {
	const uint64_t seed = 0x9E3779B97F4A7C15ULL;
	uint64_t hash = seed;
	hash ^= static_cast<uint64_t>(best) + seed + (hash << 6) + (hash >> 2);
	for (int score : history) {
		hash ^= static_cast<uint64_t>(score) + seed + (hash << 6) + (hash >> 2);
	}
	return hash;
}

TEST_F(ScoreManagerTest, FreshDirCreatesV2Files) {
	ScoreManager m(dir());
	EXPECT_EQ(m.bestScore(Level::Normal), 0);
	EXPECT_EQ(m.totalGames(Level::Normal), 0u);
	EXPECT_EQ(read("scores_v2.txt"), "");
	EXPECT_TRUE(fs::exists(dir_ / "scores_v2.checksum"));
}

TEST_F(ScoreManagerTest, ScoresAreKeptPerLevel) {
	ScoreManager m(dir());
	m.saveScore(Level::Easy, 5);
	m.saveScore(Level::Hard, 9);
	m.saveScore(Level::Easy, 7);
	EXPECT_EQ(m.bestScore(Level::Easy), 7);
	EXPECT_EQ(m.totalGames(Level::Easy), 2u);
	EXPECT_DOUBLE_EQ(m.averageScore(Level::Easy), 6.0);
	EXPECT_EQ(m.bestScore(Level::Hard), 9);
	EXPECT_EQ(m.totalGames(Level::Hard), 1u);
	EXPECT_EQ(m.bestScore(Level::Normal), 0);
	EXPECT_EQ(m.totalGames(Level::Normal), 0u);
	EXPECT_DOUBLE_EQ(m.averageScore(Level::Normal), 0.0);
}

TEST_F(ScoreManagerTest, FileHoldsOneLinePerGame) {
	ScoreManager m(dir());
	m.saveScore(Level::Normal, 5);
	m.saveScore(Level::Easy, 1);
	EXPECT_EQ(read("scores_v2.txt"), "N 5\nE 1\n");
}

TEST_F(ScoreManagerTest, AverageUsesLastTenGamesOfTheLevel) {
	ScoreManager m(dir());
	m.saveScore(Level::Normal, 100);
	m.saveScore(Level::Normal, 100);
	for (int s = 1; s <= 10; ++s) m.saveScore(Level::Normal, s);
	m.saveScore(Level::Hard, 1000);
	EXPECT_DOUBLE_EQ(m.averageScore(Level::Normal), 5.5);
	EXPECT_EQ(m.totalGames(Level::Normal), 12u);
}

TEST_F(ScoreManagerTest, AverageUsesAllGamesBelowWindow) {
	ScoreManager m(dir());
	for (int s : {1, 2, 3}) m.saveScore(Level::Normal, s);
	EXPECT_DOUBLE_EQ(m.averageScore(Level::Normal), 2.0);
}

TEST_F(ScoreManagerTest, TopScoresDescendingKeepsTiesAndCaps) {
	ScoreManager m(dir());
	EXPECT_TRUE(m.topScores(Level::Normal, 10).empty());
	for (int s : {5, 9, 9, 1}) m.saveScore(Level::Normal, s);
	m.saveScore(Level::Easy, 50);
	EXPECT_EQ(m.topScores(Level::Normal, 10), (std::vector<int>{9, 9, 5, 1}));
	EXPECT_EQ(m.topScores(Level::Normal, 2), (std::vector<int>{9, 9}));
	EXPECT_EQ(m.topScores(Level::Easy, 10), (std::vector<int>{50}));
}

TEST_F(ScoreManagerTest, StatsMatchTheGetters) {
	ScoreManager m(dir());
	m.saveScore(Level::Hard, 4);
	m.saveScore(Level::Hard, 8);
	ScoreStats st = m.stats(Level::Hard);
	EXPECT_EQ(st.best, 8);
	EXPECT_EQ(st.games, 2u);
	EXPECT_DOUBLE_EQ(st.average, 6.0);
}

TEST_F(ScoreManagerTest, PersistsAcrossInstances) {
	{
		ScoreManager m(dir());
		m.saveScore(Level::Hard, 5);
		for (int s = 1; s <= 12; ++s) m.saveScore(Level::Normal, s);
	}
	ScoreManager m2(dir());
	EXPECT_EQ(m2.bestScore(Level::Hard), 5);
	EXPECT_EQ(m2.totalGames(Level::Normal), 12u);
	EXPECT_DOUBLE_EQ(m2.averageScore(Level::Normal), 7.5);
}

TEST_F(ScoreManagerTest, MissingChecksumWipesHistory) {
	{
		ScoreManager m(dir());
		m.saveScore(Level::Normal, 5);
		m.saveScore(Level::Easy, 7);
	}
	fs::remove(dir_ / "scores_v2.checksum");
	ScoreManager m2(dir());
	EXPECT_EQ(m2.totalGames(Level::Normal), 0u);
	EXPECT_EQ(m2.totalGames(Level::Easy), 0u);
	EXPECT_EQ(read("scores_v2.txt"), "");
	EXPECT_TRUE(fs::exists(dir_ / "scores_v2.checksum"));
}

TEST_F(ScoreManagerTest, EditedScoreZeroesScoresKeepsRecords) {
	{
		ScoreManager m(dir());
		m.saveScore(Level::Normal, 5);
		m.saveScore(Level::Easy, 7);
	}
	write("scores_v2.txt", "N 50\nE 7\n");
	{
		ScoreManager m2(dir());
		EXPECT_EQ(m2.totalGames(Level::Normal), 1u);
		EXPECT_EQ(m2.totalGames(Level::Easy), 1u);
		EXPECT_EQ(m2.bestScore(Level::Normal), 0);
		EXPECT_EQ(m2.bestScore(Level::Easy), 0);
	}
	// The checksum was rewritten, so a third instance sees the same consistent state
	ScoreManager m3(dir());
	EXPECT_EQ(m3.totalGames(Level::Normal), 1u);
	EXPECT_EQ(m3.bestScore(Level::Normal), 0);
}

TEST_F(ScoreManagerTest, GarbageLinesAreSkipped) {
	{
		ScoreManager m(dir());
		m.saveScore(Level::Normal, 5);
		m.saveScore(Level::Hard, 8);
	}
	write("scores_v2.txt", "N 5\nhello\nX 4\nN abc\n\nH 8\n");
	EXPECT_NO_THROW({
		ScoreManager m2(dir());
		EXPECT_EQ(m2.totalGames(Level::Normal), 1u);
		EXPECT_EQ(m2.bestScore(Level::Normal), 5);
		EXPECT_EQ(m2.bestScore(Level::Hard), 8);
	});
}

TEST_F(ScoreManagerTest, DataDirIsAFileDoesNotThrow) {
	fs::path file = dir_ / "data";
	{
		std::ofstream f(file);
		f << "x";
	}
	EXPECT_NO_THROW({
		ScoreManager m(file.string());
		m.saveScore(Level::Normal, 3);
		EXPECT_EQ(m.bestScore(Level::Normal), 3);
	});
	EXPECT_TRUE(fs::is_regular_file(file));
}

TEST_F(ScoreManagerTest, MigratesValidOldFilesAsNormalAndLeavesThemAlone) {
	write("score_history.txt", "3\n8\n");
	write("best_score.txt", "8\n");
	write("score_checksum.txt", std::to_string(oldChecksum(8, {3, 8})) + "\n");
	{
		ScoreManager m(dir());
		EXPECT_EQ(m.totalGames(Level::Normal), 2u);
		EXPECT_EQ(m.bestScore(Level::Normal), 8);
		EXPECT_EQ(m.totalGames(Level::Easy), 0u);
		EXPECT_EQ(m.totalGames(Level::Hard), 0u);
	}
	EXPECT_EQ(read("score_history.txt"), "3\n8\n");
	EXPECT_EQ(read("best_score.txt"), "8\n");
	EXPECT_EQ(read("score_checksum.txt"), std::to_string(oldChecksum(8, {3, 8})) + "\n");
	// A second run reads the v2 file and does not import again
	ScoreManager m2(dir());
	EXPECT_EQ(m2.totalGames(Level::Normal), 2u);
}

TEST_F(ScoreManagerTest, OldFilesWithBadChecksumImportNothing) {
	write("score_history.txt", "3\n8\n");
	write("best_score.txt", "8\n");
	write("score_checksum.txt", "12345\n");
	ScoreManager m(dir());
	EXPECT_EQ(m.totalGames(Level::Normal), 0u);
	EXPECT_EQ(m.bestScore(Level::Normal), 0);
}

TEST_F(ScoreManagerTest, OldFilesWithoutChecksumImportNothing) {
	write("score_history.txt", "3\n8\n");
	write("best_score.txt", "8\n");
	ScoreManager m(dir());
	EXPECT_EQ(m.totalGames(Level::Normal), 0u);
}

TEST_F(ScoreManagerTest, ExistingV2FileWinsOverOldFiles) {
	{
		ScoreManager m(dir());
		m.saveScore(Level::Hard, 4);
	}
	write("score_history.txt", "3\n8\n");
	write("best_score.txt", "8\n");
	write("score_checksum.txt", std::to_string(oldChecksum(8, {3, 8})) + "\n");
	ScoreManager m2(dir());
	EXPECT_EQ(m2.totalGames(Level::Normal), 0u);
	EXPECT_EQ(m2.bestScore(Level::Hard), 4);
}
