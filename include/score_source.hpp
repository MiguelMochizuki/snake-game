/**
 * score_source.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Read-only view of the saved scores, per level
 */
#ifndef SCORE_SOURCE_HPP
#define SCORE_SOURCE_HPP

#include "level.hpp"
#include <cstddef>
#include <vector>

/**
 * Score figures of one level
 *
 * Attributes:
 * int best: Best score
 * std::size_t games: Games played
 * double average: Average score over the last games
 */
struct ScoreStats {
	int best;
	std::size_t games;
	double average;
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
	 * Returns ScoreStats: Best, games played and average of the last games
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
};

#endif
