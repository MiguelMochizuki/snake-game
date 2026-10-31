/**
 * score_manager.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Class ScoreManager headers
 */
#ifndef SCORE_MANAGER_HPP
#define SCORE_MANAGER_HPP

#include "snake_game/game/level.hpp"
#include "snake_game/score/score_source.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

/**
 * Saves and reads game scores per level, with a checksum against hand edits
 *
 * Attributes:
 * std::string dataDir_: Directory holding the score files
 * std::string scoresFile_: scores_v2.txt, one "E|N|H score" line per game, oldest first
 * std::string checksumFile_: scores_v2.checksum
 * std::vector<std::pair<Level, int>> records_: Every game, oldest first
 * uint64_t checksum_: Checksum read from the file
 */
class ScoreManager : public IScoreSource {
public:
	static constexpr std::size_t AVERAGE_WINDOW = 10;

private:
	std::string dataDir_;
	std::string scoresFile_;
	std::string checksumFile_;
	std::vector<std::pair<Level, int>> records_;
	uint64_t checksum_;

	/**
	 * Creates the data directory if it does not exist
	 */
	void createDataDirectory();

	/**
	 * Reads the v2 score file, skipping lines that are not "E|N|H score"
	 */
	void loadRecords();

	/**
	 * Imports the pre-2.0 files as Normal games when their checksum is valid; the old files are left untouched
	 */
	void migrateOldFiles();

	/**
	 * Computes the checksum of the records
	 *
	 * Returns uint64_t: The checksum
	 */
	uint64_t computeChecksum() const;

	/**
	 * Reads the checksum file
	 *
	 * Returns bool: true if the file existed
	 */
	bool loadChecksum();

	/**
	 * Computes and writes the checksum file
	 */
	void writeChecksum();

	/**
	 * Rewrites the whole score file from the records
	 */
	void writeRecords() const;

public:
	/**
	 * Constructor for the ScoreManager class
	 *
	 * Parameters:
	 * const std::string& dataDir: Directory for the score files (default: "data")
	 */
	explicit ScoreManager(const std::string& dataDir = "data");

	/**
	 * Destructor for the ScoreManager class
	 */
	~ScoreManager() override = default;

	/**
	 * Gets the best score of a level
	 *
	 * Parameters:
	 * Level level: The level
	 *
	 * Returns int: The best score, 0 if the level has no games
	 */
	int bestScore(Level level) const;

	/**
	 * Gets the number of games played on a level
	 *
	 * Parameters:
	 * Level level: The level
	 *
	 * Returns std::size_t: The number of games
	 */
	std::size_t totalGames(Level level) const;

	/**
	 * Gets the average of the last AVERAGE_WINDOW games of a level
	 *
	 * Parameters:
	 * Level level: The level
	 *
	 * Returns double: The average, 0.0 if the level has no games
	 */
	double averageScore(Level level) const;

	/**
	 * Saves the score of a finished game
	 *
	 * Parameters:
	 * Level level: The level it was played on
	 * int score: The final score
	 */
	void saveScore(Level level, int score);

	ScoreStats stats(Level level) const override;
	std::vector<int> topScores(Level level, std::size_t n) const override;
};

#endif
