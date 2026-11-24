/**
 * menu_settings_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Unit tests for the remembered menu options in settings.txt
 */
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "engine_fakes.hpp"
#include "snake_game/menu/menu_settings.hpp"

using namespace fakes;

namespace {

/**
 * Writes settings.txt in a folder
 *
 * Parameters:
 * const TempDir& dir: The folder
 * const std::string& text: The content
 */
void writeSettings(const TempDir& dir, const std::string& text) {
	std::ofstream(dir.path / "settings.txt") << text;
}

}  // namespace

TEST(MenuSettings, MissingFileGivesTheDefaults) {
	TempDir dir;
	MenuSettings s = loadMenuSettings(dir.path.string());
	EXPECT_EQ(s.level, Level::Normal);
	EXPECT_TRUE(s.sound);
}

TEST(MenuSettings, RoundTripsEveryLevelAndSound) {
	TempDir dir;
	for (Level level : {Level::Easy, Level::Normal, Level::Hard}) {
		for (bool sound : {true, false}) {
			saveMenuSettings(dir.path.string(), MenuSettings{level, sound});
			MenuSettings s = loadMenuSettings(dir.path.string());
			EXPECT_EQ(s.level, level);
			EXPECT_EQ(s.sound, sound);
		}
	}
}

TEST(MenuSettings, BadValuesAndUnknownLinesKeepTheDefaults) {
	TempDir dir;
	writeSettings(dir, "level=Impossible\nsound=maybe\nvolume=11\nnonsense\n=\n");
	MenuSettings s = loadMenuSettings(dir.path.string());
	EXPECT_EQ(s.level, Level::Normal);
	EXPECT_TRUE(s.sound);
}

TEST(MenuSettings, OneBadLineDoesNotSpoilTheGoodOne) {
	TempDir dir;
	writeSettings(dir, "level=Nope\nsound=off\n");
	MenuSettings s = loadMenuSettings(dir.path.string());
	EXPECT_EQ(s.level, Level::Normal);
	EXPECT_FALSE(s.sound);
}

TEST(MenuSettings, SavingToAFolderThatDoesNotExistIsIgnored) {
	TempDir dir;
	saveMenuSettings((dir.path / "missing").string(), MenuSettings{Level::Hard, false});
	SUCCEED();
}
