/**
 * score_manager.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Score management and persistence
 */
#ifndef SCORE_MANAGER_HPP
#define SCORE_MANAGER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstddef>
#include <cstdint>

/**
 * Manages score persistence including best score and score history
 *
 * Attributes:
 * std::string dataDir_: Directory for storing score files
 * std::string historyFile_: Path to score history file
 * std::string bestFile_: Path to best score file
 * std::string checksumFile_: Path to integrity checksum file
 * int bestScore_: Current best score
 * std::vector<int> scoreHistory_: History of all scores
 * uint64_t checksum_: Stored integrity checksum
 */
class ScoreManager {
public:
	/**
	 * Number of most recent games the average score is computed over
	 */
	static constexpr std::size_t AVERAGE_WINDOW = 10;

private:
	std::string dataDir_;
	std::string historyFile_;
	std::string bestFile_;
	std::string checksumFile_;
	int bestScore_;
	std::vector<int> scoreHistory_;
	uint64_t checksum_;

	/**
	 * Creates data directory if it doesn't exist
	 */
	void createDataDirectory();

	/**
	 * Loads best score from file
	 */
	void loadBestScore();

	/**
	 * Loads score history from file
	 */
	void loadHistory();

	/**
	 * Computes integrity checksum from in-memory scores
	 */
	uint64_t computeChecksum() const;

	/**
	 * Loads stored checksum from file
	 *
	 * Returns bool: true if checksum file existed and was read
	 */
	bool loadChecksum();

	/**
	 * Writes integrity checksum to file
	 */
	void writeChecksum();

	/**
	 * Resets all scores to zero, keeping play count. Rewrites both score files.
	 */
	void resetScoresToZero();

	/**
	 * Clears all history entirely. Rewrites both score files from scratch.
	 */
	void clearAllHistory();

public:
	/**
	 * Constructor for the ScoreManager class
	 *
	 * Parameters:
	 * const std::string& dataDir: Directory for storing score files (default: "data")
	 */
	explicit ScoreManager(const std::string& dataDir = "data");

	/**
	 * Destructor for the ScoreManager class
	 */
	~ScoreManager() = default;

	/**
	 * Gets the average score over the last AVERAGE_WINDOW games (or all games if fewer)
	 *
	 * Returns double: Average score, or 0 if no scores
	 */
	double getAverageScore() const;

	/**
	 * Gets the best score
	 *
	 * Returns int: The best score
	 */
	int getBestScore() const;

	/**
	 * Gets the score history
	 *
	 * Returns const std::vector<int>&: Const reference to score history
	 */
	const std::vector<int>& getHistory() const;

	/**
	 * Gets the total number of games played
	 *
	 * Returns size_t: Number of games
	 */
	size_t getTotalGames() const;

	/**
	 * Saves a new score
	 *
	 * Parameters:
	 * int score: The score to save
	 */
	void saveScore(int score);
};

#endif
