/**
 * user_data_dir_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for the per-user data folder; built only with SNAKE_GAME_BUILD_GUI=ON
 */
#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>

#include "snake_game/gui/user_data_dir.hpp"

TEST(UserDataDir, IsAnExistingFolderNamedAfterTheGame) {
	const std::filesystem::path home = std::filesystem::temp_directory_path() / "snake_userdata_test";
	std::filesystem::remove_all(home);
#ifdef _WIN32
	_putenv_s("XDG_DATA_HOME", home.string().c_str());
#else
	setenv("XDG_DATA_HOME", home.c_str(), 1);
#endif
	const std::string dir = userDataDir();
	EXPECT_TRUE(std::filesystem::is_directory(dir)) << dir;
	EXPECT_EQ(std::filesystem::path(dir).filename(), "snake-game");
	EXPECT_NE(dir.back(), '/');
	EXPECT_NE(dir.back(), '\\');
	std::filesystem::remove_all(home);
}
