/**
 * menu_settings.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Reads and writes settings.txt, one "key=value" line per remembered menu option
 */
#include "snake_game/menu/menu_settings.hpp"
#include <fstream>

namespace {

/**
 * Finds the level with a given name
 *
 * Parameters:
 * const std::string& name: "Easy", "Normal" or "Hard"
 * Level& level: Set when the name is one of them
 *
 * Returns bool: true if it was
 */
bool levelFromName(const std::string& name, Level& level) {
	for (Level candidate : {Level::Easy, Level::Normal, Level::Hard}) {
		if (name == levelName(candidate)) {
			level = candidate;
			return true;
		}
	}
	return false;
}

}  // namespace

MenuSettings loadMenuSettings(const std::string& dataDir) {
	MenuSettings settings;
	std::ifstream file(dataDir + "/settings.txt");
	std::string line;
	while (std::getline(file, line)) {
		const std::size_t eq = line.find('=');
		if (eq == std::string::npos) {
			continue;
		}
		const std::string key = line.substr(0, eq);
		const std::string value = line.substr(eq + 1);
		if (key == "level") {
			levelFromName(value, settings.level);
		} else if (key == "sound" && (value == "on" || value == "off")) {
			settings.sound = value == "on";
		}
	}
	return settings;
}

void saveMenuSettings(const std::string& dataDir, const MenuSettings& settings) {
	std::ofstream file(dataDir + "/settings.txt", std::ios::trunc);
	file << "level=" << levelName(settings.level) << "\n";
	file << "sound=" << (settings.sound ? "on" : "off") << "\n";
}
