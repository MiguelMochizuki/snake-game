/**
 * audio_device.cpp
 * Author: Miguel Mochizuki Silva
 * Description: AudioDevice implementation on top of miniaudio
 */
#include "snake_game/audio/audio_device.hpp"
#include <cstdlib>
#include <cstring>

// Only the playback device is needed; the null backend is off so a machine without a sound card reports "no device"
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MA_NO_ENGINE
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NULL
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

/**
 * Holds the miniaudio device and whether it was initialised
 *
 * Attributes:
 * ma_device device: The playback device
 * bool started: Whether ma_device_init succeeded and the device must be uninitialised
 */
struct AudioDevice::Impl {
	ma_device device;
	bool started = false;
};

namespace {

/**
 * miniaudio data callback: fills the buffer from the synth
 *
 * Parameters:
 * ma_device* device: The device; pUserData holds the Synth
 * void* output: Buffer of mono floats
 * const void*: Capture data, unused
 * ma_uint32 frames: Number of samples to produce
 */
void onAudio(ma_device* device, void* output, const void*, ma_uint32 frames) {
	static_cast<Synth*>(device->pUserData)->render(static_cast<float*>(output), static_cast<int>(frames));
}

}  // namespace

AudioDevice::AudioDevice() : impl_(new Impl) {
}

/**
 * Stops the device; returns once the audio thread no longer touches the synth
 */
AudioDevice::~AudioDevice() {
	if (impl_->started) {
		ma_device_uninit(&impl_->device);
	}
}

/**
 * Opens the default playback device and starts pulling samples from the synth
 *
 * Parameters:
 * Synth& synth: Source of samples, must outlive the returned device
 *
 * Returns std::unique_ptr<AudioDevice>: The device, or nullptr if SNAKE_AUDIO is exactly "off" or no device can be opened
 */
std::unique_ptr<AudioDevice> AudioDevice::open(Synth& synth) {
	const char* env = std::getenv("SNAKE_AUDIO");
	if (env && std::strcmp(env, "off") == 0) {
		return nullptr;
	}
	std::unique_ptr<AudioDevice> audio(new AudioDevice);
	ma_device_config config = ma_device_config_init(ma_device_type_playback);
	config.playback.format = ma_format_f32;
	config.playback.channels = 1;
	config.sampleRate = 44100;
	config.dataCallback = onAudio;
	config.pUserData = &synth;
	if (ma_device_init(nullptr, &config, &audio->impl_->device) != MA_SUCCESS) {
		return nullptr;
	}
	audio->impl_->started = true;
	if (ma_device_start(&audio->impl_->device) != MA_SUCCESS) {
		return nullptr;  // the destructor uninitialises the device
	}
	return audio;
}
