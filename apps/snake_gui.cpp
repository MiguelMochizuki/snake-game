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
#include <cstdio>
#include <memory>

/**
 * Program entry point
 *
 * Returns int: 0 on a normal exit, 1 if no window can be opened
 */
int main() {
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
