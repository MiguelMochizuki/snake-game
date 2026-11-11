/**
 * synth.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Synth implementation
 */
#include "snake_game/audio/synth.hpp"
#include "snake_game/audio/score.hpp"
#include <algorithm>
#include <cmath>

namespace {

const float ARPEGGIO_GAIN = 0.15f;
const float MELODY_GAIN = 0.10f;
const float CHORD_GAIN = 0.04f;  // per note of the final chord
const int CHORD_STEPS = 16;      // the final chord rings for one bar
const double MELODY_ATTACK = 0.008;   // seconds
const double MELODY_RELEASE = 0.03;   // seconds, so repeated notes do not click

struct BlipSpec {
	double hz;
	double seconds;
	float gain;
};

// Indexed by Blip: Eat, GameOver, MenuMove, MenuSelect, Win
const BlipSpec BLIPS[5] = {{1046.5, 0.08, 0.3f}, {196.0, 0.25, 0.3f}, {523.25, 0.04, 0.25f}, {1174.66, 0.06, 0.25f}, {1318.5, 0.35, 0.3f}};

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

/**
 * One sample of a triangle wave that starts at zero, so a note can begin without a click
 *
 * Parameters:
 * double phase: Position in the period, in [0, 1)
 *
 * Returns float: Value in [-1, 1]
 */
float triangle(double phase) {
	if (phase < 0.25) {
		return static_cast<float>(4.0 * phase);
	}
	if (phase < 0.75) {
		return static_cast<float>(2.0 - 4.0 * phase);
	}
	return static_cast<float>(4.0 * phase - 4.0);
}

}  // namespace

/**
 * Constructor for the Synth class
 *
 * Parameters:
 * int sampleRate: Sample rate in Hz
 */
Synth::Synth(int sampleRate) : rate_(sampleRate) {
}

/**
 * Starts the music from the beginning
 */
void Synth::startMusic() {
	restartRequest_ = true;
	musicOn_ = true;
}

/**
 * Stops the music
 */
void Synth::stopMusic() {
	musicOn_ = false;
}

/**
 * Cuts the loop and plays only the last bars once, then stops by itself
 */
void Synth::finale() {
	finaleRequest_ = true;
	musicOn_ = true;
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
	// The finale request is read first: finale() sets it before turning the music on, so seeing the music on implies seeing the request
	const bool finaleNow = finaleRequest_.exchange(false);
	const bool restart = restartRequest_.exchange(false);
	const bool on = musicOn_ || finaleNow;
	if (restart || (on && !wasOn_)) {
		step_ = 0;
		stepPos_ = 0;
		melodyIdx_ = 0;
		finaleActive_ = false;
	}
	if (finaleNow) {
		step_ = FINALE_START_STEP;
		stepPos_ = 0;
		melodyIdx_ = 0;  // the loop below skips the notes that ended before the first step
		finaleActive_ = true;
	}
	wasOn_ = on;
	bool playing = on;
	const int request = blipRequest_.exchange(0);
	if (request > 0) {
		const BlipSpec& b = BLIPS[request - 1];
		blipHz_ = b.hz;
		blipGain_ = b.gain;
		blipTotal_ = blipLeft_ = static_cast<int>(b.seconds * rate_);
		blipPos_ = 0;
	}
	// 10022 samples per sixteenth at 44100 Hz and 66 BPM
	const int stepLen = static_cast<int>(rate_ * 60.0 / TEMPO_BPM / 4.0);
	const int attack = static_cast<int>(MELODY_ATTACK * rate_);
	const int release = static_cast<int>(MELODY_RELEASE * rate_);
	for (int i = 0; i < frames; ++i) {
		float sample = 0.0f;
		if (playing) {
			// Every note restarts its phase, so the whole piece repeats exactly
			if (step_ < ARPEGGIO_STEPS) {
				const double phase = std::fmod(midiFrequency(ARPEGGIO_DATA[step_]) * stepPos_ / rate_, 1.0);
				const float fade = 1.0f - 0.55f * static_cast<float>(stepPos_) / static_cast<float>(stepLen);
				sample += ARPEGGIO_GAIN * fade * triangle(phase);
			} else if (step_ < ARPEGGIO_STEPS + CHORD_STEPS) {
				const int t = (step_ - ARPEGGIO_STEPS) * stepLen + stepPos_;
				const float fade = 1.0f - static_cast<float>(t) / static_cast<float>(CHORD_STEPS * stepLen);
				for (int note : FINAL_CHORD) {
					sample += CHORD_GAIN * fade * triangle(std::fmod(midiFrequency(note) * t / rate_, 1.0));
				}
			}
			while (melodyIdx_ < MELODY_COUNT && MELODY[melodyIdx_].start + MELODY[melodyIdx_].length <= step_) {
				++melodyIdx_;
			}
			if (melodyIdx_ < MELODY_COUNT && MELODY[melodyIdx_].start <= step_) {
				const MelodyNote& m = MELODY[melodyIdx_];
				const int t = (step_ - m.start) * stepLen + stepPos_;
				const int left = m.length * stepLen - t;
				float env = 1.0f;
				if (t < attack) {
					env = static_cast<float>(t) / static_cast<float>(attack);
				}
				if (left < release) {
					env = std::min(env, static_cast<float>(left) / static_cast<float>(release));
				}
				sample += MELODY_GAIN * env * square(std::fmod(midiFrequency(m.pitch) * t / rate_, 1.0));
			}
			if (++stepPos_ >= stepLen) {
				stepPos_ = 0;
				if (finaleActive_ && step_ + 1 >= FINALE_END_STEP) {
					// The ending is over: stop here instead of looping or resting
					finaleActive_ = false;
					musicOn_ = false;
					playing = false;
				}
				if (++step_ >= SCORE_STEPS) {
					step_ = 0;
					melodyIdx_ = 0;
				}
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
