/**
 * main.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Main entry point
 */
#include "snake_game/input/arrow_input_handler.hpp"
#include "snake_game/input/command_input_handler.hpp"
#include "snake_game/input/composite_input_handler.hpp"
#include "snake_game/game/game_engine.hpp"
#include "snake_game/menu/menu_controller.hpp"
#include "snake_game/score/score_manager.hpp"
#include "snake_game/render/sound.hpp"
#ifdef SNAKE_HAVE_AUDIO
#include "snake_game/audio/audio_device.hpp"
#include "snake_game/audio/synth.hpp"
#include "snake_game/audio/synth_sound.hpp"
#endif
#include "snake_game/menu/terminal_menu_view.hpp"
#include "snake_game/input/terminal_key_source.hpp"
#include "snake_game/input/wasd_input_handler.hpp"
#include "env_options.hpp"
#include "snake_game/render/terminal_renderer.hpp"
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <optional>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

/**
 * Tells whether the terminal can play the bell, i.e. output goes to a terminal
 *
 * Returns bool: true if stdout is a terminal
 */
static bool soundCapable() {
#ifdef _WIN32
	return _isatty(_fileno(stdout)) != 0;
#else
	return isatty(STDOUT_FILENO) != 0;
#endif
}

/**
 * Program entry point
 *
 * Returns int: Exit status
 */
int main() {
#ifdef _WIN32
	// Enable ANSI escape processing; failure is ignored (output stays usable)
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD mode = 0;
	if (console != INVALID_HANDLE_VALUE && GetConsoleMode(console, &mode)) {
		SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}
#endif
	TerminalKeySource keys;
	CompositeInputHandler input;
	input.add(std::make_unique<WASDInputHandler>());
	input.add(std::make_unique<ArrowInputHandler>());
	input.add(std::make_unique<CommandInputHandler>());
	const bool color = TerminalRenderer::colorEnabledFromEnv();
	GameEngine engine(keys, input, 20, 20, 200, readSeed(), color);
	TerminalBell bell(std::cout);
	const bool canBeep = soundCapable();
	ISound* cues = canBeep ? &bell : nullptr;
#ifdef SNAKE_HAVE_AUDIO
	// The device is declared after the synth so it is stopped first; without a device the bell is used as before
	Synth synth;
	SynthSound synthSound(synth);
	std::unique_ptr<AudioDevice> audio = AudioDevice::open(synth);
	if (audio) {
		cues = &synthSound;
	}
#endif
	MenuModel menu;
	bool showMenu = !skipMenu();
	do {
		Level level = Level::Normal;
		int speedMs = levelSpeedMs(level);
		bool sound = true;
		if (showMenu) {
			// Read the scores afresh: the round that just ended may have changed them
			ScoreManager scores;
			TerminalMenuView view(std::cout, color);
			MenuResult choice = runMenu(keys, input, view, menu, scores, cues);
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
