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
 * int renders, clears, resets, gameOvers: Call counts
 * int lastScore: finalScore of the last printGameOver
 * std::vector<int> pauses: Argument of every pause()
 */
class RecordingRenderer : public IRenderer {
public:
	int renders = 0, clears = 0, resets = 0, gameOvers = 0, lastScore = -1;
	std::vector<int> pauses;

	void reset() override { ++resets; }
	void clear() override { ++clears; }
	void render(const GameState&) override { ++renders; }
	void printGameOver(Level, int finalScore, const ScoreStats&) override {
		++gameOvers;
		lastScore = finalScore;
	}
	void pause(int ms) override { pauses.push_back(ms); }
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
