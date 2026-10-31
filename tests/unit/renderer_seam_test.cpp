/**
 * renderer_seam_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests that GameEngine drives an injected IRenderer
 */
#include <gtest/gtest.h>

#include <chrono>
#include <deque>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "snake_game/game/game_engine.hpp"
#include "snake_game/input/arrow_input_handler.hpp"
#include "snake_game/input/command_input_handler.hpp"
#include "snake_game/input/composite_input_handler.hpp"
#include "snake_game/input/wasd_input_handler.hpp"
#include "engine_fakes.hpp"
#include "snake_game/render/renderer.hpp"

using namespace fakes;


TEST(RendererSeam, EngineDrivesInjectedRenderer) {
	TempDir dir;
	ScriptedKeys keys;
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	GameEngine engine(keys, input, renderer, 20, 20, 7, 42, dir.path.string());

	EXPECT_EQ(engine.run(), GameEnd::Quit);

	EXPECT_GE(renderer.pauses.size(), 1u);
	EXPECT_EQ(renderer.renders, static_cast<int>(renderer.pauses.size()) + 1);
	for (int ms : renderer.pauses) {
		EXPECT_EQ(ms, 7);
	}
	EXPECT_EQ(renderer.gameOvers, 1);
	EXPECT_EQ(renderer.resets, 0);
}

TEST(RendererSeam, RestartResetsRendererAndPlaysAgain) {
	TempDir dir;
	ScriptedKeys keys;
	keys.waits.push_back(charKey('r'));
	CompositeInputHandler input = makeInput();
	RecordingRenderer renderer;
	GameEngine engine(keys, input, renderer, 20, 20, 7, 42, dir.path.string());

	EXPECT_EQ(engine.run(), GameEnd::Quit);

	EXPECT_EQ(renderer.gameOvers, 2);
	EXPECT_EQ(renderer.resets, 1);
}
