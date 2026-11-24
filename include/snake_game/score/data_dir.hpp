/**
 * data_dir.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Picks the folder where the terminal game keeps its score files
 */
#ifndef DATA_DIR_HPP
#define DATA_DIR_HPP

#include <string>

/**
 * Picks the folder for the score files: SNAKE_DATA_DIR if set, else "data" in the current directory when it
 * already holds scores (v2 or v1), else the per-user data folder of the OS (XDG_DATA_HOME or ~/.local/share,
 * ~/Library/Application Support, %APPDATA%) plus "snake-game-terminal" (the graphical game keeps its own "snake-game")
 *
 * Returns std::string: The folder without a trailing separator, or "data" if the OS gives none
 */
std::string defaultDataDir();

#endif
