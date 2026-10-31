/**
 * sdl_key_source.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Key source fed by SDL keyboard events
 */
#ifndef SDL_KEY_SOURCE_HPP
#define SDL_KEY_SOURCE_HPP

#include "snake_game/gui/sdl_app.hpp"
#include "snake_game/input/key_source.hpp"

/**
 * Serves the keys the SdlApp collected
 *
 * Attributes:
 * SdlApp& app_: The event pump, must outlive the source
 */
class SdlKeySource : public IKeySource {
	SdlApp& app_;

public:
	/**
	 * Constructor for the SdlKeySource class
	 *
	 * Parameters:
	 * SdlApp& app: The event pump
	 */
	explicit SdlKeySource(SdlApp& app);

	/**
	 * Returns std::vector<Key>: Keys received since the last call, without waiting
	 */
	std::vector<Key> poll() override;

	/**
	 * Returns std::optional<Key>: The next key, waiting for one; nullopt only when the app is not interactive and no key is queued
	 */
	std::optional<Key> waitKey() override;

	/**
	 * Discards keys received so far
	 */
	void flush() override;
};

#endif
