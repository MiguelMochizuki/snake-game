/**
 * score_manager.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Score management and persistence implementation
 */
#include "score_manager.hpp"

#ifdef _WIN32
	#include <direct.h>
	#include <io.h>
	#define MKDIR(dir) _mkdir(dir)
#else
	#include <sys/stat.h>
	#include <unistd.h>
	#define MKDIR(dir) mkdir(dir, 0755)
#endif

/**
 * Constructor for the ScoreManager class
 *
 * Parameters:
 * const std::string& dataDir: Directory for storing score files
 */
ScoreManager::ScoreManager(const std::string& dataDir) : bestScore_(0), checksum_(0) {
	dataDir_ = dataDir;
	historyFile_ = dataDir_ + "/score_history.txt";
	bestFile_ = dataDir_ + "/best_score.txt";
	checksumFile_ = dataDir_ + "/score_checksum.txt";

	createDataDirectory();
	loadBestScore();
	loadHistory();

	uint64_t computed = computeChecksum();
	bool checksumExists = loadChecksum();

	if (!checksumExists) {
		clearAllHistory();
	} else if (computed != checksum_) {
		resetScoresToZero();
		writeChecksum();
	}
}

/**
 * Creates data directory if it doesn't exist
 */
void ScoreManager::createDataDirectory() {
#ifdef _WIN32
	if (_access(dataDir_.c_str(), 0) == -1) {
		MKDIR(dataDir_.c_str());
	}
#else
	struct stat st;
	if (stat(dataDir_.c_str(), &st) == -1) {
		MKDIR(dataDir_.c_str());
	}
#endif
}

/**
 * Gets the average score over the last AVERAGE_WINDOW games (or all games if fewer)
 *
 * Returns double: Average score, or 0 if no scores
 */
double ScoreManager::getAverageScore() const {
	if (scoreHistory_.empty()) {
		return 0.0;
	}

	std::size_t count = std::min(AVERAGE_WINDOW, scoreHistory_.size());
	long long sum = 0;
	for (auto it = scoreHistory_.end() - static_cast<std::ptrdiff_t>(count); it != scoreHistory_.end(); ++it) {
		sum += *it;
	}
	return static_cast<double>(sum) / static_cast<double>(count);
}

/**
 * Gets the best score
 *
 * Returns int: The best score
 */
int ScoreManager::getBestScore() const {
	return bestScore_;
}

/**
 * Gets the score history
 *
 * Returns const std::vector<int>&: Const reference to score history
 */
const std::vector<int>& ScoreManager::getHistory() const {
	return scoreHistory_;
}

/**
 * Gets the total number of games played
 *
 * Returns size_t: Number of games
 */
size_t ScoreManager::getTotalGames() const {
	return scoreHistory_.size();
}

/**
 * Loads best score from file
 */
void ScoreManager::loadBestScore() {
	std::ifstream file(bestFile_);
	if (file.is_open()) {
		file >> bestScore_;
		file.close();
	} else {
		bestScore_ = 0;
	}
}

/**
 * Loads score history from file
 */
void ScoreManager::loadHistory() {
	scoreHistory_.clear();
	std::ifstream file(historyFile_);
	if (file.is_open()) {
		int score;
		while (file >> score) {
			scoreHistory_.push_back(score);
		}
		file.close();
	}
}

/**
 * Saves a new score
 *
 * Parameters:
 * int score: The score to save
 */
void ScoreManager::saveScore(int score) {
	scoreHistory_.push_back(score);

	if (score > bestScore_) {
		bestScore_ = score;
	}

	std::ofstream bestFile(bestFile_);
	if (bestFile.is_open()) {
		bestFile << bestScore_ << "\n";
		bestFile.close();
	}

	std::ofstream historyFile(historyFile_, std::ios::app);
	if (historyFile.is_open()) {
		historyFile << score << "\n";
		historyFile.close();
	}

	writeChecksum();
}

/**
 * Computes integrity checksum from in-memory scores
 *
 * Returns uint64_t: Simple hash of bestScore_ and all scoreHistory_ entries
 */
uint64_t ScoreManager::computeChecksum() const {
	uint64_t hash = 0x9E3779B97F4A7C15ULL;
	hash ^= static_cast<uint64_t>(bestScore_) + 0x9E3779B97F4A7C15ULL + (hash << 6) + (hash >> 2);
	for (int score : scoreHistory_) {
		hash ^= static_cast<uint64_t>(score) + 0x9E3779B97F4A7C15ULL + (hash << 6) + (hash >> 2);
	}
	return hash;
}

/**
 * Loads stored checksum from file
 *
 * Returns bool: true if checksum file existed and was read
 */
bool ScoreManager::loadChecksum() {
	std::ifstream file(checksumFile_);
	if (file.is_open()) {
		file >> checksum_;
		file.close();
		return true;
	}
	checksum_ = 0;
	return false;
}

/**
 * Writes integrity checksum to file
 */
void ScoreManager::writeChecksum() {
	checksum_ = computeChecksum();
	std::ofstream file(checksumFile_);
	if (file.is_open()) {
		file << checksum_ << "\n";
		file.close();
	}
}

/**
 * Resets all scores to zero, keeping play count. Rewrites both score files.
 */
void ScoreManager::resetScoresToZero() {
	bestScore_ = 0;
	for (auto& score : scoreHistory_) {
		score = 0;
	}

	std::ofstream bestFile(bestFile_);
	if (bestFile.is_open()) {
		bestFile << bestScore_ << "\n";
		bestFile.close();
	}

	std::ofstream historyFile(historyFile_);
	if (historyFile.is_open()) {
		for (auto score : scoreHistory_) {
			historyFile << score << "\n";
		}
		historyFile.close();
	}

	writeChecksum();
}

/**
 * Clears all history entirely. Rewrites both score files from scratch.
 */
void ScoreManager::clearAllHistory() {
	bestScore_ = 0;
	scoreHistory_.clear();

	std::ofstream bestFile(bestFile_);
	if (bestFile.is_open()) {
		bestFile << bestScore_ << "\n";
		bestFile.close();
	}

	std::ofstream historyFile(historyFile_);
	if (historyFile.is_open()) {
		historyFile.close();
	}

	writeChecksum();
}
