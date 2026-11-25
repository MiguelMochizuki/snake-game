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
 * const Piece* pieces_, int count_: The repertoire (PIECES and PIECE_COUNT, unless a test gives its own)
 * std::atomic<int> command_: The last music command posted (0 none, 1 start, 2 stop, 3 new piece); the render thread takes it
 * once per call, so when several arrive between two calls the last one wins
 * std::atomic<int> blipRequest_: 0 for none, else 1 + the Blip to start
 * bool playing_: Render-thread state: whether the music sounds
 * int piece_, long long pos_, int next_: Render-thread state of the music: the piece playing, the sample position
 * inside it and the next note that has not started yet. Stopping keeps them, so the music goes on where it stopped
 * int forced_: Piece every game plays when SNAKE_PIECE names one, -1 to choose at random
 * std::mt19937 rng_: Chooses the pieces, used by the render thread only
 * Voice voices_[MAX_VOICES]: The notes sounding now
 * float share_: Render-thread state: the gain of each accompaniment note, which follows 1/sqrt(notes sounding) smoothly
 * int stem_: Which notes play: -1 all (the default), 1 the melody only, 0 the accompaniment only (SNAKE_STEM)
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
	const Piece* pieces_;
	int count_;
	std::atomic<int> command_{0};
	std::atomic<int> blipRequest_{0};
	bool playing_ = false;
	int piece_ = 0;
	int forced_ = -1;
	long long pos_ = 0;
	int next_ = 0;
	std::mt19937 rng_;
	Voice voices_[MAX_VOICES];
	float share_ = 0.0f;
	int stem_ = -1;
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
	 * Starts a note in a free voice; when all are busy it takes the one closest to its end, an accompaniment note
	 * before a melody note (tools/gen_score.py keeps every piece within MAX_VOICES, so this is a safety net)
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
	 * Constructor for the Synth class with a repertoire of its own instead of the soundtrack (tests)
	 *
	 * Parameters:
	 * int sampleRate: Sample rate in Hz
	 * unsigned seed: Seed of the choice of pieces
	 * const Piece* pieces: The pieces, which must outlive the synth
	 * int count: How many there are
	 */
	Synth(int sampleRate, unsigned seed, const Piece* pieces, int count);

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
	 * Returns int: Index into the repertoire (PIECES)
	 */
	int piece() const;
};

#endif
