/**
 * snake_gui.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Graphical entry point of the game, drawn with SDL
 */
#include "env_options.hpp"
#include "snake_game/audio/audio_device.hpp"
#include "snake_game/audio/synth.hpp"
#include "snake_game/audio/synth_sound.hpp"
#include "snake_game/game/game_engine.hpp"
#include "snake_game/gui/canvas.hpp"
#include "snake_game/gui/demo_pilot.hpp"
#include "snake_game/gui/frame_dumper.hpp"
#include "snake_game/gui/sdl_app.hpp"
#include "snake_game/gui/sdl_key_source.hpp"
#include "snake_game/gui/sdl_menu_view.hpp"
#include "snake_game/gui/sdl_renderer.hpp"
#include "snake_game/input/arrow_input_handler.hpp"
#include "snake_game/input/command_input_handler.hpp"
#include "snake_game/input/composite_input_handler.hpp"
#include "snake_game/input/wasd_input_handler.hpp"
#include "snake_game/menu/menu_controller.hpp"
#include "snake_game/score/score_manager.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

namespace {

/**
 * Command-line options
 *
 * Attributes:
 * bool demo: Play with the autopilot, menu skipped
 * std::string framesDir: With --frames, the directory that receives the PPM frames; draws off-screen
 * int stride: Save one frame in this many
 * int maxSteps: Steps after which a demo ends
 */
struct Options {
	bool demo = false;
	std::string framesDir;
	int stride = 1;
	int maxSteps = 3000;
};

/**
 * Parses the command line
 *
 * Parameters:
 * int argc: Argument count
 * char** argv: Arguments
 * Options& options: Receives the options
 *
 * Returns bool: false if an argument is unknown or lacks its value
 */
bool parseOptions(int argc, char** argv, Options& options) {
	for (int i = 1; i < argc; ++i) {
		const std::string arg = argv[i];
		if (arg == "--demo") {
			options.demo = true;
		} else if (arg == "--frames" && i + 1 < argc) {
			options.framesDir = argv[++i];
			options.demo = true;
		} else if (arg == "--stride" && i + 1 < argc) {
			options.stride = std::max(1, std::atoi(argv[++i]));
		} else if (arg == "--max-steps" && i + 1 < argc) {
			options.maxSteps = std::max(1, std::atoi(argv[++i]));
		} else {
			return false;
		}
	}
	return true;
}

/**
 * Plays an autopilot game, in a window or off-screen with frames written to disk
 *
 * Parameters:
 * const Options& options: Demo options
 *
 * Returns int: 0 on success, 1 if no window or canvas can be created
 */
int runDemo(const Options& options) {
	const bool offscreen = !options.framesDir.empty();
	std::unique_ptr<Canvas> canvas = offscreen ? Canvas::offscreen(640, 720) : Canvas::window("Snake", 640, 720);
	if (!canvas) {
		std::fprintf(stderr, "snake-gui: cannot create the %s: %s\n", offscreen ? "canvas" : "window", SDL_GetError());
		return 1;
	}
	SdlApp app(*canvas, !offscreen);
	SdlRenderer sdlRenderer(*canvas, app);
	FrameDumper dumper(sdlRenderer, *canvas, options.framesDir, options.stride);
	IRenderer& renderer = offscreen ? static_cast<IRenderer&>(dumper) : static_cast<IRenderer&>(sdlRenderer);
	auto stop = [&] { return (offscreen ? dumper.renders() : 0) >= options.maxSteps; };
	DemoKeys keys(sdlRenderer, app, stop);
	CompositeInputHandler input;
	input.add(std::make_unique<WASDInputHandler>());
	input.add(std::make_unique<ArrowInputHandler>());
	input.add(std::make_unique<CommandInputHandler>());
	// A demo must not change the real leaderboard, nor depend on earlier games
	const std::filesystem::path scores = std::filesystem::temp_directory_path() / ("snake_demo_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
	{
		GameEngine engine(keys, input, renderer, 20, 20, 100, readSeed(), scores.string());
		engine.setLevel(Level::Normal, readFrameMs().value_or(100));
		engine.run();
	}
	std::filesystem::remove_all(scores);
	return 0;
}

}  // namespace

/**
 * Program entry point
 *
 * Parameters:
 * int argc: Argument count
 * char** argv: Arguments: --demo, or --frames DIR with optional --stride N and --max-steps M
 *
 * Returns int: 0 on a normal exit, 1 if no window can be opened
 */
int main(int argc, char** argv) {
	Options options;
	if (!parseOptions(argc, argv, options)) {
		std::fprintf(stderr, "usage: %s [--demo] [--frames DIR [--stride N] [--max-steps M]]\n", argv[0]);
		return 2;
	}
	if (options.demo) {
		return runDemo(options);
	}
	std::unique_ptr<Canvas> canvas = Canvas::window("Snake", 640, 720);
	if (!canvas) {
		std::fprintf(stderr, "snake-gui: cannot open a window: %s\n", SDL_GetError());
		return 1;
	}
	SdlApp app(*canvas, true);
	SdlKeySource keys(app);
	SdlRenderer renderer(*canvas, app);
	SdlMenuView menuView(*canvas, app);
	CompositeInputHandler input;
	input.add(std::make_unique<WASDInputHandler>());
	input.add(std::make_unique<ArrowInputHandler>());
	input.add(std::make_unique<CommandInputHandler>());

	// The device is declared after the synth so it is stopped first; without a device the game is silent
	Synth synth;
	SynthSound synthSound(synth);
	std::unique_ptr<AudioDevice> audio = AudioDevice::open(synth);
	ISound* cues = audio ? &synthSound : nullptr;

	GameEngine engine(keys, input, renderer, 20, 20, 200, readSeed());
	MenuModel menu;
	bool showMenu = !skipMenu();
	do {
		Level level = Level::Normal;
		int speedMs = levelSpeedMs(level);
		bool sound = true;
		if (showMenu) {
			// Read the scores afresh: the round that just ended may have changed them
			ScoreManager scores;
			MenuResult choice = runMenu(keys, input, menuView, menu, scores, cues);
			if (!choice.start) {
				return 0;
			}
			level = choice.level;
			speedMs = choice.speedMs;
			sound = choice.sound;
		}
		// A valid SNAKE_FRAME_MS (test hook) overrides the speed, never the level
		engine.setLevel(level, readFrameMs().value_or(speedMs));
		engine.setSound(sound ? cues : nullptr);
		showMenu = true;
	} while (engine.run() == GameEnd::Menu);
	return 0;
}
