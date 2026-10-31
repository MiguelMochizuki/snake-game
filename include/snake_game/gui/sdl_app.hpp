/**
 * sdl_app.hpp
 * Author: Miguel Mochizuki Silva
 * Description: SDL event pump shared by the graphical renderer, menu view and key source
 */
#ifndef SDL_APP_HPP
#define SDL_APP_HPP

#include "snake_game/gui/canvas.hpp"
#include "snake_game/input/input_handler.hpp"
#include <SDL3/SDL_keycode.h>
#include <deque>
#include <functional>
#include <optional>

/**
 * Maps an SDL key to the game's key type
 *
 * Parameters:
 * SDL_Keycode code: The SDL key
 *
 * Returns std::optional<Key>: Arrows to Up/Down/Left/Right, Return and keypad Enter to Enter, letters to the lower-case char; nullopt for any other key
 */
std::optional<Key> keyFromSdl(SDL_Keycode code);

/**
 * Handles SDL events: queues keys, turns a window close into Q and redraws the current scene when the window changes
 *
 * Attributes:
 * Canvas& canvas_: The canvas the scene is drawn on
 * bool interactive_: false for off-screen use, where there are no events and pump only sleeps
 * std::deque<Key> keys_: Keys received and not yet taken
 * std::function<void()> scene_: Redraws the last thing shown
 */
class SdlApp {
	Canvas& canvas_;
	bool interactive_;
	std::deque<Key> keys_;
	std::function<void()> scene_;

	void handle(const void* event);

public:
	/**
	 * Constructor for the SdlApp class
	 *
	 * Parameters:
	 * Canvas& canvas: The canvas, must outlive the app
	 * bool interactive: Whether SDL events are read
	 */
	SdlApp(Canvas& canvas, bool interactive);

	/**
	 * Handles events
	 *
	 * Parameters:
	 * int timeoutMs: 0 drains the pending events, a positive value handles events for that many milliseconds, -1 blocks until at least one event arrives
	 */
	void pump(int timeoutMs);

	/**
	 * Returns std::deque<Key>&: Keys received and not yet taken
	 */
	std::deque<Key>& keys();

	/**
	 * Returns bool: Whether SDL events are read
	 */
	bool interactive() const;

	/**
	 * Sets what to redraw when the window is resized or exposed
	 *
	 * Parameters:
	 * std::function<void()> redraw: Draws the current screen
	 */
	void setScene(std::function<void()> redraw);

	/**
	 * Redraws the current screen, if any
	 */
	void redraw();
};

#endif
