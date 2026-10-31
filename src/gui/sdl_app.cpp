/**
 * sdl_app.cpp
 * Author: Miguel Mochizuki Silva
 * Description: SdlApp implementation
 */
#include "snake_game/gui/sdl_app.hpp"
#include <SDL3/SDL.h>

/**
 * Maps an SDL key to the game's key type
 *
 * Parameters:
 * SDL_Keycode code: The SDL key
 *
 * Returns std::optional<Key>: Arrows, Enter or a lower-case letter; nullopt for any other key
 */
std::optional<Key> keyFromSdl(SDL_Keycode code) {
	switch (code) {
		case SDLK_UP: return specialKey(KeyKind::Up);
		case SDLK_DOWN: return specialKey(KeyKind::Down);
		case SDLK_LEFT: return specialKey(KeyKind::Left);
		case SDLK_RIGHT: return specialKey(KeyKind::Right);
		case SDLK_RETURN:
		case SDLK_KP_ENTER: return specialKey(KeyKind::Enter);
		default: break;
	}
	if (code >= SDLK_A && code <= SDLK_Z) {
		return charKey(static_cast<char>('a' + (code - SDLK_A)));
	}
	return std::nullopt;
}

/**
 * Constructor for the SdlApp class
 *
 * Parameters:
 * Canvas& canvas: The canvas, must outlive the app
 * bool interactive: Whether SDL events are read
 */
SdlApp::SdlApp(Canvas& canvas, bool interactive) : canvas_(canvas), interactive_(interactive) {
	if (interactive_) {
		SDL_InitSubSystem(SDL_INIT_EVENTS);
	}
}

/**
 * Handles one SDL event
 *
 * Parameters:
 * const void* event: A pointer to an SDL_Event
 */
void SdlApp::handle(const void* event) {
	const SDL_Event& e = *static_cast<const SDL_Event*>(event);
	switch (e.type) {
		case SDL_EVENT_KEY_DOWN:
			if (std::optional<Key> key = keyFromSdl(e.key.key)) {
				keys_.push_back(*key);
			}
			break;
		case SDL_EVENT_QUIT:
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			keys_.push_back(charKey('q'));
			break;
		case SDL_EVENT_WINDOW_RESIZED:
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
		case SDL_EVENT_WINDOW_EXPOSED:
			redraw();
			break;
		default:
			break;
	}
}

/**
 * Handles events
 *
 * Parameters:
 * int timeoutMs: 0 drains the pending events, a positive value handles events for that many milliseconds, -1 blocks until at least one event arrives
 */
void SdlApp::pump(int timeoutMs) {
	if (!interactive_) {
		if (timeoutMs > 0) {
			SDL_Delay(static_cast<Uint32>(timeoutMs));
		}
		return;
	}
	SDL_Event e;
	if (timeoutMs < 0) {
		if (SDL_WaitEvent(&e)) {
			handle(&e);
		}
	} else if (timeoutMs > 0) {
		const Uint64 deadline = SDL_GetTicks() + static_cast<Uint64>(timeoutMs);
		for (Uint64 now = SDL_GetTicks(); now < deadline; now = SDL_GetTicks()) {
			if (SDL_WaitEventTimeout(&e, static_cast<Sint32>(deadline - now))) {
				handle(&e);
			}
		}
	}
	while (SDL_PollEvent(&e)) {
		handle(&e);
	}
}

std::deque<Key>& SdlApp::keys() {
	return keys_;
}

bool SdlApp::interactive() const {
	return interactive_;
}

/**
 * Sets what to redraw when the window is resized or exposed
 *
 * Parameters:
 * std::function<void()> redraw: Draws the current screen
 */
void SdlApp::setScene(std::function<void()> redraw) {
	scene_ = std::move(redraw);
}

/**
 * Redraws the current screen, if any
 */
void SdlApp::redraw() {
	if (scene_) {
		scene_();
	}
}
