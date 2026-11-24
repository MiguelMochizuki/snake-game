/**
 * menu_settings.hpp
 * Author: Miguel Mochizuki Silva
 * Description: The menu options that are remembered between runs (level and sound), kept in settings.txt next to the scores
 */
#ifndef MENU_SETTINGS_HPP
#define MENU_SETTINGS_HPP

#include "snake_game/game/level.hpp"
#include <string>

/**
 * The options the menu starts with
 *
 * Attributes:
 * Level level: Chosen level
 * bool sound: Whether sound is on
 */
struct MenuSettings {
	Level level = Level::Normal;
	bool sound = true;
};

/**
 * Reads settings.txt from a folder
 *
 * Parameters:
 * const std::string& dataDir: The folder that holds the score files
 *
 * Returns MenuSettings: What the file says; a missing file, a missing or unknown line and a bad value keep the default
 */
MenuSettings loadMenuSettings(const std::string& dataDir);

/**
 * Writes settings.txt into a folder; a folder that cannot be written is ignored, since these are only preferences
 *
 * Parameters:
 * const std::string& dataDir: The folder that holds the score files
 * const MenuSettings& settings: What to remember
 */
void saveMenuSettings(const std::string& dataDir, const MenuSettings& settings);

#endif
