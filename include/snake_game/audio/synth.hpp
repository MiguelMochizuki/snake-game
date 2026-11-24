/**
 * synth.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Soundtrack player and blip synth; produces samples, does no I/O
 */
#ifndef SYNTH_HPP
#define SYNTH_HPP

#include "snake_game/audio/score.hpp"
#include <atomic>
#include <random>

/**
 * Short sound effects mixed on top of the music
 */
enum class Blip { Eat, GameOver, MenuMove, MenuSelect, Win };

/**
 * Plays the soundtrack (one of the pieces of score.hpp at a time, chosen at random) and generates the blips.
 * The accompaniment is a soft triangle wave and the melody a square wave.
 *
 * startMusic, stopMusic and trigger may be called from any thread; render must be called from one thread at a time
 *
 * Attributes:
 * int rate_: Sample rate in Hz
 * std::atomic<int> command_: The last music command posted (0 none, 1 start, 2 stop, 3 new piece); the render thread takes it
 * once per call, so when several arrive between two calls the last one wins
 * std::atomic<int> blipRequest_: 0 for none, else 1 + the Blip to start
 * bool playing_: Render-thread state: whether the music sounds
 * int piece_, long long pos_, int next_: Render-thread state of the music: the piece playing, the sample position
 * inside it and the next note that has not started yet. Stopping keeps them, so the music goes on where it stopped
 * int forced_: Piece every game plays when SNAKE_PIECE names one, -1 to choose at random
 * std::mt19937 rng_: Chooses the pieces, used by the render thread only
 * Voice voices_[MAX_VOICES]: The notes sounding now
 * int blipLeft_, blipTotal_, blipPos_: Render-thread state of the current blip
 * double blipHz_, float blipGain_: Frequency and loudness of the current blip
 */
class Synth {
	/**
	 * A note that is sounding
	 *
	 * Attributes:
	 * double hz: Frequency
	 * int age: Samples played so far
	 * int length: Samples it lasts; the voice is free once age reaches it
	 * bool melody: Whether it is a melody note
	 * float gain: Loudness of the note, from its velocity
	 */
	struct Voice {
		double hz = 0.0;
		int age = 0;
		int length = 0;
		bool melody = false;
		float gain = 1.0f;
	};

	static constexpr int MAX_VOICES = 16;

	int rate_;
	std::atomic<int> command_{0};
	std::atomic<int> blipRequest_{0};
	bool playing_ = false;
	int piece_ = 0;
	int forced_ = -1;
	long long pos_ = 0;
	int next_ = 0;
	std::mt19937 rng_;
	Voice voices_[MAX_VOICES];
	int blipLeft_ = 0;
	int blipTotal_ = 0;
	int blipPos_ = 0;
	double blipHz_ = 0.0;
	float blipGain_ = 0.0f;

	/**
	 * Moves to another piece, chosen at random and never the one that was playing, and starts it from the beginning
	 */
	void chooseOtherPiece();

	/**
	 * Starts a note, taking the place of the oldest voice when all are busy
	 *
	 * Parameters:
	 * const ScoreNote& note: The note to start
	 */
	void startNote(const ScoreNote& note);

public:
	/**
	 * Constructor for the Synth class
	 *
	 * Parameters:
	 * int sampleRate: Sample rate in Hz (default: 44100)
	 * unsigned seed: Seed of the choice of pieces (default: a random one); the first piece is the first of the score until one is chosen
	 */
	explicit Synth(int sampleRate = 44100, unsigned seed = std::random_device{}());

	/**
	 * Starts the music, where it stopped (from the first piece when it never played)
	 */
	void startMusic();

	/**
	 * Starts the music with a piece chosen at random, other than the last one, from its beginning
	 */
	void startRandomPiece();

	/**
	 * Stops the music
	 */
	void stopMusic();

	/**
	 * Starts a blip, replacing one still playing
	 *
	 * Parameters:
	 * Blip blip: Which sound to play
	 */
	void trigger(Blip blip);

	/**
	 * Fills a buffer with mono samples in [-1, 1]
	 *
	 * Parameters:
	 * float* out: Buffer to fill
	 * int frames: Number of samples
	 */
	void render(float* out, int frames);

	/**
	 * Tells which piece is playing or was the last to play; it is render-thread state, so call it only when no
	 * other thread is inside render (tests)
	 *
	 * Returns int: Index into PIECES
	 */
	int piece() const;
};

#endif
