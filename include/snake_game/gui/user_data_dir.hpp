/**
 * user_data_dir.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Where the graphical game keeps its scores, in the folder each OS reserves for app data
 */
#ifndef USER_DATA_DIR_HPP
#define USER_DATA_DIR_HPP

#include <string>

/**
 * Finds, and creates if needed, the per-user data folder of the game: $XDG_DATA_HOME/snake-game (or
 * ~/.local/share/snake-game) on Linux, ~/Library/Application Support/snake-game on macOS and
 * %APPDATA%\snake-game on Windows
 *
 * Returns std::string: The folder without a trailing separator, or "data" (next to where the game was
 * started, like the terminal version) if the OS gives no such folder
 */
std::string userDataDir();

#endif
