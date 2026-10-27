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

TEST_F(ScoreManagerTest, FreshDirCreatesFiles) {
	ScoreManager m(dir());
	EXPECT_EQ(m.getBestScore(), 0);
	EXPECT_TRUE(m.getHistory().empty());
	EXPECT_EQ(read("best_score.txt"), "0\n");
	EXPECT_EQ(read("score_history.txt"), "");
	EXPECT_TRUE(fs::exists(dir_ / "score_checksum.txt"));
}

TEST_F(ScoreManagerTest, SaveScoreAppendsAndUpdatesBest) {
	ScoreManager m(dir());
	m.saveScore(3);
	m.saveScore(1);
	EXPECT_EQ(read("score_history.txt"), "3\n1\n");
	EXPECT_EQ(read("best_score.txt"), "3\n");
	EXPECT_EQ(m.getBestScore(), 3);
	EXPECT_EQ(m.getTotalGames(), 2u);
}

TEST_F(ScoreManagerTest, AverageScore) {
	ScoreManager m(dir());
	EXPECT_DOUBLE_EQ(m.getAverageScore(), 0.0);
	m.saveScore(3);
	m.saveScore(4);
	EXPECT_DOUBLE_EQ(m.getAverageScore(), 3.5);
}

TEST_F(ScoreManagerTest, AverageUsesAllGamesBelowWindow) {
	ScoreManager m(dir());
	for (int s : {1, 2, 3}) m.saveScore(s);
	EXPECT_DOUBLE_EQ(m.getAverageScore(), 2.0);
}

TEST_F(ScoreManagerTest, AverageExactlyWindow) {
	ScoreManager m(dir());
	for (int s = 1; s <= 10; ++s) m.saveScore(s);
	EXPECT_DOUBLE_EQ(m.getAverageScore(), 5.5);
}

TEST_F(ScoreManagerTest, AverageUsesLastTenOnly) {
	ScoreManager m(dir());
	m.saveScore(100); m.saveScore(100);           // outside the window
	for (int s = 1; s <= 10; ++s) m.saveScore(s);
	EXPECT_DOUBLE_EQ(m.getAverageScore(), 5.5);
	EXPECT_EQ(m.getTotalGames(), 12u);
}

TEST_F(ScoreManagerTest, AverageWindowSurvivesReload) {
	{ ScoreManager m(dir()); for (int s = 1; s <= 12; ++s) m.saveScore(s); }
	ScoreManager m2(dir());
	EXPECT_DOUBLE_EQ(m2.getAverageScore(), 7.5);  // mean of 3..12
}

TEST_F(ScoreManagerTest, AverageAfterTamperIsZero) {
	{
		ScoreManager m(dir());
		for (int s = 1; s <= 12; ++s) m.saveScore(s);
	}
	std::string tampered = "50\n";
	for (int i = 1; i < 12; ++i) tampered += "1\n";
	write("score_history.txt", tampered);
	ScoreManager m2(dir());
	EXPECT_DOUBLE_EQ(m2.getAverageScore(), 0.0);
	EXPECT_EQ(m2.getTotalGames(), 12u);
}

TEST_F(ScoreManagerTest, PersistsAcrossInstances) {
	{
		ScoreManager m(dir());
		m.saveScore(5);
	}
	ScoreManager m2(dir());
	EXPECT_EQ(m2.getBestScore(), 5);
	EXPECT_EQ(m2.getTotalGames(), 1u);
}

TEST_F(ScoreManagerTest, MissingChecksumWipesHistory) {
	{
		ScoreManager m(dir());
		m.saveScore(5);
		m.saveScore(7);
	}
	fs::remove(dir_ / "score_checksum.txt");
	ScoreManager m2(dir());
	EXPECT_EQ(m2.getTotalGames(), 0u);
	EXPECT_EQ(m2.getBestScore(), 0);
	EXPECT_EQ(read("best_score.txt"), "0\n");
	EXPECT_EQ(read("score_history.txt"), "");
	EXPECT_TRUE(fs::exists(dir_ / "score_checksum.txt"));
}

TEST_F(ScoreManagerTest, TamperedHistoryZeroesScoresKeepsCount) {
	{
		ScoreManager m(dir());
		m.saveScore(5);
		m.saveScore(7);
	}
	write("score_history.txt", "50\n7\n");
	{
		ScoreManager m2(dir());
		EXPECT_EQ(m2.getTotalGames(), 2u);
		EXPECT_EQ(m2.getHistory(), (std::vector<int>{0, 0}));
		EXPECT_EQ(m2.getBestScore(), 0);
	}
	// Checksum was rewritten, so a third instance sees a consistent state
	ScoreManager m3(dir());
	EXPECT_EQ(m3.getTotalGames(), 2u);
	EXPECT_EQ(m3.getHistory(), (std::vector<int>{0, 0}));
}

TEST_F(ScoreManagerTest, GarbageFileContentIsTreatedAsTamper) {
	{
		ScoreManager m(dir());
		m.saveScore(5);
	}
	write("best_score.txt", "abc");
	EXPECT_NO_THROW({
		ScoreManager m2(dir());
		EXPECT_EQ(m2.getBestScore(), 0);
		EXPECT_EQ(m2.getTotalGames(), 1u);
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
		m.saveScore(3);
		EXPECT_EQ(m.getBestScore(), 3);
	});
	EXPECT_TRUE(fs::is_regular_file(file));
}
