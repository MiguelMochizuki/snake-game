/**
 * audio_device.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Sound output that plays a Synth through the system audio device
 */
#ifndef AUDIO_DEVICE_HPP
#define AUDIO_DEVICE_HPP

#include "snake_game/audio/synth.hpp"
#include <memory>

/**
 * A running playback device that pulls samples from a Synth on the audio thread
 *
 * Attributes:
 * std::unique_ptr<Impl> impl_: The platform audio device
 */
class AudioDevice {
	struct Impl;
	std::unique_ptr<Impl> impl_;

	AudioDevice();

public:
	/**
	 * Stops the device; returns once the audio thread no longer touches the synth
	 */
	~AudioDevice();

	/**
	 * Opens the default playback device and starts pulling samples from the synth
	 *
	 * Parameters:
	 * Synth& synth: Source of samples, must outlive the returned device
	 *
	 * Returns std::unique_ptr<AudioDevice>: The device, or nullptr if SNAKE_AUDIO is exactly "off" or no device can be opened; never prints
	 */
	static std::unique_ptr<AudioDevice> open(Synth& synth);
};

#endif
