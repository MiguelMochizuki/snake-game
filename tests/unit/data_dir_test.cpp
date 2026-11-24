/**
 * data_dir_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests where the terminal game keeps its scores: override, legacy ./data, then the OS folder
 */
#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include "snake_game/score/data_dir.hpp"

#ifndef _WIN32

namespace fs = std::filesystem;

namespace {

/**
 * Runs in a fresh working directory with a controlled environment, and restores both on teardown
 *
 * Attributes:
 * fs::path cwd_: The directory the test runs in
 * fs::path previous_: The working directory before the test
 */
class DataDirTest : public ::testing::Test {
protected:
	fs::path cwd_;
	fs::path previous_;

	/**
	 * Moves into an empty temp directory and clears the variables the choice depends on
	 */
	void SetUp() override {
		previous_ = fs::current_path();
		cwd_ = fs::temp_directory_path() / ("snake_dd_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
		fs::create_directories(cwd_);
		fs::current_path(cwd_);
		unsetenv("SNAKE_DATA_DIR");
		setenv("HOME", "/home/someone", 1);
		unsetenv("XDG_DATA_HOME");
	}

	/**
	 * Returns to the previous working directory and removes the temp one
	 */
	void TearDown() override {
		fs::current_path(previous_);
		std::error_code ec;
		fs::remove_all(cwd_, ec);
		unsetenv("SNAKE_DATA_DIR");
	}

	/**
	 * Creates an empty file under ./data
	 *
	 * Parameters:
	 * const std::string& name: File name
	 */
	void touch(const std::string& name) const {
		fs::create_directories("data");
		std::ofstream("data/" + name);
	}
};

}  // namespace

#ifdef __APPLE__
TEST_F(DataDirTest, DefaultsToTheApplicationSupportFolder) {
	EXPECT_EQ(defaultDataDir(), "/home/someone/Library/Application Support/snake-game-terminal");
}
#else
TEST_F(DataDirTest, DefaultsToTheUserDataFolder) {
	EXPECT_EQ(defaultDataDir(), "/home/someone/.local/share/snake-game-terminal");
}

TEST_F(DataDirTest, XdgDataHomeWins) {
	setenv("XDG_DATA_HOME", "/xdg", 1);
	EXPECT_EQ(defaultDataDir(), "/xdg/snake-game-terminal");
}
#endif

TEST_F(DataDirTest, EnvironmentOverrideWinsOverEverything) {
	touch("scores_v2.txt");
	setenv("SNAKE_DATA_DIR", "/elsewhere", 1);
	EXPECT_EQ(defaultDataDir(), "/elsewhere");
}

TEST_F(DataDirTest, EmptyOverrideIsIgnored) {
	setenv("SNAKE_DATA_DIR", "", 1);
	EXPECT_NE(defaultDataDir(), "");
	EXPECT_NE(defaultDataDir(), "data");
}

TEST_F(DataDirTest, ExistingScoresInCurrentDataFolderKeepWorking) {
	touch("scores_v2.txt");
	EXPECT_EQ(defaultDataDir(), "data");
}

TEST_F(DataDirTest, V1FilesInCurrentDataFolderKeepWorking) {
	for (const char* name : {"score_history.txt", "best_score.txt", "score_checksum.txt"}) {
		fs::remove_all("data");
		touch(name);
		EXPECT_EQ(defaultDataDir(), "data") << name;
	}
}

TEST_F(DataDirTest, EmptyCurrentDataFolderDoesNotCount) {
	fs::create_directories("data");
	EXPECT_NE(defaultDataDir(), "data");
}

TEST_F(DataDirTest, WithoutAnyHomeItFallsBackToData) {
	unsetenv("HOME");
	EXPECT_EQ(defaultDataDir(), "data");
}

#endif
