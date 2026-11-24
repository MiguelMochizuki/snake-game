/**
 * engine_fakes.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Shared test doubles for driving GameEngine: scripted keys, a recording renderer, a temp score directory
 */
#ifndef ENGINE_FAKES_HPP
#define ENGINE_FAKES_HPP

#include <chrono>
#include <deque>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "snake_game/input/arrow_input_handler.hpp"
#include "snake_game/input/command_input_handler.hpp"
#include "snake_game/input/composite_input_handler.hpp"
#include "snake_game/input/key_source.hpp"
#include "snake_game/input/wasd_input_handler.hpp"
#include "snake_game/render/renderer.hpp"
#include "snake_game/score/score_manager.hpp"

namespace fakes {

/**
 * Key source with no keys during play and scripted waitKey() answers
 *
 * Attributes:
 * std::deque<std::optional<Key>> waits: Results of successive waitKey() calls, nullopt once empty
 */
class ScriptedKeys : public IKeySource {
public:
	std::deque<std::optional<Key>> waits;

	std::vector<Key> poll() override { return {}; }

	std::optional<Key> waitKey() override {
		if (waits.empty()) {
			return std::nullopt;
		}
		std::optional<Key> k = waits.front();
		waits.pop_front();
		return k;
	}

	void flush() override {}
};

/**
 * Renderer that records every call
 *
 * Attributes:
 * int renders, clears, resets, gameOvers, victories: Call counts
 * int lastScore: finalScore of the last printGameOver or printVictory
 * std::vector<int> pauses: Argument of every pause()
 */
class RecordingRenderer : public IRenderer {
public:
	int renders = 0, clears = 0, resets = 0, gameOvers = 0, victories = 0, pausedPanels = 0, lastScore = -1;
	std::vector<int> pauses;
	std::vector<std::pair<int, int>> heads;
	std::vector<int> bests;
	bool practice = false;
	ScoreStats lastStats{0, 0};

	void reset() override { ++resets; }
	void clear() override { ++clears; }
	void render(const GameState& state) override {
		++renders;
		bests.push_back(state.getBestScore());
		const SnakeSegment* head = state.getSnake().getHead();
		heads.push_back({head->getX(), head->getY()});
	}
	void printGameOver(Level, int finalScore, const ScoreStats& stats) override {
		++gameOvers;
		lastStats = stats;
		lastScore = finalScore;
	}
	void printVictory(Level, int finalScore, const ScoreStats& stats) override {
		++victories;
		lastStats = stats;
		lastScore = finalScore;
	}
	void pause(int ms) override { pauses.push_back(ms); }
	void printPaused() override { ++pausedPanels; }
	void setPractice(bool value) override { practice = value; }
};

/**
 * Temporary score directory removed on destruction
 */
struct TempDir {
	std::filesystem::path path;

	TempDir() : path(std::filesystem::temp_directory_path() / ("snake_renderer_seam_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))) {
		std::filesystem::create_directories(path);
	}
	~TempDir() { std::filesystem::remove_all(path); }
};

inline CompositeInputHandler makeInput() {
	CompositeInputHandler input;
	input.add(std::make_unique<WASDInputHandler>());
	input.add(std::make_unique<ArrowInputHandler>());
	input.add(std::make_unique<CommandInputHandler>());
	return input;
}

}  // namespace fakes

#endif
