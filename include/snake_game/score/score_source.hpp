/**
 * score_source.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Interfaces over the saved scores, per level: a read-only view and a store that also records games
 */
#ifndef SCORE_SOURCE_HPP
#define SCORE_SOURCE_HPP

#include "snake_game/game/level.hpp"
#include <cstddef>
#include <string>
#include <vector>

/**
 * Score figures of one level
 *
 * Attributes:
 * int best: Best score
 * std::size_t games: Games played
 * std::string unsavedDir: The score folder when the last save failed, empty when saving works
 */
struct ScoreStats {
	int best;
	std::size_t games;
	std::string unsavedDir = "";
};

/**
 * Where the menu reads scores from; ScoreManager implements it
 */
class IScoreSource {
public:
	/**
	 * Destructor for the IScoreSource interface
	 */
	virtual ~IScoreSource() = default;

	/**
	 * Gets the figures of a level
	 *
	 * Parameters:
	 * Level level: The level
	 *
	 * Returns ScoreStats: Best and games played
	 */
	virtual ScoreStats stats(Level level) const = 0;

	/**
	 * Gets the best scores of a level
	 *
	 * Parameters:
	 * Level level: The level
	 * std::size_t n: Maximum number of scores
	 *
	 * Returns std::vector<int>: Up to n scores, highest first, ties kept
	 */
	virtual std::vector<int> topScores(Level level, std::size_t n) const = 0;

	/**
	 * Deletes every saved game of one level, leaving the others
	 *
	 * Parameters:
	 * Level level: The level to clear
	 */
	virtual void clear(Level level) = 0;

	/**
	 * Tells whether the scores were reset in this session because the score file failed its check
	 *
	 * Returns bool: true if they were
	 */
	virtual bool wasReset() const {
		return false;
	}
};

/**
 * What the game engine needs from the saved scores: the menu's view plus recording finished games
 */
class IScoreStore : public IScoreSource {
public:
	/**
	 * Gets the best score of a level
	 *
	 * Parameters:
	 * Level level: The level
	 *
	 * Returns int: The best score, 0 if the level has no games
	 */
	virtual int bestScore(Level level) const = 0;

	/**
	 * Saves the score of a finished game
	 *
	 * Parameters:
	 * Level level: The level it was played on
	 * int score: The final score
	 */
	virtual void saveScore(Level level, int score) = 0;
};

#endif
