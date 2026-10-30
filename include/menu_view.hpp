/**
 * menu_view.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Presentation interface of the start menu and its screens
 */
#ifndef MENU_VIEW_HPP
#define MENU_VIEW_HPP

#include "level.hpp"
#include "menu_model.hpp"
#include "score_source.hpp"
#include <vector>

/**
 * Everything a view needs to draw the menu
 *
 * Attributes:
 * MenuItem selected: Selected row
 * Level level: Chosen level
 * ScoreStats stats: Score figures of the chosen level
 * bool sound: Whether sound is on
 */
struct MenuSnapshot {
	MenuItem selected;
	Level level;
	ScoreStats stats;
	bool sound;
};

/**
 * Everything a view needs to draw the leaderboard
 *
 * Attributes:
 * Level level: Level shown
 * std::vector<int> scores: Best scores of that level, highest first
 */
struct LeaderboardSnapshot {
	Level level;
	std::vector<int> scores;
};

/**
 * Draws the start menu and its screens; a terminal or a GUI implements it
 */
class IMenuView {
public:
	/**
	 * Destructor for the IMenuView interface
	 */
	virtual ~IMenuView() = default;

	/**
	 * Draws the menu
	 *
	 * Parameters:
	 * const MenuSnapshot& snapshot: What to draw
	 *
	 * Returns void:
	 */
	virtual void show(const MenuSnapshot& snapshot) = 0;

	/**
	 * Draws the leaderboard
	 *
	 * Parameters:
	 * const LeaderboardSnapshot& snapshot: What to draw
	 *
	 * Returns void:
	 */
	virtual void showLeaderboard(const LeaderboardSnapshot& snapshot) = 0;

	/**
	 * Draws the how-to-play screen
	 *
	 * Returns void:
	 */
	virtual void showHelp() = 0;
};

#endif
