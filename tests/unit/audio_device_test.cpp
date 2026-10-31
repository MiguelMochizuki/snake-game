/**
 * audio_device_test.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Tests for AudioDevice::open; built only with SNAKE_GAME_BUILD_AUDIO=ON
 */
#include <gtest/gtest.h>

#include <cstdlib>

#include "snake_game/audio/audio_device.hpp"
#include "snake_game/audio/synth.hpp"

namespace {

/**
 * Sets SNAKE_AUDIO for the scope of a test and restores the previous state
 */
class AudioEnv {
	bool had_;
	std::string old_;

public:
	explicit AudioEnv(const char* value) {
		const char* current = std::getenv("SNAKE_AUDIO");
		had_ = current != nullptr;
		if (had_) {
			old_ = current;
		}
		set(value);
	}

	~AudioEnv() {
		set(had_ ? old_.c_str() : nullptr);
	}

private:
	static void set(const char* value) {
#ifdef _WIN32
		_putenv_s("SNAKE_AUDIO", value ? value : "");
#else
		if (value) {
			setenv("SNAKE_AUDIO", value, 1);
		} else {
			unsetenv("SNAKE_AUDIO");
		}
#endif
	}
};

}  // namespace

TEST(AudioDevice, OpenReturnsNullWhenDisabled) {
	AudioEnv env("off");
	Synth synth;
	EXPECT_EQ(AudioDevice::open(synth), nullptr);
}

TEST(AudioDevice, OpenWithGarbageValueDoesNotThrow) {
	AudioEnv env("loud");
	Synth synth;
	EXPECT_NO_THROW({ std::unique_ptr<AudioDevice> device = AudioDevice::open(synth); });
}
