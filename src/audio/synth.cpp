/**
 * synth.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Synth implementation
 */
#include "snake_game/audio/synth.hpp"
#include <algorithm>
#include <cmath>

namespace {

const int STEPS = 64;
const int STEPS_PER_BAR = 16;
const double BPM = 140.0;
const float MUSIC_GAIN = 0.2f;

// MIDI notes of C, Am, F and G, lowest to highest
const int CHORDS[4][3] = {{60, 64, 67}, {57, 60, 64}, {53, 57, 60}, {55, 59, 62}};
const int PATTERN[4] = {0, 1, 2, 1};

struct BlipSpec {
	double hz;
	double seconds;
	float gain;
};

// Indexed by Blip: Eat, GameOver, MenuMove, MenuSelect
const BlipSpec BLIPS[4] = {{1046.5, 0.08, 0.3f}, {196.0, 0.25, 0.3f}, {523.25, 0.04, 0.25f}, {1174.66, 0.06, 0.25f}};

/**
 * One sample of a square wave
 *
 * Parameters:
 * double phase: Position in the period, in [0, 1)
 *
 * Returns float: +1 for the first half of the period, -1 for the second
 */
float square(double phase) {
	return phase < 0.5 ? 1.0f : -1.0f;
}

}  // namespace

/**
 * Frequency of one sixteenth-note step of the C - Am - F - G loop
 *
 * Parameters:
 * int step: Sixteenth-note index, wraps modulo 64
 *
 * Returns double: Frequency in Hz
 */
double stepFrequency(int step) {
	const int s = ((step % STEPS) + STEPS) % STEPS;
	const int note = CHORDS[s / STEPS_PER_BAR][PATTERN[s % 4]];
	return 440.0 * std::pow(2.0, (note - 69) / 12.0);
}

/**
 * Constructor for the Synth class
 *
 * Parameters:
 * int sampleRate: Sample rate in Hz
 */
Synth::Synth(int sampleRate) : rate_(sampleRate) {
}

/**
 * Starts the music from the first chord
 */
void Synth::startMusic() {
	musicOn_ = true;
}

/**
 * Stops the music
 */
void Synth::stopMusic() {
	musicOn_ = false;
}

/**
 * Starts a blip, replacing one still playing
 *
 * Parameters:
 * Blip blip: Which sound to play
 */
void Synth::trigger(Blip blip) {
	blipRequest_ = static_cast<int>(blip) + 1;
}

/**
 * Fills a buffer with mono samples in [-1, 1]
 *
 * Parameters:
 * float* out: Buffer to fill
 * int frames: Number of samples
 */
void Synth::render(float* out, int frames) {
	const bool on = musicOn_;
	if (on && !wasOn_) {
		step_ = 0;
		stepPos_ = 0;
	}
	wasOn_ = on;
	const int request = blipRequest_.exchange(0);
	if (request > 0) {
		const BlipSpec& b = BLIPS[request - 1];
		blipHz_ = b.hz;
		blipGain_ = b.gain;
		blipTotal_ = blipLeft_ = static_cast<int>(b.seconds * rate_);
		blipPos_ = 0;
	}
	// 4725 samples per sixteenth at 44100 Hz and 140 BPM
	const int stepLen = static_cast<int>(rate_ * 60.0 / BPM / 4.0);
	for (int i = 0; i < frames; ++i) {
		float sample = 0.0f;
		if (on) {
			// Phase restarts at every step, so the loop repeats exactly
			const double phase = std::fmod(stepFrequency(step_) * stepPos_ / rate_, 1.0);
			const float fade = 1.0f - 0.6f * static_cast<float>(stepPos_) / static_cast<float>(stepLen);
			sample += MUSIC_GAIN * fade * square(phase);
			if (++stepPos_ >= stepLen) {
				stepPos_ = 0;
				step_ = (step_ + 1) % STEPS;
			}
		}
		if (blipLeft_ > 0) {
			const double phase = std::fmod(blipHz_ * blipPos_ / rate_, 1.0);
			const float fade = 1.0f - static_cast<float>(blipPos_) / static_cast<float>(blipTotal_);
			sample += blipGain_ * fade * square(phase);
			++blipPos_;
			--blipLeft_;
		}
		out[i] = std::max(-1.0f, std::min(1.0f, sample));
	}
}
