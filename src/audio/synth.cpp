/**
 * synth.cpp
 * Author: Miguel Mochizuki Silva
 * Description: Synth implementation
 */
#include "snake_game/audio/synth.hpp"
#include "snake_game/audio/score.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <string>

namespace {

// With several accompaniment notes at once each one gets 1/sqrt(n) of the gain, so a full chord is not much louder.
// A note of velocity 100 plays at these gains; the soundtrack keeps its notes around 60, so that the louder notes
// (crescendos, low notes that tools/gen_score.py raises) still have room below velocity 127
const float ACCOMPANIMENT_GAIN = 0.24f;
const float MELODY_GAIN = 0.16f;
const double NOTE_ATTACK = 0.008;   // seconds
const double NOTE_RELEASE = 0.03;   // seconds, so repeated notes do not click
// The accompaniment's share follows the number of notes through a 5 ms smoothing, so a ringing note does not jump in
// volume (a click) when a chord starts or ends around it
const double SHARE_SMOOTHING = 0.005;  // seconds

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
Synth::Synth(int sampleRate, unsigned seed) : Synth(sampleRate, seed, PIECES, PIECE_COUNT) {
}

/**
 * Constructor for the Synth class with a repertoire of its own (tests)
 *
 * Parameters:
 * int sampleRate: Sample rate in Hz
 * unsigned seed: Seed of the choice of pieces
 * const Piece* pieces: The pieces
 * int count: How many there are
 */
Synth::Synth(int sampleRate, unsigned seed, const Piece* pieces, int count)
	: rate_(sampleRate), pieces_(pieces), count_(count), rng_(seed) {
	// SNAKE_STEM=melody or accompaniment plays only those notes (tools/check_balance.py measures them apart)
	const char* stem = std::getenv("SNAKE_STEM");
	if (stem != nullptr) {
		stem_ = std::string(stem) == "melody" ? 1 : (std::string(stem) == "accompaniment" ? 0 : -1);
	}
	// SNAKE_PIECE=<number from 1 or part of the title> plays that piece in every game, to listen to a piece on demand
	const char* wanted = std::getenv("SNAKE_PIECE");
	if (wanted == nullptr || *wanted == '\0') {
		return;
	}
	std::string text(wanted);
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return std::tolower(c); });
	const bool number = std::all_of(text.begin(), text.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
	for (int i = 0; i < count_; ++i) {
		std::string title(pieces_[i].title);
		std::transform(title.begin(), title.end(), title.begin(), [](unsigned char c) { return std::tolower(c); });
		if (number ? std::atoi(text.c_str()) == i + 1 : title.find(text) != std::string::npos) {
			forced_ = piece_ = i;
			return;
		}
	}
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
 * Moves to another piece (the one SNAKE_PIECE names, if any), chosen at random and never the one that was playing, and starts it from the beginning.
 * Each easter egg has 1% of the chance and the other pieces share the rest equally
 */
void Synth::chooseOtherPiece() {
	if (forced_ >= 0) {
		piece_ = forced_;
	} else if (count_ > 1) {
		int rare = 0;
		for (int i = 0; i < count_; ++i) {
			rare += pieces_[i].rare ? 1 : 0;
		}
		const double common = rare < count_ ? (100.0 - rare) / (count_ - rare) : 1.0;
		double total = 0.0;
		for (int i = 0; i < count_; ++i) {
			total += i == piece_ ? 0.0 : (pieces_[i].rare ? 1.0 : common);
		}
		double pick = static_cast<double>(rng_() % 1000000u) / 1000000.0 * total;
		int chosen = piece_;
		for (int i = 0; i < count_; ++i) {
			if (i == piece_) {
				continue;
			}
			chosen = i;
			pick -= pieces_[i].rare ? 1.0 : common;
			if (pick < 0.0) {
				break;
			}
		}
		piece_ = chosen;
	}
	pos_ = 0;
	next_ = 0;
	for (Voice& v : voices_) {
		v.age = v.length = 0;
	}
	share_ = 0.0f;
}

/**
 * Starts a note in a free voice; when all are busy it takes the one closest to its end, an accompaniment note before
 * a melody note
 *
 * Parameters:
 * const ScoreNote& note: The note to start
 */
void Synth::startNote(const ScoreNote& note) {
	if (stem_ >= 0 && note.melody != stem_) {
		return;
	}
	Voice* slot = nullptr;
	for (Voice& v : voices_) {
		if (v.age >= v.length) {
			slot = &v;
			break;
		}
		if (slot == nullptr || (slot->melody && !v.melody) ||
		    (slot->melody == v.melody && v.length - v.age < slot->length - slot->age)) {
			slot = &v;
		}
	}
	slot->hz = midiFrequency(note.pitch);
	slot->age = 0;
	slot->length = std::max(1, static_cast<int>(static_cast<long long>(note.length) * rate_ / 1000));
	slot->melody = note.melody != 0;
	slot->gain = static_cast<float>(note.velocity) / 100.0f;
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
	const float smoothing = static_cast<float>(1.0 - std::exp(-1.0 / (SHARE_SMOOTHING * rate_)));
	for (int i = 0; i < frames; ++i) {
		float sample = 0.0f;
		if (playing_) {
			const Piece& piece = pieces_[piece_];
			while (next_ < piece.count && static_cast<long long>(piece.notes[next_].start) * rate_ / 1000 <= pos_) {
				startNote(piece.notes[next_++]);
			}
			int accompaniment = 0;
			for (const Voice& v : voices_) {
				if (v.age < v.length && !v.melody) {
					++accompaniment;
				}
			}
			if (accompaniment > 0) {
				const float target = ACCOMPANIMENT_GAIN / std::sqrt(static_cast<float>(accompaniment));
				share_ = share_ <= 0.0f ? target : share_ + (target - share_) * smoothing;
			}
			const float share = share_;
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
					sample += MELODY_GAIN * v.gain * env * square(phase);
				} else {
					// A plucked note: it fades while it rings
					const float fade = 1.0f - 0.55f * static_cast<float>(v.age) / static_cast<float>(v.length);
					sample += share * v.gain * env * fade * triangle(phase);
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
