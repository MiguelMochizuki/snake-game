/**
 * score_manager.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Class ScoreManager implementation
 */
#include "snake_game/score/score_manager.hpp"
#include <algorithm>
#include <functional>
#include <fstream>
#include <sstream>

#ifdef _WIN32
	#include <direct.h>
	#include <io.h>
	#define MKDIR(dir) _mkdir(dir)
#else
	#include <sys/stat.h>
	#include <unistd.h>
	#define MKDIR(dir) mkdir(dir, 0755)
#endif

namespace {

const uint64_t SEED = 0x9E3779B97F4A7C15ULL;

/**
 * Folds a value into a running hash
 *
 * Parameters:
 * uint64_t& hash: The running hash
 * uint64_t value: The value to fold in
 */
void mix(uint64_t& hash, uint64_t value) {
	hash ^= value + SEED + (hash << 6) + (hash >> 2);
}

}  // namespace

/**
 * Constructor for the ScoreManager class
 *
 * Parameters:
 * const std::string& dataDir: Directory for the score files
 */
ScoreManager::ScoreManager(const std::string& dataDir) : checksum_(0) {
	dataDir_ = dataDir;
	scoresFile_ = dataDir_ + "/scores_v2.txt";
	checksumFile_ = dataDir_ + "/scores_v2.checksum";
	createDataDirectory();

	std::ifstream existing(scoresFile_);
	if (!existing.is_open()) {
		migrateOldFiles();
		writeRecords();
		writeChecksum();
		return;
	}
	existing.close();

	loadRecords();
	uint64_t computed = computeChecksum();
	if (!loadChecksum()) {
		records_.clear();
		writeRecords();
		writeChecksum();
	} else if (computed != checksum_) {
		for (auto& record : records_) {
			record.second = 0;
		}
		writeRecords();
		writeChecksum();
	}
}

/**
 * Creates the data directory if it does not exist
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
 * Reads the v2 score file, skipping lines that are not "E|N|H score"
 */
void ScoreManager::loadRecords() {
	records_.clear();
	std::ifstream file(scoresFile_);
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream in(line);
		std::string code;
		int score;
		std::string extra;
		if (!(in >> code >> score) || code.size() != 1 || (in >> extra)) {
			continue;
		}
		std::optional<Level> level = levelFromCode(code[0]);
		if (level) {
			records_.push_back({*level, score});
		}
	}
}

/**
 * Imports the pre-2.0 files as Normal games when their checksum is valid; the old files are left untouched
 */
void ScoreManager::migrateOldFiles() {
	std::ifstream historyFile(dataDir_ + "/score_history.txt");
	std::ifstream checksumFile(dataDir_ + "/score_checksum.txt");
	if (!historyFile.is_open() || !checksumFile.is_open()) {
		return;
	}
	uint64_t stored = 0;
	if (!(checksumFile >> stored)) {
		return;
	}
	std::vector<int> history;
	int score;
	while (historyFile >> score) {
		history.push_back(score);
	}
	int best = 0;
	std::ifstream bestFile(dataDir_ + "/best_score.txt");
	if (bestFile.is_open()) {
		bestFile >> best;
	}
	uint64_t hash = SEED;
	mix(hash, static_cast<uint64_t>(best));
	for (int s : history) {
		mix(hash, static_cast<uint64_t>(s));
	}
	if (hash != stored) {
		return;
	}
	for (int s : history) {
		records_.push_back({Level::Normal, s});
	}
}

/**
 * Computes the checksum of the records
 *
 * Returns uint64_t: The checksum
 */
uint64_t ScoreManager::computeChecksum() const {
	uint64_t hash = SEED;
	for (const auto& record : records_) {
		mix(hash, static_cast<uint64_t>(levelCode(record.first)));
		mix(hash, static_cast<uint64_t>(record.second));
	}
	return hash;
}

/**
 * Reads the checksum file
 *
 * Returns bool: true if the file existed
 */
bool ScoreManager::loadChecksum() {
	std::ifstream file(checksumFile_);
	if (file.is_open()) {
		file >> checksum_;
		return true;
	}
	checksum_ = 0;
	return false;
}

/**
 * Computes and writes the checksum file
 */
void ScoreManager::writeChecksum() {
	checksum_ = computeChecksum();
	std::ofstream file(checksumFile_);
	if (file.is_open()) {
		file << checksum_ << "\n";
	}
}

/**
 * Rewrites the whole score file from the records
 */
void ScoreManager::writeRecords() const {
	std::ofstream file(scoresFile_, std::ios::trunc);
	if (!file.is_open()) {
		return;
	}
	for (const auto& record : records_) {
		file << levelCode(record.first) << " " << record.second << "\n";
	}
}

/**
 * Gets the best score of a level
 *
 * Parameters:
 * Level level: The level
 *
 * Returns int: The best score, 0 if the level has no games
 */
int ScoreManager::bestScore(Level level) const {
	int best = 0;
	for (const auto& record : records_) {
		if (record.first == level) {
			best = std::max(best, record.second);
		}
	}
	return best;
}

/**
 * Gets the number of games played on a level
 *
 * Parameters:
 * Level level: The level
 *
 * Returns std::size_t: The number of games
 */
std::size_t ScoreManager::totalGames(Level level) const {
	return static_cast<std::size_t>(std::count_if(records_.begin(), records_.end(),
		[level](const std::pair<Level, int>& record) { return record.first == level; }));
}

/**
 * Gets the average of the last AVERAGE_WINDOW games of a level
 *
 * Parameters:
 * Level level: The level
 *
 * Returns double: The average, 0.0 if the level has no games
 */
double ScoreManager::averageScore(Level level) const {
	long long sum = 0;
	std::size_t count = 0;
	for (auto it = records_.rbegin(); it != records_.rend() && count < AVERAGE_WINDOW; ++it) {
		if (it->first == level) {
			sum += it->second;
			++count;
		}
	}
	return count == 0 ? 0.0 : static_cast<double>(sum) / static_cast<double>(count);
}

/**
 * Saves the score of a finished game
 *
 * Parameters:
 * Level level: The level it was played on
 * int score: The final score
 */
void ScoreManager::saveScore(Level level, int score) {
	records_.push_back({level, score});
	std::ofstream file(scoresFile_, std::ios::app);
	if (file.is_open()) {
		file << levelCode(level) << " " << score << "\n";
	}
	writeChecksum();
}

/**
 * Gets the figures of a level
 *
 * Parameters:
 * Level level: The level
 *
 * Returns ScoreStats: Best, games played and average of the last games
 */
ScoreStats ScoreManager::stats(Level level) const {
	return ScoreStats{bestScore(level), totalGames(level), averageScore(level)};
}

/**
 * Gets the best scores of a level
 *
 * Parameters:
 * Level level: The level
 * std::size_t n: Maximum number of scores
 *
 * Returns std::vector<int>: Up to n scores, highest first, ties kept
 */
std::vector<int> ScoreManager::topScores(Level level, std::size_t n) const {
	std::vector<int> scores;
	for (const auto& record : records_) {
		if (record.first == level) {
			scores.push_back(record.second);
		}
	}
	std::sort(scores.begin(), scores.end(), std::greater<int>());
	if (scores.size() > n) {
		scores.resize(n);
	}
	return scores;
}
