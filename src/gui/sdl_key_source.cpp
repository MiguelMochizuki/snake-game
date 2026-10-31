/**
 * sdl_key_source.cpp
 * Author: Miguel Mochizuki Silva
 * Description: SdlKeySource implementation
 */
#include "snake_game/gui/sdl_key_source.hpp"

/**
 * Constructor for the SdlKeySource class
 *
 * Parameters:
 * SdlApp& app: The event pump
 */
SdlKeySource::SdlKeySource(SdlApp& app) : app_(app) {
}

/**
 * Returns std::vector<Key>: Keys received since the last call, without waiting
 */
std::vector<Key> SdlKeySource::poll() {
	app_.pump(0);
	std::vector<Key> keys(app_.keys().begin(), app_.keys().end());
	app_.keys().clear();
	return keys;
}

/**
 * Returns std::optional<Key>: The next key, waiting for one; nullopt when no key is queued and the app is not interactive or the window was closed
 */
std::optional<Key> SdlKeySource::waitKey() {
	while (app_.keys().empty()) {
		// A closed window ends the program: end of input is what the engine and menu treat as quit
		if (!app_.interactive() || app_.closeRequested()) {
			return std::nullopt;
		}
		app_.pump(-1);
	}
	Key key = app_.keys().front();
	app_.keys().pop_front();
	return key;
}

/**
 * Discards keys received so far
 */
void SdlKeySource::flush() {
	app_.pump(0);
	app_.keys().clear();
}
