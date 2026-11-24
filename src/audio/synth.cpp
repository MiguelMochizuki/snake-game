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

// With several accompaniment notes at once each one gets 1/sqrt(n) of the gain, so a full chord is not much louder
const float ACCOMPANIMENT_GAIN = 0.15f;
const float MELODY_GAIN = 0.10f;
const double NOTE_ATTACK = 0.008;   // seconds
const double NOTE_RELEASE = 0.03;   // seconds, so repeated notes do not click

struct BlipSpec {
	double hz;
	double seconds;
	float gain;
};

// Music commands posted by startMusic, startRandomPiece and stopMusic
const int CMD_NONE = 0, CMD_START = 1, CMD_STOP = 2, CMD_NEW_PIECE = 3;

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
Synth::Synth(int sampleRate, unsigned seed) : rate_(sampleRate), rng_(seed) {
}

/**
 * Starts the music, where it stopped (from the first piece when it never played)
 */
void Synth::startMusic() {
	command_ = CMD_START;
}

/**
 * Starts the music with a piece chosen at random, other than the last one, from its beginning
 */
void Synth::startRandomPiece() {
	command_ = CMD_NEW_PIECE;
}

/**
 * Stops the music
 */
void Synth::stopMusic() {
	command_ = CMD_STOP;
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
 * Moves to another piece, chosen at random and never the one that was playing, and starts it from the beginning
 */
void Synth::chooseOtherPiece() {
	if (PIECE_COUNT > 1) {
		const int pick = static_cast<int>(rng_() % static_cast<unsigned>(PIECE_COUNT - 1));
		piece_ = pick >= piece_ ? pick + 1 : pick;
	}
	pos_ = 0;
	next_ = 0;
	for (Voice& v : voices_) {
		v.age = v.length = 0;
	}
}

/**
 * Starts a note, taking the place of the oldest voice when all are busy
 *
 * Parameters:
 * const ScoreNote& note: The note to start
 */
void Synth::startNote(const ScoreNote& note) {
	Voice* slot = &voices_[0];
	for (Voice& v : voices_) {
		if (v.age >= v.length) {
			slot = &v;
			break;
		}
		if (v.age > slot->age) {
			slot = &v;
		}
	}
	slot->hz = midiFrequency(note.pitch);
	slot->age = 0;
	slot->length = std::max(1, static_cast<int>(static_cast<long long>(note.length) * rate_ / 1000));
	slot->melody = note.melody != 0;
}

int Synth::piece() const {
	return piece_;
}

/**
 * Fills a buffer with mono samples in [-1, 1]
 *
 * Parameters:
 * float* out: Buffer to fill
 * int frames: Number of samples
 */
void Synth::render(float* out, int frames) {
	switch (command_.exchange(CMD_NONE)) {
	case CMD_START:
		playing_ = true;
		break;
	case CMD_STOP:
		playing_ = false;
		break;
	case CMD_NEW_PIECE:
		chooseOtherPiece();
		playing_ = true;
		break;
	default:
		break;
	}
	const int request = blipRequest_.exchange(0);
	if (request > 0) {
		const BlipSpec& b = BLIPS[request - 1];
		blipHz_ = b.hz;
		blipGain_ = b.gain;
		blipTotal_ = blipLeft_ = static_cast<int>(b.seconds * rate_);
		blipPos_ = 0;
	}
	const int attack = static_cast<int>(NOTE_ATTACK * rate_);
	const int release = static_cast<int>(NOTE_RELEASE * rate_);
	for (int i = 0; i < frames; ++i) {
		float sample = 0.0f;
		if (playing_) {
			const Piece& piece = PIECES[piece_];
			while (next_ < piece.count && static_cast<long long>(piece.notes[next_].start) * rate_ / 1000 <= pos_) {
				startNote(piece.notes[next_++]);
			}
			int accompaniment = 0;
			for (const Voice& v : voices_) {
				if (v.age < v.length && !v.melody) {
					++accompaniment;
				}
			}
			const float share = accompaniment > 0 ? ACCOMPANIMENT_GAIN / std::sqrt(static_cast<float>(accompaniment)) : 0.0f;
			for (Voice& v : voices_) {
				if (v.age >= v.length) {
					continue;
				}
				const double phase = std::fmod(v.hz * v.age / rate_, 1.0);
				float env = 1.0f;
				if (v.age < attack) {
					env = static_cast<float>(v.age) / static_cast<float>(attack);
				}
				const int left = v.length - v.age;
				if (left < release) {
					env = std::min(env, static_cast<float>(left) / static_cast<float>(release));
				}
				if (v.melody) {
					sample += MELODY_GAIN * env * square(phase);
				} else {
					// A plucked note: it fades while it rings
					const float fade = 1.0f - 0.55f * static_cast<float>(v.age) / static_cast<float>(v.length);
					sample += share * env * fade * triangle(phase);
				}
				++v.age;
			}
			if (++pos_ >= static_cast<long long>(piece.endMs + REST_BETWEEN_PIECES_MS) * rate_ / 1000) {
				chooseOtherPiece();
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
